#pragma once
#include <string>
#include <algorithm>
#include <vector>
#include <optional>
#include <cstdint>
#include <exception>
#include "cec/domain/Diagnostics.h"
#include "cec/domain/Comprobante.h"
#include "cec/parsing/ComprobanteRequestDTO.h"
#include "cec/rules/RuleEngine.h"
#include "cec/calculation/CalculationEngine.h"
#include "cec/visitor/EscritorXml.h"
#include "cec/crypto/XadesSigner.h"

namespace cec::service {

struct ResultMetadata {
    std::string coreVersion = "2.0.0";
    std::string standardVersion = "4.4";
    std::string schemaVersion = "1.0.0";
    std::string rulesetVersion = "2026.1";
};

struct ComprobanteResult {
    bool isSuccess = false;
    std::optional<domain::ComprobanteCanonico> comprobante;
    domain::DiagnosticList diagnostics;
    std::string xml;
    ResultMetadata metadata;
};

struct FirmaResult {
    bool isSuccess = false;
    std::string xml;
    /// Solo en prepararFirma: SignedInfo canonicalizado que se firma con RSA-SHA256.
    std::string signedInfo;
    domain::DiagnosticList diagnostics;
    crypto::CertificadoInfo certInfo;
    ResultMetadata metadata;
};

class ComprobanteService {
public:
    static FirmaResult signXml(const std::string& xmlSinFirmar, const std::vector<uint8_t>& p12Bytes, const std::string& pin) {
        FirmaResult result;
        auto [signer, loadDiags] = crypto::Pkcs12Signer::loadFromBytes(p12Bytes, pin);
        if (loadDiags.hasErrors()) {
            result.isSuccess = false;
            result.diagnostics = loadDiags;
            return result;
        }
        return signXml(xmlSinFirmar, signer);
    }

    static FirmaResult signXml(const std::string& xmlSinFirmar, const crypto::Pkcs12Signer& signer) {
        FirmaResult result;
        result.certInfo = signer.getCertificadoInfo();
        auto [signedXml, signDiags] = crypto::XadesSigner::signXml(xmlSinFirmar, signer);
        result.diagnostics = signDiags;
        if (signDiags.hasErrors()) {
            result.isSuccess = false;
            return result;
        }
        result.xml = std::move(signedXml);
        result.isSuccess = true;
        return result;
    }

    /**
     * @brief Firma en dos pasos (paso 1): prepara la firma XAdES-EPES con el certificado público (X.509 DER).
     * El XML devuelto lleva SignatureValue vacío y `signedInfo` trae lo que se firma con la llave privada.
     */
    static FirmaResult prepararFirma(const std::string& xmlSinFirmar, const std::vector<uint8_t>& certificadoDer) {
        FirmaResult result;
        auto [certInfo, certDiags] = crypto::Pkcs12Signer::leerCertificadoDer(certificadoDer);
        result.diagnostics = certDiags;
        if (certDiags.hasErrors()) return result;
        result.certInfo = certInfo;
        auto [preparada, diags] = crypto::XadesSigner::prepararFirma(xmlSinFirmar, certInfo);
        result.diagnostics.merge(diags);
        if (diags.hasErrors()) return result;
        result.xml = std::move(preparada.xml);
        result.signedInfo = std::move(preparada.signedInfo);
        result.isSuccess = true;
        return result;
    }

    /**
     * @brief Firma en dos pasos (paso 2): inserta el SignatureValue (RSA-SHA256 de signedInfo, en Base64) y
     * verifica la firma con el certificado de KeyInfo.
     */
    static FirmaResult completarFirma(const std::string& xmlPreparado, const std::string& signatureValueBase64) {
        FirmaResult result;
        auto [xml, diags] = crypto::XadesSigner::completarFirma(xmlPreparado, signatureValueBase64, true);
        result.diagnostics = diags;
        if (diags.hasErrors()) return result;
        result.xml = std::move(xml);
        result.isSuccess = true;
        return result;
    }

