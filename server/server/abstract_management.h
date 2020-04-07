#pragma once

#include <QtCore/qstring.h>

#include "global.h"
#include "abstract_component.h"
#include "db_access.h"
#include "job_queue.h"

#include "../include/rapidjson/document.h"
#include "../include/rapidjson/writer.h"
#include "../include/rapidjson/stringbuffer.h"

namespace MemoServer
{

class AbstractManager : public AbstractComponent<JobQueue<RegPointer>>
{
public:
	AbstractManager(const QString &db_name) : _db(db_name), _dom()
	{
	}

	virtual void Start() = 0;

	JobQueue<RegPointer> *GetHandle() override
	{
		return &_jobs;
	}

protected:
	virtual bool Parse(const std::string &str) = 0;

	JobQueue<RegPointer> _jobs;
	DBAccess _db;
	rapidjson::Document _dom;
	rapidjson::Writer<rapidjson::StringBuffer> _writer;

};

}; //namespace MemoServer
