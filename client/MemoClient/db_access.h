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
        "record_text nvarchar(240) default \"\","
        "is_done boolean not null default false);"
    };
    const QString kAddRec = "insert into records values (?, ?, ?, false);";
    const QString kGetMaxId = "select max(record_id) from records;";
    const QString kGetRecSize = "select count(*) from records;";
    const QString kSetAttr = "update records set %1 = ? where record_id = ?;";
    const QString kGetAttr = "select %1 from records where record_id = ?;";
    const QString kRemoveRec = "delete from records where record_id = ?;";
    const QString kUpdateID = "update records set record_id = record_id - 1 where record_id > ?;";
    const QString kGetUserSize = "select count(*) from user;";
    const QString kSetId = "insert into user values (?);";
    const QString kGetId = "select id from user;";
    const QString kDeleteUser = "delete from user;";
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

        _max_id = getMaxId();
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
        qDebug() << res;
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
            _db_con.rollback();
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

    bool setId(const QString &id)
    {
        QSqlQuery query;
        int id_num = getUserSize();
        if (id_num > 0)
        {
            if (!deleteUser())
            {
                return false;
            }
        }
        else if (id_num == -1)
        {
            return false;
        }

        query.prepare(kSetId);
        query.addBindValue(id);

        if (!query.exec())
        {
            qDebug() << query.lastError();
            return false;
        }
        return true;
    }

    int getMaxId() // return negative number when query fails
    {
        QSqlQuery query;
        int size = getRecSize();
        if (size)
        {
            if (!query.exec(kGetMaxId))
            {
                qDebug() << query.lastError();
                return -1;
            }

            query.next();
            return query.value(0).toInt();
        }
        else
        {
            return 0;
        }
    }

    QString getId()
    {
        QSqlQuery query;
        if (!query.exec(kGetId))
        {
            qDebug() << query.lastError();
            return "";
        }
        query.next();
        return query.value(0).toString();
    }


private:
    bool _is_inited = false;
    QSqlDatabase _db_con;
    int _max_id;

    int getUserSize()
    {
        QSqlQuery query;
        if (!query.exec(kGetUserSize))
        {
            qDebug() << query.lastError();
            return -1;
        }

        query.next();
        return query.value(0).toInt();
    }

    bool deleteUser()
    {
        QSqlQuery query;
        if (!query.exec(kDeleteUser))
        {
            qDebug() << query.lastError();
            return false;
        }
        return true;
    }

    bool setAttr(const int rec_id, QString attr_name, QVariant val)
    {
        QSqlQuery query;
        query.prepare(kSetAttr.arg(attr_name));
        // query.addBindValue(attr_name);
        query.addBindValue(val);
        query.addBindValue(rec_id);
        if (!query.exec())
        {
            // qDebug() << attr_name << "\n" << val << "\n" << rec_id << "\n" << query.lastQuery();
            // qDebug() << query.executedQuery();
            qDebug() << query.lastError();
            return false;
        }
        return true;
    }

    QVariant getAttr(const int rec_id, QString attr_name)
    {
        QSqlQuery query;
        query.prepare(kGetAttr.arg(attr_name));
        // query.addBindValue(attr_name);
        query.addBindValue(rec_id);
        if (!query.exec())
        {
            qDebug() << query.lastError();

            return QVariant();
        }
        qDebug() << query.executedQuery();
        query.next();
        return query.value(0);
    }

    int getRecSize() // return negative number when query fails.
    {
        QSqlQuery query;
        if (!query.exec(kGetRecSize))
        {
            qDebug() << query.lastError();
            return -1;
        }
        query.next();
        return query.value(0).toInt();
    }

};


#endif // DB_ACCESS_H