    /**
     * @brief Lee el certificado de un PKCS#12 sin firmar nada (para mostrar titular y vigencia antes de usarlo).
     */
    static FirmaResult leerCertificado(const std::vector<uint8_t>& p12Bytes, const std::string& pin) {
        FirmaResult result;
        auto [signer, loadDiags] = crypto::Pkcs12Signer::loadFromBytes(p12Bytes, pin);
        result.diagnostics = loadDiags;
        if (loadDiags.hasErrors()) return result;
        result.certInfo = signer.getCertificadoInfo();
        result.isSuccess = true;
        return result;
    }

    static ComprobanteResult process(const parsing::ComprobanteRequestDTO& request) {
        static const rules::RuleEngine defaultEngine;
        return process(request, defaultEngine);
    }

    /**
     * @brief Igual que process(request) pero con un motor de reglas propio
     * (por ejemplo, con un reloj/tolerancia distintos para la fecha de emisión).
     */
    static ComprobanteResult process(const parsing::ComprobanteRequestDTO& request, const rules::RuleEngine& engine) {
        ComprobanteResult result;

        // 1. Parsing y Normalización a Dominio Tipado
        domain::ComprobanteCanonico inv;
        domain::DiagnosticList parseDiags = parseDto(request, inv);
        if (parseDiags.hasErrors()) {
            result.isSuccess = false;
            result.diagnostics = parseDiags;
            return result; // Política de parada: error estructural detiene el pipeline
        }

        // 2. Validación de Reglas de Negocio
        domain::DiagnosticList ruleDiags = engine.validate(inv);
        if (ruleDiags.hasErrors()) {
            result.isSuccess = false;
            result.diagnostics = ruleDiags;
            return result; // Política de parada: cálculo solo corre si no hay errores previos
        }

        // 3. Motor de Cálculo Monetario Determinista
        domain::DiagnosticList calcDiags = calculation::CalculationEngine::calculate(inv);
        if (calcDiags.hasErrors()) {
            result.isSuccess = false;
            result.diagnostics = calcDiags;
            return result;
        }

        // 4. Validación Post-cálculo (Invariantes)
        domain::DiagnosticList postDiags = validateInvariants(inv);
        if (postDiags.hasErrors()) {
            result.isSuccess = false;
            result.diagnostics = postDiags;
            return result;
        }

        // 5. Serialización XML Canónica (Anexo 1 v4.4). Otros formatos: cualquier visitor::IFormatoSalida sobre result.comprobante.
        result.xml = visitor::FormatoXml{}.escribir(inv);
        result.comprobante = std::move(inv);
        result.isSuccess = true;
        result.diagnostics.merge(parseDiags);
        result.diagnostics.merge(ruleDiags);
        result.diagnostics.merge(calcDiags);
        result.diagnostics.merge(postDiags);

        return result;
    }

    /**
     * @brief Vista previa del cálculo: parsea, corre las reglas solo para informar y calcula aunque haya errores
     * de reglas (los de parseo sí lo detienen). Nunca genera XML ni marca el resultado como exitoso si hay errores.
     * Útil para mostrar montos mientras se edita un comprobante incompleto.
     */
    static ComprobanteResult calcular(const parsing::ComprobanteRequestDTO& request) {
        static const rules::RuleEngine defaultEngine;
        return calcular(request, defaultEngine);
    }

    static ComprobanteResult calcular(const parsing::ComprobanteRequestDTO& request, const rules::RuleEngine& engine) {
        ComprobanteResult result;
        domain::ComprobanteCanonico inv;
        result.diagnostics = parseDto(request, inv);
        if (result.diagnostics.hasErrors()) return result;

        result.diagnostics.merge(engine.validate(inv));
        try {
            result.diagnostics.merge(calculation::CalculationEngine::calculate(inv));
        } catch (const std::exception& e) {
            result.diagnostics.addError("CALC_ERROR", "", std::string("No se pudo calcular el comprobante: ") + e.what(), domain::DiagnosticCategory::PostCondition);
            return result;
        }
        result.isSuccess = !result.diagnostics.hasErrors();
        result.comprobante = std::move(inv);
        return result;
    }

private:
    /// @brief Código de catálogo ("01") a su valor numérico; 0 si no es numérico (la regla correspondiente lo rechaza).
    static int codigoNumerico(const std::string& c) {
        if (c.empty() || c.size() > 3 || !std::all_of(c.begin(), c.end(), ::isdigit)) return 0;
        return std::stoi(c);
    }

