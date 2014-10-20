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
    QString request(QString application, QString topic, QString item);
    void poke(QString application, QString topic, QString item, QString text);
    void execute(QString application, QString topic, QString command);

    unsigned long openConversation(QString application, QString topic);
    void closeConversation(unsigned long conversation);

    void advise(unsigned long conversation, QString item);
    void unadvise(unsigned long conversation, QString item);

signals:
    void advised(unsigned long conversation, QString item, QString text);

    void log(QString msg);   
};

#endif // DDECOMM_H
