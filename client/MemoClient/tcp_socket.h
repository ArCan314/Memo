#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include <QtNetwork>
#include <QByteArray>

class TcpSocket : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString host READ getHost WRITE setHost NOTIFY hostChanged)
    Q_PROPERTY(quint16 port READ getPort WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(QAbstractSocket::SocketState state READ getState WRITE setState NOTIFY stateChanged)

signals:
    void hostChanged();
    void portChanged();
    void stateChanged();

    void read(const QString &message);
    void connected();
    void disconnected();

public:
    TcpSocket(QObject *parent = nullptr) : QObject(parent)
    {
        _socket = new QTcpSocket(this);

        QObject::connect(_socket, &QAbstractSocket::stateChanged,
                [=](QAbstractSocket::SocketState state)
                {
                    setProperty("state", state);
                });

        QObject::connect(_socket, &QAbstractSocket::readyRead,
                [=]()
                {
                     emit read(QString::fromUtf8(QByteArray().fromBase64(_socket->readAll())));
                });

        QObject::connect(_socket, &QAbstractSocket::connected,
                [=]()
                {
                    emit connected();
                });

        QObject::connect(_socket, &QAbstractSocket::disconnected,
                [=]()
                {
                     emit disconnected();
                });
    }

    QString getHost()
    {
        return _host;
    }

    void setHost(const QString &host)
    {
        _host = host;
    }

    quint16 getPort()
    {
        return _port;
    }

    void setPort(const quint16 port)
    {
        _port = port;
    }

    QAbstractSocket::SocketState getState()
    {
        return _state;
    }

    void setState(QAbstractSocket::SocketState state)
    {
        _state = state;
    }

public slots:
    bool connect()
    {
        _socket->connectToHost(_host, _port);

        return _socket->waitForConnected(_timeout_ms);
    }

    void write(const QString &msg)
    {
        _socket->write(msg.toUtf8().toBase64());
    }

    void setTimeOut(const int timeout_ms)
    {
        _timeout_ms = timeout_ms;
    }

    void disconnect()
    {
        _socket->disconnectFromHost();
    }



private:
    QTcpSocket *_socket = nullptr;
    QAbstractSocket::SocketState _state = QAbstractSocket::UnconnectedState;
    QString _host;
    quint16 _port;


    int _timeout_ms = 30000;
};

#endif // TCP_SOCKET_H
