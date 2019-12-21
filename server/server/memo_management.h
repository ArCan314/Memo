#pragma once

#include <vector>
#include <string>
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

struct MemoRec
{
	int id{ 0 };
	std::string date{ "" };
	std::string text{ "" };
};

struct Memo
{
	int id{ 0 };
	std::string title{ "" };
	std::vector<MemoRec> recs;
};

struct MemoData
{
	std::string id{ "1" };
	std::vector<Memo> memos;
	std::string GetString(const std::string &event) const;
	bool GenData(const rapidjson::Document &dom);
};

class MemoManager
{
public:
	MemoManager(const QString &db_name) : _db(db_name), _dom()
	{
	}

	void Start()
	{
		bool res;
		res = _db.OpenConnection();
		if (!res)
		{
			WRITE_LOG(LogLevel::ERROR,
					  __Str("Failed to connect to the database."));

			return; // add error handling
		}

		while (true)
		{
			RegPointer job = _jobs.Pop();
			bool res;
			if (Parse(job->second))
			{
				switch (_type)
				{
				case RecvEventType::SYNC_CLIENT:
					res = SyncClient();
					break;
				case RecvEventType::SYNC_SERVER:
					res = SyncServer();
					break;
				default:
					WRITE_LOG(LogLevel::ERROR,
							  __Str("The switch statement shouldn't enter this entry."));

					assert(1 == 0);
					break;
				}
			}
			else
			{
				WRITE_LOG(LogLevel::WARN,
						  __Str("Failed to parse: ")
						  .append(job->second));

				_res_str = "{\"EventGroup\":\"Data\",\"Event\":\"SyncReply\",\"SyncResult\": false}";
			}

			_res_str = base64_encode(reinterpret_cast<const unsigned char *>(_res_str.c_str()), _res_str.size());
			
			job->second.swap(_res_str);
			job->first.Signal();
			_res_str.clear();
		}
	}

	JobQueue<RegPointer> *GetHandle()
	{
		return &_jobs;
	}

private:
	JobQueue<RegPointer> _jobs; // jobs with JSON
	DBAccess _db;
	std::string _id;
	rapidjson::Document _dom;
	rapidjson::Writer<rapidjson::StringBuffer> _writer;
	RecvEventType _type;
	std::string _res_str;

	bool Parse(const std::string &str);

	bool SyncClient();
	bool SyncServer();
};

class MemoManagerPool
{
public:
	MemoManagerPool(std::size_t pool_size = 4)
		: _manager_handles(pool_size), _max_manager(pool_size), _manager_threads(pool_size),
		_current(0), _prepared(0)
	{
		WRITE_LOG(LogLevel::DEBUG,
				  __Str("Initialize resource pool of MemoManager."));

		std::string db_name_base("mysql_db_data_");
		for (std::size_t i = 0; i < pool_size; i++)
		{
			_manager_threads[i] = std::move(std::thread(&MemoManagerPool::RunOneManager, this, i, db_name_base + std::to_string(i)));
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
			_manager_handles[_current]->Push(job);
			if (++_current == _max_manager) // Round-Robin
				_current = 0;
		}
	}

	JobQueue<RegPointer> *GetHandle()
	{
		return &_jobs;
	}

private:
	std::vector<JobQueue<RegPointer> *> _manager_handles;
	std::vector<std::thread> _manager_threads;
	JobQueue<RegPointer> _jobs;
	std::size_t _current;
	std::size_t _max_manager;
	Semaphore _prepared;

	void RunOneManager(std::size_t index, const std::string db_name)
	{
		MemoManager manager(QString::fromStdString(db_name));
		_manager_handles[index] = manager.GetHandle();
		if (index == _max_manager - 1)
			_prepared.Signal();
		manager.Start(); // block call		
	}
};
};