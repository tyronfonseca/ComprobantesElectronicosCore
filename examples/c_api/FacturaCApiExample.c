/*
 * Ejemplo de la API de C: arma una Factura Electrónica con structs, la procesa e imprime el XML
 * o los diagnósticos. Datos ficticios.
 *
 *   make examples
 */
#include <stdio.h>
#include "cec/c_api/cec_c_api.h"

int main(void) {
    const char* correos[] = { "facturacion@software.example.com" };
    cec_ubicacion_c ubicacion = { "1", "01", "08", NULL, "Avenida 10, edificio azul" };
    cec_telefono_c telefono = { "506", "22000101" };
    cec_identificacion_c idReceptor = { "02", "3101000202" };
    cec_receptor_c receptor = { "COMERCIALIZADORA CLIENTE EJEMPLO S.A.", &idReceptor, NULL, NULL, NULL, NULL,
                                "cuentasporpagar@cliente.example.com" };
    cec_impuesto_c iva = { "01", NULL, "08", NULL, NULL, NULL, NULL };   /* IVA 13% */
    cec_medio_pago_c medio = { "04", NULL, NULL };                       /* Transferencia */

    cec_linea_c linea = { 0 };
    linea.numeroLinea = CEC_SIN_VALOR;   /* Se numera según la posición */
    linea.codigoCABYS = "8314100000200";
    linea.cantidad = "40";
    linea.unidadMedida = "h";
    linea.detalle = "Desarrollo de software a la medida (horas)";
    linea.precioUnitario = "25000";
    linea.impuestos = &iva;
    linea.numImpuestos = 1;

    cec_request_c req = { 0 };
    req.tipoDocumento = "01";
    req.clave = "50601102600310100010100100001010000000001110000001";
    req.proveedorSistemas = "3101000999";
    req.codigoActividadEmisor = "6201.0";
    req.numeroConsecutivo = "00100001010000000001";
    req.fechaEmision = "2026-10-01T10:00:00-06:00";
    req.emisor.nombre = "SOLUCIONES DE SOFTWARE EJEMPLO S.A.";
    req.emisor.identificacion.tipo = "02";
    req.emisor.identificacion.numero = "3101000101";
    req.emisor.ubicacion = &ubicacion;
    req.emisor.telefono = &telefono;
    req.emisor.correoElectronico = correos;
    req.emisor.numCorreoElectronico = 1;
    req.receptor = &receptor;
    req.condicionVenta = "01";
    req.plazoCredito = CEC_SIN_VALOR;
    req.detalleServicio = &linea;
    req.numDetalleServicio = 1;
    req.codigoMoneda = "CRC";
    req.mediosPago = &medio;
    req.numMediosPago = 1;

    cec_result* res = NULL;
    if (cec_build(&req, &res) != CEC_STATUS_OK) {
        fprintf(stderr, "Error de uso de la API\n");
        return 1;
    }

    int32_t exito = 0;
    cec_result_is_success(res, &exito);
    if (exito) {
        cec_resumen_totales_c totales;
        cec_result_totales(res, &totales);
        const uint8_t* xml = NULL;
        size_t len = 0;
        cec_result_xml(res, &xml, &len);
        printf("Total comprobante: %lld.%05lld\n", (long long)(totales.totalComprobante / 100000),
               (long long)(totales.totalComprobante % 100000));
        printf("%.*s", (int)len, (const char*)xml);
    } else {
        size_t total = 0;
        cec_result_total_diagnosticos(res, &total);
        for (size_t i = 0; i < total; ++i) {
            const char *codigo, *ruta, *mensaje;
            cec_result_diagnostico(res, i, NULL, &codigo, &ruta, &mensaje);
            printf("%s [%s]: %s\n", codigo, ruta, mensaje);
        }
    }
    cec_result_free(res);
    return exito ? 0 : 1;
}
