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
    ulong mDdeInstance;

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
    ulong _getDdeIdInst() {
        return mDdeInstance;
    }
    bool isAvailable(QString application, QString topic);

public:
    QString request(QString application, QString topic, QString item);
    bool poke(QString application, QString topic, QString item, QString text);
    bool execute(QString application, QString topic, QString command);
    bool advise(ulong conversation, QString item);
    bool unadvise(ulong conversation, QString item);
    ulong open(QString application, QString topic); // conversation to advise: if the function fails, the return value is 0L.
    bool close(ulong conversation); // conversation to advise

signals:
    void requested(ulong conversation, QString topic, QString item, QString text);
    void poked(ulong conversation, QString topic, QString item, QString text);
    void executed(ulong conversation, QString topic, QString command);
    void advised(ulong conversation, QString topic, QString item, QString text);
    void adviceUpdated(ulong conversation, QString item, bool started);
    void opened(ulong conversation, QString application, QString topic);
    void closed(ulong conversation);

    void log(QString msg);
};

#endif // DDECOMM_H
