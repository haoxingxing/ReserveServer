#include "Reversal.h"
#include "tools.h"
#include <QTcpSocket>
#include "Controller.h"
#include <iostream>

Reversal::Reversal(QObject* parent, qintptr port, QString nickname)
	: QObject(parent),port(port),nickname(nickname)
{
}

Reversal::~Reversal()
{
	delete server;
	DEB << "closing";
	emit send_control(nickname + " closing");
	emit ended();
}

void Reversal::init()
{
	server = new _MyTcpServer;
	connect(server, &_MyTcpServer::socketDescriptor, this, &Reversal::newc);
	if(!server->listen(QHostAddress::Any, port))
	{
		DEB << "failed creating socket " + server->errorString();
		emit send_control(nickname + " failed " + server->errorString());
		this->deleteLater();
	}
	else
	{
		DEB << "succeed creating socket on " << port;
		emit send_control(nickname + " success");
	}
	
}

void Reversal::process(QList<QByteArray> r, QTcpSocket* _this)
{
	if (r[0] == "close")
		this->deleteLater();
	if (r[0] == "connect")
	{
			QEventLoop l;
			DEB << "locking";
			mutex.lock();
			DEB << "locked";
			connect(server, &QTcpServer::newConnection, &l, [&] {l.exit(true); });
			connect(_this, &QTcpSocket::disconnected, &l, [&] {mutex.unlock(); l.exit(false); });
			if (!cur_pd.empty()||l.exec()) {
				_this->disconnect(&l);
				server->disconnect(&l);
				emit send_control(nickname + " connected " + QString::number(cur_pd.front()));
				DEB << "connected " << cur_pd.front();
				_this->write("connected\r\n");
				_this->flush();
				auto socket = new QTcpSocket();
				socket->setSocketDescriptor(cur_pd.takeFirst());
				mutex.unlock();
				DEB << "unlocked";
				QEventLoop loop;
				connect(socket, &QTcpSocket::readyRead, &loop, [&] {loop.exit(true); });
				connect(socket, &QTcpSocket::disconnected, &loop, [&] {loop.exit(false); });
				connect(_this, &QTcpSocket::disconnected, &loop, [&] {loop.exit(false); });
				connect(_this, &QTcpSocket::readyRead, &loop, [&] {loop.exit(true); });
				while (loop.exec())
				{
					std::cout << "+";
					socket->write(_this->readAll());
					_this->write(socket->readAll());
					socket->flush();
					_this->flush();
				}
				loop.disconnect();
				socket->disconnect();
				_this->disconnect();
				_this->disconnectFromHost();
				socket->disconnectFromHost();
			}
	}
}

void Reversal::newc(qintptr p)
{
	cur_pd.enqueue(p);
	DEB << "need_connector " << p;
	emit send_control(nickname + " need_connector " + QString::number(p));
}
