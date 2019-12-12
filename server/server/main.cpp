
#include <iostream>
#include <thread>
#include <vector>
#include <utility>

#include "controller.h"
#include "memo_management.h"
#include "server.h"
#include "user_management.h"

MemoServer::Controller *test;

void RunServer(boost::asio::io_context &io_context, MemoServer::Controller *ctrler)
{
	try
	{
		MemoServer::Server s(io_context, 12345, ctrler);
		io_context.run();
	}
	catch (std::exception & e)
	{
		std::cerr << "Exception:: " << e.what() << std::endl;
	}

}

void RunAccountPool(MemoServer::AccountManagerPool &pool)
{
	pool.Start();
}

void RunMemoPool(MemoServer::MemoManagerPool &pool)
{
	pool.Start();
}

void RunController()
{
	std::vector<std::thread> pools;

	MemoServer::Controller controller;
	MemoServer::AccountManagerPool account_pool;
	MemoServer::MemoManagerPool memo_pool;

	controller.RegHandle(MemoServer::ComponentType::ACCOUNT_MANAGER_POOL, account_pool.GetHandle());
	controller.RegHandle(MemoServer::ComponentType::DATA_MANAGER_POOL, memo_pool.GetHandle());

	pools.emplace_back(RunAccountPool, std::ref(account_pool));
	pools.emplace_back(RunMemoPool, std::ref(memo_pool));

	test = &controller;

	for (int i = 0; i < pools.size(); i++)
	{
		pools[i].join();
	}
}

int main(void)
{
	boost::asio::io_context io_context;
	std::vector<std::thread> servers;

	std::thread ctrler(RunController);
	ctrler.join();





	for (int i = 0; i < 32; i++)
		servers.push_back(std::thread(RunServer, std::ref(io_context), test));

	for (int i = 0; i < 32; i++)
		servers[i].join();

	return 0;
}