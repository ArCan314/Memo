#include <mutex>
#include <vector>
#include <iostream>
#include <QtCore\qstring.h>

#include "db_access.h"
#include "log.h"


using MemoServer::DBAccess;

static bool has_init = false;
static std::mutex mtx_init;
static const std::vector<QString> init_stmts =
{
	"CREATE DATABASE IF NOT EXISTS memo_data;",

	"USE memo_data;",

	"CREATE TABLE IF NOT EXISTS accounts ("\
	"	id VARCHAR(36) NOT NULL,"\
	"	pswd VARCHAR(256) NOT NULL,"\
	"	PRIMARY KEY(id)"\
	");",

	"CREATE TABLE IF NOT EXISTS records("\
	"id VARCHAR(36) NOT NULL,"\
	"record_id INT NOT NULL,"\
	"due_date DATE,"\
	"record_text VARCHAR(250) NOT NULL,"\
	"is_done BOOLEAN NOT NULL,"\
	"PRIMARY KEY(id, record_id),"\
	"CONSTRAINT records_accounts_ref FOREIGN KEY(id) REFERENCES accounts(id) ON DELETE CASCADE"\
	");"
};

namespace MemoServer
{
static bool InitDB()
{
	std::lock_guard<std::mutex> lock(mtx_init);
	bool res = true;
	if (!has_init)
	{
		WRITE_LOG(LogLevel::INFO,
				  __Str("Initializing database."));

		DBAccess init_db{ "db_init_con" };
		if (!(res = init_db.OpenConnection()))
		{
			WRITE_LOG(LogLevel::ERROR,
					  __Str("Failed to connect to the database."));
			return false;
		}

		QSqlQuery tmp_qry(init_db.GetQuery());
		for (const auto &stmt : init_stmts)
		{
			res = tmp_qry.exec(stmt);
			if (!res)
			{
				WRITE_LOG(LogLevel::ERROR,
						  __Str("Failed to execute sql query statement: ")
						  .append(stmt.toStdString())
						  .append("\n\tError message: ")
						  .append(tmp_qry.lastError().text().toStdString()));

				break;
			}
		}

		if (res)
		{
			has_init = true;
		}
	}
	return res;
}

bool InitDataBase()
{
	return InitDB();
}
}; // namespace MemoServer
