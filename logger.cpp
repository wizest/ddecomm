#include <iostream>
#include <string>
#include <QDateTime>

#include "logger.h"

using namespace std;

Logger::Logger(QObject *parent) :
    QObject(parent)
{
}

void Logger::log(QString msg)
{
    wcout << QDateTime::currentDateTime().toString("yyMMdd-hhmmss.zzz ").toStdWString() ;
          wcout << msg.toStdWString() << endl;
}
