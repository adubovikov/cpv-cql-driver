#include <CQLDriver/Common/Exceptions/Exception.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST(TestException, construct) {
	cql::Exception exception(CQL_CODEINFO, "some error");
	std::string error = exception.what();
	// std::cout << error << std::endl;
	ASSERT_TRUE(error.find("some error") != std::string::npos);
	ASSERT_TRUE(error.find("TestException.cpp") != std::string::npos);
}

