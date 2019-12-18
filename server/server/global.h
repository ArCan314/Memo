#pragma once

#include <memory>
#include <string>
#include <utility>
#include <map>
#include <vector>

#include <QtCore/qstring.h>

#include "semaphore.h"

namespace MemoServer
{
using RegType = std::pair<Semaphore, std::string>;
using RegPointer = std::shared_ptr<RegType>;

constexpr char kEventGroupStr[] = "EventGroup";
constexpr char kEventTypeStr[] = "Event";
enum class RecvEventType { LOG_IN, LOG_OUT, CREATE_ACCOUNT, SYNC_SERVER, SYNC_CLIENT };
enum class SendEventType { RE, SYNC_DATA, SYNC_RE };
enum class ComponentType { ACCOUNT_MANAGER_POOL, DATA_MANAGER_POOL };
const std::map<ComponentType, std::string> kCompTypeToStr =
{
	{ComponentType::ACCOUNT_MANAGER_POOL, "AccountResourcePool"},
	{ComponentType::DATA_MANAGER_POOL, "DataResourcePool"}
};

const std::map<std::string, ComponentType>
kEventGroupToCompType =
{
	{"Account", ComponentType::ACCOUNT_MANAGER_POOL},
	{"Data", ComponentType::DATA_MANAGER_POOL}
};

const std::map<std::string, RecvEventType>
kRecvStrToEventType =
{
	{"LogIn", RecvEventType::LOG_IN},
	{"LogOut", RecvEventType::LOG_OUT},
	{"CreateAccount", RecvEventType::CREATE_ACCOUNT},
	{"SyncFromServer", RecvEventType::SYNC_SERVER},
	{"SyncFromClient", RecvEventType::SYNC_CLIENT}
};

const std::map<SendEventType, std::string> 
kEventTypeToStr =
{
	{SendEventType::RE, "Reply"},
	{SendEventType::SYNC_DATA, "SyncData"},
	{SendEventType::SYNC_RE, "SyncReply"}
};

const std::map<RecvEventType, std::vector<QString>>
kEventTypeToSQLQueryStr =
{
	{ 
		RecvEventType::LOG_IN,
		{
			"USE memo_data;",
			"SELECT COUNT(*) FROM accounts WHERE id = ? AND pswd = ?;"
		}
	},

	{ RecvEventType::LOG_OUT,
		{
			"USE memo_data;",
			"SELECT COUNT(*) FROM accounts WHERE id = ?;"
		}
	},

	{ 
		RecvEventType::CREATE_ACCOUNT,
		{
			"USE memo_data;",
			"INSERT INTO accounts(id, pswd) VALUES (?, ?);"
		}
	},

	{ 
		RecvEventType::SYNC_SERVER,
		{
			"USE memo_data;",
			"SELECT memo_id FROM id_memo;",
			"SELECT memo_title FROM memos WHERE memo_id = ?;",
			"SELECT record_id, due_date, record_text FROM records WHERE memo_id = ?;"
		}
	},

	{ 
		RecvEventType::SYNC_CLIENT,
		{
			"USE memo_data;",
			"DELETE FROM memos WHERE memo_id IN (SELECT memo_id FROM id_memo);", // delete all posible records
			"INSERT INTO memos VALUES(?, ?);",
			"INSERT INTO id_memo VALUES (?, ?);",
			"INSERT INTO records VALUES (?, ?, ?, ?);" // insert records
		}
	}
};
};