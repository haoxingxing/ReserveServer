#include "Controller.h"
#include "tools.h"
#include <utility>

Controller::Controller(QObject* parent, qintptr socketDescriptor, QString nickname)
	: QObject(parent), nickname(std::move(nickname)), socketDescriptor(socketDescriptor)
{
}

QMap<QString, QThread*> Controller::pool;

Controller::~Controller()
{
	DEB << "Controller ended";
}

void Controller::process()
{
	control = new QTcpSocket(this);
	control->setSocketDescriptor(socketDescriptor);
	DEB << "Controller started";
	QByteArray data;
	while (control->waitForReadyRead())
	{
		while (!control->atEnd())
		{
			data.append(control->read(100));
		}
		if (data.size() < 2 || data.right(2) != "\r\n")
			continue;
		data.chop(2);
		if (data == "heartbeat")
		{
			control->write("heartbeat\r\n");
			control->flush();
		}
		else
		{
			DEB << data;
			auto r = data.split(' ');
			if (r.size() < 1)
			{
				control->write("failed err_cmd\r\n");
				control->flush();
			}
			else if (r[0].length() == 10)
			{
				emit worker_msg(r);
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
						connect(t, &QThread::started, r, &Reversal::init);
						connect(r, &Reversal::send_control, this, &Controller::send_msg, Qt::DirectConnection);
						connect(this, &Controller::worker_msg, r, &Reversal::process, Qt::DirectConnection);
						r->moveToThread(t);
						pool.insert(key, t);
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
						DEB << "closing sockets named " << r[1];
						this->send_msg("failed not_support");
					}
					else
					{
						control->disconnectFromHost();
					}
				}
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
