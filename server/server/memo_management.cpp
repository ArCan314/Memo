#include "memo_management.h"

#include "user_management.h"

#include <QtCore/qvariant.h>

bool MemoServer::MemoManager::Parse(const std::string &str)
{
	bool res = true;
	// _dom.Clear();
	rapidjson::Document().Swap(_dom);
	_dom.Parse(str.c_str());
	if (_dom.HasMember("Event") &&
		_dom["Event"].IsString() &&
		kRecvStrToEventType.count(_dom["Event"].GetString()))
	{
		_type = kRecvStrToEventType.at(_dom["Event"].GetString());
		switch (_type)
		{
		case RecvEventType::SYNC_CLIENT:
		case RecvEventType::SYNC_SERVER:
			if (!_dom.HasMember("ID"))
			{
				res = false;
				break;
			}
			_id = _dom["ID"].GetString();
			break;
		default:
			assert(1 == 0);
			break;
		}
	}
	else
	{
		res = false;
	}
	return res;
}

static void SetFailedReply(std::string &rep)
{
	rep = "{\"EventGroup\":\"Data\",\"Event\":\"SyncReply\",\"SyncResult\": false}";
}

bool MemoServer::MemoManager::SyncClient()
{
	using rapidjson::Value;
	bool res = true;
	MemoData data;

	res = data.GenData(_dom);
	if (!res)
	{
		WRITE_LOG(LogLevel::INFO,
				  __Str("Failed to parse the query."));
		SetFailedReply(_res_str);
		return false;
	}
	QVariantList var_list[4]{};
	QSqlQuery query = _db.GetQuery();
	const auto &query_vec = kEventTypeToSQLQueryStr.at(RecvEventType::SYNC_CLIENT);

	if (AccountManager::HasAccountID(data.id))
	{

		res = query.exec(query_vec.at(0));
		if (!res)
		{
			WRITE_LOG(LogLevel::INFO,
					  __Str("Cannot execute db query, error msg: ")
					  .append(query.lastError().text().toStdString()));
			SetFailedReply(_res_str);
			return false;
		}

		res = query.exec(query_vec.at(1)); // DELETE FROM memos WHERE memo_id IN (SELECT memo_id FROM id_memo);
		if (!res)
		{
			WRITE_LOG(LogLevel::INFO,
					  __Str("Cannot execute db query, error msg: ")
					  .append(query.lastError().text().toStdString()));
			SetFailedReply(_res_str);
			return false;
		}

		res = query.prepare(query_vec.at(2)); // INSERT INTO memos VALUES (?, ?);
		if (!res)
		{
			WRITE_LOG(LogLevel::WARN,
					  __Str("Cannot prepare db query, error msg: ")
					  .append(query.lastError().text().toStdString()));
			SetFailedReply(_res_str);
			return false;
		}


		{
			QVariantList &ids = var_list[0], &titles = var_list[1];
			for (int i = 0; i < data.memos.size(); i++)
			{
				ids.push_back(data.memos[i].id);
				titles.push_back(QString::fromStdString(data.memos[i].title));
			}
			// std::cerr << var_list[0].size() << ", " << var_list[1].size() << std::endl;
			query.addBindValue(ids);
			query.addBindValue(titles);
		}

		
 		res = query.execBatch();
		if (!res)
		{
			WRITE_LOG(LogLevel::INFO,
					  __Str("Cannot execute db query, error msg: ")
					  .append(query.lastError().text().toStdString()));
			SetFailedReply(_res_str);
			return false;
		}

		res = query.prepare(query_vec.at(3)); // INSERT INTO id_memo VALUES (?, ?);
		if (!res)
		{
			WRITE_LOG(LogLevel::WARN,
					  __Str("Cannot prepare db query, error msg: ")
					  .append(query.lastError().text().toStdString()));
			SetFailedReply(_res_str);
			return false;
		}

		{
			QVariantList &ids = var_list[0], &memo_ids = var_list[1];
			ids.clear(), memo_ids.clear();

			for (int i = 0; i < data.memos.size(); i++)
			{
				ids.push_back(QString::fromStdString(_id));
				memo_ids.push_back(data.memos[i].id);
			}
			query.addBindValue(ids);
			query.addBindValue(memo_ids);
		}

		res = query.execBatch();
		if (!res)
		{
			WRITE_LOG(LogLevel::WARN,
					  __Str("Cannot prepare db query, error msg: ")
					  .append(query.lastError().text().toStdString()));
			SetFailedReply(_res_str);
			return false;
		}


		if (!query.prepare(query_vec[4])) // INSERT INTO records VALUES (?, ?, ?, ?);
		{
			WRITE_LOG(LogLevel::WARN,
					  __Str("Cannot prepare db query, error msg: ")
					  .append(query.lastError().text().toStdString()));
			SetFailedReply(_res_str);
			return false;
		}

		for (auto &list : var_list)
		{
			list.clear();
		}

		QVariantList &memo_ids = var_list[0],
			&record_ids = var_list[1],
			&due_dates = var_list[2],
			&record_texts = var_list[3];

		for (int i = 0; i < data.memos.size(); i++)
		{
			auto &recs = data.memos[i].recs;
			for (int j = 0; j < recs.size(); j++)
			{
				memo_ids.push_back(data.memos[i].id);
				record_ids.push_back(recs[j].id);
				due_dates.push_back(QString::fromStdString(recs[j].date));
				record_texts.push_back(QString::fromStdString(recs[j].text));
			}
		}

		query.addBindValue(memo_ids);
		query.addBindValue(record_ids);
		query.addBindValue(due_dates);
		query.addBindValue(record_texts);

		res = query.execBatch();
		if (!res)
		{
			WRITE_LOG(LogLevel::INFO,
					  __Str("Cannot execute db query, error msg: ")
					  .append(query.lastError().text().toStdString()));
			SetFailedReply(_res_str);
			return false;
		}

		auto &allocator = _dom.GetAllocator();
		_dom.SetObject().AddMember("EventGroup", Value("Data"), allocator)
			.AddMember("Event", Value("SyncReply"), allocator)
			.AddMember("SyncResult", Value(res), allocator);

		rapidjson::StringBuffer str_buf;
		rapidjson::Writer<rapidjson::StringBuffer> writer(str_buf);
		_dom.Accept(writer);
		_res_str = str_buf.GetString();
	}
	else
	{
		SetFailedReply(_res_str);
	}
	return res;
}

