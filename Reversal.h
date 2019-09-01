#pragma once

#include <QObject>
#include <QTcpServer>
#include <QThread>
#include <QQueue>
#include <QMutex>
class worker : public QObject
{
	Q_OBJECT
signals:
	void ended();
public:
	void process();
};
class _MyTcpServer : public QTcpServer
{
	Q_OBJECT
signals:
	void socketDescriptor(qintptr);
protected:
	void incomingConnection(qintptr _socketDescriptor) override
	{
		emit socketDescriptor(_socketDescriptor);
	}
};
class Controller;
class Reversal : public QObject
{
	Q_OBJECT

public:
	Reversal(QObject *parent, qintptr port,QString nickname);
	~Reversal();
	void init();
	void process(QList<QByteArray> r,QTcpSocket* _this = nullptr);
	void newc(qintptr p);
signals:
	void send_control(QString);
	void ended();
private:
	QMutex mutex,mutex_new;
	QQueue<qintptr> cur_pd;
	qintptr port;
	QString nickname;
	_MyTcpServer* server= nullptr;
};
