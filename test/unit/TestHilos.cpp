#include <gtest/gtest.h>
#include <atomic>
#include <thread>
#include <vector>

#include "../support/CertificadoPrueba.h"
#include "../support/Fixtures.h"
#include "cec/c_api/cec_c_api.h"
#include "cec/mensaje/MensajeReceptor.h"

using namespace cec;

// La biblioteca se puede llamar desde varios hilos a la vez. Sin un sanitizador este test solo comprueba que
// los resultados no cambien; con ThreadSanitizer (make tsan) detecta además las condiciones de carrera.
TEST(Hilos, ProcesarFirmarYConsultarCatalogosEnParalelo) {
    const auto req = fixtures::request("fe_completa");
    const auto base = fixtures::procesar(req);
    ASSERT_TRUE(base.isSuccess) << fixtures::diagnosticos(base);
    const auto p12 = fixtures::generarP12Prueba("1234");
    const uint32_t totalTarifas = cec_catalogo_total_elementos("TarifasIVA");
    ASSERT_GT(totalTarifas, 0u);

    mensaje::MensajeReceptorRequest mensajeReceptor;
    mensajeReceptor.clave = "50616102600310100010100100001010000000001110000001";
    mensajeReceptor.numeroCedulaEmisor = "3101000101";
    mensajeReceptor.numeroCedulaReceptor = "3101000202";
    mensajeReceptor.fechaEmisionDoc = "2026-10-16T10:00:00-06:00";
    mensajeReceptor.mensaje = "1";
    mensajeReceptor.totalFactura = "113000";
    mensajeReceptor.numeroConsecutivoReceptor = "00100001050000000001";

    std::atomic<int> fallos{0};
    auto trabajo = [&] {
        for (int i = 0; i < 10; ++i) {
            if (fixtures::procesar(req).xml != base.xml) ++fallos;
            if (!service::ComprobanteService::calcular(req).comprobante) ++fallos;
            if (!service::ComprobanteService::signXml(base.xml, p12, "1234").isSuccess) ++fallos;
            if (!mensaje::procesar(mensajeReceptor).isSuccess) ++fallos;

            // Catálogos por nombre (caché por hilo) y división territorial.
            const char *codigo = nullptr, *descripcion = nullptr;
            if (cec_catalogo_total_elementos("TarifasIVA") != totalTarifas) ++fallos;
            for (uint32_t k = 0; k < totalTarifas; ++k) {
                if (cec_catalogo_elemento_por_indice("TarifasIVA", k, &codigo, &descripcion) == 0) ++fallos;
            }
            if (cec_dta_validar_ubicacion(1, 1, 1) == 0) ++fallos;
        }
    };

    std::vector<std::thread> hilos;
    for (int i = 0; i < 8; ++i) hilos.emplace_back(trabajo);
    for (auto& h : hilos) h.join();
    EXPECT_EQ(fallos.load(), 0);
}
