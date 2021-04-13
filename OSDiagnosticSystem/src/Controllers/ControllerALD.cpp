#include <QDir>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QProcess>
#include <QTextCodec>
#include <QHostInfo>
#include <QSqlQuery>
#include <QDateTime>
#include <QMutex>
#include <QThreadPool>

#include "ControllerALD.h"
#include "ControllerLogin.h"
#include "httpsessionstore.h"
#include "osdsapi.h"
#include "validation.h"
#include "constants.h"
#include "osdsexception.h"
#include "dbqueriesstrings.h"
#include "HostTrace.h"


using namespace stefanfrings;

namespace osds {

  QString ControllerALD::_Username_str = "";
  QString  ControllerALD::_Password_str = "";

  QObject * CallbackCreateControllerALD_po()
  {
    return new ControllerALD;
  }

  ControllerALD::ControllerALD(QObject * parent) : ControllerAbstract(parent)
  {
    _ControllerName_str = "ControllerALD";
  }

  void ControllerALD::service(HttpRequest& request, HttpResponse& response)
  {
    /*ALD RAW users and groups*/

    if(request.getParameterMap().contains("userRawAld")) {
     if (request.getParameter("userRawAld") == "userlist"){
       QVariantMap resourceNode;
       QString Command_str = "user-list";
       int RetCode = -1;
       int cntUsers = 0;
       QStringList user_list = osdsapi->GetValidation_po()->ALDProcess_lst(Command_str, RetCode);
       user_list.removeAll(QString(""));
       QVariantMap map;
       QVariantMap hostNode;
       map.insert("version", "1.1.1.1");
       map.insert("object", 0);
       map.insert("action", "");
       for (auto pKey: user_list){
         hostNode.insert(QString::number(cntUsers).toUtf8(), pKey.toUtf8());
         cntUsers++;
       }
       if(RetCode != 0){
         map.insert("state",  QString::number(RetCode).toUtf8());
         QString codeErr(("Ошибка: " + QString::number(RetCode).toUtf8() + "\r"));
         hostNode.insert("update", codeErr.toUtf8());
       }
       else{
         map.insert("state", 0);
         if(cntUsers)
           map.insert("data", hostNode);
       }
       QJsonObject json = QJsonObject::fromVariantMap(map);
       QJsonDocument JsonDocument_o(json);
       //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
       if (!JsonDocument_o.isNull()) {
         response.setHeader("Content-Type", "text/html; charset=UTF-8");
         response.write(JsonDocument_o.toJson());
       }
       if(RetCode != 0)
         qWarning()<<"Command:"<<Command_str<<" unsuccess ret code:"<<RetCode;
     }
    }

    if(request.getParameterMap().contains("groupRawAld")) {
     if (request.getParameter("groupRawAld") == "grouplist"){
       QVariantMap resourceNode;
       QString Command_str = "group-list";
       int RetCode = -1;
       int cntUsers = 0;
       QStringList user_list = osdsapi->GetValidation_po()->ALDProcess_lst(Command_str, RetCode);
       user_list.removeAll(QString(""));
       QVariantMap map;
       QVariantMap hostNode;
       map.insert("version", "1.1.1.1");
       map.insert("object", 0);
       map.insert("action", "");
       for (auto pKey: user_list){
         hostNode.insert(QString::number(cntUsers).toUtf8(), pKey.toUtf8());
         cntUsers++;
       }
       if(RetCode != 0){
         map.insert("state",  QString::number(RetCode).toUtf8());
         QString codeErr(("Ошибка: " + QString::number(RetCode).toUtf8() + "\r"));
         hostNode.insert("update", codeErr.toUtf8());
       }
       else{
         map.insert("state", 0);
         if(cntUsers)
           map.insert("data", hostNode);
       }
       QJsonObject json = QJsonObject::fromVariantMap(map);
       QJsonDocument JsonDocument_o(json);
       //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
       if (!JsonDocument_o.isNull()) {
         response.setHeader("Content-Type", "text/html; charset=UTF-8");
         response.write(JsonDocument_o.toJson());
       }
       if(RetCode != 0)
         qWarning()<<"Command:"<<Command_str<<" unsuccess ret code:"<<RetCode;
     }
    }


    /**/
      if(request.getParameterMap().contains("hostsAld")) {
       if (request.getParameter("hostsAld") == "hostlist"){
         DatabaseWorker DatabaseWorker_o(this);
         QSqlQuery Query_lstIns;
         static int test_db = 0;
         if(!test_db) InitTables();
         test_db = 1;

         if(DatabaseWorker_o.QueryExec_lstExt(QString("DELETE FROM hosts_ald_alive"), false, Query_lstIns)){
           qWarning()<<"hosts_ald_alive atsrt..."<<test_db;
         }

         QVariantMap resourceNode;
         QString Command_str = "host-list";
         int RetCode = -1;
         int xntHosts = 0;
         QStringList Host_list = osdsapi->GetValidation_po()->ALDProcess_lst(Command_str, RetCode);
         Host_list.removeAll(QString(""));
         QVariantMap map;
         QVariantMap hostNode;
         map.insert("version", "1.1.1.1");
         map.insert("object", 0);
         map.insert("action", "");
         for (auto pKey: Host_list){
           QVariantMap mapNode;
           mapNode.insert("host_hame", pKey);

           //alive pool
           if(DatabaseWorker_o.QueryExec_lstExt(QString("INSERT INTO hosts_ald_alive (hostname,id_state,time_stop) VALUES('" + pKey + "','-1', (datetime('now','localtime')));"), false, Query_lstIns)){
             qWarning()<<"hosts_ald_alive insert in table..."<<pKey;
             TraceDef mTraceDef;
             mTraceDef.mHost    = pKey;
             mTraceDef.mSShUser =  ControllerALD::_Username_str;
             mTraceDef.mSShPass =  ControllerALD::_Password_str;
             QThreadPool::globalInstance()->start(new HostTrace(mTraceDef));
             qWarning()<<"hosts_ald_alive move to thread complete..."<<pKey;
           }
           //alive pool

           QHostInfo info = QHostInfo::fromName(pKey);
           const auto addresses = info.addresses();
           //for (const QHostAddress &address : addresses)
           QHostAddress address = info.addresses().first();
           mapNode.insert("host_ip", address.toString());
           mapNode.insert("host_path", "/");
           mapNode.insert("host_alive", "");
           Command_str = QString("host-get --stat " + pKey);
           QStringList HostInfListNode = osdsapi->GetValidation_po()->ALDProcess_lst(Command_str, RetCode);
           HostInfListNode.removeAll(QString(""));
           int iParams = 0;
           QVariantMap mapNodeVal;
           for (auto& pKeyNode: HostInfListNode){
             mapNodeVal.insert(QString::number(iParams).toUtf8(), pKeyNode);
             iParams++;
           }
           if(iParams)  mapNode.insert("details", mapNodeVal);
           hostNode.insert(QString::number(xntHosts).toUtf8(), mapNode);
           xntHosts++;
         }

         if (RetCode != 0){
           map.insert("state",  QString::number(RetCode).toUtf8());
           QString codeErr(("Ошибка: " + QString::number(RetCode).toUtf8() + "\r"));
           QString errInfo = "";
           switch(RetCode){
           case 101: errInfo="Неизвестная ошибка."; break;
           case 102: errInfo="Внутренняя ошибка."; break;
           case 103: errInfo="Ошибка выполнения операции Kerberos."; break;
           case 104:  errInfo="Ошибка выполнения операции LDAP"; break;
           case 105:  errInfo="Ошибка исполнения внешней команды."; break;
           case 106:  errInfo="Конфигурационная ошибка."; break;
           case 107:  errInfo="Ошибка условий выполнения."; break;
           case 108:  errInfo="Ошибка выполнения команд с опцией --force (недостаток данных)."; break;
           case 109:  errInfo="Объект не найден."; break;
           case 110:  errInfo="Отказ пользователя при выполнении команды."; break;
           case 111:  errInfo="Ошибка ввода/вывода при работе с файлами."; break;
           case 112:  errInfo="Неверное использование."; break;
           case 113:  errInfo="Доступ запрещен (недостаточно полномочий для выполнения)."; break;
           default:   errInfo="Неизвестная ошибка"; break;
           }
           codeErr += errInfo;
           resourceNode.insert("update", codeErr.toUtf8());
           map.insert("data", resourceNode);
         }
         else
            map.insert("state",  0);
         if(xntHosts)
           map.insert("data", hostNode);
         QJsonObject json = QJsonObject::fromVariantMap(map);
         QJsonDocument JsonDocument_o(json);
         //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
         if (!JsonDocument_o.isNull()) {
           response.setHeader("Content-Type", "text/html; charset=UTF-8");
           response.write(JsonDocument_o.toJson());
           //-------------------
           /*
           {
               bool state = false;
               try{
                 SSHWorker SSHWorkerObj("127.0.0.1",ControllerALD::_Username_str,ControllerALD::_Password_str);
                 QString PingResCommand = "sudo timeout 2 sudo traceroute 127.0.0.1  2>/dev/null | grep -Po  \"(?<=\\bms).*\"";

                 if(SSHWorkerObj.ExecCommandOnHost_b(QString(PingResCommand + " 2>&1"))){
                    QStringList SSHlst = SSHWorkerObj.GetLastOutput_lst();
                    SSHlst.removeAll(QString(""));
                    if(SSHlst.size())
                      state = true;
                  }
                  state = false;
               }
               catch(OSDSException & e) {
                 state = false;
                 qWarning()<<"SSH ERROR = "<< e.what();
               }
               if(!state) RestartSSh();
           }
           */
           //-------------------
         }
         if(RetCode != 0)
           qWarning()<<"Command:"<<Command_str<<" unsuccess ret code:"<<RetCode;
       }
      }
    /**/

   /* if(request.getParameterMap().contains("hostsAld")) {
     if (request.getParameter("hostsAld") == "hostlist"){
       QVariantMap resourceNode;
       QStringList Host_list;
       QVariantMap map;
       QVariantMap hostNode;
       int RetCode = -1;
       int xntHosts = 0;

       SSHWorker SSHWorkerObj("127.0.0.1",ControllerALD::_Username_str, ControllerALD::_Password_str);
       QString Command_str = "sudo ald-admin  host-list --pass-file=/tmp/osdsfile.pas";
       try{
         if(SSHWorkerObj.ExecCommandOnHost_b(Command_str)){
           Host_list = SSHWorkerObj.GetLastOutput_lst();
           Host_list.removeAll(QString(""));
           //rot data
           map.insert("version", "1.1.1.1");
           map.insert("object", 0);
           map.insert("action", "");
           for (auto pKey: Host_list){
             QVariantMap mapNode;
             mapNode.insert("host_hame", pKey);
             QHostInfo info = QHostInfo::fromName(pKey);
             const auto addresses = info.addresses();
             QHostAddress address = info.addresses().first();
             mapNode.insert("host_ip", address.toString());
             mapNode.insert("host_path", "/");             
             //alive

             QString PingResCommand = "sudo timeout 1  ping  -c 1  '" + pKey + "' | grep \".*ttl=.*time=[[:digit:]].[[:digit:]].*ms\"";
             if(SSHWorkerObj.ExecCommandOnHost_b(PingResCommand)){
               QStringList SSHlst = SSHWorkerObj.GetLastOutput_lst();
               SSHlst.removeAll(QString(""));
               if(SSHlst.size())
                 mapNode.insert("host_alive", "OK");
               else
                 mapNode.insert("host_alive", "DISCONNECT");
             }
             else
               mapNode.insert("host_alive", "UNKNOWN");

             mapNode.insert("host_alive", "OK");
             //chek info host
             QString Command_str_get = "sudo ald-admin host-get --stat \"" + pKey + "\" --pass-file=/tmp/osdsfile.pas";
             if(SSHWorkerObj.ExecCommandOnHost_b(Command_str_get)){
               QStringList HostInfListNode = SSHWorkerObj.GetLastOutput_lst();
               HostInfListNode.removeAll(QString(""));
               int iParams = 0;
               QVariantMap mapNodeVal;
               for (auto& pKeyNode: HostInfListNode){
                 mapNodeVal.insert(QString::number(iParams).toUtf8(), pKeyNode);
                 iParams++;
               }
               if(iParams)  mapNode.insert("details", mapNodeVal);
             }
             hostNode.insert(QString::number(xntHosts).toUtf8(), mapNode);
             xntHosts++;
           }
           RetCode = 0;
         }
       }
       catch(OSDSException & e) {
         RetCode =  e.code();
         qWarning()<<"SSH ERROR = "<< e.what();
       }
       //-------------------------------------------------
       if (RetCode != 0){
         map.insert("state",  QString::number(RetCode).toUtf8());
         QString codeErr(("Ошибка: " + QString::number(RetCode).toUtf8() + "\r"));
         QString errInfo = "";
         switch(RetCode){
         case 101: errInfo="Неизвестная ошибка."; break;
         case 102: errInfo="Внутренняя ошибка."; break;
         case 103: errInfo="Ошибка выполнения операции Kerberos."; break;
         case 104:  errInfo="Ошибка выполнения операции LDAP"; break;
         case 105:  errInfo="Ошибка исполнения внешней команды."; break;
         case 106:  errInfo="Конфигурационная ошибка."; break;
         case 107:  errInfo="Ошибка условий выполнения."; break;
         case 108:  errInfo="Ошибка выполнения команд с опцией --force (недостаток данных)."; break;
         case 109:  errInfo="Объект не найден."; break;
         case 110:  errInfo="Отказ пользователя при выполнении команды."; break;
         case 111:  errInfo="Ошибка ввода/вывода при работе с файлами."; break;
         case 112:  errInfo="Неверное использование."; break;
         case 113:  errInfo="Доступ запрещен (недостаточно полномочий для выполнения)."; break;
         default:   errInfo="Неизвестная ошибка"; break;
         }
         codeErr += errInfo;
         resourceNode.insert("update", codeErr.toUtf8());
         map.insert("data", resourceNode);
       }
       else
          map.insert("state",  0);
       if(xntHosts)
         map.insert("data", hostNode);
       QJsonObject json = QJsonObject::fromVariantMap(map);
       QJsonDocument JsonDocument_o(json);
       //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
       if (!JsonDocument_o.isNull()) {
         response.setHeader("Content-Type", "text/html; charset=UTF-8");
         response.write(JsonDocument_o.toJson());
       }
       if(RetCode != 0)
         qWarning()<<"Command:"<<Command_str<<" unsuccess ret code:"<<RetCode;
     }
    }*/
    else if(request.getParameterMap().contains("users") && request.getParameter("users") == "load") {

    }
  }