    /// @brief Decimal opcional del request. Vacío = no informado; con formato inválido agrega PARSE_DECIMAL_INVALID.
    static std::optional<domain::Money> decimal(const std::string& v, const std::string& path, domain::DiagnosticList& diags) {
        if (v.empty()) return std::nullopt;
        try {
            return domain::Money::fromString(v);
        } catch (const std::exception& e) {
            diags.addError("PARSE_DECIMAL_INVALID", path, "Valor decimal inválido '" + v + "': " + e.what(), domain::DiagnosticCategory::Structural);
            return std::nullopt;
        }
    }

    /// @brief Decimal obligatorio: si falta queda en cero y la regla de negocio lo reporta.
    static domain::Money monto(const std::string& v, const std::string& path, domain::DiagnosticList& diags) {
        return decimal(v, path, diags).value_or(domain::Money{});
    }

    static std::optional<int64_t> escalado(const std::string& v, const std::string& path, domain::DiagnosticList& diags) {
        auto m = decimal(v, path, diags);
        return m ? std::optional<int64_t>(m->scaledValue()) : std::nullopt;
    }

    static domain::Identificacion identificacion(const parsing::IdentificacionInput& id) {
        return { static_cast<domain::TipoIdentificacion>(codigoNumerico(id.tipo)), id.numero };
    }

    static domain::Ubicacion ubicacion(const parsing::UbicacionInput& u) {
        return { u.provincia, u.canton, u.distrito,
                 u.barrio.empty() ? std::nullopt : std::optional<std::string>(u.barrio), u.otrasSenas };
    }

    static std::vector<domain::CodigoComercial> codigosComerciales(const std::vector<parsing::CodigoComercialInput>& in) {
        std::vector<domain::CodigoComercial> out;
        for (const auto& c : in) out.push_back({ c.tipo, c.codigo });
        return out;
    }

    static std::vector<domain::DescuentoLinea> descuentos(const std::vector<parsing::DescuentoInput>& in, const std::string& p,
                                                         domain::DiagnosticList& diags) {
        std::vector<domain::DescuentoLinea> out;
        for (size_t i = 0; i < in.size(); ++i) {
            const auto& d = in[i];
            const std::string dp = p + ".descuentos[" + std::to_string(i) + "]";
            out.push_back({ monto(d.montoDescuento, dp + ".montoDescuento", diags), d.codigoDescuento,
                            d.naturalezaDescuento, d.codigoDescuentoOTRO });
        }
        return out;
    }

