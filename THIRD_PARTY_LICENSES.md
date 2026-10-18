# Third-Party Software Notices and Licenses

This project (`ComprobantesElectronicosCore`) incorporates, vendors, or references the following third-party open-source libraries.

---

## Table of Third-Party Dependencies

| Library | Type | License | Author / Copyright | Website / Repository |
|---|:---:|:---:|---|---|
| **pugixml** | Vendored (`libs/`) | MIT | © 2006–2026 Arseny Kapoulkine | [pugixml.org](https://pugixml.org/) / [github.com/zeux/pugixml](https://github.com/zeux/pugixml) |
| **GoogleTest** | Git Reference (Testing) | BSD 3-Clause | © 2008 Google Inc. | [github.com/google/googletest](https://github.com/google/googletest) |
| **nlohmann/json** | Vendored (`libs/json.hpp`) | MIT | © 2013–2023 Niels Lohmann | [github.com/nlohmann/json](https://github.com/nlohmann/json) |
| **OpenSSL** | System library (linked; statically linked in the WebAssembly build) | Apache 2.0 | © 1998–2025 The OpenSSL Project Authors | [openssl.org](https://www.openssl.org/) / [github.com/openssl/openssl](https://github.com/openssl/openssl) |

---

## Public documents of the Costa Rican Ministry of Finance (Hacienda)

The files in `docs/hacienda_schemas` (XSD schemas, the territorial coding document and the pharmaceutical-form catalog) are public information published by the Dirección General de Tributación on its official portal. They are included unmodified by this project only as a technical reference. Their authorship and rights belong to the Ministerio de Hacienda and they are **not covered by this project's MIT license**. The summaries in `docs/reglas_condiciones_campos` and the catalogs embedded in the source code are transcriptions or derivatives of the official document "Anexos y Estructuras para la Emisión de Comprobantes Electrónicos". The official portal is always the authoritative source.

---

## Full License Texts

### 1. pugixml (MIT License)
MIT License

Copyright (c) 2006-2026 Arseny Kapoulkine

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

---

### 2. GoogleTest (BSD 3-Clause License)
```text
Code of conduct
Contributing
BSD-3-Clause license
Security
Copyright 2008, Google Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.
    * Neither the name of Google Inc. nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

---

### 3. nlohmann/json (MIT License)
MIT License

Copyright (c) 2013-2023 Niels Lohmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

---

### 4. OpenSSL (Apache License 2.0)
Copyright (c) 1998-2025 The OpenSSL Project Authors.

OpenSSL 3.x is licensed under the Apache License, Version 2.0. You may not use this library except in compliance with the License. A copy of the License is available at https://www.apache.org/licenses/LICENSE-2.0 and in the `LICENSE.txt` file of the OpenSSL distribution.

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
