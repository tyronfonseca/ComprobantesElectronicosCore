#pragma once
#include <string>
#include <string_view>
#include "cec/service/ComprobanteService.h"

namespace cec::codec {

class JsonCodec {
public:
    static parsing::ComprobanteRequestDTO decodeRequest(std::string_view jsonStr);
    static std::string encodeResult(const service::ComprobanteResult& result);
    static std::string encodeSignResult(const service::FirmaResult& result);
};

} // namespace cec::codec