    static std::vector<domain::ImpuestoLinea> impuestos(const std::vector<parsing::ImpuestoInput>& in, const std::string& p,
                                                       domain::DiagnosticList& diags) {
        std::vector<domain::ImpuestoLinea> out;
        for (size_t i = 0; i < in.size(); ++i) {
            const auto& dto = in[i];
            const std::string ip = p + ".impuestos[" + std::to_string(i) + "]";
            domain::ImpuestoLinea imp{};
            imp.codigo = static_cast<domain::CodigoImpuesto>(codigoNumerico(dto.codigo));
            imp.codigoImpuestoOTRO = dto.codigoImpuestoOTRO;
            imp.codigoTarifa = static_cast<domain::CodigoTarifaIVA>(codigoNumerico(dto.codigoTarifaIVA));
            // La tarifa del IVA (01, 07) sale de su código (Nota 8.1); los impuestos 02, 12 y 99 traen la suya.
            if (auto tarifa = escalado(dto.tarifa, ip + ".tarifa", diags)) {
                imp.tarifaPorcentajeScaled = *tarifa;
            } else if (imp.codigo == domain::CodigoImpuesto::Cemento) {
                imp.tarifaPorcentajeScaled = 5 * domain::Money::SCALE; // El impuesto al cemento es del 5 %.
            } else {
                imp.tarifaPorcentajeScaled = dto.codigoTarifaIVA.empty() ? 0 : domain::getTarifaPorcentajeScaled(imp.codigoTarifa);
            }
            imp.factorCalculoIVAScaled = escalado(dto.factorCalculoIVA, ip + ".factorCalculoIVA", diags);
            if (dto.datosImpuestoEspecifico.has_value()) {
                const auto& d = *dto.datosImpuestoEspecifico;
                const std::string dp = ip + ".datosImpuestoEspecifico";
                domain::DatosImpuestoEspecifico datos;
                datos.cantidadUnidadMedidaScaled = escalado(d.cantidadUnidadMedida, dp + ".cantidadUnidadMedida", diags).value_or(0);
                datos.porcentajeScaled = escalado(d.porcentaje, dp + ".porcentaje", diags);
                datos.proporcionScaled = escalado(d.proporcion, dp + ".proporcion", diags);
                datos.volumenUnidadConsumoScaled = escalado(d.volumenUnidadConsumo, dp + ".volumenUnidadConsumo", diags);
                datos.impuestoUnidad = monto(d.impuestoUnidad, dp + ".impuestoUnidad", diags);
                imp.datosEspecificos = datos;
            }
            imp.montoExportacion = monto(dto.montoExportacion, ip + ".montoExportacion", diags);
            out.push_back(imp);
        }
        return out;
    }

