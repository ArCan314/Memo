#include <string>
#include <iostream>

#include "cpp-base64/base64.h"
#include "PicoSHA2/picosha2.h"

int main()
{
	std::string test = "一二三四五六七";
	std::string sha_res;
	picosha2::hash256_hex_string(test, sha_res);
	std::string test_b64(base64_encode(reinterpret_cast<const unsigned char *>(test.c_str()), test.size()));
	std::string test_decode(base64_decode(test_b64));

	std::cout << "test: " << test << std::endl
		<< "sha_res: " << sha_res << std::endl
		<< "test_b64: " << test_b64 << std::endl
		<< "test_decode: " << test_decode << std::endl;

	return 0;
}