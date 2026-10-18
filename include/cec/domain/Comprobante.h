#pragma once
#include <string>
#include <vector>
#include <optional>
#include <cstdint>
#include "cec/domain/Money.h"
#include "cec/domain/Types.h"

namespace cec::domain {

struct Identificacion {
    TipoIdentificacion tipo;
    std::string numero;
};

struct Ubicacion {
    std::string provincia;
    std::string canton;
    std::string distrito;
    std::optional<std::string> barrio;
    std::string otrasSenas;
};

struct Telefono {
    std::string codigoPais;
    std::string numero;
};

struct Emisor {
    std::string nombre;
    Identificacion identificacion;
    std::string registroFiscal8707;
    std::string nombreComercial;
    std::vector<std::string> correoElectronico;
    std::optional<Ubicacion> ubicacion;
    std::optional<Telefono> telefono;
    std::optional<std::string> otrasSenasExtranjero;
};

struct Receptor {
    std::string nombre;
    std::optional<Identificacion> identificacion;
    std::string nombreComercial;
    std::string correoElectronico;
    std::optional<Ubicacion> ubicacion;
    std::optional<Telefono> telefono;
    std::optional<std::string> otrasSenasExtranjero;
};

struct DescuentoLinea {
    Money monto;
    std::string codigo;
    std::string naturaleza;          // Obligatorio con código 99 (Nota 20), 3..80
    std::string codigoDescuentoOTRO{}; // Obligatorio con código 99 (Nota 20), 5..100
};

/**
 * @brief Datos para impuestos específicos no tarifarios (códigos 03, 04, 05 y 06 de la Nota 8).
 */
struct DatosImpuestoEspecifico {
    int64_t cantidadUnidadMedidaScaled = 0;   // 03: litros; 04/05: ml del envase; 06: unidades
    std::optional<int64_t> porcentajeScaled;  // 04: % de alcohol por volumen (13 = 13%)
    std::optional<int64_t> proporcionScaled;  // 04: ml de alcohol absoluto (CantidadUnidadMedida * Porcentaje)
    std::optional<int64_t> volumenUnidadConsumoScaled; // 05: volumen de referencia (ml) o gramos de jabón
    Money impuestoUnidad;
};

struct ImpuestoLinea {
    CodigoImpuesto codigo;
    CodigoTarifaIVA codigoTarifa;
    int64_t tarifaPorcentajeScaled; // ej: 1300000 = 13% en base SCALE=100000 (13.00000)
    Money monto;                    // Calculado por el motor
    std::string codigoImpuestoOTRO;                   // Obligatorio con código 99, 5..100
    std::optional<int64_t> factorCalculoIVAScaled;    // Obligatorio con código 08 (bienes usados)
    std::optional<DatosImpuestoEspecifico> datosEspecificos; // Obligatorio con códigos 03..06
    Money montoExportacion;                           // Solo FEE / NC / ND
};

struct ExoneracionLinea {
    std::string tipoDocumento;
    std::string numeroDocumento;
    std::string nombreInstitucion;
    std::string fechaEmision;
    int64_t porcentajeExoneracionScaled; // TarifaExonerada, misma escala que la tarifa (13% = 1300000)
    Money montoExoneracion;              // Calculado por el motor
    std::string tipoDocumentoOTRO{};      // Obligatorio con tipo 99 (Nota 10.1), 5..100
    std::string nombreInstitucionOtros{}; // Obligatorio con institución 99 (Nota 23), 5..160
    std::optional<int> articulo{};        // Obligatorio con tipos 02, 03, 06, 07, 08
    std::optional<int> inciso{};
};

/**
 * @brief Código comercial de un producto/servicio (Nota 12). En surtidos habilitados se exige el tipo 03
 * (código del fabricante: SKU, GTIN o equivalente).
 */
struct CodigoComercial {
    std::string tipo;   // Nota 12: 01, 02, 03, 04 o 99
    std::string codigo; // Máximo 20 caracteres
};

struct ComponenteSurtido {
    std::string codigoCABYS;
    std::vector<CodigoComercial> codigosComerciales; // {0,5}
    int64_t cantidadScaled = 0;   // Escala 100,000
    std::string unidadMedida;
    std::string unidadMedidaComercial;
    std::string descripcion;
    Money precioUnitario;
    Money montoTotal;
    std::vector<DescuentoLinea> descuentos;
    Money montoDescuento;
    Money subTotal;
    Money baseImponible;
    std::vector<ImpuestoLinea> impuestos;
};

/**
 * @brief Línea de comprobante canónico con su desglose completo inmutable.
 */
struct LineaDetalle {
    int numeroLinea;
    std::string partidaArancelaria;
    std::string cabys;
    std::vector<CodigoComercial> codigosComerciales; // {0,5}
    std::string descripcion;
    std::string unidadMedida; // Catálogo Hacienda (ej. "Sp", "Unid", "Al")
    
