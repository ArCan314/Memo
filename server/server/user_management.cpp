#include "user_management.h"

#include "../include/cpp-base64/base64.h"
#include "../include/PicoSHA2/picosha2.h"

bool MemoServer::AccountManager::Parse(const std::string &str)
{
	bool res = true;
	_dom.Clear();
	_dom.Parse(str.c_str());
	// parse error cannot hanppen(checked before)
	if (_dom.HasMember("Event") &&
		_dom["Event"].IsString() &&
		kRecvStrToEventType.count(_dom["Event"].GetString()))
	{
		_type = kRecvStrToEventType.at(_dom["Event"].GetString());
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

bool MemoServer::AccountManager::CreateAccount()
{
	bool res;
	QSqlQuery query = _db.GetQuery();
	res = query.prepare(kEventTypeToSQLQueryStr.at(RecvEventType::CREATE_ACCOUNT));
	if (!res)
	{
		// log
		assert(1 == 0);
	}

	query.addBindValue(_id.c_str());
	query.addBindValue(HashPswd(_pswd, _id).c_str());

	res = query.exec();
	if (!res)
	{
		// already has user names _id.
		res = false;
	}

	return res;
}

/*
bool MemoServer::AccountManager::LogOut()
{
	bool res;
	QSqlQuery query = _db.GetQuery();
	res = query.prepare(kEventTypeToSQLQueryStr.at(RecvEventType::LOG_OUT));
	if (!res)
	{
		// log
		assert(1 == 0);
	}

	return res;
}
*/

bool MemoServer::AccountManager::LogIn()
{
	bool res;
	QSqlQuery query = _db.GetQuery();
	res = query.prepare(kEventTypeToSQLQueryStr.at(RecvEventType::CREATE_ACCOUNT));
	if (!res)
	{
		// log
		assert(1 == 0);
	}

	query.addBindValue(_id.c_str());
	query.addBindValue(HashPswd(_pswd, _id).c_str());

	res = query.exec();
	if (!res)
	{
		// log
		assert(1 == 0);
	}

	if (query.isSelect() && query.isActive() && query.size() == 1)
	{
		query.next();
		if (query.value(0).toInt() == 1) // no user named id or pswd is not correct
			res = true;
		else
			res = false;
	}
	else
	{
		// log
	}

	return res;
}

std::string MemoServer::AccountManager::HashPswd(std::string &pswd, const std::string &id)
{
	AppendSalt(pswd, id);
	return picosha2::hash256_hex_string(pswd);
}

void MemoServer::AccountManager::AppendSalt(std::string &pswd, const std::string &id)
{
	pswd += base64_encode(reinterpret_cast<const unsigned char *>(id.c_str()), id.size());
}
