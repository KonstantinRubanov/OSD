#ifndef AUDITPROCESS_H
#define AUDITPROCESS_H

#include <QObject>
#include <QDir>
#include <QString>
#include <QSqlQuery>
#include <QPair>
#include <QVariant>
#include <QRunnable>
#include <QHostInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <vector>
#include <memory>
#include <mutex>
#include <cassert>
#include "sshworker.h"
#include "osdsexception.h"

namespace osds {

typedef struct{
  QString UID;
  QString Command;
  QString AuditHost;
  QString AuditPath;
  QString SSHHost;
  QString SSHUser;
  QString SSHPass;
  QVariantMap map;
  int State;
} AuditTask;


class spin_lock
{
    std::atomic<unsigned int> m_spin ;
public:
    spin_lock(): m_spin(0) {
        lock();
    }
    ~spin_lock() {
       //assert( m_spin.load(std::memory_order_relaxed) == 0);
       unlock();
    }

    void lock()
    {
        unsigned int nCur;
        do { nCur = 0; }
        while ( !m_spin.compare_exchange_weak( nCur, 1, std::memory_order_acquire ));
    }
    void unlock()
    {
        m_spin.store( 0, std::memory_order_release );
    }
};

class AuditProcess : public QObject, public QRunnable
{
    Q_OBJECT

    void run() override
     {
        doWork();
     }
    AuditTask mTask;
public:
    explicit AuditProcess(AuditTask&);
    bool findTask(const QString mUid, AuditTask& mTask);
public slots:
    void doWork();

signals:
    void resultReady(const int& result);
};

}
#endif // AUDITPROCESS_H
