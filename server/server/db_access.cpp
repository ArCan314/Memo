#include <mutex>
#include <vector>
#include <iostream>
#include <QtCore\qstring.h>

#include "db_access.h"


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

	"CREATE TABLE IF NOT EXISTS memos("\
	"	memo_id INT NOT NULL,"\
	"	memo_title VARCHAR(20),"\
	"	PRIMARY KEY(memo_id)"\
	");",

	"CREATE TABLE IF NOT EXISTS records("\
	"	memo_id INT NOT NULL,"\
	"	record_id INT NOT NULL,"\
	"	due_date DATE,"\
	"	record_text VARCHAR(80) NOT NULL,"\
	"	PRIMARY KEY(memo_id, record_id),"\
	"	CONSTRAINT records_memo_ref FOREIGN KEY(memo_id) REFERENCES memos(memo_id) ON DELETE CASCADE"\
	");",

	"CREATE TABLE IF NOT EXISTS id_memo("\
	"	id VARCHAR(36) NOT NULL,"\
	"	memo_id INT NOT NULL,"\
	"	PRIMARY KEY(id, memo_id),"\
	"	CONSTRAINT id_memo_id_ref FOREIGN KEY(id) REFERENCES accounts(id),"\
	"	CONSTRAINT id_memo_memo_ref FOREIGN KEY(memo_id) REFERENCES memos(memo_id) ON DELETE CASCADE"\
	");"
};


static bool InitDB()
{
	std::lock_guard<std::mutex> lock(mtx_init);
	bool res = true;
	if (!has_init)
	{
		DBAccess init_db{ "db_init_con" };
		if (!(res = init_db.OpenConnection()))
		{
			// log
			std::cerr << init_db.dbg_get().lastError().text().toStdString() << std::endl;
			return false;
		}

		QSqlQuery tmp_qry(init_db.GetQuery());
		for (const auto &stmt : init_stmts)
		{
			res = tmp_qry.exec(stmt);
			if (!res)
			{
				// log
				std::cerr << tmp_qry.lastError().text().toStdString() << std::endl;
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

bool MemoServer::InitDataBase()
{
	return InitDB();
}
