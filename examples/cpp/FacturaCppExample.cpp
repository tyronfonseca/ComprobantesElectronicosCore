/*
 * Ejemplo de la API de C++: arma el request de una Factura Electrónica, la procesa e imprime el XML,
 * los totales y el comprobante en JSON (mismo recorrido que el XML). Datos ficticios.
 *
 *   make examples
 */
#include <iostream>
#include "cec/CEC.h"

int main() {
    cec::parsing::ComprobanteRequestDTO req;
    req.tipoDocumento = "01";
    req.clave = "50601102600310100010100100001010000000001110000001";
    req.proveedorSistemas = "3101000999";
    req.codigoActividadEmisor = "6201.0";
    req.numeroConsecutivo = "00100001010000000001";
    req.fechaEmision = "2026-10-01T10:00:00-06:00";

    req.emisor.nombre = "SOLUCIONES DE SOFTWARE EJEMPLO S.A.";
    req.emisor.identificacion = {"02", "3101000101"};
    req.emisor.ubicacion = cec::parsing::UbicacionInput{"1", "01", "08", "", "Avenida 10, edificio azul"};
    req.emisor.telefono = cec::parsing::TelefonoInput{"506", "22000101"};
    req.emisor.correoElectronico = {"facturacion@software.example.com"};

    cec::parsing::ReceptorInput receptor;
    receptor.nombre = "COMERCIALIZADORA CLIENTE EJEMPLO S.A.";
    receptor.identificacion = cec::parsing::IdentificacionInput{"02", "3101000202"};
    receptor.correoElectronico = "cuentasporpagar@cliente.example.com";
    req.receptor = receptor;

    req.condicionVenta = "01";

    cec::parsing::LineaDetalleInput linea;
    linea.codigoCABYS = "8314100000200";
    linea.cantidad = "40";
    linea.unidadMedida = "h";
    linea.detalle = "Desarrollo de software a la medida (horas)";
    linea.precioUnitario = "25000";
    cec::parsing::ImpuestoInput iva;
    iva.codigo = "01";
    iva.codigoTarifaIVA = "08";   // IVA 13%
    linea.impuestos = {iva};
    req.detalleServicio = {linea};

    req.codigoMoneda = "CRC";
    req.mediosPago = {{"04", "", ""}};   // Transferencia

    const auto resultado = cec::service::ComprobanteService::process(req);
    if (!resultado.isSuccess) {
        for (const auto& d : resultado.diagnostics.items()) {
            std::cout << d.code << " [" << d.path << "]: " << d.message << "\n";
        }
        return 1;
    }

    std::cout << "Total comprobante: " << resultado.comprobante->resumen.totalComprobante.toString(5) << "\n";
    std::cout << resultado.xml;
    std::cout << cec::codec::FormatoJson::comoJson(*resultado.comprobante).dump(2) << "\n";
    return 0;
}
