#include <QCoreApplication>
#include <QTcpServer>
#include <QThread>
#include "Controller.h"
QVector<QThread*> pool;
long long idpool = 0;
class MyTcpServer : public QTcpServer
{
protected:
	void incomingConnection(qintptr socketDescriptor) override
	{
		auto t = new QThread;
		Controller* c = new Controller(nullptr,socketDescriptor,QString::number(idpool++));
		qDebug() << "New Connection Handled at" << socketDescriptor << "named" << idpool - 1;
		QThread::connect(t, &QThread::started, c, &Controller::process);
		QThread::connect(c, &Controller::ended, t, &QThread::quit);
		pool.append(t);
		c->moveToThread(t);
		t->start();
	}
};
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
	int port = 8888;
	MyTcpServer server;
	server.listen(QHostAddress::Any, port);
	qDebug() << "Started Listening at port" << port;
    return a.exec();
}
