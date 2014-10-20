#ifndef DDECOMM_H
#define DDECOMM_H

#include <QObject>
#include <QMutex>
#include <QByteArray>

/// @brief      DDE client
/// @author     Sanghoon Kim <wizest@gmail.com>
/// @date       2014-10-19
/// @note       Only support CF_TEXT type
class DdeComm : public QObject
{
    Q_OBJECT
private:
    QMutex mSync;
    unsigned long mDdeInstance;

public:
    static DdeComm* getInstance() {
        static DdeComm instance;
        return &instance;
    }

private:
    explicit DdeComm(QObject *parent = 0);
    ~DdeComm();

    void initialize();
    void release();

public:
    unsigned long _getDdeIdInst() {
        return mDdeInstance;
    }

public:
    QString request(QString application, QString topic, QString item);
    void poke(QString application, QString topic, QString item, QString text);
    void execute(QString application, QString topic, QString command);
    void advise(unsigned long conversation, QString item);
    void unadvise(unsigned long conversation, QString item);
    unsigned long open(QString application, QString topic); // conversation to advise
    void close(unsigned long conversation); // conversation to advise

signals:
    void requested(unsigned long conversation, QString topic, QString item, QString text);
    void poked(unsigned long conversation, QString topic, QString item, QString text);
    void executed(unsigned long conversation, QString topic, QString command);
    void advised(unsigned long conversation, QString topic, QString item, QString text);
    void adviceUpdated(unsigned long conversation, QString item, bool started);
    void opened(unsigned long conversation, QString application, QString topic);
    void closed(unsigned long conversation);

    void log(QString msg);
};

#endif // DDECOMM_H
