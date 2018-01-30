#include <CqlDriver/Common/ColumnTypes/CqlAscii.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST(TestCqlAscii, getset) {
	cql::CqlAscii value("abc");
	ASSERT_EQ(value.get(), "abc");
	value.get().append("aaaaa", 5);
	ASSERT_EQ(value.get(), "abcaaaaa");

	value.set("abc\x00\x01");
	ASSERT_EQ(value.get(), makeTestString("abc\x00\x01"));

	value.set("abcde", 3);
	ASSERT_EQ(value.get(), "abc");

	value = cql::CqlAscii();
	ASSERT_EQ(value.get(), "");

	value = cql::CqlAscii("qwert");
	ASSERT_EQ(value.get(), "qwert");
	ASSERT_EQ(value.get(), seastar::sstring(value.data(), value.size()));
	ASSERT_FALSE(value.empty());
	value.reset();
	ASSERT_EQ(value.get(), "");
	ASSERT_TRUE(value.empty());
}

TEST(TestCqlAscii, encode) {
	cql::CqlAscii value("abc\x00\x01");
	seastar::sstring data;
	value.encode(data);
	ASSERT_EQ(data, makeTestString("abc\x00\x01"));
}

TEST(TestCqlAscii, decode) {
	auto data = makeTestString("abc\x00\x01");
	cql::CqlAscii value;
	value.decode(data.data(), data.size());
	ASSERT_TRUE(value == makeTestString("abc\x00\x01"));
}

TEST(TestCqlAscii, operations) {
	cql::CqlAscii value;
	value = seastar::sstring("aaa");

	seastar::sstring str = value;
	ASSERT_EQ(str, "aaa");

	value = "abc\x00\x01";
	str = value;
	ASSERT_EQ(str, makeTestString("abc\x00\x01"));

	value = "abc";
	ASSERT_TRUE(value == seastar::sstring("abc"));
	ASSERT_FALSE(value == seastar::sstring("cba"));
	ASSERT_TRUE(value != seastar::sstring("cba"));
	ASSERT_FALSE(value != seastar::sstring("abc"));
	ASSERT_TRUE(value == "abc");
	ASSERT_FALSE(value == "cba");
	ASSERT_TRUE(value != "cba");
	ASSERT_FALSE(value != "abc");
}
