#ifndef AUDITPROCESS_H
#define AUDITPROCESS_H

#include <QObject>
#include <QDir>
#include <QString>
#include <QSqlQuery>
#include <QPair>
#include <QVariant>
#include <QRunnable>
#include <memory>
#include <mutex>
#include "sshworker.h"

namespace osds {

typedef struct{
  QString AuditHost;
  QString AuditPath;
  QString SSHHost;
  QString SSHUser;
  QString SSHPass;
  QVariantMap map;
  int State;
} AuditTask;

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
public slots:
    void doWork();

signals:
    void resultReady(const int& result);
};

}
#endif // AUDITPROCESS_H
