#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "tcp_socket.h"
#include "db_access.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    qmlRegisterType<TcpSocket>("com.ac.socket", 1, 0, "TcpSocket");
    DBAccess *db_access = new DBAccess(&app);
    if (!db_access->isInit())
    {
        return -1;
    }
    else
    {
        engine.rootContext()->setContextProperty("db", db_access);
    }

    engine.load(url);

    return app.exec();
}
