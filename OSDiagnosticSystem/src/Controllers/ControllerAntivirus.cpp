#include <QString>
#include <QStringList>
#include <QDir>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QThread>
#include <QHostInfo>
#include <QHostAddress>
#include <QSqlQuery>
#include <QDateTime>
#include <QMutex>

#include "ControllerAntivirus.h"
#include "ControllerLogin.h"
#include "osdsapi.h"
#include "validation.h"
#include "httpsessionstore.h"
#include "sshworker.h"
#include "osdsexception.h"
#include "databaseworker.h"
#include "fullscanprocess.h"
#include "antivupdateprocess.h"
#include "antivinstallprocess.h"
#include "antivremoveprocess.h"

using namespace stefanfrings;

namespace osds {

  QString ControllerAntivirus::_DrWebPath_str = "/tmp";
  QString ControllerAntivirus::_DrWebLicenseKey_str = "";
  QString ControllerAntivirus::_DrWebUpdateUrl_str = "";
  QString ControllerAntivirus::_KasperskyPath_str = "/tmp";
  QString ControllerAntivirus::_KasperskyLicenseKey_str = "";
  QString ControllerAntivirus::_KasperskyAutoinstall_str = "";
  QString ControllerAntivirus::_KasperskyUpdateUrl_str = "";
  QMap<QString, EnHostRunState> ControllerAntivirus::_HostScanState_smap;
  QMap<QString, EnHostRunState> ControllerAntivirus::_AntivirusUpdateState_smap;
  QMap<QString, EnHostRunState> ControllerAntivirus::_AntivirusInstallState_smap;
  QMap<QString, EnHostRunState> ControllerAntivirus::_AntivirusRemoveState_smap;
  QMutex ControllerAntivirus::_Mutex_so;

  QObject * CallbackCreateControllerAntivirus_po()
  {
    return new ControllerAntivirus;
  }

  ControllerAntivirus::ControllerAntivirus(QObject * parent) : ControllerAbstract(parent)
  {
    _ControllerName_str = "ControllerAntivirus";
  }

  void ControllerAntivirus::service(HttpRequest& request, HttpResponse& response)
  {

    try{
    // Лист всех хостов на которые можно установить антивирус
    // Формирует JSON в котором будет имя хоста, установленный ли антивирус и какой,
    // А также данные о версии антивирусов в наличии (последняя версия, если несколько)

   if(request.getParameterMap().contains("settings_dir") && request.getParameter("settings_dir") == "get_lan_settings_dir") {
       ListSaveFullSettings(response);
   }
   else if(request.getParameterMap().contains("settings_dir") && request.getParameter("settings_dir") == "replace_lan_settings_dir") {
      ListSaveFullUpdateSettings(request, response);
   }
   else if(request.getParameterMap().contains("append_dir") && request.getParameter("append_dir") == "listdirs") {
      ListSaveFullDirs(response);
   }
   else if(request.getParameterMap().contains("append_dir") &&  request.getParameter("append_dir") == "fullscan_add" && request.getParameterMap().contains("data")) {
      ListSaveFullAppendDirs(request, response);
   }
   else if(request.getParameterMap().contains("remove_dir") &&  request.getParameter("remove_dir") == "fullscan_remove" && request.getParameterMap().contains("data")) {
      ListSaveFullRemoveDirs(request, response);
   }

   else if(request.getParameterMap().contains("get") && request.getParameter("get") == "host-list") {
      GetHostsList_v(response);
    }
    else if(request.getParameterMap().contains("install") && request.getParameter("install") == "antiv") {
      if(request.getParameterMap().contains("data")) {
        qWarning()<<"Satrt install  =  ontrollerAntivirus::service(HttpRequest& request, HttpResponse& response) split...";
        QStringList Data_lst = QString(request.getParameter("data")).split(":");
        if(Data_lst.size() == 2) {
          qWarning()<<"ControllerAntivirus::service(HttpRequest& request, HttpResponse& response) split...InstallAntivirusOnHost_v";
          InstallAntivirusOnHost_v(Data_lst[0], Data_lst[1], response);
        }
        else {
          qWarning()<<"ControllerAntivirus::service->"<<"Failed request data of install antivirus on host";
        }
      } else if(request.getParameterMap().contains("host") && request.getParameterMap().contains("command")) {
        if(request.getParameter("command") == "state") { // Запрос статуса установки
          GetStateAntivirusInstall_v(request.getParameter("host"), response);
        }
      }
    } // Удаление антивируса с хоста
    else if(request.getParameterMap().contains("remove") && request.getParameter("remove") == "antiv") {
      if(request.getParameterMap().contains("data")) {
        RemoveAntivirus_v(request.getParameter("data"), response);
      } else if(request.getParameterMap().contains("host_state")) {
        GetStateAntivirusRemove_v(request.getParameter("host_state"), response);
      }
    }
    else if (request.getParameterMap().contains("scan") && request.getParameter("scan") == "host_dirs") {
      if(request.getParameterMap().contains("host") && request.getParameterMap().contains("dirs")) {
        ScanDirsOnHost_v(request.getParameter("host"), QString(request.getParameter("dirs")).split(";"), response);
      }
    } // Настройка способа сканирования хоста
    else if(request.getParameterMap().contains("scan") && request.getParameter("scan") == "host_type") {
      if(request.getParameterMap().contains("host") && request.getParameterMap().contains("type") && request.getParameterMap().contains("dir_list")) {
        SetHostType_v(request.getParameter("host"), request.getParameter("type"), request.getParameter("dir_list"), response);
      }
    } // Запрос файловой системы хоста
    else if(request.getParameterMap().contains("get") && request.getParameter("get") == "dir_list") {
      if(request.getParameterMap().contains("host") && request.getParameterMap().contains("dir")) {
        GetHostDirList_v(request.getParameter("host"), request.getParameter("dir"), response);
      }
    }// Сканирование хоста
    else if(request.getParameterMap().contains("scan") && request.getParameter("scan") == "host") {
      if(request.getParameterMap().contains("host") && request.getParameterMap().contains("command")) {
        if(request.getParameter("command") == "start") {  // Запуск сканирования

          bool is_full_lan = (request.getParameter("scan_policy") == "1")?true:false;
          ScanHost_v(request.getParameter("host"), response, is_full_lan);
        } else if (request.getParameter("command") == "state") {  //Запрос статуса сканирования
          GetStateScanHost_v(request.getParameter("host"), response);
        } else if (request.getParameter("command") == "stop") {
          StopHostScanned_v(request.getParameter("host"), response);
        }
      }
    } // Запрос журнала с последней проверки
    else if(request.getParameterMap().contains("get") && request.getParameter("get") == "scan_journal") {
      if(request.getParameterMap().contains("host")) {
        GetScanJournal_v(request.getParameter("host"), response);
      }
    }
    else if(request.getParameterMap().contains("antiv") && request.getParameter("antiv") == "update") {
      if(request.getParameterMap().contains("host") && request.getParameterMap().contains("command")) {
        if(request.getParameter("command") == "start") {
          AntivirusUpdate_v(request.getParameter("host"), response);
        } else if (request.getParameter("command") == "state") {
          GetStateAntivirusUpdate_v(request.getParameter("host"), response);
        }
      }
    }
    else if(request.getParameterMap().contains("get") && request.getParameter("get") == "license_state") {
      if(request.getParameterMap().contains("host")) {
         GetLicenseState_v(request.getParameter("host"), response);
      }
    }
      }
      catch(OSDSException & e) {
            qWarning()<<"ControllerAntivirus::service(HttpRequest& request, HttpResponse& response)->"<<e.what();
      }
  }

