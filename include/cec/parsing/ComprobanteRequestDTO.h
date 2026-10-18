#pragma once
#include <optional>
#include <string>
#include <vector>

namespace cec::parsing {

/**
 * Request de un comprobante: solo los datos de entrada, sin los campos que calcula el motor
 * (MontoTotal, SubTotal, montos de impuesto, totales del resumen...).
 *
 * Cada campo se llama como su etiqueta en el Anexo 1 v4.4, en camelCase; las listas van en plural.
 * Los códigos de catálogo son texto de dos dígitos ("01") y los montos y cantidades son decimales en texto
 * ("1500.5"), para no perder precisión.
 */

struct IdentificacionInput {
    std::string tipo;   // Nota 4
    std::string numero;
};

struct UbicacionInput {
    std::string provincia;
    std::string canton;
    std::string distrito;
    std::string barrio;
    std::string otrasSenas;
};

struct TelefonoInput {
    std::string codigoPais;
    std::string numTelefono;
};

struct EmisorInput {
    std::string nombre;
    IdentificacionInput identificacion;
    std::string registroFiscal8707;
    std::string nombreComercial;
    std::optional<UbicacionInput> ubicacion;
    std::string otrasSenasExtranjero;
    std::optional<TelefonoInput> telefono;
    std::vector<std::string> correoElectronico; // {1,4}
};

struct ReceptorInput {
    std::string nombre;
    std::optional<IdentificacionInput> identificacion;
    std::string nombreComercial;
    std::optional<UbicacionInput> ubicacion;
    std::string otrasSenasExtranjero;
    std::optional<TelefonoInput> telefono;
    std::string correoElectronico;
};

struct CodigoComercialInput {
    std::string tipo;   // Nota 12
    std::string codigo;
};

struct DescuentoInput {
    std::string montoDescuento;
    std::string codigoDescuento;     // Nota 20
    std::string codigoDescuentoOTRO; // Con código 99
    std::string naturalezaDescuento; // Con código 99
};

struct DatosImpuestoEspecificoInput {
    std::string cantidadUnidadMedida;
    std::string porcentaje;
    std::string proporcion;
    std::string volumenUnidadConsumo;
    std::string impuestoUnidad;
};

struct ExoneracionInput {
    std::string tipoDocumentoEX1;       // Nota 10.1
    std::string tipoDocumentoOTRO;
    std::string numeroDocumento;
    std::optional<int> articulo;
    std::optional<int> inciso;
    std::string nombreInstitucion;      // Nota 23
    std::string nombreInstitucionOtros;
    std::string fechaEmisionEX;
    std::string tarifaExonerada;
};

struct ImpuestoInput {
    std::string codigo;             // Nota 8
    std::string codigoImpuestoOTRO; // Con código 99
    std::string codigoTarifaIVA;    // Nota 8.1, con códigos 01 y 07
    std::string tarifa;             // Tarifa propia de los impuestos 02, 12 y 99
    std::string factorCalculoIVA;   // Con código 08
    std::optional<DatosImpuestoEspecificoInput> datosImpuestoEspecifico; // Con códigos 03-06
    std::string montoExportacion;   // FEE, NC y ND: exportación de mercancías con impuestos especiales
};

struct LineaDetalleSurtidoInput {
    std::string codigoCABYS;
    std::vector<CodigoComercialInput> codigosComerciales;
    std::string cantidad;
    std::string unidadMedida;
    std::string unidadMedidaComercial;
    std::string detalle;
    std::string precioUnitario;
    std::vector<DescuentoInput> descuentos;
    std::vector<ImpuestoInput> impuestos;
};

struct LineaDetalleInput {
    std::optional<int> numeroLinea; // Si se omite, se numera según la posición
    std::string partidaArancelaria;
    std::string codigoCABYS;
    std::vector<CodigoComercialInput> codigosComerciales;
    std::string cantidad;
    std::string unidadMedida;
    std::string tipoTransaccion;    // Nota 22
    std::string unidadMedidaComercial;
    std::string detalle;
    std::vector<std::string> numerosVINoSerie;
    std::string registroMedicamento;
    std::string formaFarmaceutica;
    std::vector<LineaDetalleSurtidoInput> detalleSurtido;
    std::string precioUnitario;
    std::string montoTotal;         // Solo REP: monto a pagar de lo facturado, sin impuestos (no hay cantidad ni precio)
    std::vector<DescuentoInput> descuentos;
    std::string subTotal;           // Solo REP: monto del pago a registrar
    std::string ivaCobradoFabrica;  // Nota 21
    std::string baseImponible;      // Solo si es editable (IVACobradoFabrica 01 o impuesto 07)
    std::vector<ImpuestoInput> impuestos;
    std::string impuestoAsumidoEmisorFabrica;
    std::optional<ExoneracionInput> exoneracion; // Exonera el IVA de la línea; en el XML va dentro de su Impuesto
};

struct OtrosCargosInput {
    std::string tipoDocumentoOC;    // Nota 16
    std::string tipoDocumentoOTROS;
    std::optional<IdentificacionInput> identificacionTercero;
    std::string nombreTercero;
    std::string detalle;
    std::string porcentajeOC;
    std::string montoCargo;
};

struct MedioPagoInput {
    std::string tipoMedioPago;      // Nota 6
    std::string medioPagoOtros;
    std::string totalMedioPago;
};

struct InformacionReferenciaInput {
    std::string tipoDocIR;          // Nota 10
    std::string tipoDocRefOTRO;
    std::string numero;
    std::string fechaEmisionIR;
    std::string codigo;             // Nota 9
    std::string codigoReferenciaOTRO;
    std::string razon;
};

struct ComprobanteRequestDTO {
    std::string tipoDocumento;      // Nota 3: 01, 02, 03, 04, 08, 09, 10
    std::string clave;
    std::string proveedorSistemas;
    std::string codigoActividadEmisor;
    std::string codigoActividadReceptor;
    std::string numeroConsecutivo;
    std::string fechaEmision;

    EmisorInput emisor;
    std::optional<ReceptorInput> receptor;

    std::string condicionVenta;     // Nota 5
    std::string condicionVentaOtros;
    std::optional<int> plazoCredito;

    std::vector<LineaDetalleInput> detalleServicio;
    std::vector<OtrosCargosInput> otrosCargos;

    std::string codigoMoneda;
    std::string tipoCambio;
    std::vector<MedioPagoInput> mediosPago;

    std::vector<InformacionReferenciaInput> informacionReferencia;
};

} // namespace cec::parsing
