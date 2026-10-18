/**
 * Tests del binding contra el módulo WebAssembly real (dist/wasm/cec.js, generado con
 * scripts/build_wasm.sh). Los requests son los casos golden del core: el XML debe ser idéntico.
 */
import { before, describe, test } from "node:test";
import assert from "node:assert/strict";
import { execFileSync } from "node:child_process";
import { createPrivateKey, sign } from "node:crypto";
import { mkdtempSync, readFileSync, rmSync } from "node:fs";
import { createRequire } from "node:module";
import { tmpdir } from "node:os";
import { join } from "node:path";
import {
    FacturaElectronica,
    fechaEmision,
    generarClave,
    generarConsecutivo,
    type ComprobanteRequest,
} from "../src/index.ts";

const raiz = join(import.meta.dirname, "..", "..", "..");
const golden = join(raiz, "test", "golden");
const require = createRequire(import.meta.url);

function caso(nombre: string): ComprobanteRequest {
    return JSON.parse(readFileSync(join(golden, nombre, "request.json"), "utf8"));
}

let fe: FacturaElectronica;

before(async () => {
    fe = await FacturaElectronica.cargar(require(join(raiz, "dist", "wasm", "cec.js")));
});

describe("procesar", () => {
    for (const nombre of ["fe_servicios", "fe_completa", "fe_surtido", "te", "nc", "nd", "fec", "fee", "rep"]) {
        test(`${nombre}: mismo XML que el golden del core`, () => {
            const r = fe.procesar(caso(nombre));
            assert.equal(r.exito, true, JSON.stringify(r.diagnosticos));
            assert.equal(r.xml, readFileSync(join(golden, nombre, "expected.xml"), "utf8"));
        });
    }

    test("devuelve el comprobante calculado con la estructura del XML", () => {
        const r = fe.procesar(caso("fe_completa"));
        assert.equal(r.comprobante?.ResumenFactura.TotalComprobante, "1645798.69500");
        assert.equal(r.comprobante?.ResumenFactura.TotalOtrosCargos, "33827.00000");
        assert.deepEqual(Object.keys(r.comprobante!).slice(0, 2), ["Clave", "ProveedorSistemas"]);
    });

    test("un comprobante inválido trae diagnósticos con la ruta del campo", () => {
        const req = caso("fe_servicios");
        req.detalleServicio![0].codigoCABYS = "123";
        const r = fe.procesar(req);
        assert.equal(r.exito, false);
        assert.equal(r.xml, undefined);
        const d = r.diagnosticos.find((x) => x.codigo === "RULE_CABYS_FORMAT");
        assert.ok(d, JSON.stringify(r.diagnosticos));
        assert.equal(d.severidad, "error");
        assert.equal(d.ruta, "detalleServicio[0].codigoCABYS");
    });

    test("un tipo incorrecto en el request es un diagnóstico, no una excepción", () => {
        const req = caso("fe_servicios") as any;
        req.detalleServicio = { no: "es lista" };
        const r = fe.procesar(req);
        assert.equal(r.exito, false);
        assert.equal(r.diagnosticos[0].codigo, "JSON_DECODE_ERROR");
        assert.match(r.diagnosticos[0].mensaje, /detalleServicio: se esperaba una lista/);
    });

    test("acepta montos numéricos", () => {
        const req = caso("fe_servicios");
        req.detalleServicio![0].precioUnitario = 89270;
        assert.equal(fe.procesar(req).comprobante?.ResumenFactura.TotalComprobante, "100875.10000");
    });

    test("textos con caracteres fuera de ASCII", () => {
        const req = caso("fe_servicios");
        req.detalleServicio![0].detalle = "Diseño & análisis — señalización";
        const r = fe.procesar(req);
        assert.equal(r.exito, true, JSON.stringify(r.diagnosticos));
        assert.match(r.xml!, /<Detalle>Diseño &amp; análisis — señalización<\/Detalle>/);
    });
});

describe("calcular", () => {
    test("da montos aunque el comprobante esté incompleto", () => {
        const req = caso("fe_servicios");
        req.emisor.nombre = "";
        const r = fe.calcular(req);
        assert.equal(r.exito, false);
        assert.ok(r.diagnosticos.some((d) => d.codigo === "RULE_EMISOR_NOMBRE_LENGTH"));
        assert.equal(r.comprobante?.ResumenFactura.TotalComprobante, "100875.10000");
        assert.equal("xml" in r, false);
    });
});

