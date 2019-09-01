#include "Controller.h"
#include "tools.h"
#include <utility>

Controller::Controller(QObject* parent, qintptr socketDescriptor, QString nickname)
	: QObject(parent), nickname(std::move(nickname)), socketDescriptor(socketDescriptor)
{
}

QMap<QString, Reversal*> Controller::pool;

Controller::~Controller()
{
	DEB << "Controller ended";
}

void Controller::process()
{
	control = new QTcpSocket(this);
	control->setSocketDescriptor(socketDescriptor);
	connect(control, &QTcpSocket::readyRead, &loop, [&] {loop.exit(true); });
	connect(control, &QTcpSocket::disconnected, &loop, [&] {loop.exit(false); });
	DEB << "Controller started";
	QByteArray data;
	while (loop.exec())
	{
		while (!control->atEnd())
		{
			data.append(control->read(100));
		}
		if (data.size() < 2 || !data.contains('.'))
			continue;
		data = data.simplified();
		data.chop(1);
		if (data == "heartbeat")
		{
			send_msg("heartbeat");
		}
		else
		{
			DEB << data;
			auto r = data.split(' ');
			if (r.empty())
			{
				send_msg("failed err_cmd");
			}
			else if (r[0] == "create")
			{
				if (r.size() != 3)
				{
					send_msg("failed err_args");
				}
				else
				{
					// NOW only supports tcp connection
					if (r[1] != "tcp")
					{
						send_msg("failed only_support_tcp");
					}
					else
					{
						qintptr port = r[2].toLongLong();
						DEB << "creating sockets at " << port;
						auto key = generateRandomString(10);
						QThread* t = new QThread;
						Reversal* r = new Reversal(nullptr, port, key);
						r->moveToThread(t);
						connect(t, &QThread::started, r, &Reversal::init);
						connect(r, &Reversal::send_control, this, &Controller::send_msg);
						connect(control, &QTcpSocket::disconnected, r, &Reversal::deleteLater);
						connect(r, &Reversal::ended, t, [=] {t->quit(); t->deleteLater(); });
						pool.insert(key, r);
						t->start();
						send_msg("success " + key);
					}
				}
			}
			else if (r[0] == "close")
			{
				if (r.size() != 2)
				{
					send_msg("failed err_args");
				}
				else
				{
					if (r[1] != "0")
					{
						if (pool.contains(r[1]))
						{
							DEB << "closing sockets named " << r[1];
							pool[r[1]]->process({ "close" });
							pool.remove(r[1]);
						}
						else
						{
							send_msg("failed not_found");
						}
					}
					else
					{
						control->disconnectFromHost();
					}
				}
			}
			else if (r[0] == "connect")
			{
				if (r.size() != 2)
				{
					send_msg("failed err_args");
				}
				else
				{
					if(pool.contains(r[1]))
					{
						pool[r[1]]->process({ "connect"},control);
					}
					else
					{
						send_msg("failed not_found");
					}
				}
			}
			else
			{
				send_msg("failed cmd_not_found");
			}
		}

		data.clear();
	}
	DEB << "Connection closed or timeout(30s)";
	delete control;
	emit ended();
	control = nullptr;
	this->deleteLater();
}

void Controller::send_msg(QString str)
{
	if (control != nullptr)
	{
		control->write(str.toLocal8Bit() + "\r\n");
		control->flush();
	}
}
