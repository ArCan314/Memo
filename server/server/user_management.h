#pragma once

#include <map>
#include <type_traits>
#include <utility>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <string>
#include <QtCore/qstring.h>
#include <QtSql/qsqlerror.h>
#include <QtCore/qvariant.h>

#include "abstract_management.h"
#include "global.h"
#include "job_queue.h"
#include "db_access.h"

#include "../include/cpp-base64/base64.h"

#define RAPID_JSON_HAS_STDSTRING 1
#include "../include/rapidjson/document.h"
#include "../include/rapidjson/writer.h"
#include "../include/rapidjson/stringbuffer.h"

namespace MemoServer
{
class AccountManager : public AbstractManager
{
public:
	AccountManager(const QString &db_name) : AbstractManager(db_name)
	{
	}

	static bool HasAccountID(const QString &id);
	static bool HasAccountID(const std::string &id);

	void Start() override
	{
		bool res = true;
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
			bool res = false;
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
					WRITE_LOG(LogLevel::ERROR,
							  __Str("The switch statement shouldn't enter this entry."));

					// res = LogOut();
					break;
				default:
					WRITE_LOG(LogLevel::ERROR,
							  __Str("The switch statement shouldn't enter this entry."));
					
					assert(1 == 0);
					break;
				}

				// _dom.Clear();
				_dom.SetObject();
				_dom.AddMember("EventGroup", rapidjson::Value("Account"), _dom.GetAllocator());
				_dom.AddMember("Event", rapidjson::Value("Reply"), _dom.GetAllocator());
				_dom.AddMember("Result", rapidjson::Value(res), _dom.GetAllocator());

				rapidjson::StringBuffer str_buf;
				_writer.Reset(str_buf);
				_dom.Accept(_writer);
				_res_str = str_buf.GetString();
			}
			else
			{
				WRITE_LOG(LogLevel::WARN,
						  __Str("Failed to parse: ")
						  .append(job->second));

				_res_str = "{\"EventGroup\":\"Account\",\"Event\":\"Reply\",\"Result\":false}";
			}

			_res_str = base64_encode(reinterpret_cast<const unsigned char *>(_res_str.c_str()), _res_str.size());

			job->second.swap(_res_str);
			job->first.Signal();
		}
	}

private:
	std::string _id;
	std::string _pswd;
	RecvEventType _type;
	std::string _res_str;

	bool Parse(const std::string &str) override;

	bool CreateAccount();
	bool LogIn();

	static std::string HashPswd(std::string &pswd, const std::string &id); // Hash(pswd) = SHA256(pswd + Salt(pswd)), changes pswd and return it
	static void AppendSalt(std::string &pswd, const std::string &id); // Salt(pswd) = pswd + base64(id), pswd is changed
};

};