  void ControllerAntivirus::SetSettings_sv(QSettings & Settings_ro)
  {
    ControllerAntivirus::_DrWebPath_str = Settings_ro.value("drwebInstallPath").toString();
    ControllerAntivirus::_DrWebLicenseKey_str = Settings_ro.value("drwebKey").toString();
    ControllerAntivirus::_DrWebUpdateUrl_str = Settings_ro.value("drwebUpdateUrl").toString();
    ControllerAntivirus::_KasperskyPath_str = Settings_ro.value("kasperskyInstallPath").toString();
    ControllerAntivirus::_KasperskyLicenseKey_str = Settings_ro.value("kasperskyKey").toString();
    ControllerAntivirus::_KasperskyAutoinstall_str = Settings_ro.value("kasperskyAutoinstall").toString();
    ControllerAntivirus::_KasperskyUpdateUrl_str = Settings_ro.value("kasperskyUpdateUrl").toString();
  }

  void ControllerAntivirus::InitializeDataBase_sv()
  {
    try {
      DatabaseWorker DatabaseWorker_o;
      DatabaseWorker_o.QueryExec_lst("CREATE TABLE IF NOT EXISTS hosts_antiv(hostname text primary key, last_check text, alert int, journal text, last_update text, antiv_state text)", false);
      DatabaseWorker_o.QueryExec_lst("CREATE TABLE IF NOT EXISTS hosts_antiv_state(hostname text primary key, scan_type text, dir_list text)", false);
      DatabaseWorker_o.QueryExec_lst("CREATE TABLE IF NOT EXISTS full_antiv_type_scan(scan_policy INTEGER NOT NULL,  scan_type INTEGER NOT NULL, PRIMARY KEY (scan_policy,scan_type))", false);
      DatabaseWorker_o.QueryExec_lst("CREATE TABLE IF NOT EXISTS full_antiv_dirs_scan(scan_dir text PRIMARY KEY)", false);
    } catch(OSDSException & e) {
      qWarning()<<"ControllerAntivirus::InitializeDataBase_sv->"<<e.what();
    }
    InitializeDataBase_settings();
  }

  void  ControllerAntivirus::InitializeDataBase_settings(){
    try{
      DatabaseWorker DatabaseWorker_o;
      QSqlQuery Query_lst;
      int cntRow = 0;
      if(!DatabaseWorker_o.QueryExec_lstExt(QString("SELECT COUNT(*) FROM full_antiv_type_scan"), false, Query_lst)){
              qWarning()<<"ControllerAntivirus::InitializeDataBase_sv() init errors"<<endl;
          }
          else{
            Query_lst.first();
            if(!Query_lst.value("COUNT(*)").toString().isEmpty())
              cntRow = Query_lst.value("COUNT(*)").toInt();
            qWarning()<<"ControllerAntivirus::InitializeDataBase_sv() init = "<< cntRow <<endl;
          }
          if(!cntRow){
            QSqlQuery Query_lstIns;
            if(DatabaseWorker_o.QueryExec_lstExt("INSERT INTO full_antiv_type_scan (scan_policy, scan_type) VALUES (\"0\", \"0\")", false, Query_lstIns)){
              qWarning()<<"ControllerAntivirus::InitializeDataBase_sv->Complete!";
            }
            else
             qWarning()<<"ControllerAntivirus::InitializeDataBase_sv->ErrorInsert Stste!";
          }
          else
            qWarning()<<"ControllerAntivirus::InitializeDataBase_sv->AlredyExist!";

    } catch(OSDSException & e) {
    qWarning()<<"ControllerAntivirus::InitializeDataBase_sv->"<<e.what();
    }

  }

  void ControllerAntivirus::GetHostsList_v(HttpResponse & response)
  {
    QStringList Hosts_lst = osdsapi->GetValidation_po()->ALDProcess_lst("host-list");
    try {
      DatabaseWorker DatabaseWorker_o(this);
#if defined(__NO_ASTRA_LINUX__)
      Hosts_lst.clear();
      Hosts_lst.push_back("astra.vbox");
      Hosts_lst.push_back("astra1.vbox");
#endif  //__NO_ASTRA_LINUX__
      QJsonObject MainObject_o;
      SetJsonPackageHead_v(MainObject_o, "get_hosts", "done");
      QJsonArray DataArray_o = MainObject_o["data"].toArray();
      foreach(const QString & Name_str, Hosts_lst) {
        try {
          SAntivirusState SDrWebState_o = GetDrWebVersionOnHost_str(Name_str);
          SAntivirusState SKasperskyState_o = GetKasperskyVersionOnHost_str(Name_str);
          QJsonObject ArrayObject_o;
          ArrayObject_o["host"] = Name_str;
          ArrayObject_o["exchange"] = "accept";
          QHostInfo info = QHostInfo::fromName(Name_str);
          foreach(const QHostAddress & addr, info.addresses()) {
            ArrayObject_o["ip"] = addr.toString();
          }
          {
            QSqlQuery Query_lst = DatabaseWorker_o.QueryExec_lst("SELECT * FROM hosts_antiv WHERE hostname=\"" + Name_str + "\"");
            Query_lst.next();
            ArrayObject_o["drweb"] = SDrWebState_o._Version_str;
            ArrayObject_o["drweb_state"] = SDrWebState_o._IsAntivirusStateOk_b ? "true" : "false";
            ArrayObject_o["kaspersky"] = SKasperskyState_o._Version_str;
            ArrayObject_o["kaspersky_state"] = SKasperskyState_o._IsAntivirusStateOk_b ? "true" : "false";
            ArrayObject_o["check"] = Query_lst.value("last_check").toString();
            ArrayObject_o["journal"] = Query_lst.value("alert").toString();
            ArrayObject_o["update"] = Query_lst.value("last_update").toString();
            ArrayObject_o["scan_state"] = Query_lst.value("antiv_state").toString();
          }
          {
            QSqlQuery Query_lst = DatabaseWorker_o.QueryExec_lst("SELECT * FROM hosts_antiv_state WHERE hostname=\"" + Name_str + "\"");
            Query_lst.next();
            ArrayObject_o["scan_type"] = Query_lst.value("scan_type").toString();
            ArrayObject_o["dir_list"] = Query_lst.value("dir_list").toString();
          }
          DataArray_o.append(ArrayObject_o);
        }
        catch(OSDSException & e) {
          QJsonObject ArrayObject_o;
          ArrayObject_o["host"] = Name_str;
          ArrayObject_o["exchange"] = "discard";
          DataArray_o.append(ArrayObject_o);
          qWarning()<<"ControllerAntivirus::GetHostsList_v->"<<e.what();
        }
      }
      MainObject_o["data"] = DataArray_o;
      QJsonArray OptionsArray_o = MainObject_o["options"].toArray();
      QDir DrWebDirectory_o(_DrWebPath_str);
      DrWebDirectory_o.setFilter(QDir::Files | QDir::NoSymLinks);
      QFileInfoList DrWebFiles_lst = DrWebDirectory_o.entryInfoList(QStringList()<<"drweb-*.run", QDir::NoFilter, QDir::Time);
      foreach(const QFileInfo & FileInfo_rc, DrWebFiles_lst) {
        QJsonObject ArrayObject_o;
        ArrayObject_o["av_type"] = "drweb";
        ArrayObject_o["av_version"] = FileInfo_rc.fileName();
        OptionsArray_o.append(ArrayObject_o);
      }
      QDir KasperskyDirectory_o(_KasperskyPath_str);
      KasperskyDirectory_o.setFilter(QDir::Files | QDir::NoSymLinks);
      QFileInfoList KasFiles_lst = KasperskyDirectory_o.entryInfoList(QStringList()<<"kesl*.deb", QDir::NoFilter, QDir::Time);
      foreach(const QFileInfo & FileInfo_rc, KasFiles_lst) {
        QJsonObject ArrayObject_o;
        ArrayObject_o["av_type"] = "kaspersky";
        ArrayObject_o["av_version"] = FileInfo_rc.fileName();
        OptionsArray_o.append(ArrayObject_o);
      }
      MainObject_o["options"] = OptionsArray_o;
      response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
    }
    catch(OSDSException & e) {
      qWarning()<<"ControllerAntivirus::GetHostsList_v->"<<e.what();
    }
  }

