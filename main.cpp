#include <QCoreApplication>
#include <QThread>

//#include "task.h"

#include "logger.h"
#include "ddecomm.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Logger logger;
    DdeComm mon("KBSTARDS", "eds");
    QThread thread;

    QObject::connect(&mon, SIGNAL(log(QString)), &logger, SLOT(log(QString)));
    QObject::connect(&thread, SIGNAL(started()), &mon, SLOT(initiate()));

    mon.moveToThread(&thread);
    thread.start();

    return a.exec();
}
