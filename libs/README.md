# Dependencias de terceros

Bibliotecas de terceros que usa el proyecto. Las de este directorio están incluidas en el código fuente (*vendored*) o como submódulo de Git; OpenSSL se toma del sistema.

| Biblioteca | Ubicación | Versión | Licencia | Uso |
|---|---|---|---|---|
| [pugixml](https://pugixml.org/) | `pugixml.hpp`, `pugixml.cpp`, `pugiconfig.hpp` | 1.16 | MIT | Lectura del XML para la canonicalización y la firma |
| [nlohmann/json](https://github.com/nlohmann/json) | `json.hpp` | 3.11.3 | MIT | Lectura del request JSON y escritura del resultado JSON |
| [GoogleTest](https://github.com/google/googletest) | `googletest/` (submódulo) | 1.18.0 | BSD 3-Clause | Tests de C++ |
| [OpenSSL](https://www.openssl.org/) | Sistema (Homebrew `openssl@3`, `libssl-dev`); para WebAssembly se compila en `deps/` | 3.x | Apache 2.0 | PKCS#12, SHA-256 y firma RSA |

Para inicializar el submódulo de GoogleTest:

```bash
git submodule update --init
```

Los textos completos de las licencias están en [`THIRD_PARTY_LICENSES.md`](../THIRD_PARTY_LICENSES.md).