  void ControllerALD::SetSettings_sv(QSettings & Settings_ro)
  {
    SetSshParams(Settings_ro.value("username").toString(), Settings_ro.value("password").toString());

  }

  bool ControllerALD::RestartSSh(){
      //restart ssh
      try{
        QString Output_str;
        QString Command_str = QString("sudo service ssh restart | sudo service sshd restart");
        QProcess Process_o(this);
        Process_o.start(Command_str);
        if(!Process_o.waitForStarted(10000) || !Process_o.waitForFinished(-1)) {
          Output_str = QString::fromUtf8(Process_o.readAllStandardOutput());
          qWarning()<<"ControllerALD::start error..."<<Output_str;
          return false;
        }
        qWarning()<<"ControllerALD::start success...";
        return true;
       }
      catch(OSDSException & e) {
        qWarning()<<"ControllerALD::start ERROR = "<< e.what();
      }
  }


  void ControllerALD::InitializeDataBase_sv()
  {

  }

  bool ControllerALD::InitTables(){
      try {
        qWarning()<<"tables db save init...";
        DatabaseWorker DatabaseWorker_o(this);
        QSqlQuery Query_lst;
        for(auto& pSql: InitDBSave){
          if(DatabaseWorker_o.QueryExec_lstExt(pSql, false, Query_lst))
            qWarning()<<"tables db save init end"<<endl;
          else
            qWarning()<<"tables db save init errors..."<<endl;
        }
        return true;
      } catch (OSDSException & e) {
          qWarning()<<"tables db save init errors..."<<endl;
          qWarning()<<""<<e.what();
          return false;
      }
  }


  void ControllerALD::SetSshParams(const QString _Username_, const QString _Password_){
      ControllerALD::_Username_str = _Username_;
      ControllerALD::_Password_str = _Password_;
  }
}
