#pragma once
#include <cstdint>
#include <core/sstring.hh>
#include <core/byteorder.hh>
#include "../Exceptions/CqlDecodeException.hpp"
#include "CqlIntegerOperations.hpp"

namespace cql {
	/** Base class of integer types */
	template <class IntType>
	class CqlIntegerBase {
	public:
		using CqlUnderlyingType = IntType;
		static const std::size_t CqlEncodeSize = sizeof(IntType);

		/** Get the integer value */
		IntType get() const& { return value_; }

		/** Get the mutable integer value */
		IntType& get() & { return value_; }

		/** Set the integer value */
		void set(IntType value) { value_ = value; }

		/** Reset to initial state */
		void reset() { value_ = 0; }

		/** Encode to binary data */
		void encodeBody(seastar::sstring& data) const {
			auto value = seastar::cpu_to_be(value_);
			data.append(reinterpret_cast<const char*>(&value), sizeof(value));
		}

		/** Decode from binary data */
		void decodeBody(const char* ptr, std::int32_t size) {
			if (size == 0) {
				value_ = 0; // empty
			} else if (size != sizeof(value_)) {
				throw CqlDecodeException(CQL_CODEINFO,
					"integer length not matched, expected to be", sizeof(value_),
					"but actual is", size);
			} else {
				std::memcpy(&value_, ptr, sizeof(value_));
				value_ = seastar::be_to_cpu(value_);
			}
		}

		/** Constructors */
		CqlIntegerBase() : value_(0) { }
		explicit CqlIntegerBase(IntType value) : value_(value) { }

		/** Allow assign from integer */
		void operator=(IntType value) {
			value_ = value;
		}

		/** Allow cast to integer implicitly */
		operator IntType() const {
			return value_;
		}

		/** Allow convert to boolean */
		operator bool() const {
			return static_cast<bool>(value_);
		}

	protected:
		IntType value_;
	};
}