describe("catálogos", () => {
    test("por nombre", () => {
        assert.ok(fe.catalogos.nombres().includes("TiposDocumentoReferencia"));
        const codigos = fe.catalogos.porNombre("CodigosReferencia").map((e) => e.codigo);
        assert.ok(codigos.includes("17"));
        assert.ok(!codigos.includes("03"));
        assert.throws(() => fe.catalogos.porNombre("NoExiste" as any), /Catálogo desconocido/);
    });

    test("monedas, unidades y formas farmacéuticas", () => {
        assert.ok(fe.catalogos.monedas().some((m) => m.codigo === "CRC"));
        assert.ok(fe.catalogos.unidadesMedida().some((u) => u.codigo === "Sp"));
        assert.ok(fe.catalogos.formasFarmaceuticas().length > 0);
    });

    test("división territorial con los códigos del request", () => {
        const provincias = fe.territorio.provincias();
        assert.equal(provincias.length, 7);
        assert.deepEqual(provincias[0], { codigo: "1", nombre: provincias[0].nombre });
        const cantones = fe.territorio.cantones("1");
        assert.equal(cantones[0].codigo, "01");
        assert.ok(fe.territorio.distritos("1", "08").length > 0);
        assert.equal(fe.territorio.esUbicacionValida("1", "08", "01"), true);
        assert.equal(fe.territorio.esUbicacionValida("1", "99", "01"), false);
        assert.deepEqual(fe.territorio.cantones("x"), []);
    });
});

describe("clave y consecutivo", () => {
    test("reproduce la clave de un caso válido", () => {
        const req = caso("nc");
        const consecutivo = generarConsecutivo({ sucursal: 1, terminal: 1, tipoDocumento: "03", numero: 1 });
        assert.equal(consecutivo, req.numeroConsecutivo);
        const clave = generarClave({
            fechaEmision: req.fechaEmision,
            identificacion: req.emisor.identificacion.numero,
            numeroConsecutivo: consecutivo,
            codigoSeguridad: "12345678",
        });
        assert.equal(clave, req.clave);
    });

    test("valida los rangos", () => {
        assert.throws(() => generarConsecutivo({ sucursal: 0, terminal: 1, tipoDocumento: "01", numero: 1 }), RangeError);
        assert.throws(() => generarClave({ fechaEmision: "01/10/2026", identificacion: "1", numeroConsecutivo: "0".repeat(20), codigoSeguridad: 1 }), RangeError);
    });

    test("fecha de emisión en hora de Costa Rica", () => {
        assert.equal(fechaEmision(new Date("2026-10-15T03:30:00Z")), "2026-10-14T21:30:00-06:00");
    });
});

/** Certificado autofirmado de prueba generado con el CLI de openssl; null si no está instalado. */
function generarP12(pin: string): Uint8Array | null {
    return generarCertificado(pin)?.p12 ?? null;
}

/** El .p12 y su llave privada en PEM, para simular un keystore externo que firma SignedInfo. */
function generarCertificado(pin: string): { p12: Uint8Array; llave: string } | null {
    const dir = mkdtempSync(join(tmpdir(), "fe-ts-"));
    try {
        const k = join(dir, "k.pem"), c = join(dir, "c.pem"), p = join(dir, "c.p12");
        execFileSync("openssl", ["req", "-x509", "-newkey", "rsa:2048", "-nodes", "-keyout", k, "-out", c,
            "-days", "30", "-subj", "/CN=TITULAR DE PRUEBA/C=CR"], { stdio: "ignore" });
        execFileSync("openssl", ["pkcs12", "-export", "-inkey", k, "-in", c, "-out", p, "-passout", `pass:${pin}`,
            "-keypbe", "AES-256-CBC", "-certpbe", "AES-256-CBC", "-macalg", "sha256"], { stdio: "ignore" });
        return { p12: new Uint8Array(readFileSync(p)), llave: readFileSync(k, "utf8") };
    } catch {
        return null;
    } finally {
        rmSync(dir, { recursive: true, force: true });
    }
}

describe("firma y certificado", () => {
    const pin = "pin-de-prueba";
    const p12 = generarP12(pin);

    test("firma el XML de un comprobante", { skip: !p12 && "openssl no disponible" }, () => {
        const xml = fe.procesar(caso("fe_servicios")).xml!;
        const r = fe.firmar(xml, p12!, pin);
        assert.equal(r.exito, true, JSON.stringify(r.diagnosticos));
        assert.match(r.xml!, /<ds:Signature xmlns:ds="http:\/\/www.w3.org\/2000\/09\/xmldsig#"/);
        assert.match(r.certificado!.sujeto, /TITULAR DE PRUEBA/);
    });

    test("lee el certificado y detecta el PIN incorrecto", { skip: !p12 && "openssl no disponible" }, () => {
        const ok = fe.leerCertificado(p12!, pin);
        assert.equal(ok.exito, true, JSON.stringify(ok.diagnosticos));
        assert.equal(ok.certificado!.vigente, true);
        assert.equal(ok.certificado!.sha256Hex.length, 64);

        const malo = fe.leerCertificado(p12!, "otro");
        assert.equal(malo.exito, false);
        assert.equal(malo.certificado, undefined);
        assert.equal(malo.diagnosticos[0].codigo, "CRYPTO_P12_AUTH_FAILED");
    });
});

