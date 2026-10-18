/*
 * Ejemplo del binding de Swift: arma una Factura Electrónica, la procesa e imprime el XML y los totales,
 * o los diagnósticos. Datos ficticios.
 *
 *   swift run FacturaSwiftExample
 */
import Foundation
import ComprobantesElectronicosCore

let fecha = fechaEmision()   // Hora actual de Costa Rica
let consecutivo = try generarConsecutivo(sucursal: 1, terminal: 1, tipoDocumento: .facturaElectronica, numero: 1)
let clave = try generarClave(fechaEmision: fecha, identificacion: "3101000101", numeroConsecutivo: consecutivo,
                             codigoSeguridad: "10000001")

let request = ComprobanteRequest(
    tipoDocumento: .facturaElectronica,
    clave: clave,
    proveedorSistemas: "3101000999",
    codigoActividadEmisor: "6201.0",
    numeroConsecutivo: consecutivo,
    fechaEmision: fecha,
    emisor: Emisor(
        nombre: "SOLUCIONES DE SOFTWARE EJEMPLO S.A.",
        identificacion: Identificacion(tipo: .juridica, numero: "3101000101"),
        ubicacion: Ubicacion(provincia: "1", canton: "01", distrito: "08", otrasSenas: "Avenida 10, edificio azul"),
        telefono: Telefono(numTelefono: "22000101"),
        correoElectronico: ["facturacion@software.example.com"]),
    receptor: Receptor(
        nombre: "COMERCIALIZADORA CLIENTE EJEMPLO S.A.",
        identificacion: Identificacion(tipo: .juridica, numero: "3101000202"),
        correoElectronico: "cuentasporpagar@cliente.example.com"),
    condicionVenta: .contado,
    detalleServicio: [
        LineaDetalle(codigoCABYS: "8314100000200", cantidad: 40, unidadMedida: "h",
                     detalle: "Desarrollo de software a la medida (horas)", precioUnitario: 25000,
                     impuestos: [.iva(.general13)])
    ],
    resumenFactura: ResumenFacturaRequest(mediosPago: [MedioPago(tipoMedioPago: .transferencia)]))

let resultado = try FacturaElectronica.procesar(request)
if resultado.exito, let xml = resultado.xml, let totales = resultado.totales {
    print("Total comprobante: \(totales.totalComprobante)")
    print(xml)
} else {
    for d in resultado.diagnosticos {
        print("\(d.codigo) [\(d.ruta)]: \(d.mensaje)")
    }
    exit(1)
}
