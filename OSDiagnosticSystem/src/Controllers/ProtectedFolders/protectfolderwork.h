#ifndef PROTECTFOLDERWORK_H
#define PROTECTFOLDERWORK_H

#include <QObject>
#include <QDir>
#include <QString>
#include <QSqlQuery>
#include <QPair>
#include "sshworker.h"

#include "ControllerAction.h"

namespace osds {

typedef struct {
    QString mIP;
    QString mHost;
    QString mFolder;
    QString mUid;
    QString mFileArc;
    int     mOperation;

    QString mSShUser;
    QString mSShPass;
    QString mTempSavePath;

    QString mFtpHost;
    QString mFtpUser;
    QString mFtpPass;
    QString mFtpSavePath;
} ObjProtect;

typedef struct{
   QString result_message;
   int result_code;
} ret_thread;

class ProtectFolderWork : public QObject, public QRunnable
{
    Q_OBJECT

    void run() override
     {
        doWork(pObjProtect);
     }

public:
    ProtectFolderWork(const ObjProtect& mObjProtec);

    public slots:
        void doWork(const ObjProtect& mObjProtect);

    signals:
        void resultReady(const ret_thread &result);

private:
    bool doFtpSend(SSHWorker& SSHWorkerObj, const ObjProtect& pObjProtect, QString& ErrorText_str);

    QString mUid;
    ObjProtect pObjProtect;
};

}

#endif // PROTECTFOLDERWORK_H