describe("firma en dos pasos", () => {
    const pin = "pin-dos-pasos";
    const cert = generarCertificado(pin);
    const omitir = { skip: !cert && "openssl no disponible" };

    test("prepara con el certificado público y completa con una firma externa", omitir, () => {
        const der = Buffer.from(fe.leerCertificado(cert!.p12, pin).certificado!.certificadoBase64, "base64");
        const xml = fe.procesar(caso("fe_servicios")).xml!;

        const prep = fe.prepararFirma(xml, der);
        assert.equal(prep.exito, true, JSON.stringify(prep.diagnosticos));
        assert.match(prep.xml!, /<ds:SignatureValue\/>/);
        assert.match(prep.signedInfo!, /^<ds:SignedInfo /);

        // Lo que haría el keystore del servidor (SHA256withRSA sobre los bytes UTF-8 de SignedInfo)
        const firma = sign("sha256", Buffer.from(prep.signedInfo!, "utf8"), createPrivateKey(cert!.llave)).toString("base64");
        const r = fe.completarFirma(prep.xml!, firma);
        assert.equal(r.exito, true, JSON.stringify(r.diagnosticos));
        assert.ok(r.xml!.includes(`<ds:SignatureValue>${firma}</ds:SignatureValue>`));
    });

    test("rechaza una firma que no corresponde al certificado", omitir, () => {
        const der = Buffer.from(fe.leerCertificado(cert!.p12, pin).certificado!.certificadoBase64, "base64");
        const prep = fe.prepararFirma(fe.procesar(caso("fe_servicios")).xml!, der);
        const otra = generarCertificado(pin)!;
        const firma = sign("sha256", Buffer.from(prep.signedInfo!, "utf8"), createPrivateKey(otra.llave)).toString("base64");
        const r = fe.completarFirma(prep.xml!, firma);
        assert.equal(r.exito, false);
        assert.equal(r.diagnosticos[0].codigo, "SIGN_VERIFY_FAILED");
    });
});

describe("mensaje receptor", () => {
    const pin = "pin-mensaje";
    const cert = generarCertificado(pin);
    const omitir = { skip: !cert && "openssl no disponible" };

    test("lee la factura firmada del proveedor, arma el mensaje y lo firma en dos pasos", omitir, () => {
        const factura = fe.firmar(fe.procesar(caso("fe_servicios")).xml!, cert!.p12, pin).xml!;
        const leida = fe.leerComprobante(factura);
        assert.equal(leida.exito, true, JSON.stringify(leida.diagnosticos));
        const c = leida.comprobante!;
        assert.equal(c.tipoDocumento, "01");
        assert.equal(c.firmado, true);
        assert.match(c.plazoConfirmacion, /^\d{4}-\d{2}-\d{2}$/);

        const impuesto = c.totalImpuesto;
        const r = fe.mensajeReceptor({
            clave: c.clave,
            numeroCedulaEmisor: c.emisor.numeroIdentificacion,
            fechaEmisionDoc: c.fechaEmision,
            mensaje: "1",
            montoTotalImpuesto: impuesto,
            codigoActividad: c.codigoActividadReceptor || "471100",
            condicionImpuesto: Number(impuesto) > 0 ? "01" : undefined,
            montoTotalImpuestoAcreditar: Number(impuesto) > 0 ? impuesto : undefined,
            totalFactura: c.totalComprobante,
            numeroCedulaReceptor: c.receptor.numeroIdentificacion,
            numeroConsecutivoReceptor: generarConsecutivo({ sucursal: 1, terminal: 1, tipoDocumento: "05", numero: 1 }),
            fechaEmisionComprobante: c.fechaEmision,
        });
        assert.equal(r.exito, true, JSON.stringify(r.diagnosticos));
        assert.equal(r.plazoConfirmacion, c.plazoConfirmacion);

        const der = Buffer.from(fe.leerCertificado(cert!.p12, pin).certificado!.certificadoBase64, "base64");
        const prep = fe.prepararFirma(r.xml!, der);
        assert.equal(prep.exito, true, JSON.stringify(prep.diagnosticos));
        const firma = sign("sha256", Buffer.from(prep.signedInfo!, "utf8"), createPrivateKey(cert!.llave)).toString("base64");
        const firmado = fe.completarFirma(prep.xml!, firma);
        assert.equal(firmado.exito, true, JSON.stringify(firmado.diagnosticos));
        assert.match(firmado.xml!, /^<\?xml[^>]*>\n<MensajeReceptor xmlns="https:\/\/cdn.comprobanteselectronicos.go.cr\/xml-schemas\/v4.4\/mensajeReceptor"/);
        assert.match(firmado.xml!, /<\/ds:Signature><\/MensajeReceptor>$/);
    });

    test("no confirma un MensajeHacienda ni un rechazo con condición de impuesto", () => {
        assert.equal(fe.leerComprobante("<MensajeHacienda/>").diagnosticos[0].codigo, "MR_NO_ES_COMPROBANTE");
        const r = fe.mensajeReceptor({
            clave: "50601102600310112345600100001010000000001112345678",
            numeroCedulaEmisor: "3101123456",
            fechaEmisionDoc: "2026-10-05T09:00:00-06:00",
            mensaje: "3",
            condicionImpuesto: "01",
            totalFactura: "113000",
            numeroCedulaReceptor: "3101987654",
            numeroConsecutivoReceptor: "00100001070000000001",
        });
        assert.equal(r.exito, false);
        assert.ok(r.diagnosticos.some((d) => d.codigo === "RULE_MR_RECHAZO_SIN_CONDICION"));
    });
});
