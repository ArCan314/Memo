#include <boost/asio.hpp>
#include <boost/locale.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <locale>
#include <codecvt>
#include <cstring>
#include <cstdlib>

#include "../include/cpp-base64/base64.h"
using namespace boost::asio;

static const char *kHost = "localhost";
static const char *kPort = "12345";

std::string ToUTF8(const std::string &gb2312)
{
	return boost::locale::conv::to_utf<char>(gb2312.c_str(), std::string("gb2312"));
}

int main()
{
	std::system("chcp 936");


	io_context ioc;
	try
	{
		ip::tcp::resolver resolver(ioc);
		auto ep = resolver.resolve(ip::tcp::v4(), kHost, kPort);

		ip::tcp::socket socket(ioc);

		connect(socket, ep);

		while (true)
		{
			std::string request;

			while (!request.size())
			{
				std::getline(std::cin, request);
			}

			std::cout << "raw: " << request << std::endl;
			// request = ToUTF8(request);
			std::cout << "UTF8" << request << std::endl;
			request = base64_encode(reinterpret_cast<const unsigned char *>(request.c_str()), request.size());
			std::cout << "b64: " << request << std::endl;
			write(socket, buffer(request, request.size()));

			std::vector<unsigned char> buf(10240);
			std::string reply;
			auto len = socket.read_some(buffer(buf));
			for (int i = 0; i < len; i++)
				reply.push_back(buf[i]);

			std::cout << "b64_reply: " << reply << std::endl;
			std::cout << "reply: " << base64_decode(reply) << std::endl;
		}


	}
	catch (const std::exception & e)
	{
		std::cerr << e.what() << std::endl;
	}





}