  void ControllerAntivirus::InstallAntivirusOnHost_v(const QString & Host_str, const QString & Distrib_str, HttpResponse & response)
  {
    QString State_str = "process";

    qWarning()<<"ControllerAntivirus::InstallAntivirusOnHost_v...";
    //if( Distrib_str.indexOf("kesl") != -1 || Distrib_str.indexOf("drweb") != -1 ) {
      qWarning()<<"ControllerAntivirus::InstallAntivirusOnHost_v... StartAntivirusInstallInThread_po";
      StartAntivirusInstallInThread_po(Host_str, Distrib_str);
    /*} else {
      State_str = "error";
      qWarning()<<"ControllerAntivirus::InstallAntivirusOnHost_v... error";
    }*/
    QJsonObject MainObject_o;
    SetJsonPackageHead_v(MainObject_o, "antiv_install", State_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerAntivirus::RemoveAntivirus_v(const QString & Host_str, HttpResponse & response)
  {
    QJsonObject MainObject_o;
    QString State_str = "done";
    try {
      SAntivirusState SDrWebState_o = GetDrWebVersionOnHost_str(Host_str);
      SAntivirusState SAntivirusState_o = GetKasperskyVersionOnHost_str(Host_str);

      if(SDrWebState_o._Version_str != "0") {
        StartAntivirusRemoveInThread_po(Host_str, "drweb");
      } else if(SAntivirusState_o._Version_str != "0") {
        StartAntivirusRemoveInThread_po(Host_str, "kesl");
      }
      else {  // Ошибка
        State_str = "error";
        MainObject_o["error"] = "Антивирус не установлен";
        qDebug()<<"ControllerAntivirus::RemoveAntivirus_v->No install antivirus";
      }
    } catch(OSDSException & e) {
      State_str = "error";
      MainObject_o["error"] = "Нет подключения по SSH";
      qWarning()<<"ControllerAntivirus::RemoveAntivirus_v->"<<e.what();
    }
    SetJsonPackageHead_v(MainObject_o, "antiv_remove", State_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }


  QThread * ControllerAntivirus::StartAntivirusRemoveInThread_po(const QString & Host_str, const QString & Distrib_str)
  {
    AntivRemoveProcess * AntivProcess_po = new AntivRemoveProcess(Host_str, Distrib_str);
    _Mutex_so.lock();
    _AntivirusRemoveState_smap[Host_str] = _HRSProcess_en;
    _Mutex_so.unlock();
    QThread * ScanThread_po = new QThread;
    AntivProcess_po->moveToThread(ScanThread_po);
    connect(ScanThread_po, SIGNAL(started()), AntivProcess_po, SLOT(process()));
    connect(AntivProcess_po, &AntivRemoveProcess::Output_sig, [=]( bool IsOk_b ){
      _Mutex_so.lock();
      _AntivirusRemoveState_smap[Host_str] = IsOk_b ? _HRSStop_en : _HRSError_en;
      _Mutex_so.unlock();
    });
    connect(AntivProcess_po, SIGNAL(finished()), ScanThread_po, SLOT(quit()));
    connect(AntivProcess_po, SIGNAL(finished()), AntivProcess_po, SLOT(deleteLater()));
    connect(ScanThread_po, SIGNAL(finished()), ScanThread_po, SLOT(deleteLater()));
    ScanThread_po->start(); // Запуск сканирования хостов (в потоках)
    return ScanThread_po;
  }

  void ControllerAntivirus::GetStateAntivirusRemove_v(const QString & Host_str, HttpResponse & response)
  {
    QString Status_str = "process";
    QJsonObject MainObject_o;
    _Mutex_so.lock();
    EnHostRunState RunState_en = _AntivirusRemoveState_smap[Host_str];
    _Mutex_so.unlock();
    if(RunState_en == _HRSStop_en) {
      Status_str = "stop";
    }
    SetJsonPackageHead_v(MainObject_o, "antiv_remove",  Status_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerAntivirus::ScanDirsOnHost_v(const QString & Host_str, QStringList Dirs_lst, stefanfrings::HttpResponse& response)
  {
    try {
      bool IsDone_b = true;
      QJsonObject MainObject_o;
      QJsonArray DataArray_o = MainObject_o["data"].toArray();
      SSHWorker SSHWorker_o(Host_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
      if(GetDrWebVersionOnHost_str(Host_str)._Version_str != "0") {  // Если установлен Dr.Web
        if(!SSHWorker_o.ExecCommandOnHost_b("sudo drweb-ctl " + Dirs_lst.join(" "))) {
          qWarning()<<"ControllerAntivirus::ScanDirsOnHost_v->"<<"Dr.Web scan path crashed";
          IsDone_b = false;
        }
        foreach(const QString & Output_str, SSHWorker_o.GetLastOutput_lst()) {
          if(Output_str.contains("infected")) { // Если есть зараженные файлы
            QJsonObject ArrayObject_o;
            ArrayObject_o["infected"] = Output_str;
            DataArray_o.append(ArrayObject_o);
          }
        }
      }
      else if (GetKasperskyVersionOnHost_str(Host_str)._Version_str != "0") {  // Если установлен kaspersky

         QString TempDirList_str;
         foreach(const QString & Path_str, Dirs_lst) {
            TempDirList_str += "\"" + Path_str + "\" ";
         }
         if( SSHWorker_o.SendFileToHost_b("/opt/asb/scripts/scan_tmpl.sh", "/tmp") == false) {
             qWarning()<<"ERROR copy ssh file: scan_tmpl.sh";
         }
         QString TemplScan = "kesl=\"1\"; sudo /bin/sh  /tmp/scan_tmpl.sh \"" + TempDirList_str + " \"";

         //TemplScan = "kesl-control --scan-file "  +  TempDirList_str;
         qWarning()<<TemplScan;
         if(!SSHWorker_o.ExecCommandOnHost_b(TemplScan + " 2>&1")) {
          qWarning()<<"ControllerAntivirus::ScanDirsOnHost_v->"<<"Kaspersky scan path crashed";
          IsDone_b = false;
        }
      }
      MainObject_o["data"] = DataArray_o;
      SetJsonPackageHead_v(MainObject_o, "scan_antiv", IsDone_b ? "done" : "error");
      response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
    }
    catch(OSDSException & e) {
      qWarning()<<"ControllerAntivirus::ScanDirsOnHost_v->"<<e.what();
      QJsonObject MainObject_o;
      SetJsonPackageHead_v(MainObject_o, "scan_antiv", "error");
      response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
    }
  }

  void ControllerAntivirus::SetHostType_v(const QString & Host_str, const QString & Type_str, const QString & DirList_str, HttpResponse & response)
  {
    QJsonObject MainObject_o;
    try {
      DatabaseWorker DatabaseWorker_o;
      DatabaseWorker_o.QueryExec_lst("INSERT INTO hosts_antiv_state VALUES(\"" + Host_str + "\", \"" + Type_str + "\", \"" + DirList_str + "\")", false);
      DatabaseWorker_o.QueryExec_lst("UPDATE hosts_antiv_state SET scan_type=\"" + Type_str + "\", dir_list=\"" + DirList_str +
                                     "\" WHERE hostname = \"" + Host_str + "\" ");
      SetJsonPackageHead_v(MainObject_o, "host_type", "done");
    } catch (OSDSException & e) {
      qWarning()<<"ControllerAntivirus::SetHostType_v->"<<e.what();
      SetJsonPackageHead_v(MainObject_o, "host_type", "error");
    }
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerAntivirus::GetHostDirList_v(const QString & Host_str, const QString & Dir_str, HttpResponse & response)
  {
    QJsonObject MainObject_o;
    try {
      SSHWorker SSHWorker_o(Host_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
      QJsonArray DataArray_o = MainObject_o["data"].toArray();
      if(SSHWorker_o.ExecCommandOnHost_b("sudo find \"" + Dir_str + "\" -maxdepth 1 -type d")) {
        QStringList Output_lst = SSHWorker_o.GetLastOutput_lst();
        Output_lst.removeFirst();
        qSort(Output_lst);
        foreach(QString Line_str, Output_lst) {
          if(!Line_str.isEmpty()) {
            QStringList Pathes_lst = Line_str.split("/");
            QJsonObject ArrayObject_o;
            ArrayObject_o["fs_type"] = "dir";
            ArrayObject_o["fs_name"] = Pathes_lst[Pathes_lst.size() - 1];
            DataArray_o.append(ArrayObject_o);
          }
        }
      }
      if(SSHWorker_o.ExecCommandOnHost_b("sudo find \"" + Dir_str + "\" -maxdepth 1 -type f")) {
        QStringList Output_lst = SSHWorker_o.GetLastOutput_lst();
        qSort(Output_lst);
        foreach(QString Line_str, Output_lst) {
          if(!Line_str.isEmpty()) {
            QStringList Pathes_lst = Line_str.split("/");
            QJsonObject ArrayObject_o;
            ArrayObject_o["fs_type"] = "file";
            ArrayObject_o["fs_name"] = Pathes_lst[Pathes_lst.size() - 1];
            DataArray_o.append(ArrayObject_o);
          }
        }
      }
      MainObject_o["data"] = DataArray_o;
      SetJsonPackageHead_v(MainObject_o, "dir_list", "done");
      response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
    } catch (OSDSException & e) {
      qWarning()<<"ControllerAntivirus::GetHostDirList_v->"<<e.what();
      SetJsonPackageHead_v(MainObject_o, "dir_list", "error");
    }

  }

  void ControllerAntivirus::ScanHost_v(const QString & Host_str, HttpResponse & response, bool is_full_lan = false)
  {
    QString RequestState_str;
    QString ScanCommand_str = GetScanCommand_str(Host_str, is_full_lan);
    if(!ScanCommand_str.isEmpty()) { // Только если команда существует
      StartScanHostInThread_po(Host_str, ScanCommand_str);
    }
    QJsonObject MainObject_o;
    SetJsonPackageHead_v(MainObject_o, "scan_host", "process");
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerAntivirus::GetStateScanHost_v(const QString & Host_str, HttpResponse & response)
  {
    QString Status_str;
    _Mutex_so.lock();
    EnHostRunState RunState_en = _HostScanState_smap[Host_str];
    _Mutex_so.unlock();
    if(RunState_en == _HRSStop_en) Status_str = "stop";
    else if(RunState_en == _HRSProcess_en) Status_str = "process";
    else Status_str = "error";
    QJsonObject MainObject_o;
    if(Status_str == "stop") { // Если сканирование остановлено
      try {
        DatabaseWorker DatabaseWorker_o(this);
        QSqlQuery Query_lst = DatabaseWorker_o.QueryExec_lst("SELECT * FROM hosts_antiv WHERE hostname=\"" + Host_str + "\"");
        DatabaseWorker_o.QueryExec_lst("UPDATE hosts_antiv SET antiv_state=\"stop\" WHERE hostname = \"" + Host_str + "\"");
        Query_lst.next();
        MainObject_o["host"] = Host_str;
        MainObject_o["check"] = Query_lst.value("last_check").toString();
        MainObject_o["journal"] = Query_lst.value("alert").toString();        
      } catch (OSDSException & e) {
        qWarning()<<"ControllerAntivirus::GetStateScanHost_v->"<<e.what();
      }
    }
    SetJsonPackageHead_v(MainObject_o, "scan_host",  Status_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerAntivirus::StopHostScanned_v(const QString & Host_str, HttpResponse & response)
  {
    QJsonObject MainObject_o;
    QString State_str = "done";
    try {
      SSHWorker SSHWorker_o(Host_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
      int ExitStatus_i = 0;
      if(SSHWorker_o.ExecCommandOnHost_b("sudo kesl-control --get-task-list", ExitStatus_i)) {
        if(ExitStatus_i == 0) { // Только если касперский установлен
          QStringList IDNumbers_lst;
          QStringList Output_lst = SSHWorker_o.GetLastOutput_lst();
          for(int i = 0; i < Output_lst.size(); ++i) {
            QString Line_str = Output_lst[i];
            if(Line_str.indexOf("Scan_My_Computer") != -1 || Line_str.indexOf("Boot_Scan") != -1 || Line_str.indexOf("Scan_File") != -1) {  // ID = 2
              if(Output_lst[i + 3].indexOf("Запущено") != -1) {
                QStringList lst = Output_lst[i + 1].split(" : ");
                if(lst.size() >= 2) {
                  IDNumbers_lst.push_back(lst[1]);
                }
              }
            }
          }
          foreach(const QString & IDCode_str, IDNumbers_lst) {
            if(!SSHWorker_o.ExecCommandOnHost_b("sudo kesl-control --stop-task " + IDCode_str, ExitStatus_i)) {
              State_str = "ERROR: ssh command->--stop-task";
            } else if (ExitStatus_i != 0) {
              State_str = "scan_stop_crashed:::" + QString::number(ExitStatus_i);
            }
          }
        }
        else if (SSHWorker_o.ExecCommandOnHost_b("sudo killall drweb-filecheck.real", ExitStatus_i)) {
          QString State_str = "done";
        } else {
          State_str = "ERROR: no antivirus found";
          qWarning()<<"ControllerAntivirus::StopHostScanned_v->No antivirus found";
        }
      }
      else {
        State_str = "ERROR: ssh command->--get-task-list";
      }
    } catch(OSDSException & e) {
      State_str = "ssh connecting error";
      qWarning()<<"ControllerAntivirus::StopHostScanned_v->"<<e.what();
    }
    SetJsonPackageHead_v(MainObject_o, "dir_list", State_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerAntivirus::GetScanJournal_v(const QString & Host_str, HttpResponse & response)
  {
    QString Journal_str;
    try {
      DatabaseWorker DatabaseWorker_o;
      QSqlQuery Query_lst = DatabaseWorker_o.QueryExec_lst("SELECT * FROM hosts_antiv WHERE hostname=\"" + Host_str + "\"");
      Query_lst.next();
      Journal_str = Query_lst.value("journal").toString();
    } catch (OSDSException & e) {
      qWarning()<<"ControllerAntivirus::StopHostScanned_v->"<<e.what();
    }
    QString State_str = "error";
    QJsonObject MainObject_o;
    if(Journal_str.size() > 0) {
      QJsonArray DataArray_o = MainObject_o["data"].toArray();
      QStringList Journal_lst = Journal_str.split(";");
      foreach(const QString & Text_str, Journal_lst) {
        if(!Text_str.isEmpty()) {
          QJsonObject ArrayObject_o;
          ArrayObject_o["string"] = Text_str;
          DataArray_o.append(ArrayObject_o);
        }
      }
      MainObject_o["data"] = DataArray_o;
      State_str = "done";
    }
    SetJsonPackageHead_v(MainObject_o, "get_journal", State_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerAntivirus::AntivirusUpdate_v(const QString & Host_str, HttpResponse & response)
  {
    QString State_str = "done";
    QJsonObject MainObject_o;
    try {
      SAntivirusState SDrWebState_o = GetDrWebVersionOnHost_str(Host_str);
      SAntivirusState SAntivirusState_o = GetKasperskyVersionOnHost_str(Host_str);
      if(SDrWebState_o._Version_str != "0") {
        StartAntivirusUpdateInThread_po(Host_str, "drweb", _DrWebUpdateUrl_str);
      } else if(SAntivirusState_o._Version_str != "0") {
        StartAntivirusUpdateInThread_po(Host_str, "kesl", _KasperskyUpdateUrl_str);
      }
      else {  // Ошибка
        State_str = "error";
        MainObject_o["error"] = "Антивирус не установлен";
      }
    } catch (OSDSException & e) {
      qWarning()<<"ControllerAntivirus::AntivirusUpdate_v->"<<e.what();
      State_str = "error";
      MainObject_o["error"] = "Нет подключения по SSH";
    }
    SetJsonPackageHead_v(MainObject_o, "antiv_update", State_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerAntivirus::GetStateAntivirusUpdate_v(const QString & Host_str, HttpResponse & response)
  {
    QString Status_str;
    _Mutex_so.lock();
    EnHostRunState RunState_en = _AntivirusUpdateState_smap[Host_str];
    _Mutex_so.unlock();
    if(RunState_en == _HRSStop_en) Status_str = "stop";
    else if(RunState_en == _HRSProcess_en) Status_str = "process";
    else Status_str = "error";
    QJsonObject MainObject_o;
    if(Status_str == "stop") {
      DatabaseWorker DatabaseWorker_o(this);
      QSqlQuery Query_lst = DatabaseWorker_o.QueryExec_lst("SELECT * FROM hosts_antiv WHERE hostname=\"" + Host_str + "\"");
      Query_lst.next();
      MainObject_o["host"] = Host_str;
      MainObject_o["update"] = Query_lst.value("last_update").toString();
    }
    SetJsonPackageHead_v(MainObject_o, "antiv_update",  Status_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerAntivirus::GetLicenseState_v(const QString & Host_str, HttpResponse & response)
  {
    try {
      SSHWorker SSHWorker_o(Host_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
      SSHWorker_o.ExecCommandOnHost_b("sudo kesl-control --app-info");

    } catch(OSDSException & e) {
      qWarning()<<"ControllerAntivirus::GetLicenseState_v->"<<e.what();
    }
  }

  ControllerAntivirus::SAntivirusState ControllerAntivirus::GetDrWebVersionOnHost_str(const QString & Host_str)
  {
    SSHWorker SSHWorker_o(Host_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
    SAntivirusState SAntivirusState_o;
    SAntivirusState_o._Version_str = "0";
    SAntivirusState_o._IsAntivirusStateOk_b = true;
    if(SSHWorker_o.ExecCommandOnHost_b("drweb-ctl --version && drweb-ctl license")) {
      QStringList DrWeb_lst = SSHWorker_o.GetLastOutput_lst();
      foreach (const QString & Version_str, DrWeb_lst) {
        if(Version_str.contains("drweb")) {
          QStringList Temp_lst = Version_str.split(" ");
          if(Temp_lst.size() >= 2) {
            SAntivirusState_o._Version_str = Temp_lst[1];
          }
        } else if (Version_str.contains("expired")) {
          SAntivirusState_o._IsAntivirusStateOk_b = false;
        }
      }
    }
    if(SAntivirusState_o._Version_str == "0") {
      SAntivirusState_o._IsAntivirusStateOk_b = false;
    }
    return SAntivirusState_o;
  }

  ControllerAntivirus::SAntivirusState ControllerAntivirus::GetKasperskyVersionOnHost_str(const QString & Host_str)
  {
    SSHWorker SSHWorker_o(Host_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
    SAntivirusState SAntivirusState_o;
    SAntivirusState_o._IsAntivirusStateOk_b = false;
    SAntivirusState_o._Version_str = "0";
    if(SSHWorker_o.ExecCommandOnHost_b("sudo /opt/kaspersky/kesl/bin/kesl-control --app-info")){
      QStringList Kesl_lst = SSHWorker_o.GetLastOutput_lst();
      foreach (const QString & Version_str, Kesl_lst) {
        if(Version_str.contains(trUtf8("Версия:")) || Version_str.contains(trUtf8("Version:"))) {
          SAntivirusState_o._Version_str = QString(Version_str).remove(QRegExp("[^0-9.']"));
        } else if (Version_str.contains(trUtf8("Действительно"))) {
          SAntivirusState_o._IsAntivirusStateOk_b = true;
        }
      }
    }
    return SAntivirusState_o;
  }

  QString ControllerAntivirus::GetScanCommand_str(const QString & Host_str, bool is_full_lan = false)
  {
    QString ScanCommand_str = "";
    SAntivirusState SDrWebState_o = GetDrWebVersionOnHost_str(Host_str);
    SAntivirusState SAntivirusState_o = GetKasperskyVersionOnHost_str(Host_str);
    if(SDrWebState_o._Version_str != "0" || SAntivirusState_o._Version_str != "0") { // Если подсоеденились к хосту и нашли антивирус
      QString ScanType_str, DirList_str;
      try {
        DatabaseWorker DatabaseWorker_o;
        QSqlQuery Query_lst;
        int scan_policy = 0, scan_type = 0;

        if(is_full_lan){
          {
           DatabaseWorker_o.QueryExec_lstExt(QString("SELECT scan_policy, scan_type  FROM full_antiv_type_scan "), false, Query_lst);
           Query_lst.first();
           if(!Query_lst.value("scan_policy").toString().isEmpty())
               scan_policy = Query_lst.value("scan_policy").toInt();
           if(!Query_lst.value("scan_type").toString().isEmpty())
               scan_type = Query_lst.value("scan_type").toInt();
          }
          if(scan_type == 0)      ScanType_str = "fast";
          else if(scan_type == 1) ScanType_str = "full";
          else if(scan_type == 2) ScanType_str = "select";
          else ScanType_str = "fast";

          {
            DatabaseWorker_o.QueryExec_lstExt(QString("SELECT scan_dir FROM full_antiv_dirs_scan "), false, Query_lst);
            Query_lst.first();
            int cnt_row_dir = 0;
            do{
               if(!Query_lst.value("scan_dir").toString().isEmpty()){
                 if(cnt_row_dir > 0)
                   DirList_str += ";" + Query_lst.value("scan_dir").toString();
                 else
                   DirList_str = Query_lst.value("scan_dir").toString();
               }
               cnt_row_dir++;
            } while(Query_lst.next());
          }

        }
        else{
          Query_lst = DatabaseWorker_o.QueryExec_lst("SELECT * FROM hosts_antiv_state WHERE hostname=\"" + Host_str + "\"");
          Query_lst.next();
          ScanType_str = Query_lst.value("scan_type").toString();
          DirList_str = Query_lst.value("dir_list").toString();
        }
      } catch(OSDSException & e) {
        qWarning()<<"ControllerAntivirus::GetScanCommand_str->"<<e.what();
      }
      if(ScanType_str == "full") { // для полной проверки
        qDebug()<<"ControllerAntivirus::GetScanCommand_str->Start full scan host: " + Host_str;
        ScanCommand_str = SDrWebState_o._Version_str != "0" ?
              "sudo drweb-ctl scan / | grep -v \" - Ok\" | grep -v \"File not found\" | grep -v \"Read error\" | grep -v \"Password protected\" | grep -v \"Engine hang-up detected\" | grep -v \"File size too big\" | grep -v \"Archive nesting level limit reached\" " :
              "full=1; sudo kesl-control --start-task 2 -W | grep -A 10 -e \"ThreatDetected\" | grep -e \"DetectName\" -e \"FileName\"";
      } else if(ScanType_str == "select") { // Для выборочной проверки
        qDebug()<<"ControllerAntivirus::GetScanCommand_str->Start selected scan host: " + Host_str + ", dirs: " + DirList_str;
        QStringList Dirs_lst = DirList_str.split(';');
        Dirs_lst.removeAll(QString(""));
        QString TempDirList_str;
        QString tmpEmpty="";

        if(Dirs_lst.size() <=0){
           tmpEmpty+="echo \"Отсутствуют каталоги выборочной проверки АРМ! Требуется указать каталоги сканирования.\"; ";
        }

        foreach(const QString & Path_str, Dirs_lst) {
          TempDirList_str += "\"" + Path_str + "\" ";
        }
        {
          SSHWorker SSHWorker_o(Host_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
          if( SSHWorker_o.SendFileToHost_b("/opt/asb/scripts/scan_tmpl.sh", "/tmp") == false) {
            qWarning()<<"ERROR copy ssh file: scan_tmpl.sh";
          }
        }

        QString TemplScan  = tmpEmpty + " select=1; kesl=\"1\"; sudo /bin/sh  /tmp/scan_tmpl.sh \"" + TempDirList_str + "\" ";
        QString drWebQuery = /*(Dirs_lst.size() <=0)? tmpEmpty : */ tmpEmpty + " res=$(sudo drweb-ctl scan " + TempDirList_str + " 2>&1 | grep -v \" - Ok\" | grep -v \"File not found\" | grep -v \"Read error\" | grep -v \"Password protected\"); while read -r i; do    unknown=$(echo $i | grep -Po '(?<=No such file or directory: ).*'); if [ \"$unknown\" != \"\" ]; then echo \"Отсутствует файл или каталог: \"$unknown; else echo $i; fi;  done <<< $res;";

        //QString TemplScan = "kesl-control --scan-file "  +  TempDirList_str;
        qDebug()<<TemplScan;
        ScanCommand_str = SDrWebState_o._Version_str != "0" ?  drWebQuery:TemplScan;
      } else { // По умолчанию FAST
        qDebug()<<"ControllerAntivirus::GetScanCommand_str->Start fast scan host: " + Host_str;
        ScanCommand_str = SDrWebState_o._Version_str != "0" ?
              "sudo drweb-ctl procscan  | grep -v \" - Ok\" | grep -v \"File not found\" | grep -v \"Read error\" | grep -v \"Password protected\"" :
              "fast=1; sudo kesl-control --start-task 4 -W";
      }
      //ScanCommand_str.append(" > /tmp/antiv_log.txt;");
      if(ScanType_str == "full" &&  SAntivirusState_o._Version_str != "0") {
        ScanCommand_str.append("echo \"DetectNameQty=\" `grep -c \"DetectName\" /tmp/antiv_log.txt` >> /tmp/antiv_log.txt;");
      }
      //ScanCommand_str.append(" tail -n20 /tmp/antiv_log.txt");
    }

    return ScanCommand_str;
  }

  QThread * ControllerAntivirus::StartScanHostInThread_po(const QString & Host_str, const QString & ScanCommand_str)
  {
    try{
      DatabaseWorker DatabaseWorker_o;
      DatabaseWorker_o.QueryExec_lst("INSERT INTO hosts_antiv VALUES(\"" + Host_str + "\", \"\", 0,\"\", \"Не обновлялся\", \"stop\")", false);
      DatabaseWorker_o.QueryExec_lst("UPDATE hosts_antiv SET antiv_state=\"process\" WHERE hostname = \"" + Host_str + "\"");
    } catch (OSDSException & e) {
      qWarning()<<"ControllerAntivirus::StartScanHostInThread_po->"<<e.what();
    }
    FullScanProcess * FullScanProcess_po = new FullScanProcess(Host_str, ScanCommand_str);
    _Mutex_so.lock(); // Лочим критическую секцию
    _HostScanState_smap[Host_str] = _HRSProcess_en;
    _Mutex_so.unlock();
    QThread * ScanThread_po = new QThread;
    FullScanProcess_po->moveToThread(ScanThread_po);
    connect(ScanThread_po, SIGNAL(started()), FullScanProcess_po, SLOT(process()));
    connect(FullScanProcess_po, &FullScanProcess::Output_sig, [=](QString Host_str, QStringList out, qint32 Infected_i32){
      // Формирование отчета о сканировании хостов
      QString CurrentDT_str = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
      switch (Infected_i32) {
      case -1: {  // Если отсутствует лицензия
        Infected_i32 = -Infected_i32;
        CurrentDT_str = trUtf8("Ошибка лицензии");
      } break;
      case -2: {
          Infected_i32 = -Infected_i32 - 1;
          CurrentDT_str = trUtf8("Ядро не работает");
      } break;
      default:
        break;
      }

      qWarning()<<"kesl process task...DatabaseWorker_o.QueryExec_lst...";

      QString Alerts_str = QString::number(Infected_i32);
      QString Journal_str = out.join(";;");
      try{
        DatabaseWorker DatabaseWorker_o;
        DatabaseWorker_o.QueryExec_lst("UPDATE hosts_antiv SET last_check=\"" + CurrentDT_str + "\", alert=" + Alerts_str +
                                       ", journal=\"" + Journal_str + "\", antiv_state=\"stop\"" + " WHERE hostname = \"" + Host_str + "\"");
        qWarning()<<"kesl process task...DatabaseWorker_o.QueryExec_lst... complete";
      }
      catch(OSDSException & e) {
        qWarning()<<"ControllerAntivirus::StartScanHostInThread_po"<<e.what();
      }
      _Mutex_so.lock();
      _HostScanState_smap[Host_str] = _HRSStop_en;
      _Mutex_so.unlock();
    });
    connect(FullScanProcess_po, SIGNAL(finished()), ScanThread_po, SLOT(quit()));
    connect(FullScanProcess_po, SIGNAL(finished()), FullScanProcess_po, SLOT(deleteLater()));
    connect(ScanThread_po, SIGNAL(finished()), ScanThread_po, SLOT(deleteLater()));
    ScanThread_po->start(); // Запуск сканирования хостов (в потоках)
    return ScanThread_po;
  }

  QThread * ControllerAntivirus::StartAntivirusUpdateInThread_po(const QString & Host_str, const QString & AntivirusType_str, const QString & UpdateUrl_str)
  {
    try{
      DatabaseWorker DatabaseWorker_o;
      DatabaseWorker_o.QueryExec_lst("INSERT INTO hosts_antiv VALUES(\"" + Host_str + "\", \"\", 0,\"\", \"Не обновлялся\", \"stop\")", false);
      DatabaseWorker_o.QueryExec_lst("UPDATE hosts_antiv SET antiv_state=\"update\" WHERE hostname = \"" + Host_str + "\"");
    } catch (OSDSException & e) {
      qWarning()<<"ControllerAntivirus::StartAntivirusUpdateInThread_po->"<<e.what();
    }
    AntivUpdateProcess * AntivUpdateProcess_po = new AntivUpdateProcess(Host_str, AntivirusType_str, UpdateUrl_str);
    _Mutex_so.lock();
    _AntivirusUpdateState_smap[Host_str] = _HRSProcess_en;
    _Mutex_so.unlock();
    QThread * UpdateThread_po = new QThread;
    AntivUpdateProcess_po->moveToThread(UpdateThread_po);
    connect(UpdateThread_po, SIGNAL(started()), AntivUpdateProcess_po, SLOT(process()));
    connect(AntivUpdateProcess_po, &AntivUpdateProcess::Output_sig, [=](bool IsOk_b, AntivirusCodeErrors CodeErrors_en) {
      QString Error_str;
      switch (CodeErrors_en) {
      case ACENoUpdates_en: Error_str = "Нет файлов обновлений"; break;
      case ACENoLicense_en: Error_str = "Нет лицензии"; break;
      case ACEEngineFailed_en: Error_str = "Ядро не работает"; break;
      case ACESetUpdateSettings: Error_str = "Не установились настройки обновления"; break;
      case ACESetUpdateProcess: Error_str = "Ошибка в процессе обновления"; break;
      default: Error_str = "Ошибка";
      }
      QString CurrentDT_str = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
      try {
        DatabaseWorker DatabaseWorker_o;
        DatabaseWorker_o.QueryExec_lst("UPDATE hosts_antiv SET last_update=\"" + (IsOk_b ? CurrentDT_str : Error_str) + "\", antiv_state=\"stop\" WHERE hostname = \"" + Host_str + "\"");
      } catch (OSDSException & e) {
        qWarning()<<"ControllerAntivirus::StartAntivirusUpdateInThread_po->"<<e.what();
      }
      _Mutex_so.lock();
      _AntivirusUpdateState_smap[Host_str] = _HRSStop_en;
      _Mutex_so.unlock();
    });
    connect(AntivUpdateProcess_po, SIGNAL(finished()), UpdateThread_po, SLOT(quit()));
    connect(AntivUpdateProcess_po, SIGNAL(finished()), AntivUpdateProcess_po, SLOT(deleteLater()));
    connect(UpdateThread_po, SIGNAL(finished()), UpdateThread_po, SLOT(deleteLater()));
    UpdateThread_po->start(); // Запуск сканирования хостов (в потоках)
    return UpdateThread_po;
  }

  QThread * ControllerAntivirus::StartAntivirusInstallInThread_po(const QString & Host_str, const QString & Distrib_str)
  {

   try{
    AntivInstallProcess * AntivInstallProcess_po = new AntivInstallProcess(Host_str, Distrib_str, _DrWebPath_str, _KasperskyPath_str,
                                                                           _DrWebLicenseKey_str, _KasperskyLicenseKey_str, _KasperskyAutoinstall_str);
    _Mutex_so.lock();
    _AntivirusInstallState_smap[Host_str] = _HRSProcess_en;
    _Mutex_so.unlock();
    QThread * ScanThread_po = new QThread;
    AntivInstallProcess_po->moveToThread(ScanThread_po);
    connect(ScanThread_po, SIGNAL(started()), AntivInstallProcess_po, SLOT(process()));
    connect(AntivInstallProcess_po, &AntivInstallProcess::Output_sig, [=]( bool IsOk_b ){
      _Mutex_so.lock();
      _AntivirusInstallState_smap[Host_str] = IsOk_b ? _HRSStop_en : _HRSError_en;
      _Mutex_so.unlock();
    });
    connect(AntivInstallProcess_po, SIGNAL(finished()), ScanThread_po, SLOT(quit()));
    connect(AntivInstallProcess_po, SIGNAL(finished()), AntivInstallProcess_po, SLOT(deleteLater()));
    connect(ScanThread_po, SIGNAL(finished()), ScanThread_po, SLOT(deleteLater()));
    ScanThread_po->start(); // Запуск сканирования хостов (в потоках)
    return ScanThread_po;
   }
    catch (OSDSException & e) {
            qWarning()<<"ControllerAntivirus::StartAntivirusInstallInThread_po->"<<e.what();
          }
  }

  void ControllerAntivirus::GetStateAntivirusInstall_v(const QString & Host_str, HttpResponse & response)
  {
    QString Status_str;
    QJsonObject MainObject_o;
    _Mutex_so.lock();
    EnHostRunState RunState_en = _AntivirusInstallState_smap[Host_str];
    _Mutex_so.unlock();
    if(RunState_en == _HRSStop_en) {
      Status_str = "stop";
      SAntivirusState SDrWebState_o = GetDrWebVersionOnHost_str(Host_str);
      SAntivirusState SAntivirusState_o = GetKasperskyVersionOnHost_str(Host_str);
      if(SDrWebState_o._Version_str != "0" || SAntivirusState_o._Version_str != "0") {  // Если антивирус установлен, то нужно сбросить обновления
        try {
          DatabaseWorker DatabaseWorker_o;
          DatabaseWorker_o.QueryExec_lst("INSERT INTO hosts_antiv VALUES(\"" + Host_str + "\", \"\", 0,\"\", \"Не обновлялся\")", false);
          DatabaseWorker_o.QueryExec_lst("UPDATE hosts_antiv SET last_update=\"Не обновлялся\" WHERE hostname = \"" + Host_str + "\"");
        }
        catch(OSDSException & e) {
          qWarning()<<"ControllerAntivirus::GetStateAntivirusInstall_v->"<<e.what();
        }
      }

      if(SDrWebState_o._Version_str != "0") {
        MainObject_o["av_version"] = "dr.Web " + SDrWebState_o._Version_str;
      } else if(SAntivirusState_o._Version_str != "0") {
        MainObject_o["av_version"] = "Kaspersky " + SAntivirusState_o._Version_str;
      }
      else {
        MainObject_o["av_version"] = "Не установлен";
      }
    }
    else if(RunState_en == _HRSProcess_en) { Status_str = "process"; }
    else { Status_str = "error"; }

    SetJsonPackageHead_v(MainObject_o, "antiv_install",  Status_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerAntivirus::ListSaveFullDirs(stefanfrings::HttpResponse& response){
     QVariantMap map;
     QVariantMap resourceNode;
     QVariantMap driveNode;
     int cntRow = 0;
     int scan_policy = 0, scan_type = 0;
     try {
       qWarning()<<"tables db save sull scan antivirus query...";
       DatabaseWorker DatabaseWorker_o(this);
       QSqlQuery Query_lst;

       DatabaseWorker_o.QueryExec_lstExt(QString("SELECT scan_dir FROM full_antiv_dirs_scan "), false, Query_lst);
       Query_lst.first();
       do{
           if(!Query_lst.value("scan_dir").toString().isEmpty()){
             resourceNode.insert(QString::number(cntRow).toUtf8(), Query_lst.value("scan_dir").toString());
             cntRow++;
           }
       } while(Query_lst.next());
       map.insert("data", resourceNode);

       DatabaseWorker_o.QueryExec_lstExt(QString("SELECT scan_policy, scan_type FROM full_antiv_type_scan "), false, Query_lst);
       Query_lst.first();

       if(!Query_lst.value("scan_policy").toString().isEmpty())
         scan_policy = Query_lst.value("scan_policy").toInt();
       if(!Query_lst.value("scan_type").toString().isEmpty())
         scan_type  = Query_lst.value("scan_type").toInt();

       qWarning()<<"tables db save full scan antivirus query...complete";
     }
     catch (OSDSException & e) {
       cntRow = -1;
       qWarning()<<"tables db save full scan antivirus query...error"<<e.what();
       resourceNode.insert("update",e.what());
        map.insert("data", resourceNode);
     }
       map.insert("version", "1.1.1.1");
       map.insert("object", 71);
       if(cntRow != -1)
         map.insert("state",  0);
       else
         map.insert("state",  1);
       map.insert("host_name", "127.0.0.1");
       map.insert("host_ip",   "127.0.0.1");

       map.insert("scan_policy", scan_policy);
       map.insert("scan_type", scan_type);

       map.insert("action", "");
       QJsonObject json = QJsonObject::fromVariantMap(map);
       QJsonDocument JsonDocument_o(json);
       if (!JsonDocument_o.isNull()) {
         response.setHeader("Content-Type", "text/html; charset=UTF-8");
         response.write(JsonDocument_o.toJson());
     }
  }

  void ControllerAntivirus::ListSaveFullAppendDirs(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response){
      QVariantMap map;
      QVariantMap resourceNode;
      QVariantMap driveNode;
      bool cntRow = false;
      try {
        qWarning()<<"tables db append scan dir antivirus query...";
        DatabaseWorker DatabaseWorker_o(this);
        QSqlQuery Query_lst;

        DatabaseWorker_o.QueryExec_lstExt(QString("INSERT INTO full_antiv_dirs_scan (scan_dir) VALUES(\"" + request.getParameter("data") + "\") "), false, Query_lst);
        resourceNode.insert("update", "OK");
        map.insert("data", resourceNode);

        qWarning()<<"tables db append scan dir antivirus query...complete";
        cntRow = true;
      }
      catch (OSDSException & e) {
        qWarning()<<"tables db append scan dir antivirus query...error"<<e.what();
        resourceNode.insert("update",e.what());
         map.insert("data", resourceNode);
      }
        map.insert("version", "1.1.1.1");
        map.insert("object", 72);
        if(cntRow)
          map.insert("state",  0);
        else
          map.insert("state",  1);
        map.insert("host_name", "127.0.0.1");
        map.insert("host_ip",   "127.0.0.1");

        map.insert("action", "");
        QJsonObject json = QJsonObject::fromVariantMap(map);
        QJsonDocument JsonDocument_o(json);
        if (!JsonDocument_o.isNull()) {
          response.setHeader("Content-Type", "text/html; charset=UTF-8");
          response.write(JsonDocument_o.toJson());
      }
     }


  void ControllerAntivirus::ListSaveFullUpdateSettings(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response){
      QVariantMap map;
      QVariantMap resourceNode;
      QVariantMap driveNode;
      bool cntRow = false;
      try {
        qWarning()<<"tables db full settings antivirus query...";
        DatabaseWorker DatabaseWorker_o(this);
        QSqlQuery Query_lst;

        DatabaseWorker_o.QueryExec_lstExt(QString("UPDATE full_antiv_type_scan SET scan_policy=\"" + request.getParameter("scan_policy") + "\", scan_type=\"" + request.getParameter("scan_type") + "\""), false, Query_lst);
        resourceNode.insert("update", "OK");
        map.insert("data", resourceNode);

        qWarning()<<"tables db full settings antivirus query...complete";
        cntRow = true;
      }
      catch (OSDSException & e) {
        qWarning()<<"tables db full settings antivirus query...error"<<e.what();
        resourceNode.insert("update",e.what());
         map.insert("data", resourceNode);
      }
        map.insert("version", "1.1.1.1");
        map.insert("object", 75);
        if(cntRow)
          map.insert("state",  0);
        else
          map.insert("state",  1);
        map.insert("host_name", "127.0.0.1");
        map.insert("host_ip",   "127.0.0.1");

        map.insert("action", "");
        QJsonObject json = QJsonObject::fromVariantMap(map);
        QJsonDocument JsonDocument_o(json);
        if (!JsonDocument_o.isNull()) {
          response.setHeader("Content-Type", "text/html; charset=UTF-8");
          response.write(JsonDocument_o.toJson());
      }
     }
     void ControllerAntivirus::ListSaveFullRemoveDirs(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response){
            QVariantMap map;
            QVariantMap resourceNode;
            QVariantMap driveNode;
            bool cntRow = false;
            try {
              qWarning()<<"tables db delete scan dir antivirus query...";
              DatabaseWorker DatabaseWorker_o(this);
              QSqlQuery Query_lst;

              DatabaseWorker_o.QueryExec_lstExt(QString("DELETE FROM full_antiv_dirs_scan WHERE scan_dir=\"" + request.getParameter("data") + "\" "), false, Query_lst);
              resourceNode.insert("update", "OK");
              map.insert("data", resourceNode);

              qWarning()<<"tables db delete scan dir antivirus query...complete";
              cntRow = true;
            }
            catch (OSDSException & e) {
              qWarning()<<"tables db delete scan dir antivirus query...error"<<e.what();
              resourceNode.insert("update",e.what());
               map.insert("data", resourceNode);
            }
              map.insert("version", "1.1.1.1");
              map.insert("object", 73);
              if(cntRow)
                map.insert("state",  0);
              else
                map.insert("state",  1);
              map.insert("host_name", "127.0.0.1");
              map.insert("host_ip",   "127.0.0.1");

              map.insert("action", "");
              QJsonObject json = QJsonObject::fromVariantMap(map);
              QJsonDocument JsonDocument_o(json);
              if (!JsonDocument_o.isNull()) {
                response.setHeader("Content-Type", "text/html; charset=UTF-8");
                response.write(JsonDocument_o.toJson());
              }
        }


     void ControllerAntivirus::ListSaveFullSettings(stefanfrings::HttpResponse& response){
         QVariantMap map;
         QVariantMap resourceNode;
         QVariantMap driveNode;
         int cntRow = 0;
         int scan_policy = 0, scan_type = 0;
         try {
           qWarning()<<"tables db save settings scan antivirus query...";
           DatabaseWorker DatabaseWorker_o(this);
           QSqlQuery Query_lst;

           DatabaseWorker_o.QueryExec_lstExt(QString("SELECT scan_policy, scan_type  FROM full_antiv_type_scan "), false, Query_lst);
           Query_lst.first();

          if(!Query_lst.value("scan_policy").toString().isEmpty())
              scan_policy = Query_lst.value("scan_policy").toInt();
          if(!Query_lst.value("scan_type").toString().isEmpty())
              scan_type = Query_lst.value("scan_type").toInt();

           resourceNode.insert("scan_policy", scan_policy);
           resourceNode.insert("scan_type", scan_type);
           map.insert("data", resourceNode);

           qWarning()<<"tables db save settings scan antivirus query...complete";
           cntRow = 1;
         }
         catch (OSDSException & e) {
           cntRow = -1;
           qWarning()<<"tables db save full scan antivirus query...error"<<e.what();
           resourceNode.insert("update",e.what());
            map.insert("data", resourceNode);
         }
           map.insert("version", "1.1.1.1");
           map.insert("object", 74);
           if(cntRow != -1)
             map.insert("state",  0);
           else
             map.insert("state",  1);
           map.insert("host_name", "127.0.0.1");
           map.insert("host_ip",   "127.0.0.1");

           map.insert("scan_policy", scan_policy);
           map.insert("scan_type", scan_type);

           map.insert("action", "");
           QJsonObject json = QJsonObject::fromVariantMap(map);
           QJsonDocument JsonDocument_o(json);
           if (!JsonDocument_o.isNull()) {
             response.setHeader("Content-Type", "text/html; charset=UTF-8");
             response.write(JsonDocument_o.toJson());
         }
     }

  //------------------------------------------------------------------------------------------------
} // namespace osds
