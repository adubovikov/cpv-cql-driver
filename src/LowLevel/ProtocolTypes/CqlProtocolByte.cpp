#include "CqlProtocolByte.hpp"
#include <CqlDriver/Exceptions/CqlInternalException.hpp>

namespace cql {
	void CqlProtocolByte::encode(seastar::sstring& data) const {
		char value = static_cast<char>(value_);
		data.append(&value, 1);
	}

	void CqlProtocolByte::decode(const char*& ptr, const char* end) {
		if (ptr >= end) {
			throw CqlInternalException(CQL_CODEINFO, "decode failed: length not enough");
		}
		value_ = static_cast<std::uint8_t>(*ptr);
		++ptr;
	}
}

