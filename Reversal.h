#pragma once

#include <QObject>
#include <QTcpServer>
class Reversal : public QObject
{
	Q_OBJECT

public:
	Reversal(QObject *parent, qintptr port,QString nickname);
	~Reversal();
	void init();
	void process(QList<QByteArray> r);
signals:
	void send_control(QString);
private:
	qintptr port;
	QString nickname;
	QTcpServer* server= nullptr;
};
