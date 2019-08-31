#pragma once
#include <qlist.h>
#include <QTime>
#include <QThread>
#define DEB qDebug().nospace() << "[" << nickname << "] "

inline QList<int> generateUniqueRandomNumber(int nums, int size)
{
	QList<int> numbersList;
	QTime t;
	t = QTime::currentTime();
	qsrand(t.msec() + t.second() * 1000);
	QString buf;
	for (int i = 0; i < nums; i++)
	{
		int test = qrand() % size;
		buf.append(QString::number(test) + " ");
		QThread::usleep(1);
		numbersList.push_back(test);
	}
	return numbersList;

}
inline QString generateRandomString(int len)
{
	static QByteArray wordlist("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890");
	auto x = generateUniqueRandomNumber(len, wordlist.size());
	QString b;
	for (int a : x)
	{
		b.push_back(wordlist.at(a));
	}
	return b;
}