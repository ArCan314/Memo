#pragma once

#include <string>
#include <QtCore/qstring.h>
#include <QtSql/qsqldatabase.h>
#include <QtSql/qsqlquery.h>

namespace MemoServer
{

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
		return _db.open();
	}

	QSqlQuery GetQuery()
	{
		return QSqlQuery(_db);
	}

private:
	QSqlDatabase _db;
};
}