    // Entradas escaladas
    int64_t cantidadScaled;   // Escala 100,000 (ej. 2 unidades = 200,000)
    Money precioUnitario;

    // Bienes Especiales v4.4
    std::vector<std::string> numerosVIN;
    std::string registroMedicamento;
    std::string formaFarmaceutica;
    std::vector<ComponenteSurtido> surtido;

    // Desglose fiscal calculado
    std::string tipoTransaccion;          // Nota 22 (opcional)
    std::string ivaCobradoFabrica;        // Nota 21: "01" o "02" (opcional)
    std::optional<Money> baseImponibleEditada; // Editable con IVACobradoFabrica 01 o impuesto 07
    std::optional<Money> subTotalEditado;      // Solo REP: monto del pago a registrar para el cálculo del IVA

    Money montoTotal;         // Cantidad * PrecioUnitario
    Money montoDescuento;     // Suma de descuentos
    Money subTotal;           // MontoTotal - MontoDescuento
    Money baseImponible;      // SubTotal (+ impuestos previos si aplica)
    
    std::vector<DescuentoLinea> descuentos;
    std::vector<ImpuestoLinea> impuestos;
    std::optional<ExoneracionLinea> exoneracion;

    Money impuestoAsumidoEmisorFabrica;
    Money impuestoNeto;       // ImpuestoBruto - Exoneracion
    Money montoTotalLinea;    // SubTotal + ImpuestoNeto
    std::string unidadMedidaComercial{};
};

struct MedioPagoInfo {
    MedioPago tipo = MedioPago::Efectivo;
    std::string otros;
    Money totalMedioPago;

    MedioPagoInfo() = default;
    MedioPagoInfo(MedioPago t) : tipo(t) {}
    operator MedioPago() const { return tipo; }
};

struct DesgloseImpuesto {
    CodigoImpuesto codigo;
    CodigoTarifaIVA codigoTarifa;
    Money totalMonto;
};

struct CodigoTipoMoneda {
    std::string codigoMoneda; // "CRC", "USD", etc.
    int64_t tipoCambioScaled; // 100,000
};

struct ResumenTotales {
    CodigoTipoMoneda codigoTipoMoneda;
    Money totalServiciosGravados;
    Money totalServiciosExentos;
    Money totalServiciosExonerados;
    Money totalServiciosNoSujetos;
    Money totalMercanciasGravadas;
    Money totalMercanciasExentas;
    Money totalMercanciasExoneradas;
    Money totalMercanciasNoSujetas;
    Money totalGravado;
    Money totalExento;
    Money totalExonerado;
    Money totalNoSujeto;
    Money totalVenta;
    Money totalDescuentos;
    Money totalVentaNeta;
    std::vector<DesgloseImpuesto> desgloseImpuestos;
    Money totalImpuesto;
    Money totalImpAsumEmisorFabrica;
    Money totalIvaDevuelto;
    Money totalOtrosCargos;
    Money totalComprobante;
};

struct OtrosCargos {
    std::string tipoDocumento;
    std::string numeroIdentidadTercero;
    std::string nombreTercero;
    std::string detalle;
    std::optional<int64_t> porcentajeScaled;
    Money montoCargo;
    std::string tipoDocumentoOTROS{};     // Obligatorio con tipo 99 (Nota 16), 5..100
    std::optional<TipoIdentificacion> tipoIdentidadTercero{};
};

struct InformacionReferencia {
    std::string tipoDoc;
    std::string numero;
    std::string fechaEmision;
    std::string codigo;
    std::string razon;
    std::string tipoDocRefOTRO{};       // Obligatorio con tipo 99 (Nota 10), 5..100
    std::string codigoReferenciaOTRO{}; // Obligatorio con código 99 (Nota 9), 5..100
};

struct ComprobanteCanonico {
    TipoDocumento tipoDocumento;
    std::string clave;
    std::string proveedorSistemas;
    std::string codigoActividadEmisor;
    std::string codigoActividadReceptor;
    std::string numeroConsecutivo;
    std::string fechaEmision; // ISO 8601
    CondicionVenta condicionVenta;
    std::string condicionVentaOtros;
    std::string plazoCredito;
    std::vector<MedioPagoInfo> mediosPago;

    Emisor emisor;
    std::optional<Receptor> receptor;
    std::vector<LineaDetalle> lineas;
    std::vector<OtrosCargos> otrosCargos;
    ResumenTotales resumen;
    std::vector<InformacionReferencia> referencias;

    // Banderas
    bool isFE  = false;
    bool isFEE = false; 
    bool isFEC = false; 
    bool isTE  = false; 
    bool isNC  = false; 
    bool isND  = false; 
    bool isREP = false; 
};

} // namespace cec::domain
