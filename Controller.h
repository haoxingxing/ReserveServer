#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QTcpSocket>
#include "Reversal.h"
#include <QTime>
#include <QThread>
#include <qeventloop.h>

class QTcpSocket;

class Controller : public QObject
{
	Q_OBJECT

public:
	Controller(QObject *parent, qintptr socketDescriptor,QString nickname);
	~Controller();
	void process();
	void send_msg(QString str);
signals:
	void ended();
	void worker_msg(QList<QByteArray>);
	void request_connecting(QByteArray name);
private:
	QString nickname;
	qintptr socketDescriptor;
	QTcpSocket* control = nullptr;
	QEventLoop loop;
	static QMap<QString, Reversal*> pool;
};
