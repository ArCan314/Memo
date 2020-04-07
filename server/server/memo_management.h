#pragma once

#include <vector>
#include <string>
#include <QtCore/qstring.h>

#include <cassert>

#include "abstract_management.h"
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
	bool done{ false };
};

struct MemoData
{
	std::string id{ "1" };
	std::vector<MemoRec> recs;
	std::string GetString(const std::string &event) const;
	bool GenData(const rapidjson::Document &dom);
};

class MemoManager : public AbstractManager
{
public:
	MemoManager(const QString &db_name) : AbstractManager(db_name)
	{
	}

	void Start() override
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

private:
	std::string _id;
	RecvEventType _type;
	std::string _res_str;

	bool Parse(const std::string &str) override;

	bool SyncClient();
	bool SyncServer();
};

};