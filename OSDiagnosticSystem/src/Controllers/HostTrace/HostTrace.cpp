#include "HostTrace.h"
#include "osdsexception.h"
#include "databaseworker.h"

namespace osds {

HostTrace::HostTrace(const TraceDef& ObjDev)
{
   mObjDevice =  ObjDev;
}


void HostTrace::doWork(const TraceDef& ObjDev){
    int state = -1;
    int alive = 0;
    try{
      DatabaseWorker DatabaseWorker_o(this);
      qWarning()<<"THREAD ::HostTrace::doWork..."<<ObjDev.mHost;
      QString Command_str = QString("host-status " + ObjDev.mHost);
      QStringList HostInfListNode = osdsapi->GetValidation_po()->ALDProcess_lst(Command_str, state);

      QString strData = HostInfListNode.join("");
      QRegExp rx("(Статус: ok)");
      if(rx.indexIn(strData) > 0)
        alive = 1;
      else
        alive = 0;
      qWarning()<<"HostTrace::Alive..."<<ObjDev.mHost<<" = " <<  alive;
      QSqlQuery Query_lst;
      QString   ErrorText_str;
      qWarning()<<"HostTrace::doWork adatabase..."<<ObjDev.mHost;
        if(DatabaseWorker_o.QueryExec_lstExt(QString("UPDATE hosts_ald_alive SET id_state='" + QString::number(alive).toUtf8() + "', time_stop=(datetime('now','localtime')) WHERE  hostname='" +  ObjDev.mHost  + "' "), false, Query_lst)){
           qWarning()<<"dbase state...complete"<<ObjDev.mHost;
        }
      /*
       * DatabaseWorker DatabaseWorker_o(this);
      QSqlQuery Query_lst;

      qWarning()<<"HostTrace::HostTrace ::doWork...";

      QString Command_str = "host-list";
      QStringList Host_list = osdsapi->GetValidation_po()->ALDProcess_lst(Command_str, state);
      Host_list.removeAll(QString(""));

      SSHWorker SSHWorkerObj("127.0.0.1", ObjDev.mSShUser, ObjDev.mSShPass);

      for (auto pKey: Host_list){
        int state_trace = 0;
        try{
            QString PingResCommand = "sudo timeout 2 sudo traceroute '" + pKey + "'  2>/dev/null | sudo grep -Po  \"(?<=\\bms).*\" | grep -v \".*\\!H\"";
            qWarning()<<pKey<<" SSH traceroute = "<<PingResCommand;
            if(SSHWorkerObj.ExecCommandOnHost_b(QString(PingResCommand + " 2>&1"))){
               QStringList SSHlst = SSHWorkerObj.GetLastOutput_lst();
               qWarning()<<"Trace Result = "<<pKey<< " " <<SSHlst;
               SSHlst.removeAll(QString(""));
               if(SSHlst.size()){
                 state_trace = 1;
               }
             }
          }
          catch(OSDSException & e) {
            state_trace = 0;
            qWarning()<<pKey<<" HostTrace::doWork - SSH ERROR = "<< e.what();
          }
        qWarning()<<"HostTrace::doWork adatabase..."<<pKey;
        if(DatabaseWorker_o.QueryExec_lstExt(QString("UPDATE hosts_ald_alive SET id_state='" + QString::number(state_trace).toUtf8() + "', time_stop=(datetime('now','localtime')) WHERE  hostname='" +  pKey  + "' "), false, Query_lst)){
           qWarning()<<"dbase state...complete "<<pKey;
        }
        else
          qWarning()<<"dbase state...error "<<pKey;
      }*/

      state = 0;
    }
    catch(OSDSException & e) {
        state  = 1;
        qWarning()<<"::HostTrace::doWork error..."<<e.what();
    }
    emit resultReady(state);
}

}
