#ifndef TASK_H
#define TASK_H

#include <QThread>

class Task : public QThread
{
    Q_OBJECT
public:
    Task(QObject *parent = 0) : QThread(parent) {}

public slots:
    void run();

signals:
    void log(QString msg);
};

#endif // TASK_H
