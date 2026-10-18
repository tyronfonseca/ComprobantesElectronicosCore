# Política de seguridad

## Versiones con soporte

El proyecto es pre-1.0 y solo se corrige la versión más reciente de la rama principal.

## Cómo reportar una vulnerabilidad

No abra un issue público para un problema de seguridad. Use el reporte privado de GitHub: en la pestaña **Security** del repositorio, **Report a vulnerability**. Incluya:

- qué componente afecta (firma, lectura de certificados, lectura de XML o JSON, API de C, binding, ejemplo web, script de envío);
- los pasos para reproducirlo, con datos ficticios;
- el impacto que usted observa y la versión o el commit.

**No adjunte certificados reales, PIN, contraseñas ni comprobantes de personas o empresas reales.** Si el reporte necesita un certificado, genere uno de prueba.

Se confirmará la recepción y se informará del avance; la corrección se publica con un aviso en el [CHANGELOG](CHANGELOG.md).

## Qué se considera de seguridad

- Corrupción de memoria o fallos al leer XML, JSON o contenedores PKCS#12 no confiables (por ejemplo, un `.p12` o un comprobante recibido manipulado).
- Firmas que se aceptan o se generan de forma incorrecta.
- Exposición del certificado, del PIN o de las credenciales del API en el código, los registros o el ejemplo web.
- Condiciones de carrera al usar la biblioteca desde varios hilos.

No son vulnerabilidades de este proyecto: el comportamiento de los servicios de Hacienda, ni que un comprobante válido para la biblioteca sea rechazado por Hacienda (ver [docs/LIMITACIONES.md](docs/LIMITACIONES.md)).

## Recomendaciones para quien usa la biblioteca

- Mantenga los certificados `.p12` y los PIN fuera del repositorio y de los registros. Si la llave privada está en un keystore o un HSM, use la firma en dos pasos (`prepararFirma` y `completarFirma`) para que no entre al proceso.
- Guarde las credenciales del API en variables de entorno o en un archivo `.env` fuera del control de versiones (el repositorio ya lo ignora).
- Pruebe siempre en el ambiente `sandbox` antes de usar producción.
- Trate como no confiable el XML de un comprobante recibido: `leerComprobante` solo extrae datos y comprueba que tenga un nodo de firma; no verifica la firma.
