
#include <iostream>
#include <thread>
#include <vector>
#include <utility>
#include "server.h"

void ThreadTest(boost::asio::io_context &io_context)
{
	try
	{
		MemoServer::Server s(io_context, 12345);
		io_context.run();
	}
	catch (std::exception & e)
	{
		std::cerr << "Exception:: " << e.what() << std::endl;
	}

}


int main(void)
{
	boost::asio::io_context io_context;
	std::vector<std::thread> recv;
	for (int i = 0; i < 32; i++)
		recv.push_back(std::thread(ThreadTest, std::ref(io_context)));

	for (int i = 0; i < 32; i++)
		recv[i].join();

	return 0;
}