    static domain::DiagnosticList parseDto(const parsing::ComprobanteRequestDTO& dto, domain::ComprobanteCanonico& outInv) {
        domain::DiagnosticList diags;

        outInv.tipoDocumento = static_cast<domain::TipoDocumento>(codigoNumerico(dto.tipoDocumento));
        switch (outInv.tipoDocumento) {
            case domain::TipoDocumento::FacturaElectronica:            outInv.isFE = true;  break;
            case domain::TipoDocumento::FacturaElectronicaExportacion: outInv.isFEE = true; break;
            case domain::TipoDocumento::FacturaElectronicaCompra:      outInv.isFEC = true; break;
            case domain::TipoDocumento::TiqueteElectronico:            outInv.isTE = true;  break;
            case domain::TipoDocumento::NotaCreditoElectronica:        outInv.isNC = true;  break;
            case domain::TipoDocumento::NotaDebitoElectronica:         outInv.isND = true;  break;
            case domain::TipoDocumento::ReciboElectronicoPago:         outInv.isREP = true; break;
            default:
                diags.addError("PARSE_TIPO_DOCUMENTO_INVALID", "tipoDocumento", "Tipo de comprobante no válido (Nota 3): '" + dto.tipoDocumento + "'.", domain::DiagnosticCategory::Structural);
                break;
        }

        outInv.clave = dto.clave;
        outInv.proveedorSistemas = dto.proveedorSistemas;
        outInv.codigoActividadEmisor = dto.codigoActividadEmisor;
        outInv.codigoActividadReceptor = dto.codigoActividadReceptor;
        outInv.numeroConsecutivo = dto.numeroConsecutivo;
        outInv.fechaEmision = dto.fechaEmision;
        outInv.condicionVenta = static_cast<domain::CondicionVenta>(codigoNumerico(dto.condicionVenta));
        outInv.condicionVentaOtros = dto.condicionVentaOtros;
        outInv.plazoCredito = dto.plazoCredito ? std::to_string(*dto.plazoCredito) : "";

        // Emisor
        auto& em = outInv.emisor;
        em.nombre = dto.emisor.nombre;
        em.identificacion = identificacion(dto.emisor.identificacion);
        em.registroFiscal8707 = dto.emisor.registroFiscal8707;
        em.nombreComercial = dto.emisor.nombreComercial;
        if (dto.emisor.ubicacion) em.ubicacion = ubicacion(*dto.emisor.ubicacion);
        if (!dto.emisor.otrasSenasExtranjero.empty()) em.otrasSenasExtranjero = dto.emisor.otrasSenasExtranjero;
        if (dto.emisor.telefono) em.telefono = domain::Telefono{ dto.emisor.telefono->codigoPais, dto.emisor.telefono->numTelefono };
        em.correoElectronico = dto.emisor.correoElectronico;

        // Receptor
        if (dto.receptor) {
            const auto& r = *dto.receptor;
            domain::Receptor rec;
            rec.nombre = r.nombre;
            if (r.identificacion) rec.identificacion = identificacion(*r.identificacion);
            rec.nombreComercial = r.nombreComercial;
            if (r.ubicacion) rec.ubicacion = ubicacion(*r.ubicacion);
            if (!r.otrasSenasExtranjero.empty()) rec.otrasSenasExtranjero = r.otrasSenasExtranjero;
            if (r.telefono) rec.telefono = domain::Telefono{ r.telefono->codigoPais, r.telefono->numTelefono };
            rec.correoElectronico = r.correoElectronico;
            outInv.receptor = rec;
        }

        // Detalle
        for (size_t i = 0; i < dto.detalleServicio.size(); ++i) {
            const auto& ld = dto.detalleServicio[i];
            const std::string p = "detalleServicio[" + std::to_string(i) + "]";

            domain::LineaDetalle l{};
            l.numeroLinea = ld.numeroLinea.value_or(static_cast<int>(i + 1));
            l.partidaArancelaria = ld.partidaArancelaria;
            l.cabys = ld.codigoCABYS;
            l.codigosComerciales = codigosComerciales(ld.codigosComerciales);
            if (outInv.isREP) {
                // En el REP no hay cantidad ni precio unitario: MontoTotal es el monto a pagar y se registra como una unidad.
                l.cantidadScaled = domain::Money::SCALE;
                l.precioUnitario = monto(ld.montoTotal, p + ".montoTotal", diags);
            } else {
                l.cantidadScaled = monto(ld.cantidad, p + ".cantidad", diags).scaledValue();
                l.precioUnitario = monto(ld.precioUnitario, p + ".precioUnitario", diags);
                if (!ld.montoTotal.empty()) {
                    diags.addError("PARSE_MONTO_TOTAL_NO_EDITABLE", p + ".montoTotal", "MontoTotal solo se informa en el Recibo Electrónico de Pago; en los demás comprobantes se calcula (cantidad x precio unitario).", domain::DiagnosticCategory::Structural);
                }
            }
            l.unidadMedida = ld.unidadMedida;
            l.tipoTransaccion = ld.tipoTransaccion;
            l.unidadMedidaComercial = ld.unidadMedidaComercial;
            l.descripcion = ld.detalle;
            l.numerosVIN = ld.numerosVINoSerie;
            l.registroMedicamento = ld.registroMedicamento;
            l.formaFarmaceutica = ld.formaFarmaceutica;

            for (size_t s = 0; s < ld.detalleSurtido.size(); ++s) {
                const auto& sd = ld.detalleSurtido[s];
                const std::string sp = p + ".detalleSurtido[" + std::to_string(s) + "]";
                domain::ComponenteSurtido comp{};
                comp.codigoCABYS = sd.codigoCABYS;
                comp.codigosComerciales = codigosComerciales(sd.codigosComerciales);
                comp.cantidadScaled = monto(sd.cantidad, sp + ".cantidad", diags).scaledValue();
                comp.unidadMedida = sd.unidadMedida;
                comp.unidadMedidaComercial = sd.unidadMedidaComercial;
                comp.descripcion = sd.detalle;
                comp.precioUnitario = monto(sd.precioUnitario, sp + ".precioUnitario", diags);
                comp.descuentos = descuentos(sd.descuentos, sp, diags);
                comp.impuestos = impuestos(sd.impuestos, sp, diags);
                l.surtido.push_back(comp);
            }

            l.descuentos = descuentos(ld.descuentos, p, diags);
            l.subTotalEditado = decimal(ld.subTotal, p + ".subTotal", diags);
            l.ivaCobradoFabrica = ld.ivaCobradoFabrica;
            l.baseImponibleEditada = decimal(ld.baseImponible, p + ".baseImponible", diags);
            l.impuestos = impuestos(ld.impuestos, p, diags);
            l.impuestoAsumidoEmisorFabrica = monto(ld.impuestoAsumidoEmisorFabrica, p + ".impuestoAsumidoEmisorFabrica", diags);

            if (ld.exoneracion) {
                const auto& e = *ld.exoneracion;
                const std::string ep = p + ".exoneracion";
                domain::ExoneracionLinea ex{};
                ex.tipoDocumento = e.tipoDocumentoEX1;
                ex.tipoDocumentoOTRO = e.tipoDocumentoOTRO;
                ex.numeroDocumento = e.numeroDocumento;
                ex.articulo = e.articulo;
                ex.inciso = e.inciso;
                ex.nombreInstitucion = e.nombreInstitucion;
                ex.nombreInstitucionOtros = e.nombreInstitucionOtros;
                ex.fechaEmision = e.fechaEmisionEX;
                ex.porcentajeExoneracionScaled = escalado(e.tarifaExonerada, ep + ".tarifaExonerada", diags).value_or(0);
                l.exoneracion = ex;
            }

            outInv.lineas.push_back(l);
        }

        // Otros cargos
        for (size_t i = 0; i < dto.otrosCargos.size(); ++i) {
            const auto& c = dto.otrosCargos[i];
            const std::string cp = "otrosCargos[" + std::to_string(i) + "]";
            domain::OtrosCargos cargo{};
            cargo.tipoDocumento = c.tipoDocumentoOC;
            cargo.tipoDocumentoOTROS = c.tipoDocumentoOTROS;
            if (c.identificacionTercero) {
                if (!c.identificacionTercero->tipo.empty()) {
                    cargo.tipoIdentidadTercero = static_cast<domain::TipoIdentificacion>(codigoNumerico(c.identificacionTercero->tipo));
                }
                cargo.numeroIdentidadTercero = c.identificacionTercero->numero;
            }
            cargo.nombreTercero = c.nombreTercero;
            cargo.detalle = c.detalle;
            cargo.porcentajeScaled = escalado(c.porcentajeOC, cp + ".porcentajeOC", diags);
            cargo.montoCargo = monto(c.montoCargo, cp + ".montoCargo", diags);
            outInv.otrosCargos.push_back(cargo);
        }

        // Resumen: moneda y medios de pago (los totales los calcula el motor)
        auto& moneda = outInv.resumen.codigoTipoMoneda;
        moneda.codigoMoneda = dto.codigoMoneda;
        if (auto tc = escalado(dto.tipoCambio, "resumenFactura.codigoTipoMoneda.tipoCambio", diags)) {
            moneda.tipoCambioScaled = *tc;
        } else {
            // En colones el tipo de cambio es 1; en moneda extranjera la regla de moneda exige informarlo.
            moneda.tipoCambioScaled = dto.codigoMoneda == "CRC" ? domain::Money::SCALE : 0;
        }
        for (size_t i = 0; i < dto.mediosPago.size(); ++i) {
            const auto& m = dto.mediosPago[i];
            domain::MedioPagoInfo mp;
            mp.tipo = static_cast<domain::MedioPago>(codigoNumerico(m.tipoMedioPago));
            mp.otros = m.medioPagoOtros;
            mp.totalMedioPago = monto(m.totalMedioPago, "resumenFactura.mediosPago[" + std::to_string(i) + "].totalMedioPago", diags);
            outInv.mediosPago.push_back(mp);
        }

        // Información de referencia
        for (const auto& r : dto.informacionReferencia) {
            domain::InformacionReferencia ref{};
            ref.tipoDoc = r.tipoDocIR;
            ref.tipoDocRefOTRO = r.tipoDocRefOTRO;
            ref.numero = r.numero;
            ref.fechaEmision = r.fechaEmisionIR;
            ref.codigo = r.codigo;
            ref.codigoReferenciaOTRO = r.codigoReferenciaOTRO;
            ref.razon = r.razon;
            outInv.referencias.push_back(ref);
        }

        return diags;
    }

