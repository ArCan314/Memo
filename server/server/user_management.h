#pragma once

#include <map>
#include <type_traits>
#include <utility>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <QtCore/qstring.h>
#include <QtSql/qsqlerror.h>
#include <QtCore/qvariant.h>

#include "global.h"
#include "job_queue.h"
#include "data_access.h"

#include "../include/cpp-base64/base64.h"

#define RAPID_JSON_HAS_STDSTRING 1
#include "../include/rapidjson/document.h"
#include "../include/rapidjson/writer.h"
#include "../include/rapidjson/stringbuffer.h"

namespace MemoServer
{
class AccountManager
{
public:
	AccountManager(const QString &db_name) : _db(db_name), _dom()
	{
	}

	void Start()
	{
		bool res;
		res = _db.OpenConnection();
		if (!res)
		{
			// log
			assert(1 == 0);
		}

		while (true)
		{
			RegPointer job = _jobs.Pop();
			bool res;
			if (Parse(job->second))
			{
				switch (_type)
				{
				case RecvEventType::CREATE_ACCOUNT:
					res = CreateAccount();
					break;
				case RecvEventType::LOG_IN:
					res = LogIn();
					break;
				case RecvEventType::LOG_OUT:
					// res = LogOut();
					break;
				default:
					assert(1 == 0);
					break;
				}
			}
			else
			{
				// log
			}

			_dom.Clear();
			_dom.SetObject();
			_dom.AddMember("EventGroup", rapidjson::Value("Account"), _dom.GetAllocator());
			_dom.AddMember("Event", rapidjson::Value("Reply"), _dom.GetAllocator());
			_dom.AddMember("Result", rapidjson::Value(res), _dom.GetAllocator());

			rapidjson::StringBuffer str_buf;
			_writer.Reset(str_buf);
			_dom.Accept(_writer);
			std::string res_str = str_buf.GetString();

			res_str = base64_encode(reinterpret_cast<const unsigned char *>(res_str.c_str()), res_str.size());

			job->second.swap(res_str);
			job->first.Signal();
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
	std::string _pswd;
	rapidjson::Document _dom;
	rapidjson::Writer<rapidjson::StringBuffer> _writer;
	RecvEventType _type;

	bool Parse(const std::string &str);

	bool CreateAccount();
	bool LogOut();
	bool LogIn();

	static std::string HashPswd(std::string &pswd, const std::string &id); // Hash(pswd) = SHA256(pswd + Salt(pswd)), changes pswd and return it
	static void AppendSalt(std::string &pswd, const std::string &id); // Salt(pswd) = pswd + base64(id), pswd is changed

};

class AccountManagerPool
{
public:
	AccountManagerPool(std::size_t pool_size = 4) 
		: _manager_handles(pool_size), _max_manager(pool_size), _manager_threads(pool_size),
		  _current(0), _prepared(0)
	{
		std::string db_name_base("mysql_db_");
		for (std::size_t i = 0; i < pool_size; i++)
		{
			_manager_threads[i] = std::move(std::thread(&AccountManagerPool::RunOneManager, this, i, db_name_base + std::to_string(i)));
		}
	};

	void Start(const RegPointer &reg_ptr)
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
		AccountManager manager(QString::fromStdString(db_name));
		_manager_handles[index] = manager.GetHandle();
		if (index == _max_manager - 1)
			_prepared.Signal();
		manager.Start(); // block call		
	}
};

};