bool MemoServer::MemoManager::SyncServer()
{
	bool res = true;
	MemoData data;
	QVariantList var_list[4]{};
	QSqlQuery query = _db.GetQuery();
	const auto &query_vec = kEventTypeToSQLQueryStr.at(RecvEventType::SYNC_SERVER);

	data.id = _id;
	// query.setForwardOnly(true);

	res = query.exec(query_vec.at(0));
	if (!res)
	{
		WRITE_LOG(LogLevel::INFO,
				  __Str("Cannot execute db query, error msg: ")
				  .append(query.lastError().text().toStdString()));
		return false;
	}

	res = query.exec(query_vec.at(1)); // SELECT memo_id FROM id_memo;
	if (!res)
	{
		WRITE_LOG(LogLevel::INFO,
				  __Str("Cannot execute db query, error msg: ")
				  .append(query.lastError().text().toStdString()));
		return false;
	}

	if (query.size())
	{
		while (query.next())
		{
			data.memos.emplace_back();
			data.memos.back().id = query.value(0).toInt();
		}

		for (int i = 0; i < data.memos.size(); i++)
		{
			query.prepare(query_vec[2]); // SELECT memo_title FROM memos WHERE memo_id = ?;
			query.addBindValue(data.memos[i].id);
			res = query.exec();
			if (!res)
			{
				WRITE_LOG(LogLevel::INFO,
						  __Str("Cannot execute db query, error msg: ")
						  .append(query.lastError().text().toStdString()));
				return false;
			}

			while (query.next())
			{
				data.memos[i].title = query.value(0).toString().toStdString();
			}
		}

		for (int i = 0; i < data.memos.size(); i++)
		{
			query.prepare(query_vec[3]); // SELECT record_id, due_date, record_text FROM records WHERE memo_id = ?;
			query.addBindValue(data.memos[i].id);
			res = query.exec();
			if (!res)
			{
				WRITE_LOG(LogLevel::INFO,
						  __Str("Cannot execute db query, error msg: ")
						  .append(query.lastError().text().toStdString()));
				return false;
			}

			while (query.next())
			{
				data.memos[i].recs.push_back({
					query.value(0).toInt(),
					query.value(1).toString().toStdString(),
					query.value(2).toString().toStdString()
											 });
			}
		}

		_res_str = data.GetString("SyncData");
	}
	else
	{
		_res_str = "{\"EventGroup\":\"Data\",\"Event\":\"SyncReply\",\"SyncResult\": false}";
	}

	return res;
}

