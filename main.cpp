/// @brief      DDE comm test
/// @author     Sanghoon Kim <wizest@gmail.com>
/// @date       2014-10-19

#include <QCoreApplication>
#include <QDebug>

#include "ddecomm.h"

#define DDEAPP      QString::fromUtf8("Excel")
#define DDETOPIC    QString::fromUtf8("[Book1]Sheet1")
#define DDEITEM     QString::fromUtf8("R1C1")
#define DDEPOKE     QString::fromUtf8("1234ABCD")
#define DDECOMMAND  QString::fromUtf8("[SELECT(\"R1C1:R2C2\")]")
//#define DDECOMMAND  QString::fromUtf8("[OPEN(\"Test.xlsx\")]")
//#define DDECOMMAND  QString::fromUtf8("[CLOSE(FALSE)]")

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    DdeComm comm;

    // Request
    QString ret = comm.request(DDEAPP, DDETOPIC, DDEITEM);
    qDebug() << ret;
    // Poke
    comm.poke(DDEAPP, DDETOPIC, DDEITEM, DDEPOKE);
    // Execute
    comm.execute(DDEAPP, DDETOPIC, DDECOMMAND);
    // Advise
    unsigned long conv = comm.open(DDEAPP, DDETOPIC);
    comm.advise(conv, DDEITEM);

    return a.exec();
}
