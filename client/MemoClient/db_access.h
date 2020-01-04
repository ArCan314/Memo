#ifndef DB_ACCESS_H
#define DB_ACCESS_H

#include <QtCore>
#include <QDebug>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

class DBAccess : public QObject
{
    Q_OBJECT
public:
    const QString kPath = ".\\user.db";
    const QString kInitTable[2] =
    {
        "create table if not exists user("
        "id nvarchar(36) not null default \"\");",
        "create table if not exists records("
        "record_id int primary key not null,"
        "due_date text default \"\","
        "record_text nvarchar(240) not null,"
        "is_done boolean not null default false);"
    };
    const QString kAddRec = "insert into records values (?, ?, ?, false);";
    const QString kGetMaxID = "select max(record_id) from records;";
    const QString kGetSize = "select count(*) from records;";
    const QString kSetAttr = "update records set ? = ? where record_id = ?;";
    const QString kGetAttr = "select ? from records where record_id = ?;";
    const QString kRemoveRec = "delete from records where record_id = ?;";
    const QString kUpdateID = "update records set record_id = record_id - 1 where record_id > ?;";
public:
    DBAccess(QObject *parent = nullptr) : QObject(parent), _db_con(QSqlDatabase::addDatabase("QSQLITE"))
    {
        _db_con.setDatabaseName(kPath);
        if (!_db_con.open())
        {
            qDebug() << _db_con.lastError();
            return;
        }

        QSqlQuery query;
        if (!query.exec(kInitTable[0]))
        {
            qDebug() << query.lastError();
            return;
        }

        if (!query.exec(kInitTable[1]))
        {
            qDebug() << query.lastError();
            return;
        }

        _max_id = getMaxID();
        if (_max_id < 0)
        {
            return;
        }

        _is_inited = true;
    }

    bool isInit() const
    {
        return _is_inited;
    }

public slots:
    bool addRecord(const QString &due_date, const QString &rec_text)
    {
        QSqlQuery query;
        query.prepare(kAddRec);

        query.addBindValue(++_max_id);
        query.addBindValue(due_date);
        query.addBindValue(rec_text);
        if (!query.exec())
        {
            qDebug() << query.lastError();
            return false;
        }
        return true;
    }

    bool setDueDate(const int rec_id, const QString &due_date)
    {
        return setAttr(rec_id, "due_date", due_date);
    }

    bool setIsDone(const int rec_id,const bool is_done)
    {
        return setAttr(rec_id, "is_done", is_done);
    }

    bool setText(const int rec_id, const QString &text)
    {
        return setAttr(rec_id, "record_text", text);
    }

    QString getDueDate(const int rec_id)
    {
        QVariant res = getAttr(rec_id, "due_date");
        if (res.isNull())
        {
            return "";
        }
        else
        {
            return res.toString();
        }
    }

    bool getIsDone(const int rec_id)
    {
        QVariant res = getAttr(rec_id, "is_done");
        if (res.isNull())
        {
            return false; // error handling
        }
        else
        {
            return res.toBool();
        }
    }

    QString getText(const int rec_id)
    {
        QVariant res = getAttr(rec_id, "record_text");
        if (res.isNull())
        {
            return "";
        }
        else
        {
            return res.toString();
        }
    }

    bool removeRecord(const int rec_id)
    {
        if (!_db_con.transaction())
        {
            qDebug() << _db_con.lastError();
            return false;
        }

        QSqlQuery query;
        query.prepare(kRemoveRec);
        query.addBindValue(rec_id);
        if (!query.exec())
        {
            qDebug() << query.lastError();
            return false;
        }

        query.prepare(kUpdateID);
        query.addBindValue(rec_id);
        if (!query.exec())
        {
            qDebug() << query.lastError();
            _db_con.rollback();
            return false;
        }

        _max_id--;
        _db_con.commit();
        return true;
    }



private:
    bool _is_inited = false;
    QSqlDatabase _db_con;
    int _max_id;

    bool setAttr(const int rec_id, QString attr_name, QVariant val)
    {
        QSqlQuery query;
        query.prepare(kSetAttr);
        query.addBindValue(attr_name);
        query.addBindValue(val);
        query.addBindValue(rec_id);
        if (!query.exec())
        {
            qDebug() << query.lastError();
            return false;
        }
        return true;
    }

    QVariant getAttr(const int rec_id, QString attr_name)
    {
        QSqlQuery query;
        query.prepare(kGetAttr);
        query.addBindValue(attr_name);
        query.addBindValue(rec_id);
        if (!query.exec())
        {
            qDebug() << query.lastError();
            return QVariant();
        }

        query.next();
        return query.value(0);
    }

    int getMaxID() // return negative number when query fails
    {
        QSqlQuery query;
        int size = getSize();
        if (size)
        {
            if (!query.exec(kGetMaxID))
            {
                qDebug() << query.lastError();
                return -1;
            }
        }
        else
        {
            return 0;
        }
    }

    int getSize() // return negative number when query fails.
    {
        QSqlQuery query;
        if (!query.exec(kGetSize))
        {
            qDebug() << query.lastError();
            return -1;
        }
        query.next();
        return query.value(0).toInt();
    }

};


#endif // DB_ACCESS_H
