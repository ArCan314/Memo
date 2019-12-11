#pragma once

#include <memory>
#include <string>
#include <utility>
#include <map>

#include <QtCore/qstring.h>

#include "semaphore.h"

namespace MemoServer
{
using RegType = std::pair<Semaphore, std::string>;
using RegPointer = std::shared_ptr<RegType>;

const char *kEventGroupStr = "EventGroup";
const char *kEventTypeStr = "Event";
enum class RecvEventType { LOG_IN, LOG_OUT, CREATE_ACCOUNT,  };
enum class SendEventType { RE };

const std::map<std::string, RecvEventType>
kRecvEventTypeStr =
{
	{"LogIn", RecvEventType::LOG_IN},
	{"LogOut", RecvEventType::LOG_OUT},
	{"CreateAccount", RecvEventType::CREATE_ACCOUNT}
};

const std::map<SendEventType, std::string> 
kSendEventTypeStr =
{
	{SendEventType::RE, "Reply"}
};

const std::map<RecvEventType, QString> 
kSQLQueryStr =
{
	{ RecvEventType::LOG_IN,
	  "SELECT COUNT(*) FROM accounts WHERE id = ? AND pswd = ?;"},
	{ RecvEventType::LOG_OUT,
	  "SELECT COUNT(*) FROM accounts WHERE id = ?;"},
	{ RecvEventType::CREATE_ACCOUNT,
	  "INSERT INTO accounts(id, pswd) VALUES (?, ?);"}
};
};