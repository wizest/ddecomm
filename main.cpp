#include <QCoreApplication>
#include <QThread>
#include <string>

#include "logger.h"
#include "ddecomm.h"

#define DDEAPP      QString::fromUtf8("Excel")
#define DDETOPIC    QString::fromUtf8("[Book1]Sheet1")
#define DDEITEM     QString::fromUtf8("R3C3")

#define DDEPOKE     QString::fromUtf8("1234ABCD")

#define DDECOMMAND  QString::fromUtf8("[SELECT(\"R1C1:R2C2\")]")
//#define DDECOMMAND  QString::fromUtf8("[OPEN(\"Test.xlsx\")]")
//#define DDECOMMAND  QString::fromUtf8("[CLOSE(FALSE)]")


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Logger logger;
    DdeComm* comm = DdeComm::getInstance();
    QObject::connect(comm, SIGNAL(log(QString)), &logger, SLOT(log(QString)), Qt::QueuedConnection);

    // Request
    QString ret = comm->request(DDEAPP, DDETOPIC, DDEITEM); logger.log(QString("request ret=%1").arg(ret));
    // Poke
    comm->poke(DDEAPP, DDETOPIC, DDEITEM, DDEPOKE);
    // Execute
    comm->execute(DDEAPP, DDETOPIC, DDECOMMAND);
    // Advise
    unsigned long conv = comm->open(DDEAPP, DDETOPIC);
    comm->advise(conv, DDEITEM);

    return a.exec();
}
