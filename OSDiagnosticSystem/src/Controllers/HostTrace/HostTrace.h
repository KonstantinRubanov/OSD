#ifndef HOSTTRACE_H
#define HOSTTRACE_H

#include <QObject>
#include <QObject>
#include <QDir>
#include <QString>
#include <QSqlQuery>
#include <QPair>
#include <QRunnable>

#include "sshworker.h"
#include "httpsessionstore.h"
#include "osdsexception.h"
#include "osdsapi.h"
#include "validation.h"
#include "databaseworker.h"

namespace osds {

typedef struct {
    QString mHost;

    QString mSShUser;
    QString mSShPass;

} TraceDef;

class HostTrace : public QObject, public QRunnable
{
    Q_OBJECT

    void run() override
     {
        doWork(mObjDevice);
     }

public:
    explicit HostTrace(const TraceDef& ObjDev);

public slots:
    void doWork(const TraceDef& ObjDev);

signals:
    void resultReady(const int &result);

private:
   TraceDef mObjDevice;
};

}

#endif // HOSTTRACE_H