    /**
     * @brief TotalIVADevuelto (Anexo 1 pág. 55): solo con servicios de salud al 4% pagados con tarjeta, nunca mayor
     * a su IVA, y obligatorio cuando se facturan esos servicios con tarjeta.
     */
    static void validarIvaDevuelto(const domain::ComprobanteCanonico& inv, domain::DiagnosticList& diags) {
        if (inv.isREP || inv.isFEE || inv.isFEC) return;
        const domain::Money ivaSalud = calculation::CalculationEngine::ivaServiciosSalud(inv);
        const bool hayTarjeta = std::any_of(inv.mediosPago.begin(), inv.mediosPago.end(),
            [](const domain::MedioPagoInfo& m) { return m.tipo == domain::MedioPago::Tarjeta; });
        const domain::Money& devuelto = inv.resumen.totalIvaDevuelto;

        if (devuelto.isPositive()) {
            if (!ivaSalud.isPositive() || !hayTarjeta) {
                diags.addError("RULE_IVA_DEVUELTO_NO_APLICA", "resumenFactura.totalIVADevuelto", "El IVA devuelto solo procede con servicios de salud (CABYS 931) al 4% pagados con tarjeta.", domain::DiagnosticCategory::BusinessRule);
            } else if (devuelto > ivaSalud) {
                diags.addError("RULE_IVA_DEVUELTO_EXCEDE", "resumenFactura.totalIVADevuelto", "El IVA devuelto no puede ser mayor al IVA calculado en los servicios de salud al 4%.", domain::DiagnosticCategory::BusinessRule);
            }
        } else if (ivaSalud.isPositive() && hayTarjeta) {
            diags.addError("RULE_IVA_DEVUELTO_REQUIRED", "resumenFactura.totalIVADevuelto", "El IVA devuelto es obligatorio al facturar servicios de salud pagados con tarjeta; en ventas mixtas se debe indicar previamente el monto pagado con tarjeta.", domain::DiagnosticCategory::BusinessRule);
        }
    }

