#pragma once
#include <cstdint>
#include <vector>
#include <core/shared_ptr.hh>

namespace cql {
	/** RAII class that hold an in use stream id for the database connection */
	class CqlConnectionStream {
	public:
		using IdType = std::uint16_t;

		/** Get the stream id */
		IdType getStreamId() const;

		/** Get whether this object hold a valid and in use stream id */
		bool isValid() const;

		/** Constructor */
		CqlConnectionStream();

		/** Constructor */
		CqlConnectionStream(
			IdType streamId,
			const seastar::lw_shared_ptr<std::vector<IdType>>& freeStreamIds);

		/** Disallow copy */
		CqlConnectionStream(const CqlConnectionStream&) = delete;
		CqlConnectionStream& operator=(const CqlConnectionStream&) = delete;

		/** Allow move */
		CqlConnectionStream(CqlConnectionStream&& stream);
		CqlConnectionStream& operator=(CqlConnectionStream&& stream);

		/** Destructor */
		~CqlConnectionStream();

	private:
		bool isValid_;
		IdType streamId_;
		seastar::lw_shared_ptr<std::vector<IdType>> freeStreamIds_;
	};
}
