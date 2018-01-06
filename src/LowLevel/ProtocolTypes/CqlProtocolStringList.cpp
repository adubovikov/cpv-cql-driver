#include "CqlProtocolStringList.hpp"
#include <CqlDriver/Exceptions/CqlEncodeException.hpp>
#include <CqlDriver/Exceptions/CqlDecodeException.hpp>
#include <core/byteorder.hh>
#include <limits>

namespace cql {
	void CqlProtocolStringList::encode(seastar::sstring& data) const {
		if (values_.size() > static_cast<std::size_t>(std::numeric_limits<LengthType>::max())) {
			throw CqlEncodeException(CQL_CODEINFO, "length too long");
		}
		LengthType size = seastar::cpu_to_be(static_cast<LengthType>(values_.size()));
		data.append(reinterpret_cast<const char*>(&size), sizeof(size));
		for (const auto& value : values_) {
			value.encode(data);
		}
	}

	void CqlProtocolStringList::decode(const char*& ptr, const char* end) {
		LengthType size = 0;
		if (ptr + sizeof(size) > end) {
			throw CqlDecodeException(CQL_CODEINFO, "length not enough");
		}
		std::memcpy(&size, ptr, sizeof(size));
		size = seastar::be_to_cpu(size);
		ptr += sizeof(size);
		values_.resize(static_cast<std::size_t>(size), CqlProtocolString(""));
		for (auto& value : values_) {
			value.decode(ptr, end);
		}
	}
}
