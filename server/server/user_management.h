#pragma once

#include <map>
#include <type_traits>
#include <utility>
#include <QtCore/qstring.h>

#include "controller.h"
#include "job_queue.h"
#include "data_manager.h"
#include "../include/cpp-base64/base64.h"
#include "../include/PicoSHA2/picosha2.h"

#define RAPID_JSON_HAS_STDSTRING 1
#include "../include/rapidjson/document.h"

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
					res = LogOut();
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
		}
	}

private:
	enum class RecvEventType {LOG_IN, LOG_OUT, CREATE_ACCOUNT};
	enum class SendEventType {RE};
	static const std::map<std::string, RecvEventType> kRecvEventTypeStr;
	static const std::map<SendEventType, std::string> kSendEventTypeStr;
	static const std::map<RecvEventType, QString> kSQLQueryStr;
	JobQueue<RegPointer> _jobs; // jobs with JSON
	DBAccess _db;
	std::string _id;
	std::string _pswd;
	rapidjson::Document _dom;
	RecvEventType _type;
	
	bool Parse(const std::string &str)
	{
		bool res = true;
		_dom.Clear();
		_dom.Parse(str.c_str());
		// parse error cannot hanppen(checked before)
		if (_dom.HasMember("Event") &&
			_dom["Event"].IsString() &&
			kRecvEventTypeStr.count(_dom["Event"].GetString()))
		{
			_type = kRecvEventTypeStr.at(_dom["Event"].GetString());
			switch (_type)
			{
			case RecvEventType::CREATE_ACCOUNT:
			case RecvEventType::LOG_IN:
				if (!_dom.HasMember("Pswd"))
				{
					res = false;
					// log
					break;
				}
				_pswd = _dom["Pswd"].GetString();
			case RecvEventType::LOG_OUT:
				if (!_dom.HasMember("ID"))
				{
					res = false;
					// log
					break;
				}
				_id = _dom["ID"].GetString();
				break;
			default:
				// log
				assert(1 == 0);
				break;
			}
		}
		else
		{
			// log
			res = false;
		}
		return res;
	}

	template <typename T>
	void AddValue(rapidjson::Document &dom, const std::string &name_str, const rapidjson::Type val_type,const T &val_par = T())
	{		
		using rapidjson::Type;
		
		rapidjson::Value name, val;
		name.SetString(name_str.c_str(), dom.GetAllocator());
		switch (val_type)
		{
		case kNullType:
			val.SetNull();
			break;
		case kFalseType:
			val.SetBool(true);
			break;
		case kTrueType:
			val.SetBool(false);
			break;
		case kStringType:
			val.SetString(val_par, dom.GetAllocator());
			break;
		case kNumberType:
			val.SetInt64(val_par);
			break;
		default:
			assert(1 == 0); // error
			break;
		}

		dom.AddMember(std::move(name), std::move(val));
	}

	bool CreateAccount()
	{
		QSqlQuery query = _db.GetQuery();
		query.prepare(kSQLQueryStr.at(RecvEventType::CREATE_ACCOUNT));
	}

	bool LogOut()
	{
	}

	bool LogIn()
	{
	}
};

const std::map<std::string, AccountManager::RecvEventType>
AccountManager::kRecvEventTypeStr =
{
	{"LogIn", RecvEventType::LOG_IN},
	{"LogOut", RecvEventType::LOG_OUT},
	{"CreateAccount", RecvEventType::CREATE_ACCOUNT}
};

const std::map<AccountManager::SendEventType, std::string>
AccountManager::kSendEventTypeStr =
{
	{SendEventType::RE, "Return"}
};

const std::map<AccountManager::RecvEventType, QString> 
AccountManager::kSQLQueryStr =
{
	{ RecvEventType::LOG_IN,
	  "SELECT COUNT(*) FROM accounts WHERE id = ? AND pswd = ?;"},
	{ RecvEventType::LOG_OUT,
	  "SELECT COUNT(*) FROM accounts WHERE id = ?;"},
	{ RecvEventType::CREATE_ACCOUNT,
	  "INSERT INTO accounts(id, pswd) VALUES (?, ?);"}
};

};