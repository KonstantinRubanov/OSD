#include "protectfolderwork.h"
#include "osdsexception.h"
#include "databaseworker.h"

namespace osds {
  ProtectFolderWork::ProtectFolderWork(const ObjProtect& mObjProtec)
  {
      pObjProtect = mObjProtec;
  }
  void ProtectFolderWork::doWork(const ObjProtect& pObjProtect) {
      ObjProtect mObjProtect = pObjProtect;
      qWarning()<<"::doWork init...";
      ret_thread pret_thread;
      pret_thread.result_code    = -1;
      pret_thread.result_message = "";
      QString mCOmmand = "SAVE_PATH=\"" + mObjProtect.mFolder + "\"; if [ ! -d \"$SAVE_PATH\" ]; then  sudo mkdir -p \"$SAVE_PATH\"; fi;TARGET_PATH=\"" + mObjProtect.mTempSavePath + "/" + mObjProtect.mHost + "\"; if [ ! -d \"$TARGET_PATH\" ]; then  sudo mkdir -p \"$TARGET_PATH\"; fi; sudo tar czvf  \"$TARGET_PATH/" + mObjProtect.mFileArc + "\" /./$SAVE_PATH >/dev/null";
      qWarning()<<mCOmmand;
      /*SSH*/         
      try{
         qWarning()<<"ssh (SSHWorkerObj start...";
         SSHWorker SSHWorkerObj(mObjProtect.mIP, mObjProtect.mSShUser,  mObjProtect.mSShPass);
         qWarning()<<"DatabaseWorker start...";
         DatabaseWorker DatabaseWorker_o(this);
        if(SSHWorkerObj.ExecCommandOnHost_b(QString(mCOmmand))){
          QStringList SHlst = SSHWorkerObj.GetLastOutput_lst();
          qWarning()<<"ssh result"<<SHlst;
          QSqlQuery Query_lst;
          QString   ErrorText_str;
          qWarning()<<"start doFtpSend...";
          if(doFtpSend(SSHWorkerObj, mObjProtect, ErrorText_str)){
             qWarning()<<"start doFtpSend...complete";
            if(DatabaseWorker_o.QueryExec_lstExt(QString("UPDATE  hosts_save_history SET id_state_process=0, time_stop=(datetime('now','localtime')), errors='" + SSHWorkerObj.GetLastError_str() + "' WHERE  uid_process='" +  mObjProtect.mUid + "' "), false, Query_lst)){
               pret_thread.result_code    = 0;
               qWarning()<<"dbase state...complete";
            }
            else{
              pret_thread.result_code    = 3;
              qWarning()<<"::hosts_save_history...update errors";
            }
          }
          else{
            qWarning()<<"ProtectFolderWork::doFtpSend error: " << ErrorText_str;
            if(DatabaseWorker_o.QueryExec_lstExt(QString("UPDATE  hosts_save_history SET id_state_process=4, time_stop=(datetime('now','localtime')), errors='" + ErrorText_str + "' WHERE  uid_process='" +  mObjProtect.mUid + "' "), false, Query_lst))
              pret_thread.result_code    = 4;
            else{
              pret_thread.result_code    = 3;
              qWarning()<<"::hosts_save_history...update errors";
            }
          }
          //qWarning()<<"::doWork SSH: " <<SSHWorkerObj.GetLastOutput_lst();
        }
        else{
           qWarning()<<"dbase fail state send FTP...";
           QSqlQuery Query_lst;
           if(DatabaseWorker_o.QueryExec_lstExt(QString("UPDATE  hosts_save_history SET id_state_process=1, time_stop=(datetime('now','localtime')) WHERE  uid_process='" +  mObjProtect.mUid + "' "), false, Query_lst)){
             pret_thread.result_code  = 2;
             qWarning()<<"dbase fail state send FTP...complete";
           }
           else{
             pret_thread.result_code  = 3;
             qWarning()<<"::hosts_save_history...update errors";
           }
           qWarning()<<"::doWork SSH err: " <<SSHWorkerObj.GetLastError_str();
        }
      }
      catch(OSDSException & e) {
          pret_thread.result_code    = 1;
          pret_thread.result_message = e.what();
          qWarning()<<"::doWork init error..."<<e.what();
      }
      emit resultReady(pret_thread);
  }


  bool ProtectFolderWork::doFtpSend(SSHWorker& SSHWorkerO, const ObjProtect& pObjProtect, QString& ErrorText_str) {
    QString State_str     = "";
    int ExitStatus_i = 0, IsDone_b = true;
    qWarning()<<"::Curl init.. ";
    if(SSHWorkerO.ExecCommandOnHost_b("sudo curl 2>&1", ExitStatus_i)) {
        QStringList SSHlst = SSHWorkerO.GetLastOutput_lst();
        QString strData = SSHlst.join("");
        QRegExp rx("\\b(command not found)\\b");
        if(rx.indexIn(strData) > 0){
            State_str = "error_no_curl";
            ErrorText_str = "curl is not installed!";
            IsDone_b = false;
        }
        qWarning()<<"::Curl init..complete "<<strData;
    }
   // SSHWorkerO.ExecCommandOnHost_b("sudo curl ftp://" + pObjProtect.mFtpUser + ":" + pObjProtect.mFtpPass + "@" + pObjProtect.mFtpSavePath + " --ftp-create-dirs", ExitStatus_i);
    QString FtpSend = QString("cd '" + pObjProtect.mTempSavePath  + "/" + pObjProtect.mHost + "'  && sudo curl -T './" + pObjProtect.mFileArc + "' ftp://" + pObjProtect.mFtpSavePath + " --user " + pObjProtect.mFtpUser + ":" + pObjProtect.mFtpPass + " 2>&1");
    qWarning()<<"::Curl "<<FtpSend<<endl;
    if(SSHWorkerO.ExecCommandOnHost_b(FtpSend, ExitStatus_i)) {
        QStringList SSHlst = SSHWorkerO.GetLastOutput_lst();
        QString strData = SSHlst.join("");
        qWarning()<<"::Curl "<<strData;
        const QString arr[] = {
          /* "\\b(Access denied)\\b",
          "\\b(Could not resolve host)\\b",*/
          "^.*(curl)\\b"
        };
        QRegExp rx("\\b(Received)\\b");
        qWarning()<<"::Curl "<<rx.indexIn(strData);
        if(rx.indexIn(strData) > 0){
        }
        else{
            State_str = "error_no_upload ftp";
            ErrorText_str = "curl not upload";
            IsDone_b = false;
        }
        for(auto& p: arr){
            QRegExp rcx(p);
            qWarning()<<"::Curl "<<rcx.indexIn(strData);
            if(rcx.indexIn(strData) >= 0){
                State_str = "error_no_upload ftp";
                ErrorText_str = "curl not upload";
                IsDone_b = false;
            }
        }



      if (ExitStatus_i == 1) { // curl не установлен
        State_str = "error_no_curl";
        ErrorText_str = "curl is not installed!";
        IsDone_b = false;
      } else if (ExitStatus_i == 6) { // Доступ к FTP невозможен
        State_str = "error_ftp_denied";
        ErrorText_str = "ftp server: access denied!";
        IsDone_b = false;
      }
    } else {
      State_str = "error";
      qWarning()<<"ControllerBackup:: send file to FTP is failed!";
      ErrorText_str = "Send file to FTP is failed!";
      IsDone_b = false;
    }
    if(!IsDone_b){
       qWarning()<<"ProtectFolderWork::doFtpSend error: " << ErrorText_str;
    }
    return IsDone_b;
 }

}
