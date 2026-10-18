// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "ComprobantesElectronicosCore",
    platforms: [
        .macOS(.v13)
    ],
    products: [
        .library(
            name: "ComprobantesElectronicosCore",
            targets: ["ComprobantesElectronicosCore"]
        )
    ],
    targets: [
        // OpenSSL del sistema (firma XAdES): Homebrew en macOS, libssl-dev en Linux.
        .systemLibrary(
            name: "COpenSSL",
            path: "bindings/swift/Sources/COpenSSL",
            pkgConfig: "openssl",
            providers: [.brew(["openssl@3"]), .apt(["libssl-dev"])]
        ),
        // Core en C++ con la API de C (include/cec/c_api, módulo ComprobantesElectronicosCoreC).
        .target(
            name: "ComprobantesElectronicosCoreC",
            dependencies: ["COpenSSL"],
            path: ".",
            sources: [
                "src/c_api/cec_c_api.cpp",
                "src/codec/JsonCodec.cpp",
                "src/crypto/Pkcs12Signer.cpp",
                "src/crypto/XmlCanonicalizer.cpp",
                "src/crypto/XadesSigner.cpp",
                "src/common/DivisionTerritorial.cpp",
                "src/common/Catalogos.cpp",
                "src/common/Monedas.cpp",
                "src/common/UnidadesMedida.cpp",
                "src/common/FormasFarmaceuticas.cpp",
                "libs/pugixml.cpp"
            ],
            publicHeadersPath: "include/cec/c_api",
            cxxSettings: [
                .headerSearchPath("include"),
                .headerSearchPath("libs")
            ]
        ),
        // Binding de Swift
        .target(
            name: "ComprobantesElectronicosCore",
            dependencies: ["ComprobantesElectronicosCoreC"],
            path: "bindings/swift/Sources/ComprobantesElectronicosCore"
        ),
        // Ejemplo: swift run FacturaSwiftExample
        .executableTarget(
            name: "FacturaSwiftExample",
            dependencies: ["ComprobantesElectronicosCore"],
            path: "examples/swift"
        ),
        .testTarget(
            name: "ComprobantesElectronicosCoreTests",
            dependencies: ["ComprobantesElectronicosCore"],
            path: "bindings/swift/Tests/ComprobantesElectronicosCoreTests"
        )
    ],
    cxxLanguageStandard: .cxx20
)
