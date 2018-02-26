#pragma once
#include <cstdint>
#include <core/sstring.hh>
#include <net/inet_address.hh>
#include "../Exceptions/CqlDecodeException.hpp"

namespace cql {
	/** A 4 byte or 16 byte sequence denoting an IPv4 or IPv6 address */
	class CqlInet {
	public:
		using CqlUnderlyingType = seastar::net::inet_address;

		/** Get the ip address */
		CqlUnderlyingType get() const& { return value_; }

		/** Get the ip address */
		CqlUnderlyingType& get() & { return value_; }

		/** Set the ip address */
		void set(const CqlUnderlyingType& value) { value_ = value; }

		/** Set the ip address by it's string representation */
		void set(const seastar::sstring& value) { value_ = CqlUnderlyingType(value); }

		/** Reset to initial state */
		void reset() { value_ = {}; }

		/** Encode to binary data */
		void encode(seastar::sstring& data) const {
			data.append(reinterpret_cast<const char*>(value_.data()), value_.size());
		}

		/** Decode from binary data */
		void decode(const char* ptr, std::int32_t size) {
			if (size == 0) {
				reset(); // empty
			} else if (size == sizeof(::in_addr)) {
				::in_addr addr_v4;
				std::memcpy(&addr_v4, ptr, size);
				value_ = CqlUnderlyingType(addr_v4);
			} else if (size == sizeof(::in6_addr)) {
				::in6_addr addr_v6;
				std::memcpy(&addr_v6, ptr, size);
				value_ = CqlUnderlyingType(addr_v6);
			} else {
				throw CqlDecodeException(CQL_CODEINFO,
					"unsupported inet address length", size);
			}
		}

		/** Constructors */
		CqlInet() : value_() { }
		template <class... Args>
		CqlInet(Args&&... args) : value_() {
			set(std::forward<Args>(args)...);
		}

		/** Allow assign from whatever function "set" would take */
		template <class... Args>
		void operator=(Args&&... args) {
			set(std::forward<Args>(args)...);
		}

		/** Allow cast to ip address implicitly */
		operator CqlUnderlyingType() const {
			return value_;
		}

	private:
		CqlUnderlyingType value_;
	};

	/** Compare operations of ip address */
	static bool operator==(const CqlInet& a, const CqlInet& b) { return a.get() == b.get(); }
	static bool operator==(const CqlInet& a, const CqlInet::CqlUnderlyingType& b) { return a.get() == b; }
	static bool operator==(const CqlInet::CqlUnderlyingType& a, const CqlInet& b) { return a == b.get(); }
	static bool operator!=(const CqlInet& a, const CqlInet& b) { return !(a.get() == b.get()); }
	static bool operator!=(const CqlInet& a, const CqlInet::CqlUnderlyingType& b) { return !(a.get() == b); }
	static bool operator!=(const CqlInet::CqlUnderlyingType& a, const CqlInet& b) { return !(a == b.get()); }

	/** Write text description of ip address */
	static std::ostream& operator<<(std::ostream& stream, const CqlInet& value) {
		return stream << value.get();
	}
}

