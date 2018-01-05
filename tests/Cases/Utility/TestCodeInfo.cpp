#include <CqlDriver/Utility/CodeInfo.hpp>
#include <TestUtility/GTestUtils.hpp>

TEST(TestCodeInfo, construct) {
	auto codeInfo = CQL_CODEINFO;
	auto codeInfoStr = codeInfo.str();
	ASSERT_TRUE(codeInfoStr.find("TestCodeInfo.cpp") != std::string::npos);
	ASSERT_TRUE(codeInfoStr.find("construct") != std::string::npos);
	ASSERT_TRUE(codeInfoStr.find("5") != std::string::npos);
}

