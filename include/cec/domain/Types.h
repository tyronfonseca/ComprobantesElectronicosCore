#pragma once
#include <cstdint>
#include <string>
#include <string_view>

namespace cec::domain {

enum class TipoDocumento : uint8_t {
    FacturaElectronica = 1,
    NotaDebitoElectronica = 2,
    NotaCreditoElectronica = 3,
    TiqueteElectronico = 4,
    FacturaElectronicaCompra = 8,
    FacturaElectronicaExportacion = 9,
    ReciboElectronicoPago = 10
};

inline std::string_view getTipoDocumentoCode(TipoDocumento tipo) noexcept {
    switch (tipo) {
        case TipoDocumento::FacturaElectronica: return "01";
        case TipoDocumento::NotaDebitoElectronica: return "02";
        case TipoDocumento::NotaCreditoElectronica: return "03";
        case TipoDocumento::TiqueteElectronico: return "04";
        case TipoDocumento::FacturaElectronicaCompra: return "08";
        case TipoDocumento::FacturaElectronicaExportacion: return "09";
        case TipoDocumento::ReciboElectronicoPago: return "10";
    }
    return "01";
}

inline std::string_view getTipoDocumentoXmlTagName(TipoDocumento tipo) noexcept {
    switch (tipo) {
        case TipoDocumento::FacturaElectronica: return "FacturaElectronica";
        case TipoDocumento::NotaDebitoElectronica: return "NotaDebitoElectronica";
        case TipoDocumento::NotaCreditoElectronica: return "NotaCreditoElectronica";
        case TipoDocumento::TiqueteElectronico: return "TiqueteElectronico";
        case TipoDocumento::FacturaElectronicaCompra: return "FacturaElectronicaCompra";
        case TipoDocumento::FacturaElectronicaExportacion: return "FacturaElectronicaExportacion";
        case TipoDocumento::ReciboElectronicoPago: return "ReciboElectronicoPago";
    }
    return "FacturaElectronica";
}

inline std::string_view getTipoDocumentoNamespace(TipoDocumento tipo) noexcept {
    switch (tipo) {
        case TipoDocumento::FacturaElectronica:
            return "https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/facturaElectronica";
        case TipoDocumento::NotaDebitoElectronica:
            return "https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/notaDebitoElectronica";
        case TipoDocumento::NotaCreditoElectronica:
            return "https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/notaCreditoElectronica";
        case TipoDocumento::TiqueteElectronico:
            return "https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/tiqueteElectronico";
        case TipoDocumento::FacturaElectronicaCompra:
            return "https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/facturaElectronicaCompra";
        case TipoDocumento::FacturaElectronicaExportacion:
            return "https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/facturaElectronicaExportacion";
        case TipoDocumento::ReciboElectronicoPago:
            return "https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/reciboElectronicoPago";
    }
    return "https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/facturaElectronica";
}

enum class TipoIdentificacion : uint8_t {
    CedulaFisica = 1,
    CedulaJuridica = 2,
    DIMEX = 3,
    NITE = 4,
    ExtranjeroNoDomiciliado = 5,
    ExtranjeroIdentificacion = 6
};

inline std::string_view getTipoIdentificacionCode(TipoIdentificacion t) noexcept {
    switch (t) {
        case TipoIdentificacion::CedulaFisica: return "01";
        case TipoIdentificacion::CedulaJuridica: return "02";
        case TipoIdentificacion::DIMEX: return "03";
        case TipoIdentificacion::NITE: return "04";
        case TipoIdentificacion::ExtranjeroNoDomiciliado: return "05";
        case TipoIdentificacion::ExtranjeroIdentificacion: return "06";
    }
    return "01";
}

enum class CondicionVenta : uint8_t {
    Contado = 1,
    Credito = 2,
    Consignacion = 3,
    Apartado = 4,
    ArrendamientoOpcionCompra = 5,
    ArrendamientoFuncionFinanciera = 6,
    CobroAFavorTercero = 7,
    ServiciosPrestadosAlEstado = 8,
    PagoServiciosPrestadosAlEstado = 9,
    VentaCreditoEnPlazo = 10,
    PagoACreditoEnPlazo = 11,
    VentaMercanciaNoNacionalizada = 12,
    VentaBienesInmuebles = 12,
    VentaBienesUsadosNoContribuyentes = 13,
    VentaPlazosMedianosLargos = 13,
    VentaPlazosMayorTresAnos = 14,
    RegimenEspecialBienesUsados = 15,
    Otros = 99
};

inline std::string_view getCondicionVentaCode(CondicionVenta c) noexcept {
    switch (c) {
        case CondicionVenta::Contado: return "01";
        case CondicionVenta::Credito: return "02";
        case CondicionVenta::Consignacion: return "03";
        case CondicionVenta::Apartado: return "04";
        case CondicionVenta::ArrendamientoOpcionCompra: return "05";
        case CondicionVenta::ArrendamientoFuncionFinanciera: return "06";
        case CondicionVenta::CobroAFavorTercero: return "07";
        case CondicionVenta::ServiciosPrestadosAlEstado: return "08";
        case CondicionVenta::PagoServiciosPrestadosAlEstado: return "09";
        case CondicionVenta::VentaCreditoEnPlazo: return "10";
        case CondicionVenta::PagoACreditoEnPlazo: return "11";
        case CondicionVenta::VentaBienesInmuebles: return "12";
        case CondicionVenta::VentaPlazosMedianosLargos: return "13";
        case CondicionVenta::VentaPlazosMayorTresAnos: return "14";
        case CondicionVenta::RegimenEspecialBienesUsados: return "15";
        case CondicionVenta::Otros: return "99";
    }
    return "01";
}

enum class MedioPago : uint8_t {
    Efectivo = 1,
    Tarjeta = 2,
    Cheque = 3,
    Transferencia = 4,
    RecaudadoPorTerceros = 5,
    SINPE = 6,
    PlataformaDigital = 7,
    Otros = 99
};

inline std::string_view getMedioPagoCode(MedioPago m) noexcept {
    switch (m) {
        case MedioPago::Efectivo: return "01";
        case MedioPago::Tarjeta: return "02";
        case MedioPago::Cheque: return "03";
        case MedioPago::Transferencia: return "04";
        case MedioPago::RecaudadoPorTerceros: return "05";
        case MedioPago::SINPE: return "06";
        case MedioPago::PlataformaDigital: return "07";
        case MedioPago::Otros: return "99";
    }
    return "01";
}

// Nota 8 (CodigoImpuestoType, XSD v4.4).
enum class CodigoImpuesto : uint8_t {
    IVA = 1,
    SelectivoConsumo = 2,
    Combustibles = 3,
    Alcoholicas = 4,
    BebidasEnvasadasJabones = 5,  // Bebidas envasadas sin contenido alcohólico y jabones de tocador
    Tabaco = 6,
    IVACalculoEspecial = 7,
    IVABienesUsados = 8,          // IVA Régimen de Bienes Usados (Factor)
    Cemento = 12,
    Otros = 99
};

inline std::string_view getCodigoImpuestoCode(CodigoImpuesto c) noexcept {
    switch (c) {
        case CodigoImpuesto::IVA: return "01";
        case CodigoImpuesto::SelectivoConsumo: return "02";
        case CodigoImpuesto::Combustibles: return "03";
        case CodigoImpuesto::Alcoholicas: return "04";
        case CodigoImpuesto::BebidasEnvasadasJabones: return "05";
        case CodigoImpuesto::Tabaco: return "06";
        case CodigoImpuesto::IVACalculoEspecial: return "07";
        case CodigoImpuesto::IVABienesUsados: return "08";
        case CodigoImpuesto::Cemento: return "12";
        case CodigoImpuesto::Otros: return "99";
    }
    return "01";
}

/// @brief true para los códigos que son IVA: 01, 07 (cálculo especial) y 08 (bienes usados).
inline constexpr bool esIVA(CodigoImpuesto c) noexcept {
    return c == CodigoImpuesto::IVA ||
           c == CodigoImpuesto::IVACalculoEspecial ||
           c == CodigoImpuesto::IVABienesUsados;
}

enum class CodigoTarifaIVA : uint8_t {
    Tarifa0 = 1,
    TarifaReducida1 = 2,
    TarifaReducida2 = 3,
    TarifaReducida4 = 4,
    TarifaTransitoria0 = 5,
    TarifaTransitoria4 = 6,
    TarifaTransitoria8 = 7,
    TarifaGeneral13 = 8,
    Tarifa05 = 9,
    TarifaExento = 10,
    Tarifa0SinCredito = 11
};

inline std::string_view getCodigoTarifaIVACode(CodigoTarifaIVA t) noexcept {
    switch (t) {
        case CodigoTarifaIVA::Tarifa0: return "01";
        case CodigoTarifaIVA::TarifaReducida1: return "02";
        case CodigoTarifaIVA::TarifaReducida2: return "03";
        case CodigoTarifaIVA::TarifaReducida4: return "04";
        case CodigoTarifaIVA::TarifaTransitoria0: return "05";
        case CodigoTarifaIVA::TarifaTransitoria4: return "06";
        case CodigoTarifaIVA::TarifaTransitoria8: return "07";
        case CodigoTarifaIVA::TarifaGeneral13: return "08";
        case CodigoTarifaIVA::Tarifa05: return "09";
        case CodigoTarifaIVA::TarifaExento: return "10";
        case CodigoTarifaIVA::Tarifa0SinCredito: return "11";
    }
    return "08";
}

inline int64_t getTarifaPorcentajeScaled(CodigoTarifaIVA t) noexcept {
    // Escala x 100,000 para el porcentaje oficial (ej. 13% = 13.00000 -> 1300000)
    switch (t) {
        case CodigoTarifaIVA::Tarifa0: return 0;
        case CodigoTarifaIVA::TarifaReducida1: return 100000;    // 1%
        case CodigoTarifaIVA::TarifaReducida2: return 200000;    // 2%
        case CodigoTarifaIVA::TarifaReducida4: return 400000;    // 4%
        case CodigoTarifaIVA::TarifaTransitoria0: return 0;
        case CodigoTarifaIVA::TarifaTransitoria4: return 400000; // 4%
        case CodigoTarifaIVA::TarifaTransitoria8: return 800000; // 8%
        case CodigoTarifaIVA::TarifaGeneral13: return 1300000;   // 13%
        case CodigoTarifaIVA::Tarifa05: return 50000;            // 0.5%
        case CodigoTarifaIVA::TarifaExento: return 0;
        case CodigoTarifaIVA::Tarifa0SinCredito: return 0;
    }
    return 1300000;
}

} // namespace cec::domain