    static domain::DiagnosticList validateInvariants(const domain::ComprobanteCanonico& inv) {
        domain::DiagnosticList diags;
        // Invariante: TotalComprobante = TotalVentaNeta + TotalImpuesto + TotalOtrosCargos - TotalIVADevuelto
        // (en REP: TotalComprobante = TotalVenta).
        domain::Money esperado = inv.isREP
            ? inv.resumen.totalVenta
            : inv.resumen.totalVentaNeta + inv.resumen.totalImpuesto + inv.resumen.totalOtrosCargos - inv.resumen.totalIvaDevuelto;
        if (inv.resumen.totalComprobante != esperado) {
            diags.addError("INVARIANT_TOTAL_MISMATCH", "resumenFactura.totalComprobante", "Discrepancia en invariante contable de total de comprobante.", domain::DiagnosticCategory::PostCondition);
        }
        // Con dos o más medios de pago, su suma debe coincidir con TotalComprobante.
        if (inv.mediosPago.size() > 1) {
            domain::Money sumaMedios;
            for (const auto& mp : inv.mediosPago) {
                sumaMedios = sumaMedios + mp.totalMedioPago;
            }
            if (sumaMedios != inv.resumen.totalComprobante) {
                diags.addError("RULE_MEDIO_PAGO_SUMA_MISMATCH", "resumenFactura.mediosPago",
                    "La suma de los montos por medio de pago (" + sumaMedios.toString(5) +
                    ") debe coincidir con el total del comprobante (" + inv.resumen.totalComprobante.toString(5) + ").",
                    domain::DiagnosticCategory::PostCondition);
            }
        }
        validarIvaDevuelto(inv, diags);
        return diags;
    }
};

} // namespace cec::service
