#!/bin/bash
set -e

# ==============================================================================
# Compila Comprobantes Electrónicos Core (CEC)
# a WebAssembly (.wasm) usando Emscripten (em++).
#
# Genera los artefactos modulares en dist/wasm/cec.js e cec.wasm
# Compatible con Node.js, navegadores, Web Workers y TypeScript.
# ==============================================================================

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$PROJECT_ROOT"

OUTPUT_DIR="dist/wasm"
CACHE_DIR="$PROJECT_ROOT/build/emscripten_cache"
export EM_CACHE="$CACHE_DIR"

DEBUG_MODE=0

for arg in "$@"; do
    case "$arg" in
        --debug|-g|debug|--source-map)
            DEBUG_MODE=1
            ;;
    esac
done

if [ "$DEBUG" = "1" ] || [ "$WASM_DEBUG" = "1" ]; then
    DEBUG_MODE=1
fi

if [ "$DEBUG_MODE" -eq 1 ]; then
    echo "=== Compilando CEC a WebAssembly (Modo DEBUG con símbolos y source map) ==="
    echo "Compila con símbolos de debug: SÍ (-g -gsource-map -sASSERTIONS=1 -O0)"
    OPT_FLAGS=(-O0 -g -gsource-map -sASSERTIONS=1)
else
    echo "=== Compilando CEC a WebAssembly (Modo RELEASE) ==="
    echo "Compila con símbolos de debug: NO (-O3)"
    OPT_FLAGS=(-O3)
    rm -f "$OUTPUT_DIR/cec.wasm.map"
fi

# 1. Verificar herramienta em++
if ! command -v em++ &> /dev/null; then
    echo "Error: Emscripten (em++) no está instalado o no se encuentra en el PATH."
    echo "Instálalo vía Homebrew: brew install emscripten"
    exit 1
fi

mkdir -p "$OUTPUT_DIR"
mkdir -p "$CACHE_DIR"

OPENSSL_WASM_DIR="$PROJECT_ROOT/deps/openssl_wasm/openssl"

if [ ! -f "$OPENSSL_WASM_DIR/libcrypto.a" ]; then
    echo "Compilando OpenSSL para WebAssembly..."
    mkdir -p "$PROJECT_ROOT/deps/openssl_wasm"
    cd "$PROJECT_ROOT/deps/openssl_wasm"
    if [ ! -d "openssl" ]; then
        git clone --depth 1 --branch openssl-3.3.2 https://github.com/openssl/openssl.git openssl
    fi
    cd openssl
    CC=emcc AR=emar RANLIB=emranlib ./Configure linux-generic32 no-asm no-threads no-engine no-shared no-tests no-apps no-docs no-legacy no-dso
    make build_generated
    make -j$(sysctl -n hw.ncpu 2>/dev/null || echo 4) libcrypto.a
    cd "$PROJECT_ROOT"
fi

SOURCES=(
    src/c_api/cec_c_api.cpp
    src/codec/JsonCodec.cpp
    src/crypto/Pkcs12Signer.cpp
    src/crypto/XmlCanonicalizer.cpp
    src/crypto/XadesSigner.cpp
    src/common/DivisionTerritorial.cpp
    src/common/Catalogos.cpp
    src/common/Monedas.cpp
    src/common/UnidadesMedida.cpp
    src/common/FormasFarmaceuticas.cpp
    libs/pugixml.cpp
)

RUNTIME_METHODS='["ccall","cwrap","UTF8ToString","stringToUTF8","lengthBytesUTF8","getValue","setValue","stackSave","stackRestore","stackAlloc","HEAPU8"]'
EXPORTED_FUNCS='["_malloc","_free","_cec_abi_version","_cec_build_json","_cec_calcular_json","_cec_sign_xml","_cec_leer_certificado","_cec_preparar_firma","_cec_completar_firma","_cec_mensaje_receptor_json","_cec_leer_comprobante_xml","_cec_result_is_success","_cec_result_xml","_cec_result_data","_cec_result_free","_cec_dta_total_provincias","_cec_dta_provincia_por_indice","_cec_dta_total_cantones","_cec_dta_canton_por_indice","_cec_dta_total_distritos","_cec_dta_distrito_por_indice","_cec_dta_total_barrios","_cec_dta_barrio_por_indice","_cec_dta_validar_provincia","_cec_dta_validar_canton","_cec_dta_validar_distrito","_cec_dta_validar_barrio","_cec_dta_validar_ubicacion","_cec_dta_nombre_provincia","_cec_dta_nombre_canton","_cec_dta_nombre_distrito","_cec_dta_nombre_barrio","_cec_catalogo_total_monedas","_cec_catalogo_moneda_por_indice","_cec_catalogo_total_unidades_medida","_cec_catalogo_unidad_medida_por_indice","_cec_catalogo_total_formas_farmaceuticas","_cec_catalogo_forma_farmaceutica_por_indice","_cec_catalogo_total_nombres","_cec_catalogo_nombre_por_indice","_cec_catalogo_total_elementos","_cec_catalogo_elemento_por_indice"]'

echo "Compilando fuentes C++20 con em++..."
em++ -std=c++20 "${OPT_FLAGS[@]}" -fexceptions \
    -Iinclude -Ilibs -I"$OPENSSL_WASM_DIR/include" \
    "${SOURCES[@]}" \
    "$OPENSSL_WASM_DIR/libcrypto.a" \
    -sWASM=1 \
    -sALLOW_MEMORY_GROWTH=1 \
    -sMODULARIZE=1 \
    -sEXPORT_NAME=createCecModule \
    -sEXPORTED_RUNTIME_METHODS="$RUNTIME_METHODS" \
    -sEXPORTED_FUNCTIONS="$EXPORTED_FUNCS" \
    -sENVIRONMENT='web,webview,worker,node' \
    -o "$OUTPUT_DIR/cec.js"

echo "=== Compilación WASM completada con éxito ==="
echo "Artefactos generados en $OUTPUT_DIR:"
ls -lh "$OUTPUT_DIR"/cec.*

if [ -d "examples/web/public" ]; then
    cp "$OUTPUT_DIR/cec.js" "$OUTPUT_DIR/cec.wasm" examples/web/public/
    if [ -f "$OUTPUT_DIR/cec.wasm.map" ]; then
        cp "$OUTPUT_DIR/cec.wasm.map" examples/web/public/
    else
        rm -f examples/web/public/cec.wasm.map
    fi
    echo "Sincronizados artefactos WASM en examples/web/public/"
fi

