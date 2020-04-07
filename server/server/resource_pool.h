#pragma once

#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <QtCore/qstring.h>

#include <cassert>


#include "global.h"
#include "job_queue.h"
#include "db_access.h"
#include "log.h"

#include "../include/cpp-base64/base64.h"

#define RAPID_JSON_HAS_STDSTRING 1
#include "../include/rapidjson/document.h"
#include "../include/rapidjson/writer.h"
#include "../include/rapidjson/stringbuffer.h"

namespace MemoServer
{
template <typename ResourceType>
class ResourcePool : public AbstractComponent<JobQueue<RegPointer>>
{
public:
	ResourcePool(std::size_t pool_size = 4)
		: _resource_handles(pool_size), _max_resource(pool_size), _resource_threads(pool_size),
		_current(0), _prepared(0)
	{
		WRITE_LOG(LogLevel::DEBUG,
				  __Str("Initialize resource pool."));

		std::default_random_engine dre(std::chrono::system_clock::now().time_since_epoch().count());
		std::uniform_int_distribution<> uid('!', '~');
		std::uniform_int_distribution<> uid_len(10, 15);

		std::string append_str;
		int len = uid_len(dre);
		for (int i = 0; i < len; i++)
			append_str.push_back(uid(dre));

		std::string db_name_base(append_str.append("mysql_db_data_"));
		for (std::size_t i = 0; i < pool_size; i++)
		{
			_resource_threads[i] = std::move(std::thread(&ResourcePool<ResourceType>::RunOneResource, this, i, db_name_base + std::to_string(i)));
		}

		WRITE_LOG(LogLevel::DEBUG,
				  __Str("Initialize resource pool of MemoManager done, resource number: ")
				  .append(NumStr(pool_size)));
	};

	void Start()
	{
		_prepared.Wait();

		while (true)
		{
			RegPointer job = _jobs.Pop();
			_resource_handles[_current]->Push(job);
			if (++_current == _max_resource) // Round-Robin
				_current = 0;
		}
	}

	JobQueue<RegPointer> *GetHandle() override
	{
		return &_jobs;
	}

private:
	std::vector<JobQueue<RegPointer> *> _resource_handles;
	std::vector<std::thread> _resource_threads;
	JobQueue<RegPointer> _jobs;
	std::size_t _current;
	std::size_t _max_resource;
	Semaphore _prepared;

	void RunOneResource(std::size_t index, const std::string db_name)
	{
		ResourceType resource(QString::fromStdString(db_name));
		_resource_handles[index] = resource.GetHandle();
		if (index == _max_resource - 1)
			_prepared.Signal();
		resource.Start(); // block call		
	}
};
} // namesapce MemoServer