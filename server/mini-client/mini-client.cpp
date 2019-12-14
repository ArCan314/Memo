#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <iostream>

#include "../include/cpp-base64/base64.h"
using namespace boost::asio;

static const char *kHost = "localhost";
static const char *kPort = "12345";

int main()
{

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

			// request = base64_encode(reinterpret_cast<const unsigned char *>(request.c_str()), request.size());

			write(socket, buffer(request, request.size()));

			std::vector<unsigned char> buf(10240);
			std::string reply;
			auto len = socket.read_some(buffer(buf));
			for (int i = 0; i < len; i++)
				reply.push_back(buf[i]);

			std::cout << reply << std::endl;
			std::cout << base64_decode(reply) << std::endl;
		}

	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}





}