std::string MemoServer::MemoData::GetString(const std::string &event) const
{
	using rapidjson::Value;
	rapidjson::Document dom;
	rapidjson::StringBuffer str_buf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(str_buf);
	rapidjson::Document::AllocatorType &allocator = dom.GetAllocator();
	std::string res;

	dom.SetObject();
	dom.AddMember("EventGroup", Value("Data"), allocator);
	dom.AddMember("Event", Value().SetString(event.c_str(), allocator), allocator);
	dom.AddMember("ID", Value().SetString(event.c_str(), allocator), allocator);
	dom.AddMember("Memos", Value().SetArray(), allocator);

	for (const auto &memo : memos)
	{
		Value memo_obj(rapidjson::kObjectType);
		memo_obj.AddMember("MemoID", Value(memo.id), allocator);
		memo_obj.AddMember("MemoTitle", Value(memo.title.c_str(), allocator), allocator);
		memo_obj.AddMember("Records", Value(rapidjson::kArrayType), allocator);
		for (const auto &rec : memo.recs)
		{
			Value rec_obj(rapidjson::kObjectType);
			rec_obj.AddMember("RecID", Value(rec.id), allocator);
			rec_obj.AddMember("Text", Value(rec.text.c_str(), allocator), allocator);
			rec_obj.AddMember("DueDate", Value(rec.date.c_str(), allocator), allocator);
			memo_obj["Records"].PushBack(std::move(rec_obj), allocator);
		}
		dom["Memos"].PushBack(std::move(memo_obj), allocator);
	}

	dom.Accept(writer);


	return str_buf.GetString();
}

bool MemoServer::MemoData::GenData(const rapidjson::Document &dom)
{
	using rapidjson::Value;
	bool res = true;

	//rapidjson::StringBuffer str_buf;
	//rapidjson::Writer<rapidjson::StringBuffer> writer(str_buf);
	//dom.Accept(writer);
	//std::cerr << str_buf.GetString() << std::endl;

	if (dom.HasMember("Memos") && dom.HasMember("ID"))
	{
		// TODO : error handling

		id = dom["ID"].GetString();
		memos.resize(dom["Memos"].Size());

		for (int i = 0; i < memos.size(); i++)
		{
			auto &obj = dom["Memos"][i];
			auto &memo = memos[i];
			
			assert(obj.HasMember("MemoID"));
			assert(obj.HasMember("MemoTitle"));
			assert(obj.HasMember("Records")); 
			assert(obj["MemoID"].IsInt());
			assert(obj["MemoTitle"].IsString()); 
			assert(obj["Records"].IsArray());
			
			memo.id = obj["MemoID"].GetInt();
			memo.title = obj["MemoTitle"].GetString();
			memo.recs.resize(obj["Records"].Size());
			for (int j = 0; j < memo.recs.size(); j++)
			{
				{
					auto &test = obj["Records"][j];
					assert(test.HasMember("RecID"));
					assert(test.HasMember("Text"));
					assert(test.HasMember("DueDate"));
					assert(test["RecID"].IsInt());
					assert(test["Text"].IsString());
					assert(test["DueDate"].IsString());
				}

				memo.recs[j].id = obj["Records"][j]["RecID"].GetInt();
				memo.recs[j].text = obj["Records"][j]["Text"].GetString();
				memo.recs[j].date = obj["Records"][j]["DueDate"].GetString();
			}
		}
	}
	else
	{
		res = false;
	}

	return res;
}
