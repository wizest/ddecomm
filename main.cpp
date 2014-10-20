#include <QCoreApplication>
#include <QThread>

#include "logger.h"
#include "ddecomm.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Logger logger;
    DdeComm* comm = DdeComm::getInstance();
    QObject::connect(comm, SIGNAL(log(QString)), &logger, SLOT(log(QString)), Qt::QueuedConnection);

    QString ret = comm->request("VBDDESV", "LinkTopic", "TextBox");
    logger.log(ret);

    comm->poke(QString("VBDDESV"), QString("LinkTopic"), QString("TextBox"), QString("1234마바사다"));
    comm->execute(QString("VBDDESV"), QString("LinkTopic"), QString("12345678"));

    unsigned long conv = comm->openConversation(QString("VBDDESV"), QString("LinkTopic"));
    comm->advise(conv, QString("TextBox"));

//    QThread thread;
//    QObject::connect(&thread, SIGNAL(started()), &mon, SLOT(initiate()));

//    mon.moveToThread(&thread);
//    thread.start();

    return a.exec();
}
