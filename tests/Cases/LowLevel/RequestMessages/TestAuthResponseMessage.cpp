#include <LowLevel/RequestMessages/AuthResponseMessage.hpp>
#include <LowLevel/RequestMessages/RequestMessageFactory.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST(TestAuthResponseMessage, encode) {
	cql::ConnectionInfo info;
	for (std::size_t i = 0; i < 3; ++i) {
		auto message = cql::RequestMessageFactory::makeRequestMessage<cql::AuthResponseMessage>();
		message->getToken().set("abc", 3);
		std::string data;
		message->getHeader().encodeHeaderPre(info, data);
		message->encodeBody(info, data);
		message->getHeader().encodeHeaderPost(info, data);
		ASSERT_EQ(data, makeTestString("\x04\x00\x00\x00\x0f\x00\x00\x00\x07""\x00\x00\x00\x03""abc"));
	}
}

