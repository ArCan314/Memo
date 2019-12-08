#include "CppUnitTest.h"
#include "../static_lib/base64.h"

#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace servertest
{
	TEST_CLASS(servertest)
	{
	public:
		
		TEST_METHOD(b64_encode_test)
		{
			std::string b64_test = "hello-world.";
			std::string b64_test_cn = "阿斯顿法国红酒看";
			std::string b64_test_jp = "あいうえおかきくけこさしすせそ";
			std::string b64_test_str(base64_encode(reinterpret_cast<const unsigned char *>(b64_test.c_str()), b64_test.size()));
			std::string b64_test_str_cn(base64_encode(reinterpret_cast<const unsigned char*>(b64_test_cn.c_str()), b64_test_cn.size()));
			std::string b64_test_str_jp(base64_encode(reinterpret_cast<const unsigned char*>(b64_test_jp.c_str()), b64_test_jp.size()));
			Assert::AreEqual(b64_test_str, std::string("aGVsbG8td29ybGQu"));
			Assert::AreEqual(b64_test_str_cn, std::string("6Zi/5pav6aG/5rOV5Zu957qi6YWS55yL"));
			Assert::AreEqual(b64_test_str_jp, std::string("44GC44GE44GG44GI44GK44GL44GN44GP44GR44GT44GV44GX44GZ44Gb44Gd"));
		}


	};
}
