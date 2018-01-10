#include <CqlDriver/Common/Exceptions/CqlDecodeException.hpp>
#include <LowLevel/ProtocolTypes/CqlProtocolColumnOptionList.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST(TestCqlProtocolColumnOptionList, getset) {
	cql::CqlProtocolColumnOptionList value({
		cql::CqlProtocolColumnOption(cql::CqlColumnType::Ascii),
		cql::CqlProtocolColumnOption(cql::CqlColumnType::Int)
	});
	ASSERT_EQ(value.get().size(), 2);
	ASSERT_EQ(value.get().at(0).get(), cql::CqlColumnType::Ascii);
	ASSERT_EQ(value.get().at(1).get(), cql::CqlColumnType::Int);

	value.get().emplace_back(cql::CqlProtocolColumnOption(cql::CqlColumnType::VarChar));
	ASSERT_EQ(value.get().size(), 3);

	value = cql::CqlProtocolColumnOptionList();
	ASSERT_TRUE(value.get().empty());
}

TEST(TestCqlProtocolColumnOptionList, encode) {
	cql::CqlProtocolColumnOptionList value({
		cql::CqlProtocolColumnOption(cql::CqlColumnType::Ascii),
		cql::CqlProtocolColumnOption(cql::CqlColumnType::Int)
	});
	seastar::sstring data;
	value.encode(data);
	ASSERT_EQ(data, seastar::sstring("\x00\x02\x00\x01\x00\x09", 6));
}

TEST(TestCqlProtocolColumnOptionList, decode) {
	cql::CqlProtocolColumnOptionList value;
	{
		seastar::sstring data("\x00\x02\x00\x01\x00\x09", 6);
		auto ptr = data.c_str();
		auto end = ptr + data.size();
		value.decode(ptr, end);
		ASSERT_TRUE(ptr == end);
		ASSERT_EQ(value.get().size(), 2);
		ASSERT_EQ(value.get().at(0).get(), cql::CqlColumnType::Ascii);
		ASSERT_EQ(value.get().at(1).get(), cql::CqlColumnType::Int);
	}
	{
		seastar::sstring data("\x00\x01\x00\x01", 4);
		auto ptr = data.c_str();
		auto end = ptr + data.size();
		value.decode(ptr, end);
		ASSERT_TRUE(ptr == end);
		ASSERT_EQ(value.get().size(), 1);
		ASSERT_EQ(value.get().at(0).get(), cql::CqlColumnType::Ascii);
	}
	{
		seastar::sstring data("\x00\x00", 2);
		auto ptr = data.c_str();
		auto end = ptr + data.size();
		value.decode(ptr, end);
		ASSERT_TRUE(ptr == end);
		ASSERT_TRUE(value.get().empty());
	}
}

TEST(TestCqlProtocolColumnOptionList, decodeError) {
	{
		cql::CqlProtocolColumnOptionList value;
		seastar::sstring data("\x01", 1);
		auto ptr = data.c_str();
		auto end = ptr + data.size();
		ASSERT_THROWS(cql::CqlDecodeException, value.decode(ptr, end));
	}
	{
		cql::CqlProtocolColumnOptionList value;
		seastar::sstring data("\x00\x02\x00\x01", 4);
		auto ptr = data.c_str();
		auto end = ptr + data.size();
		ASSERT_THROWS(cql::CqlDecodeException, value.decode(ptr, end));
	}
	{
		cql::CqlProtocolColumnOptionList value;
		seastar::sstring data("\x00\x01\x00\x20", 4);
		auto ptr = data.c_str();
		auto end = ptr + data.size();
		ASSERT_THROWS(cql::CqlDecodeException, value.decode(ptr, end));
	}
}

