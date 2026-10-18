#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <utility>

namespace cec::common {

// ==============================================================================
// Nota 2 & 3: Tipos de Comprobantes Electrónicos
// ==============================================================================
enum class TipoDocumento : uint8_t {
    FacturaElectronica = 1,
    NotaDebitoElectronica = 2,
    NotaCreditoElectronica = 3,
    TiqueteElectronico = 4,
    FacturaElectronicaCompra = 8,
    FacturaElectronicaExportacion = 9,
    ReciboElectronicoPago = 10,
    MensajeReceptor = 11,
    MensajeHacienda = 12
};

inline std::string to_string(TipoDocumento tipo) {
    switch (tipo) {
        case TipoDocumento::FacturaElectronica: return "FacturaElectronica";
        case TipoDocumento::NotaDebitoElectronica: return "NotaDebitoElectronica";
        case TipoDocumento::NotaCreditoElectronica: return "NotaCreditoElectronica";
        case TipoDocumento::TiqueteElectronico: return "TiqueteElectronico";
        case TipoDocumento::FacturaElectronicaCompra: return "FacturaElectronicaCompra";
        case TipoDocumento::FacturaElectronicaExportacion: return "FacturaElectronicaExportacion";
        case TipoDocumento::ReciboElectronicoPago: return "ReciboElectronicoPago";
        case TipoDocumento::MensajeReceptor: return "MensajeReceptor";
        case TipoDocumento::MensajeHacienda: return "MensajeHacienda";
        default: return "Desconocido";
    }
}

inline std::string get_tipo_documento_code(TipoDocumento tipo) {
    switch (tipo) {
        case TipoDocumento::FacturaElectronica: return "01";
        case TipoDocumento::NotaDebitoElectronica: return "02";
        case TipoDocumento::NotaCreditoElectronica: return "03";
        case TipoDocumento::TiqueteElectronico: return "04";
        case TipoDocumento::FacturaElectronicaCompra: return "08";
        case TipoDocumento::FacturaElectronicaExportacion: return "09";
        case TipoDocumento::ReciboElectronicoPago: return "10";
        case TipoDocumento::MensajeReceptor: return "11";
        case TipoDocumento::MensajeHacienda: return "12";
        default: return "01";
    }
}

inline std::string get_tipo_documento_description(TipoDocumento tipo) {
    switch (tipo) {
        case TipoDocumento::FacturaElectronica: return "Factura Electrónica";
        case TipoDocumento::NotaDebitoElectronica: return "Nota de Débito Electrónica";
        case TipoDocumento::NotaCreditoElectronica: return "Nota de Crédito Electrónica";
        case TipoDocumento::TiqueteElectronico: return "Tiquete Electrónico";
        case TipoDocumento::FacturaElectronicaCompra: return "Factura Electrónica de Compra";
        case TipoDocumento::FacturaElectronicaExportacion: return "Factura Electrónica de Exportación";
        case TipoDocumento::ReciboElectronicoPago: return "Recibo Electrónico de Pago";
        case TipoDocumento::MensajeReceptor: return "Mensaje Receptor";
        case TipoDocumento::MensajeHacienda: return "Mensaje Hacienda";
        default: return "Desconocido";
    }
}

inline std::string get_document_namespace(TipoDocumento tipo) {
    switch (tipo) {
        case TipoDocumento::FacturaElectronica:
            return "https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/facturaElectronica";
        case TipoDocumento::FacturaElectronicaCompra:
            return "https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/facturaElectronicaCompra";
        case TipoDocumento::FacturaElectronicaExportacion:
            return "https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/facturaElectronicaExportacion";
        case TipoDocumento::NotaCreditoElectronica:
            return "https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/notaCreditoElectronica";
        case TipoDocumento::NotaDebitoElectronica:
            return "https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/notaDebitoElectronica";
        case TipoDocumento::TiqueteElectronico:
            return "https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/tiqueteElectronico";
        case TipoDocumento::ReciboElectronicoPago:
            return "https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/reciboElectronicoPago";
        case TipoDocumento::MensajeHacienda:
            return "https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/mensajeHacienda";
        case TipoDocumento::MensajeReceptor:
            return "https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/mensajeReceptor";
        default:
            return "https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/facturaElectronica";
    }
}

// ==============================================================================
// Nota 4: Tipos de Identificación
// ==============================================================================
enum class TipoIdentificacion : uint8_t {
    CedulaFisica = 1,
    CedulaJuridica = 2,
    DIMEX = 3,
    NITE = 4,
    /**
     * Este tipo de identificación estará disponible en los Datos del Emisor de la Factura Electrónica 
     * de Compra únicamente para el registro de las adquisiciones de servicios o bienes intangibles a 
     * proveedores no domiciliados en Costa Rica, según el artículo 29 inciso d) y el Transitorio XV 
     * del Reglamento del IVA. 
     * 
     * En Factura Electrónica, estará disponible en los Datos del Receptor, únicamente cuando en el campo 
     * “Condición de la Venta” se haya seleccionado el código 12 “Ventas de Bienes No Nacionalizados” 
     * Este código también estará habilitado para su uso en la Factura Electrónica de Exportación y 
     * Tiquete Electrónico.
     */
    ExtranjeroNoDomiciliado = 5,
    /**
     * Este tipo de identificación estará disponible en los Datos del Emisor de la Factura Electrónica 
     * de Compra únicamente para el registro de las adquisiciones de bienes usados vendidos por un no 
     * contribuyente para los efectos contables que corresponda. Estará disponible únicamente cuando en el 
     * campo “Condición de la Venta” se haya seleccionado el código 13 “Ventas Bienes Usados No Contribuyentes”.
     */
    NoContribuyente = 6
};

inline std::string get_tipo_identificacion_code(TipoIdentificacion tipo) {
    switch (tipo) {
        case TipoIdentificacion::CedulaFisica: return "01";
        case TipoIdentificacion::CedulaJuridica: return "02";
        case TipoIdentificacion::DIMEX: return "03";
        case TipoIdentificacion::NITE: return "04";
        case TipoIdentificacion::ExtranjeroNoDomiciliado: return "05";
        case TipoIdentificacion::NoContribuyente: return "06";
        default: return "01";
    }
}

inline std::string get_tipo_identificacion_description(TipoIdentificacion tipo) {
    switch (tipo) {
        case TipoIdentificacion::CedulaFisica: return "Cédula Física";
        case TipoIdentificacion::CedulaJuridica: return "Cédula Jurídica";
        case TipoIdentificacion::DIMEX: return "DIMEX";
        case TipoIdentificacion::NITE: return "NITE";
        case TipoIdentificacion::ExtranjeroNoDomiciliado: return "Extranjero No Domiciliado";
        case TipoIdentificacion::NoContribuyente: return "No Contribuyente";
        default: return "Desconocido";
    }
}

// ==============================================================================
// Nota 5: Condiciones de la Venta
// ==============================================================================
enum class CondicionDeVenta : uint8_t {
    Contado = 1,
    Credito = 2,
    Consignacion = 3,
    Apartado = 4,
    ArrendamientoOpcionCompra = 5,
    ArrendamientoFuncionFinanciera = 6,
    CobroAFavorDeTercero = 7,
    /**
     * Se utiliza dicha condición de venta cuanto el servicio que se le preste al 
     * estado, una vez generada la factura no se recibe el pago de forma inmediata.
     */
    ServiciosPrestadosAlEstadoACredito = 8,
    /**
     * El código 09, Pago de servicios prestado al Estado se utilizará únicamente para 
     * efectos de los Recibos Electrónicos de Pago que cancelen una Factura Electrónica 
     * original con código 08. Cualquier modificación por medio de NC o ND, debe realizarse 
     * a la factura electrónica original, con código 08.
     */
    PagoServiciosPrestadosAlEstado = 9,    
    /**
     * Este código se debe de utilizar, cuando el comprobante electrónico que se está 
     * generando por la venta de un bien o servicio gravado con IVA es a crédito y además 
     * la venta del bien o servicio cumple con las características establecidas en el 
     * artículo 3 inciso 8) y 27 de la LIVA.
     */
    VentaCreditoIVAHasta90Dias = 10,
    /**
     * El código 11, Pago de venta a crédito en IVA hasta 90 días (Artículo 27, LIVA) se 
     * utilizará únicamente para efectos del Recibo Electrónico que cancelen una 
     * Factura Electrónica original con código 10. Cualquier modificación por medio de NC o ND, 
     * debe realizarse a la factura electrónica original, con código 10.
     */
    PagoVentaCreditoIVAHasta90Dias = 11,
    /**
     * Ventas de una mercancía cuando la misma no se encuentra nacionalizada en Costa Rica, puede 
     * ser antes o después de haber presentado la Declaración de Mercancías ante la Autoridad Aduanera 
     * del país destino. Ejemplo: Ventas Sucesivas.
     */
    VentaMercanciaNoNacionalizada = 12,
    /**
     * Ventas de bienes usados por parte un no contribuyente a un Obligado Tributario, que ocupe respaldar 
     * en su contabilidad dicha compra. Aplicará únicamente en factura electrónica de compra
     */
    VentaBienesUsadosNoContribuyente = 13,
    /**
     * Se entiende como arrendamiento operativo, lo definido en las normas tributarias, por ejemplo, 
     * el Decreto 32876-H y sus modificaciones.
     */
    ArrendamientoOperativo = 14,
    /**
     * Se entiende como arrendamiento financiero, lo definido en las normas tributarias, por ejemplo, 
     * el Decreto 32876-H y sus modificaciones.
     */
    ArrendamientoFinanciero = 15,
    Otros = 99
};

inline std::string get_condicion_venta_code(CondicionDeVenta cond) {
    if (cond == CondicionDeVenta::Otros) return "99";
    uint8_t val = static_cast<uint8_t>(cond);
    if (val < 10) return "0" + std::to_string(val);
    return std::to_string(val);
}

inline std::string get_condicion_venta_description(CondicionDeVenta cond) {
    switch (cond) {
        case CondicionDeVenta::Contado: return "Contado";
        case CondicionDeVenta::Credito: return "Crédito";
        case CondicionDeVenta::Consignacion: return "Consignación";
        case CondicionDeVenta::Apartado: return "Apartado";
        case CondicionDeVenta::ArrendamientoOpcionCompra: return "Arrendamiento con opción de compra";
        case CondicionDeVenta::ArrendamientoFuncionFinanciera: return "Arrendamiento en función financiera";
        case CondicionDeVenta::CobroAFavorDeTercero: return "Cobro a favor de un tercero";
        case CondicionDeVenta::ServiciosPrestadosAlEstadoACredito: return "Servicios prestados al Estado a crédito";
        case CondicionDeVenta::PagoServiciosPrestadosAlEstado: return "Pago de servicios prestados al Estado";
        case CondicionDeVenta::VentaCreditoIVAHasta90Dias: return "Venta a crédito en IVA hasta 90 días (Artículo 27, LIVA)";
        case CondicionDeVenta::PagoVentaCreditoIVAHasta90Dias: return "Pago de venta a crédito en IVA hasta 90 días";
        case CondicionDeVenta::VentaMercanciaNoNacionalizada: return "Venta Mercancía No Nacionalizada";
        case CondicionDeVenta::VentaBienesUsadosNoContribuyente: return "Venta de bienes usados a no contribuyentes";
        case CondicionDeVenta::ArrendamientoOperativo: return "Arrendamiento operativo";
        case CondicionDeVenta::ArrendamientoFinanciero: return "Arrendamiento financiero";
        case CondicionDeVenta::Otros: return "Otros";
        default: return "Otros";
    }
}

// ==============================================================================
// Nota 6: Medios de Pago
// ==============================================================================
enum class MedioPago : uint8_t {
    Efectivo = 1,
    Tarjeta = 2,
    Cheque = 3,
    TransferenciaDepositoBancario = 4,
    /**
     * El código 05, Recaudado por terceros se utilizará cuando el pago sea recibido 
     * por un tercero y no por quien emite el servicio o mercancía ejemplo: pago de recibos 
     * telefónicos en supermercados.
     */
    RecaudadoPorTerceros = 5,
    SINPEMovil = 6,
    PlataformaDigital = 7,
    /**
     * Se debe indicar el medio de pago en la representación gráfica
     */
    Otros = 99
};

inline std::string get_medio_pago_code(MedioPago medio) {
    if (medio == MedioPago::Otros) return "99";
    uint8_t val = static_cast<uint8_t>(medio);
    if (val < 10) return "0" + std::to_string(val);
    return std::to_string(val);
}

inline std::string get_medio_pago_description(MedioPago medio) {
    switch (medio) {
        case MedioPago::Efectivo: return "Efectivo";
        case MedioPago::Tarjeta: return "Tarjeta";
        case MedioPago::Cheque: return "Cheque";
        case MedioPago::TransferenciaDepositoBancario: return "Transferencia - depósito bancario";
        case MedioPago::RecaudadoPorTerceros: return "Recaudado por terceros";
        case MedioPago::SINPEMovil: return "SINPE Móvil";
        case MedioPago::PlataformaDigital: return "Plataforma Digital";
        case MedioPago::Otros: return "Otros";
        default: return "Otros";
    }
}

// ==============================================================================
// Nota 20: Tipos de Descuentos
// ==============================================================================
enum class CodigoDescuento : uint8_t {
    Regalia = 1,
    RegaliaIVACobradoCliente = 2,
    Bonificacion = 3,
    Volumen = 4,
    TemporadaEstacional = 5,
    Promocional = 6,
    Comercial = 7,
    Frecuencia = 8,
    Sostenido = 9,
    Otros = 99
};

inline std::string get_codigo_descuento_code(CodigoDescuento d) {
    if (d == CodigoDescuento::Otros) return "99";
    uint8_t val = static_cast<uint8_t>(d);
    if (val < 10) return "0" + std::to_string(val);
    return std::to_string(val);
}

inline std::string get_codigo_descuento_description(CodigoDescuento d) {
    switch (d) {
        case CodigoDescuento::Regalia: return "Descuento por Regalía";
        case CodigoDescuento::RegaliaIVACobradoCliente: return "Descuento por Regalía IVA Cobrado al Cliente";
        case CodigoDescuento::Bonificacion: return "Descuento por Bonificación";
        case CodigoDescuento::Volumen: return "Descuento por volumen";
        case CodigoDescuento::TemporadaEstacional: return "Descuento por Temporada (estacional)";
        case CodigoDescuento::Promocional: return "Descuento promocional";
        case CodigoDescuento::Comercial: return "Descuento Comercial";
        case CodigoDescuento::Frecuencia: return "Descuento por frecuencia";
        case CodigoDescuento::Sostenido: return "Descuento sostenido";
        case CodigoDescuento::Otros: return "Otros descuentos";
        default: return "Otros descuentos";
    }
}

// ==============================================================================
// Nota 8: Códigos de Impuesto
// ==============================================================================
enum class CodigoImpuesto : uint8_t {
    ImpuestoValorAgregado = 1,
    ImpuestoSelectivoConsumo = 2,
    ImpuestoUnicoCombustibles = 3,
    ImpuestoBebidasAlcoholicas = 4,
    ImpuestoBebidasEnvasadasJabones = 5,
    ImpuestoTabaco = 6,
    /**
     * Este código se utilizará cuando se realicen ventas de bienes o servicios donde 
     * el cálculo de la base imponible del IVA sea especial, por ejemplo, bienes usados, 
     * ventas por catálogo, bebidas alcohólicas o no alcohólicas.
     */
    IVACalculoEspecial = 7,
    IVARegimenBienesUsadosFactor = 8,
    ImpuestoEspecificoCemento = 12,
    Otros = 99
};

inline std::string get_codigo_impuesto_code(CodigoImpuesto imp) {
    if (imp == CodigoImpuesto::Otros) return "99";
    uint8_t val = static_cast<uint8_t>(imp);
    if (val < 10) return "0" + std::to_string(val);
    return std::to_string(val);
}

inline std::string get_codigo_impuesto_description(CodigoImpuesto imp) {
    switch (imp) {
        case CodigoImpuesto::ImpuestoValorAgregado: return "Impuesto al Valor Agregado";
        case CodigoImpuesto::ImpuestoSelectivoConsumo: return "Impuesto Selectivo de Consumo";
        case CodigoImpuesto::ImpuestoUnicoCombustibles: return "Impuesto único a los combustibles";
        case CodigoImpuesto::ImpuestoBebidasAlcoholicas: return "Impuesto específico de bebidas alcohólicas";
        case CodigoImpuesto::ImpuestoBebidasEnvasadasJabones: return "Impuesto específico sobre bebidas envasadas y jabones";
        case CodigoImpuesto::ImpuestoTabaco: return "Impuesto a los productos de tabaco";
        case CodigoImpuesto::IVACalculoEspecial: return "IVA (cálculo especial)";
        case CodigoImpuesto::IVARegimenBienesUsadosFactor: return "IVA Régimen de Bienes Usados (Factor)";
        case CodigoImpuesto::ImpuestoEspecificoCemento: return "Impuesto Específico al Cemento";
        case CodigoImpuesto::Otros: return "Otros";
        default: return "Otros";
    }
}

// ==============================================================================
// Nota 8.1: Tarifas de IVA
// ==============================================================================
enum class CodigoTarifaIVA : uint8_t {
    /**
     * La tarifa 0% se utilizará únicamente cuando la transacciones que se realicen 
     * correspondan a las transacciones definidas en el artículo 32, numeral 1 del 
     * Reglamento de la Ley sobre el Impuesto al Valor Agregado, que otorga derecho 
     * a crédito pleno, como por ejemplo ventas a la CCSS o a municipalidades, 
     * entre otros.
     */
    Tarifa0 = 1,
    TarifaReducida1 = 2,
    TarifaReducida2 = 3,
    TarifaReducida4 = 4,
    /**
     * El código 05, Transitorio 0% se utilizará únicamente para efectos de las Notas
     * de Crédito y Notas de Débito que lo requieran.
     */
    Transitorio0 = 5,
    /**
     * El código 06, Transitorio 4% se utilizará únicamente para efectos de las Notas 
     * de Crédito y Notas de Débito que lo requieran.
     */
    Transitorio4 = 6,
    /**
     * El código 07, Transitorio 8% se utilizará únicamente para efectos de las Notas 
     * de Crédito y Notas de Débito que lo requieran.
     */
    Transitorio8 = 7,
    TarifaGeneral13 = 8,
    TarifaReducida05 = 9,
    /**
     * El código 10, Tarifa Exenta Ley 9635, Articulo 8.
     */
    TarifaExenta = 10,
    /**
     * El código 11, se utilizará únicamente en aquellos casos en los que se registre 
     * la transacción de un bien o servicio no sujeto pero que no otorga derecho a crédito.
     */
    Tarifa0SinCredito = 11
};

inline std::string get_tarifa_iva_code(CodigoTarifaIVA t) {
    uint8_t val = static_cast<uint8_t>(t);
    if (val < 10) return "0" + std::to_string(val);
    return std::to_string(val);
}

inline std::string get_tarifa_iva_description(CodigoTarifaIVA t) {
    switch (t) {
        case CodigoTarifaIVA::Tarifa0: return "Tarifa 0% (Artículo 32, num 1, RLIVA)";
        case CodigoTarifaIVA::TarifaReducida1: return "Tarifa reducida 1%";
        case CodigoTarifaIVA::TarifaReducida2: return "Tarifa reducida 2%";
        case CodigoTarifaIVA::TarifaReducida4: return "Tarifa reducida 4%";
        case CodigoTarifaIVA::Transitorio0: return "Transitorio 0%";
        case CodigoTarifaIVA::Transitorio4: return "Transitorio 4%";
        case CodigoTarifaIVA::Transitorio8: return "Tarifa transitoria 8%";
        case CodigoTarifaIVA::TarifaGeneral13: return "Tarifa general 13%";
        case CodigoTarifaIVA::TarifaReducida05: return "Tarifa reducida 0.5%";
        case CodigoTarifaIVA::TarifaExenta: return "Tarifa Exenta";
        case CodigoTarifaIVA::Tarifa0SinCredito: return "Tarifa 0% sin derecho a crédito";
        default: return "Desconocido";
    }
}

// ==============================================================================
// Nota 3 (g): Situación del Comprobante
// ==============================================================================
enum class Estatus : uint8_t {
    Normal = 1,
    Contingencia = 2,
    SinInternet = 3
};
using SituacionComprobante = Estatus;

inline std::string get_estatus_code(Estatus s) {
    return std::to_string(static_cast<int>(s));
}

inline std::string get_estatus_description(Estatus s) {
    switch (s) {
        case Estatus::Normal: return "Normal";
        case Estatus::Contingencia: return "Contingencia";
        case Estatus::SinInternet: return "Sin Internet";
        default: return "Desconocido";
    }
}

// ==============================================================================
// Nota 22: Tipo de Transacción
// ==============================================================================
enum class TipoTransaccion : uint8_t {
    VentaNormal = 1,
    MercanciaAutoconsumoExento = 2,
    MercanciaAutoconsumoGravado = 3,
    ServicioAutoconsumoExento = 4,
    ServicioAutoconsumoGravado = 5,
    CuotaAfiliacion = 6,
    CuotaAfiliacionExenta = 7,
    BienesCapitalEmisor = 8,
    BienesCapitalReceptor = 9,
    BienesCapitalEmisorYReceptor = 10,
    BienesCapitalAutoconsumoExentoEmisor = 11,
    BienesCapitalSinContraprestacionTercerosExentoEmisor = 12,
    SinContraprestacionTerceros = 13
};

inline std::string get_tipo_transaccion_code(TipoTransaccion t) {
    uint8_t val = static_cast<uint8_t>(t);
    if (val < 10) return "0" + std::to_string(val);
    return std::to_string(val);
}

inline std::string get_tipo_transaccion_description(TipoTransaccion t) {
    switch (t) {
        case TipoTransaccion::VentaNormal: return "Venta Normal de Bienes y Servicios";
        case TipoTransaccion::MercanciaAutoconsumoExento: return "Mercancía de Autoconsumo exento";
        case TipoTransaccion::MercanciaAutoconsumoGravado: return "Mercancía de Autoconsumo gravado";
        case TipoTransaccion::ServicioAutoconsumoExento: return "Servicio de Autoconsumo exento";
        case TipoTransaccion::ServicioAutoconsumoGravado: return "Servicio de Autoconsumo gravado";
        case TipoTransaccion::CuotaAfiliacion: return "Cuota de afiliación";
        case TipoTransaccion::CuotaAfiliacionExenta: return "Cuota de afiliación Exenta";
        case TipoTransaccion::BienesCapitalEmisor: return "Bienes de Capital para el emisor";
        case TipoTransaccion::BienesCapitalReceptor: return "Bienes de Capital para el receptor";
        case TipoTransaccion::BienesCapitalEmisorYReceptor: return "Bienes de Capital para el emisor y el receptor";
        case TipoTransaccion::BienesCapitalAutoconsumoExentoEmisor: return "Bienes de Capital Autoconsumo Exento Emisor";
        case TipoTransaccion::BienesCapitalSinContraprestacionTercerosExentoEmisor: return "Bienes de Capital Sin Contraprestación Terceros Exento Emisor";
        case TipoTransaccion::SinContraprestacionTerceros: return "Sin Contraprestación Terceros";
        default: return "Desconocido";
    }
}

// ==============================================================================
// Nota 10.1: Tipo de Documento de Exoneración o Autorización
// ==============================================================================
enum class TipoExoneracion : uint8_t {
    ComprasAutorizadasDGT = 1,
    ComprasAutorizadas = 1,
    VentasExentasDiplomaticos = 2,
    AutorizadoLeyEspecial = 3,
    ExencionesDGHLocalGenerica = 4,
    ExencionesDGHTransitorioV = 5,
    ServiciosTuristicosICT = 6,
    TransitorioXVII = 7,
    ExoneracionZonaFranca = 8,
    ExoneracionServiciosComplementariosExportacion = 9,
    OrganoCorporacionesMunicipales = 10,
    ExencionesDGHAutorizacionLocalConcreta = 11,
    Otros = 99
};
using TipoDocExoneracion = TipoExoneracion;

inline std::string get_tipo_exoneracion_code(TipoExoneracion ex) {
    if (ex == TipoExoneracion::Otros) return "99";
    uint8_t val = static_cast<uint8_t>(ex);
    if (val < 10) return "0" + std::to_string(val);
    return std::to_string(val);
}

inline std::string get_tipo_exoneracion_description(TipoExoneracion ex) {
    switch (ex) {
        case TipoExoneracion::ComprasAutorizadasDGT: return "Compras autorizadas por la DGT";
        case TipoExoneracion::VentasExentasDiplomaticos: return "Ventas exentas a diplomáticos";
        case TipoExoneracion::AutorizadoLeyEspecial: return "Autorizado por Ley Especial";
        case TipoExoneracion::ExencionesDGHLocalGenerica: return "Exenciones DGH Autorización Local Genérica";
        case TipoExoneracion::ExencionesDGHTransitorioV: return "Exenciones DGH Transitorio V (servicios ingeniería/arquitectura)";
        case TipoExoneracion::ServiciosTuristicosICT: return "Servicios turísticos inscritos ante el ICT";
        case TipoExoneracion::TransitorioXVII: return "Transitorio XVII (Recolección, Clasificación de Reciclaje)";
        case TipoExoneracion::ExoneracionZonaFranca: return "Exoneración a Zona Franca";
        case TipoExoneracion::ExoneracionServiciosComplementariosExportacion: return "Exoneración servicios complementarios exportación art 11 RLIVA";
        case TipoExoneracion::OrganoCorporacionesMunicipales: return "Órgano de las corporaciones municipales";
        case TipoExoneracion::ExencionesDGHAutorizacionLocalConcreta: return "Exenciones DGH Autorización Local Concreta";
        case TipoExoneracion::Otros: return "Otros";
        default: return "Otros";
    }
}

// ==============================================================================
// Nota 23: Nombre de Institución Emisora de Exoneración
// ==============================================================================
enum class InstitucionExoneracion : uint8_t {
    MinisterioHacienda = 1,
    MinisterioRelacionesExteriores = 2,
    MinisterioAgriculturaGanaderia = 3,
    MinisterioEconomiaIndustriaComercio = 4,
    CruzRojaCostarricense = 5,
    CuerpoBomberosCostaRica = 6,
    AsociacionObrasEspirituSanto = 7,
    Fecrunapa = 8,
    EARTH = 9,
    INCAE = 10,
    JuntaProteccionSocial = 11,
    Aresep = 12,
    Otros = 99
};

inline std::string get_institucion_exoneracion_code(InstitucionExoneracion inst) {
    if (inst == InstitucionExoneracion::Otros) return "99";
    uint8_t val = static_cast<uint8_t>(inst);
    if (val < 10) return "0" + std::to_string(val);
    return std::to_string(val);
}

inline std::string get_institucion_exoneracion_description(InstitucionExoneracion inst) {
    switch (inst) {
        case InstitucionExoneracion::MinisterioHacienda: return "Ministerio de Hacienda";
        case InstitucionExoneracion::MinisterioRelacionesExteriores: return "Ministerio de Relaciones Exteriores y Culto";
        case InstitucionExoneracion::MinisterioAgriculturaGanaderia: return "Ministerio de Agricultura y Ganadería";
        case InstitucionExoneracion::MinisterioEconomiaIndustriaComercio: return "Ministerio de Economía, Industria y Comercio";
        case InstitucionExoneracion::CruzRojaCostarricense: return "Cruz Roja Costarricense";
        case InstitucionExoneracion::CuerpoBomberosCostaRica: return "Benemérito Cuerpo de Bomberos de Costa Rica";
        case InstitucionExoneracion::AsociacionObrasEspirituSanto: return "Asociación Obras del Espíritu Santo";
        case InstitucionExoneracion::Fecrunapa: return "Federación Cruzada Nacional de protección al Anciano (Fecrunapa)";
        case InstitucionExoneracion::EARTH: return "Escuela de Agricultura de la Región Húmeda (EARTH)";
        case InstitucionExoneracion::INCAE: return "Instituto Centroamericano de Administración de Empresas (INCAE)";
        case InstitucionExoneracion::JuntaProteccionSocial: return "Junta de Protección Social";
        case InstitucionExoneracion::Aresep: return "Autoridad Reguladora de los Servicios Públicos (ARESEP)";
        case InstitucionExoneracion::Otros: return "Otros";
        default: return "Otros";
    }
}

// ==============================================================================
// Nota 16: Tipos de Otros Cargos
// ==============================================================================
enum class TipoDocOtrosCargos : uint8_t {
    ContribucionParafiscal = 1,
    TimbreCruzRoja = 2,
    TimbreBomberos = 3,
    CobroTercero = 4,
    CostosExportacion = 5,
    ImpuestoServicio10 = 6,
    TimbreColegiosProfesionales = 7,
    DepositosGarantia = 8,
    MultasPenalizaciones = 9,
    InteresesMoratorios = 10,
    Otros = 99
};

inline std::string get_tipo_doc_otros_cargos_code(TipoDocOtrosCargos oc) {
    if (oc == TipoDocOtrosCargos::Otros) return "99";
    uint8_t val = static_cast<uint8_t>(oc);
    if (val < 10) return "0" + std::to_string(val);
    return std::to_string(val);
}

inline std::string get_tipo_doc_otros_cargos_description(TipoDocOtrosCargos oc) {
    switch (oc) {
        case TipoDocOtrosCargos::ContribucionParafiscal: return "Contribución parafiscal";
        case TipoDocOtrosCargos::TimbreCruzRoja: return "Timbre de la Cruz Roja";
        case TipoDocOtrosCargos::TimbreBomberos: return "Timbre de Benemérito Cuerpo de Bomberos de Costa Rica";
        case TipoDocOtrosCargos::CobroTercero: return "Cobro de un tercero";
        case TipoDocOtrosCargos::CostosExportacion: return "Costos de Exportación";
        case TipoDocOtrosCargos::ImpuestoServicio10: return "Impuesto de servicio 10%";
        case TipoDocOtrosCargos::TimbreColegiosProfesionales: return "Timbre de Colegios Profesionales";
        case TipoDocOtrosCargos::DepositosGarantia: return "Depósitos de Garantía";
        case TipoDocOtrosCargos::MultasPenalizaciones: return "Multas o Penalizaciones";
        case TipoDocOtrosCargos::InteresesMoratorios: return "Intereses Moratorios";
        case TipoDocOtrosCargos::Otros: return "Otros";
        default: return "Otros";
    }
}

// ==============================================================================
// Nota 10: Tipo de Documento de Referencia
// ==============================================================================
enum class TipoDocumentoReferencia : uint8_t {
    FacturaElectronica = 1,
    NotaDebitoElectronica = 2,
    NotaCreditoElectronica = 3,
    TiqueteElectronico = 4,
    NotaDespacho = 5,
    Contrato = 6,
    Procedimiento = 7,
    ComprobanteEmitidoContingencia = 8,
    DevolucionMercaderia = 9,
    ComprobanteRechazadoHacienda = 10,
    SustituyeFacturaRechazadaReceptor = 11,
    SustituyeFacturaExportacion = 12,
    FacturacionMesVencido = 13,
    ComprobanteRegimenEspecial = 14,
    SustituyeFacturaCompra = 15,
    ComprobanteProveedorNoDomiciliado = 16,
    NotaCreditoAFacturaCompra = 17,
    NotaDebitoAFacturaCompra = 18,
    FacturaElectronicaExportacion = 19,
    ReciboElectronicoPago = 20,
    Otros = 99
};

inline std::string get_tipo_doc_referencia_code(TipoDocumentoReferencia ref) {
    if (ref == TipoDocumentoReferencia::Otros) return "99";
    uint8_t val = static_cast<uint8_t>(ref);
    if (val < 10) return "0" + std::to_string(val);
    return std::to_string(val);
}

inline std::string get_tipo_doc_referencia_description(TipoDocumentoReferencia ref) {
    switch (ref) {
        case TipoDocumentoReferencia::FacturaElectronica: return "Factura electrónica";
        case TipoDocumentoReferencia::NotaDebitoElectronica: return "Nota de débito electrónica";
        case TipoDocumentoReferencia::NotaCreditoElectronica: return "Nota de crédito electrónica";
        case TipoDocumentoReferencia::TiqueteElectronico: return "Tiquete electrónico";
        case TipoDocumentoReferencia::NotaDespacho: return "Nota de despacho";
        case TipoDocumentoReferencia::Contrato: return "Contrato";
        case TipoDocumentoReferencia::Procedimiento: return "Procedimiento";
        case TipoDocumentoReferencia::ComprobanteEmitidoContingencia: return "Comprobante emitido en contingencia";
        case TipoDocumentoReferencia::DevolucionMercaderia: return "Devolución mercadería";
        case TipoDocumentoReferencia::ComprobanteRechazadoHacienda: return "Comprobante electrónico rechazado por el Ministerio de Hacienda";
        case TipoDocumentoReferencia::SustituyeFacturaRechazadaReceptor: return "Sustituye factura rechazada por el receptor";
        case TipoDocumentoReferencia::SustituyeFacturaExportacion: return "Sustituye factura de exportación";
        case TipoDocumentoReferencia::FacturacionMesVencido: return "Facturación de mes vencido";
        case TipoDocumentoReferencia::ComprobanteRegimenEspecial: return "Comprobante régimen especial";
        case TipoDocumentoReferencia::SustituyeFacturaCompra: return "Sustituye factura de compra";
        case TipoDocumentoReferencia::ComprobanteProveedorNoDomiciliado: return "Comprobante proveedor no domiciliado";
        case TipoDocumentoReferencia::NotaCreditoAFacturaCompra: return "Nota de crédito a factura de compra";
        case TipoDocumentoReferencia::NotaDebitoAFacturaCompra: return "Nota de débito a factura de compra";
        case TipoDocumentoReferencia::FacturaElectronicaExportacion: return "Factura electrónica de exportación";
        case TipoDocumentoReferencia::ReciboElectronicoPago: return "Recibo electrónico de pago";
        case TipoDocumentoReferencia::Otros: return "Otros";
        default: return "Otros";
    }
}

// ==============================================================================
// Nota 9: Códigos de Referencia a Utilizar
// ==============================================================================
enum class CodigoReferencia : uint8_t {
    AnulaDocumentoReferencia = 1,
    CorrigeMonto = 2,
    ReferenciaOtroDocumento = 4,
    SustituyeComprobanteProvisionalPorContingencia = 5,
    SustituyeDocumentoProvisional = 5,
    DevolucionMercancia = 6,
    SustituyeComprobanteElectronico = 7,
    FacturaEndosada = 8,
    NotaCreditoFinanciera = 9,
    NotaDebitoFinanciera = 10,
    ProveedorNoDomiciliado = 11,
    CreditoExoneracionPosterior = 12,
    AnulaDocumentoReferenciaPorErrorMaterial = 13,
    CorrigeMontoPorErrorMaterial = 14,
    SustituyeComprobanteElectronicoPorErrorMaterial = 15,
    SustituyeComprobanteElectronicoRechazado = 16,
    PagoAComprobanteElectronico = 17,
    Otros = 99
};

inline std::string get_codigo_referencia_code(CodigoReferencia ref) {
    if (ref == CodigoReferencia::Otros) return "99";
    uint8_t val = static_cast<uint8_t>(ref);
    if (val < 10) return "0" + std::to_string(val);
    return std::to_string(val);
}

inline std::string get_codigo_referencia_description(CodigoReferencia ref) {
    switch (ref) {
        case CodigoReferencia::AnulaDocumentoReferencia: return "Anula documento de referencia";
        case CodigoReferencia::CorrigeMonto: return "Corrige monto";
        case CodigoReferencia::ReferenciaOtroDocumento: return "Referencia a otro documento";
        case CodigoReferencia::SustituyeComprobanteProvisionalPorContingencia: return "Sustituye comprobante provisional por contingencia";
        case CodigoReferencia::DevolucionMercancia: return "Devolución de mercancía";
        case CodigoReferencia::SustituyeComprobanteElectronico: return "Sustituye comprobante electrónico";
        case CodigoReferencia::FacturaEndosada: return "Factura Endosada";
        case CodigoReferencia::NotaCreditoFinanciera: return "Nota de crédito financiera";
        case CodigoReferencia::NotaDebitoFinanciera: return "Nota de débito financiera";
        case CodigoReferencia::ProveedorNoDomiciliado: return "Proveedor No Domiciliado";
        case CodigoReferencia::CreditoExoneracionPosterior: return "Nota de crédito financiera por exoneración posterior a la facturación";
        case CodigoReferencia::AnulaDocumentoReferenciaPorErrorMaterial: return "Anula documento de referencia por error material";
        case CodigoReferencia::CorrigeMontoPorErrorMaterial: return "Corrige monto por error material";
        case CodigoReferencia::SustituyeComprobanteElectronicoPorErrorMaterial: return "Sustituye comprobante electrónico por error material";
        case CodigoReferencia::SustituyeComprobanteElectronicoRechazado: return "Sustituye comprobante electrónico rechazado";
        case CodigoReferencia::PagoAComprobanteElectronico: return "Pago a comprobante electrónico";
        case CodigoReferencia::Otros: return "Otros";
        default: return "Otros";
    }
}

// ==============================================================================
// Nota 12: Tipo de Código Comercial
// ==============================================================================
enum class TipoCodigoComercial : uint8_t {
    CodigoProductoVendedor = 1,
    CodigoProductoComprador = 2,
    CodigoProductoFabricante = 3,
    CodigoUsoInterno = 4,
    Otros = 99
};

inline std::string get_tipo_codigo_comercial_code(TipoCodigoComercial tc) {
    if (tc == TipoCodigoComercial::Otros) return "99";
    uint8_t val = static_cast<uint8_t>(tc);
    if (val < 10) return "0" + std::to_string(val);
    return std::to_string(val);
}

inline std::string get_tipo_codigo_comercial_description(TipoCodigoComercial tc) {
    switch (tc) {
        case TipoCodigoComercial::CodigoProductoVendedor: return "Código del producto del vendedor";
        case TipoCodigoComercial::CodigoProductoComprador: return "Código del producto del comprador";
        case TipoCodigoComercial::CodigoProductoFabricante: return "Código del producto asignado por la industria";
        case TipoCodigoComercial::CodigoUsoInterno: return "Código de uso interno";
        case TipoCodigoComercial::Otros: return "Otros";
        default: return "Otros";
    }
}

// ==============================================================================
// Nota 18: Condición del IVA para Mensaje Receptor
// ==============================================================================
enum class CondicionImpuestoReceptor : uint8_t {
    GeneraCreditoIVA = 1,
    GeneraCreditoParcialIVA = 2,
    BienesDeCapital = 3,
    GastoCorrienteNoGeneraCredito = 4,
    Proporcionalidad = 5
};

inline std::string get_condicion_impuesto_receptor_code(CondicionImpuestoReceptor ci) {
    uint8_t val = static_cast<uint8_t>(ci);
    if (val < 10) return "0" + std::to_string(val);
    return std::to_string(val);
}

inline std::string get_condicion_impuesto_receptor_description(CondicionImpuestoReceptor ci) {
    switch (ci) {
        case CondicionImpuestoReceptor::GeneraCreditoIVA: return "Genera crédito IVA";
        case CondicionImpuestoReceptor::GeneraCreditoParcialIVA: return "Genera crédito parcial IVA";
        case CondicionImpuestoReceptor::BienesDeCapital: return "Bienes de Capital";
        case CondicionImpuestoReceptor::GastoCorrienteNoGeneraCredito: return "Gasto corriente no genera crédito";
        case CondicionImpuestoReceptor::Proporcionalidad: return "Proporcionalidad";
        default: return "Desconocido";
    }
}

// ==============================================================================
// Nota 21: IVA Cobrado a Nivel de Fábrica
// ==============================================================================
enum class IVACobradoNivelFabrica : uint8_t {
    VentaConIVANivelFabrica = 1,
    VentasExentasPorHaberSoportadoCobroFabrica = 2
};

inline std::string get_iva_cobrado_fabrica_code(IVACobradoNivelFabrica fab) {
    uint8_t val = static_cast<uint8_t>(fab);
    if (val < 10) return "0" + std::to_string(val);
    return std::to_string(val);
}

inline std::string get_iva_cobrado_fabrica_description(IVACobradoNivelFabrica fab) {
    switch (fab) {
        case IVACobradoNivelFabrica::VentaConIVANivelFabrica:
            return "Venta de bienes con IVA según sistema especial a nivel de fábrica";
        case IVACobradoNivelFabrica::VentasExentasPorHaberSoportadoCobroFabrica:
            return "Ventas exentas según sistema especial a nivel de fábrica";
        default:
            return "Desconocido";
    }
}

// ==============================================================================
// Nota 11: Códigos de Mensajes
// ==============================================================================
enum class MensajeHaciendaTipo : uint8_t {
    Aceptado = 1,
    Rechazado = 3
};

inline std::string get_mensaje_hacienda_tipo_description(MensajeHaciendaTipo m) {
    switch (m) {
        case MensajeHaciendaTipo::Aceptado: return "Aceptado";
        case MensajeHaciendaTipo::Rechazado: return "Rechazado";
        default: return "Desconocido";
    }
}

enum class MensajeReceptorTipo : uint8_t {
    Aceptado = 1,
    AceptadoParcialmente = 2,
    Rechazado = 3
};

inline std::string get_mensaje_receptor_tipo_description(MensajeReceptorTipo m) {
    switch (m) {
        case MensajeReceptorTipo::Aceptado: return "Aceptado";
        case MensajeReceptorTipo::AceptadoParcialmente: return "Aceptado parcialmente";
        case MensajeReceptorTipo::Rechazado: return "Rechazado";
        default: return "Desconocido";
    }
}

// ==============================================================================
// Listas Maestras de Enums para generación de Catálogos (Single Source of Truth)
// ==============================================================================
inline const std::vector<CondicionDeVenta>& get_all_condiciones_venta() {
    static const std::vector<CondicionDeVenta> v = {
        CondicionDeVenta::Contado,
        CondicionDeVenta::Credito,
        CondicionDeVenta::Consignacion,
        CondicionDeVenta::Apartado,
        CondicionDeVenta::ArrendamientoOpcionCompra,
        CondicionDeVenta::ArrendamientoFuncionFinanciera,
        CondicionDeVenta::CobroAFavorDeTercero,
        CondicionDeVenta::ServiciosPrestadosAlEstadoACredito,
        CondicionDeVenta::PagoServiciosPrestadosAlEstado,
        CondicionDeVenta::VentaCreditoIVAHasta90Dias,
        CondicionDeVenta::PagoVentaCreditoIVAHasta90Dias,
        CondicionDeVenta::VentaMercanciaNoNacionalizada,
        CondicionDeVenta::VentaBienesUsadosNoContribuyente,
        CondicionDeVenta::ArrendamientoOperativo,
        CondicionDeVenta::ArrendamientoFinanciero,
        CondicionDeVenta::Otros
    };
    return v;
}

inline const std::vector<MedioPago>& get_all_medios_pago() {
    static const std::vector<MedioPago> v = {
        MedioPago::Efectivo,
        MedioPago::Tarjeta,
        MedioPago::Cheque,
        MedioPago::TransferenciaDepositoBancario,
        MedioPago::RecaudadoPorTerceros,
        MedioPago::SINPEMovil,
        MedioPago::PlataformaDigital,
        MedioPago::Otros
    };
    return v;
}

inline const std::vector<TipoIdentificacion>& get_all_tipos_identificacion() {
    static const std::vector<TipoIdentificacion> v = {
        TipoIdentificacion::CedulaFisica,
        TipoIdentificacion::CedulaJuridica,
        TipoIdentificacion::DIMEX,
        TipoIdentificacion::NITE,
        TipoIdentificacion::ExtranjeroNoDomiciliado,
        TipoIdentificacion::NoContribuyente
    };
    return v;
}

inline const std::vector<CodigoTarifaIVA>& get_all_tarifas_iva() {
    static const std::vector<CodigoTarifaIVA> v = {
        CodigoTarifaIVA::Tarifa0,
        CodigoTarifaIVA::TarifaReducida1,
        CodigoTarifaIVA::TarifaReducida2,
        CodigoTarifaIVA::TarifaReducida4,
        CodigoTarifaIVA::Transitorio0,
        CodigoTarifaIVA::Transitorio4,
        CodigoTarifaIVA::Transitorio8,
        CodigoTarifaIVA::TarifaGeneral13,
        CodigoTarifaIVA::TarifaReducida05,
        CodigoTarifaIVA::TarifaExenta,
        CodigoTarifaIVA::Tarifa0SinCredito
    };
    return v;
}

inline const std::vector<CodigoImpuesto>& get_all_codigos_impuesto() {
    static const std::vector<CodigoImpuesto> v = {
        CodigoImpuesto::ImpuestoValorAgregado,
        CodigoImpuesto::ImpuestoSelectivoConsumo,
        CodigoImpuesto::ImpuestoUnicoCombustibles,
        CodigoImpuesto::ImpuestoBebidasAlcoholicas,
        CodigoImpuesto::ImpuestoBebidasEnvasadasJabones,
        CodigoImpuesto::ImpuestoTabaco,
        CodigoImpuesto::IVACalculoEspecial,
        CodigoImpuesto::IVARegimenBienesUsadosFactor,
        CodigoImpuesto::ImpuestoEspecificoCemento,
        CodigoImpuesto::Otros
    };
    return v;
}

inline const std::vector<CodigoDescuento>& get_all_codigos_descuento() {
    static const std::vector<CodigoDescuento> v = {
        CodigoDescuento::Regalia,
        CodigoDescuento::RegaliaIVACobradoCliente,
        CodigoDescuento::Bonificacion,
        CodigoDescuento::Volumen,
        CodigoDescuento::TemporadaEstacional,
        CodigoDescuento::Promocional,
        CodigoDescuento::Comercial,
        CodigoDescuento::Frecuencia,
        CodigoDescuento::Sostenido,
        CodigoDescuento::Otros
    };
    return v;
}

inline const std::vector<TipoTransaccion>& get_all_tipos_transaccion() {
    static const std::vector<TipoTransaccion> v = {
        TipoTransaccion::VentaNormal,
        TipoTransaccion::MercanciaAutoconsumoExento,
        TipoTransaccion::MercanciaAutoconsumoGravado,
        TipoTransaccion::ServicioAutoconsumoExento,
        TipoTransaccion::ServicioAutoconsumoGravado,
        TipoTransaccion::CuotaAfiliacion,
        TipoTransaccion::CuotaAfiliacionExenta,
        TipoTransaccion::BienesCapitalEmisor,
        TipoTransaccion::BienesCapitalReceptor,
        TipoTransaccion::BienesCapitalEmisorYReceptor,
        TipoTransaccion::BienesCapitalAutoconsumoExentoEmisor,
        TipoTransaccion::BienesCapitalSinContraprestacionTercerosExentoEmisor,
        TipoTransaccion::SinContraprestacionTerceros
    };
    return v;
}

inline const std::vector<TipoExoneracion>& get_all_tipos_exoneracion() {
    static const std::vector<TipoExoneracion> v = {
        TipoExoneracion::ComprasAutorizadasDGT,
        TipoExoneracion::VentasExentasDiplomaticos,
        TipoExoneracion::AutorizadoLeyEspecial,
        TipoExoneracion::ExencionesDGHLocalGenerica,
        TipoExoneracion::ExencionesDGHTransitorioV,
        TipoExoneracion::ServiciosTuristicosICT,
        TipoExoneracion::TransitorioXVII,
        TipoExoneracion::ExoneracionZonaFranca,
        TipoExoneracion::ExoneracionServiciosComplementariosExportacion,
        TipoExoneracion::OrganoCorporacionesMunicipales,
        TipoExoneracion::ExencionesDGHAutorizacionLocalConcreta,
        TipoExoneracion::Otros
    };
    return v;
}

inline const std::vector<InstitucionExoneracion>& get_all_instituciones_exoneracion() {
    static const std::vector<InstitucionExoneracion> v = {
        InstitucionExoneracion::MinisterioHacienda,
        InstitucionExoneracion::MinisterioRelacionesExteriores,
        InstitucionExoneracion::MinisterioAgriculturaGanaderia,
        InstitucionExoneracion::MinisterioEconomiaIndustriaComercio,
        InstitucionExoneracion::CruzRojaCostarricense,
        InstitucionExoneracion::CuerpoBomberosCostaRica,
        InstitucionExoneracion::AsociacionObrasEspirituSanto,
        InstitucionExoneracion::Fecrunapa,
        InstitucionExoneracion::EARTH,
        InstitucionExoneracion::INCAE,
        InstitucionExoneracion::JuntaProteccionSocial,
        InstitucionExoneracion::Aresep,
        InstitucionExoneracion::Otros
    };
    return v;
}

inline const std::vector<TipoDocOtrosCargos>& get_all_tipos_otros_cargos() {
    static const std::vector<TipoDocOtrosCargos> v = {
        TipoDocOtrosCargos::ContribucionParafiscal,
        TipoDocOtrosCargos::TimbreCruzRoja,
        TipoDocOtrosCargos::TimbreBomberos,
        TipoDocOtrosCargos::CobroTercero,
        TipoDocOtrosCargos::CostosExportacion,
        TipoDocOtrosCargos::ImpuestoServicio10,
        TipoDocOtrosCargos::TimbreColegiosProfesionales,
        TipoDocOtrosCargos::DepositosGarantia,
        TipoDocOtrosCargos::MultasPenalizaciones,
        TipoDocOtrosCargos::InteresesMoratorios,
        TipoDocOtrosCargos::Otros
    };
    return v;
}

inline const std::vector<TipoDocumentoReferencia>& get_all_tipos_doc_referencia() {
    static const std::vector<TipoDocumentoReferencia> v = {
        TipoDocumentoReferencia::FacturaElectronica,
        TipoDocumentoReferencia::NotaDebitoElectronica,
        TipoDocumentoReferencia::NotaCreditoElectronica,
        TipoDocumentoReferencia::TiqueteElectronico,
        TipoDocumentoReferencia::NotaDespacho,
        TipoDocumentoReferencia::Contrato,
        TipoDocumentoReferencia::Procedimiento,
        TipoDocumentoReferencia::ComprobanteEmitidoContingencia,
        TipoDocumentoReferencia::DevolucionMercaderia,
        TipoDocumentoReferencia::ComprobanteRechazadoHacienda,
        TipoDocumentoReferencia::SustituyeFacturaRechazadaReceptor,
        TipoDocumentoReferencia::SustituyeFacturaExportacion,
        TipoDocumentoReferencia::FacturacionMesVencido,
        TipoDocumentoReferencia::ComprobanteRegimenEspecial,
        TipoDocumentoReferencia::SustituyeFacturaCompra,
        TipoDocumentoReferencia::ComprobanteProveedorNoDomiciliado,
        TipoDocumentoReferencia::NotaCreditoAFacturaCompra,
        TipoDocumentoReferencia::NotaDebitoAFacturaCompra,
        TipoDocumentoReferencia::FacturaElectronicaExportacion,
        TipoDocumentoReferencia::ReciboElectronicoPago,
        TipoDocumentoReferencia::Otros
    };
    return v;
}

inline const std::vector<CodigoReferencia>& get_all_codigos_referencia() {
    static const std::vector<CodigoReferencia> v = {
        CodigoReferencia::AnulaDocumentoReferencia,
        CodigoReferencia::CorrigeMonto,
        CodigoReferencia::ReferenciaOtroDocumento,
        CodigoReferencia::SustituyeComprobanteProvisionalPorContingencia,
        CodigoReferencia::DevolucionMercancia,
        CodigoReferencia::SustituyeComprobanteElectronico,
        CodigoReferencia::FacturaEndosada,
        CodigoReferencia::NotaCreditoFinanciera,
        CodigoReferencia::NotaDebitoFinanciera,
        CodigoReferencia::ProveedorNoDomiciliado,
        CodigoReferencia::CreditoExoneracionPosterior,
        CodigoReferencia::AnulaDocumentoReferenciaPorErrorMaterial,
        CodigoReferencia::CorrigeMontoPorErrorMaterial,
        CodigoReferencia::SustituyeComprobanteElectronicoPorErrorMaterial,
        CodigoReferencia::SustituyeComprobanteElectronicoRechazado,
        CodigoReferencia::PagoAComprobanteElectronico,
        CodigoReferencia::Otros
    };
    return v;
}

inline const std::vector<TipoCodigoComercial>& get_all_tipos_codigo_comercial() {
    static const std::vector<TipoCodigoComercial> v = {
        TipoCodigoComercial::CodigoProductoVendedor,
        TipoCodigoComercial::CodigoProductoComprador,
        TipoCodigoComercial::CodigoProductoFabricante,
        TipoCodigoComercial::CodigoUsoInterno,
        TipoCodigoComercial::Otros
    };
    return v;
}

inline const std::vector<IVACobradoNivelFabrica>& get_all_iva_cobrado_fabrica() {
    static const std::vector<IVACobradoNivelFabrica> v = {
        IVACobradoNivelFabrica::VentaConIVANivelFabrica,
        IVACobradoNivelFabrica::VentasExentasPorHaberSoportadoCobroFabrica
    };
    return v;
}

inline const std::vector<TipoDocumento>& get_all_tipos_documento() {
    static const std::vector<TipoDocumento> v = {
        TipoDocumento::FacturaElectronica,
        TipoDocumento::NotaDebitoElectronica,
        TipoDocumento::NotaCreditoElectronica,
        TipoDocumento::TiqueteElectronico,
        TipoDocumento::FacturaElectronicaCompra,
        TipoDocumento::FacturaElectronicaExportacion,
        TipoDocumento::ReciboElectronicoPago,
        TipoDocumento::MensajeReceptor,
        TipoDocumento::MensajeHacienda
    };
    return v;
}

// ==============================================================================
// Anexo 2 & 3: Constantes Oficiales de Seguridad, Firma y Conexión API v4.4
// ==============================================================================
namespace constantes_hacienda_v4_4 {
    constexpr const char* kXAdESPolicyUrl =
        "https://cdn.comprobanteselectronicos.go.cr/xml-schemas/Resoluci%C3%B3n_General_sobre_disposiciones_t%C3%A9cnicas_comprobantes_electr%C3%B3nicos_para_efectos_tributarios.pdf";
    constexpr const char* kXAdESPolicyHash =
        "DWxin1xWOeI8OuWQXazh4VjLWAaCLAA954em7DMh0h8=";
    constexpr const char* kXAdESDigestMethod =
        "http://www.w3.org/2001/04/xmlenc#sha256";
    constexpr const char* kXAdESSignatureMethod =
        "http://www.w3.org/2001/04/xmldsig-more#rsa-sha256";
    constexpr const char* kCanonicalizationMethod =
        "http://www.w3.org/2001/10/xml-exc-c14n#";

    // Endpoints API REST v4.4
    constexpr const char* kApiRecepcionProd =
        "https://api.comprobanteselectronicos.go.cr/recepcion/v1/";
    constexpr const char* kApiRecepcionSandbox =
        "https://api.comprobanteselectronicos.go.cr/recepcion-sandbox/v1/";
    constexpr const char* kApiIdpAuth =
        "https://idp.comprobanteselectronicos.go.cr/auth/realms/rut/protocol/openid-connect/token";
    constexpr const char* kApiClientIdProd = "api-prod";
    constexpr const char* kApiClientIdSandbox = "api-stag";
}

} // namespace cec::common
