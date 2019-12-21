#pragma once

#include <string>
#include <iostream>
#include <QtCore/qstring.h>
#include <QtCore/qdebug.h>
#include <QtSql/qsqldatabase.h>
#include <QtSql/qsqlquery.h>
#include <QtSql/qsqlerror.h>

#include "log.h"

namespace MemoServer
{

bool InitDataBase();

class DBAccess
{
public:
	DBAccess(const QString &db_name) 
		: _db(QSqlDatabase::addDatabase("QMYSQL", db_name))
	{
		_db.setHostName("localhost");
		_db.setUserName("me");
		_db.setPassword("whatever");
	}

	bool OpenConnection()
	{
		bool res = _db.open();
		if (!res)
		{
			WRITE_LOG(LogLevel::ERROR,
						  __Str("Cannot connect to the database, error message: ")
						  .append(_db.lastError().text().toStdString()));
		}
		return res;
	}

	QSqlQuery GetQuery()
	{
		return QSqlQuery(_db);
	}

	QSqlDatabase &dbg_get()
	{
		return _db;
	}

private:
	QSqlDatabase _db;
};
}