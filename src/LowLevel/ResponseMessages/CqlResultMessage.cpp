#include "CqlResultMessage.hpp"

namespace cql {
	/** Decode message body from binary data */
	void CqlResultMessage::decodeBody(const CqlConnectionInfo&, const seastar::temporary_buffer<char>&) {
		// TODO
	}
}
