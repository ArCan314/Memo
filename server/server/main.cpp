
#include <iostream>
#include <thread>
#include <vector>
#include <utility>
#include <cassert>

#include <QtCore/qcoreapplication.h>

#include "controller.h"
#include "memo_management.h"
#include "server.h"
#include "user_management.h"
#include "db_access.h"
#include "semaphore.h"

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

void RunController(MemoServer::Controller *&ctrler_ptr, MemoServer::Semaphore &prepared, MemoServer::Semaphore &end)
{
	std::vector<std::thread> pools;

	MemoServer::Controller controller;
	MemoServer::AccountManagerPool account_pool;
	MemoServer::MemoManagerPool memo_pool;

	controller.RegHandle(MemoServer::ComponentType::ACCOUNT_MANAGER_POOL, account_pool.GetHandle());
	controller.RegHandle(MemoServer::ComponentType::DATA_MANAGER_POOL, memo_pool.GetHandle());

	pools.emplace_back(RunAccountPool, std::ref(account_pool));
	pools.emplace_back(RunMemoPool, std::ref(memo_pool));

	ctrler_ptr = &controller;
	prepared.Signal();
	end.Wait();
}

int main(int argc, char *argv[])
{
	QCoreApplication core(argc, argv);

	MemoServer::DBAccess test("123456");
	test.OpenConnection();

	if (!MemoServer::InitDataBase()) // error handling
	{
		assert(1 == 0);
	}

	boost::asio::io_context io_context;
	std::vector<std::thread> servers;
	MemoServer::Semaphore ctrler_prepared(0);
	MemoServer::Semaphore ctrler_end(0);
	MemoServer::Controller *ctrler_ptr;

	std::thread ctrler(RunController, std::ref(ctrler_ptr), std::ref(ctrler_prepared), std::ref(ctrler_end));
	ctrler_prepared.Wait();

	for (int i = 0; i < 1; i++)
		servers.push_back(std::thread(RunServer, std::ref(io_context), ctrler_ptr));

	for (int i = 0; i < 1; i++)
		servers[i].join();

	ctrler_end.Signal();
	return 0;
}