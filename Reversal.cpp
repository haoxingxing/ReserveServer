#include "Reversal.h"
#include "tools.h"

Reversal::Reversal(QObject* parent, qintptr port, QString nickname)
	: QObject(parent),port(port),nickname(nickname)
{
}

Reversal::~Reversal()
{
}

void Reversal::init()
{
	server = new QTcpServer;
	server->listen(QHostAddress::Any, port);
}

void Reversal::process(QList<QByteArray> r)
{
	if (r[0] != nickname.toLocal8Bit())
		return;
	DEB << r;
}
