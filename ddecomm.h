#ifndef DDECOMM_H
#define DDECOMM_H

#include <QObject>
#include <QMutex>
#include <QByteArray>

class DdeContext;   // Forward declaration
class DdeComm : public QObject
{
    Q_OBJECT    
private:
    QMutex mSync;
    DdeContext* const mCtx;

public:
    explicit DdeComm(QString appName, QString topicName, QObject *parent = 0);
    ~DdeComm();

public slots:
    void initiate();
    void terminate();

//    void request();
//    void poke();

//    void advise();
//    void unadvise();

//    void execute();

signals:
    void connected(bool isConnected);
    void advised(QByteArray ddeData);

    void log(QString msg);


};

#endif // DDECOMM_H
