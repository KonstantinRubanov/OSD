#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>
#include <QThread>
#include <QDateTime>
#include <QSqlQuery>

#include "ControllerIntegrityControl.h"
#include "ControllerLogin.h"
#include "sshworker.h"
#include "httpsessionstore.h"
#include "osdsapi.h"
#include "validation.h"
#include "constants.h"
#include "integritycontrolcreate.h"
#include "integritycontroltest.h"
#include "osdsexception.h"
#include "databaseworker.h"

using namespace stefanfrings;

namespace osds {

  QMap<QString, ThreadHostState> ControllerIntegrityControl::_HostState_smap;
  QMutex ControllerIntegrityControl::_Mutex_so;
  QMap<QString, QList<ControllerIntegrityControl::CIAuditData> > ControllerIntegrityControl::_CIAuditData_smap;

  QObject * CallbackCreateControllerIntegrityControl_po()
  {
    return new ControllerIntegrityControl;
  }

  ControllerIntegrityControl::ControllerIntegrityControl(QObject * parent) : ControllerAbstract(parent)
  {
    _ControllerName_str = "ControllerIntegrityControl";
  }

  void ControllerIntegrityControl::service(HttpRequest& request, HttpResponse& response)
  {
    if(request.getParameterMap().contains("get") && request.getParameter("get") == "hosts") { // Запрос хостов
      GetHostsList_v(response);
    } // Установка контрольной точки
    else if(request.getParameterMap().contains("point") && request.getParameter("point") == "install") {
      if(request.getParameterMap().contains("host")) {
        InstallPoint_v(request.getParameter("host"), response);
      } else {
        qWarning("ControllerIntegrityControl::Command point & install no host parameter!");
      }
    } // Состояние установки контрольной точки
    else if(request.getParameterMap().contains("point") && request.getParameter("point") == "install_state") {
      if(request.getParameterMap().contains("host")) {
        GetStateInstallPoint_v(request.getParameter("host"), response);
      } else {
        qWarning("ControllerIntegrityControl::Command point & install_state no host parameter!");
      }
    } // Сравнение текущего состояния с контрольной точкой
    else if(request.getParameterMap().contains("point") && request.getParameter("point") == "compare") {
      if(request.getParameterMap().contains("host")) {
        ComparePoint_v(request.getParameter("host"), response);
      } else {
        qWarning("ControllerIntegrityControl::Command point & compare no host parameter!");
      }
    } // Состояние проведения сравнения
    else if(request.getParameterMap().contains("point") && request.getParameter("point") == "compare_state") {
      if(request.getParameterMap().contains("host")) {
        GetStateComparePoint_v(request.getParameter("host"), response);
      } else {
        qWarning("ControllerIntegrityControl::Command point & compare_install no host parameter!");
      }
    } // Запрос папок для КЦ
    else if(request.getParameterMap().contains("get") && request.getParameter("get") == "dirs") {
      if(request.getParameterMap().contains("host")) {
        GetAfickDirs_v(request.getParameter("host"), response);
      } else {
        qWarning("ControllerIntegrityControl::Command get dirs no host parameter!");
      }
    } // Задание папок для КЦ
    else if(request.getParameterMap().contains("set") && request.getParameter("set") == "dirs") {
      if(request.getParameterMap().contains("host") && request.getParameterMap().contains("set_dirs") &&
         request.getParameterMap().contains("is_audit")) {
        SetAfickDirs_v(request.getParameter("host"), request.getParameter("set_dirs"), request.getParameter("is_audit") == "true", response);
      } else {
        qWarning("ControllerIntegrityControl::Command get dirs no host parameter!");
      }
    }
  }

  void ControllerIntegrityControl::SetSettings_sv(QSettings & /*Settings_ro*/)
  {

  }

  void ControllerIntegrityControl::InitializeDataBase_sv()
  {
    try {
      DatabaseWorker DatabaseWorker_o;
      DatabaseWorker_o.QueryExec_lst("CREATE TABLE IF NOT EXISTS hosts_integrity_control(hostname text primary key, ic_zero text, ic_check text, ic_journal text, ic_errors text, ic_state text, ic_audit text)", false);
      DatabaseWorker_o.QueryExec_lst("CREATE TABLE IF NOT EXISTS hosts_integrity_control_settings(hostname text primary key, ic_is_audit text)", false);
    } catch(OSDSException & e) {
      qWarning("ControllerIntegrityControl::InitializeDataBase_sv->Don't crreate DB table: %s", e.what());
    }
  }

  void ControllerIntegrityControl::GetHostsList_v(HttpResponse & response)
  {
    QJsonObject MainObject_o;
    QStringList Hosts_lst = osdsapi->GetValidation_po()->ALDProcess_lst("host-list");
    try {
      DatabaseWorker DatabaseWorker_o(this);
//      DatabaseWorker_o.QueryExec_lst("CREATE TABLE hosts_integrity_control(hostname text primary key, ic_zero text, ic_check text, ic_journal text, ic_errors text, ic_state text)", false);

#if defined(__NO_ASTRA_LINUX__)
      Hosts_lst.clear();
      Hosts_lst.push_back("astra.vbox");
      Hosts_lst.push_back("astra1.vbox");
#endif //__NO_ASTRA_LINUX__
      QJsonArray HostsArray_o = MainObject_o["data"].toArray();
      foreach(const QString & Name_str, Hosts_lst) {
        QSqlQuery Query_lst = DatabaseWorker_o.QueryExec_lst("SELECT * FROM hosts_integrity_control WHERE hostname=\"" + Name_str + "\"");
        Query_lst.next();
        QJsonObject ArrayObject_o;
        ArrayObject_o["host"] = Name_str;
        QString Zero_str = Query_lst.value("ic_zero").toString();
        QString Check_str = Query_lst.value("ic_check").toString();
        QString Journal_str = Query_lst.value("ic_journal").toString();
        QString Errors_str = Query_lst.value("ic_errors").toString();
        ArrayObject_o["zero_point"] = Zero_str.isEmpty() ? "---" : Zero_str;
        ArrayObject_o["check"] = Check_str.isEmpty() ? "---" : Check_str;
        ArrayObject_o["journal"] = Journal_str.isEmpty() ? "###" : Journal_str;
        ArrayObject_o["errors"] = Errors_str.isEmpty() ? "0" : Errors_str;
        ArrayObject_o["state"] = Query_lst.value("ic_state").toString();
        ArrayObject_o["audit"] = Query_lst.value("ic_audit").toString();
        HostsArray_o.append(ArrayObject_o);
      }
      MainObject_o["data"] = HostsArray_o;
    } catch(OSDSException & e) {
      qWarning("ControllerIntegrityControl::Don't crreate DB table: %s", e.what());
    }

    SetJsonPackageHead_v(MainObject_o, "integrity_control", "done");
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerIntegrityControl::InstallPoint_v(const QString & Host_str, HttpResponse & response)
  {
    if(!Host_str.isEmpty()) {
      StartInstallPointThread_po(Host_str);
    }
    QJsonObject MainObject_o;
    SetJsonPackageHead_v(MainObject_o, "point_install", "done");
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  QThread * ControllerIntegrityControl::StartInstallPointThread_po(const QString & Host_str)
  {
    QStringList Aliases_lst, IgnoreDirs_lst, Dirs_lst;
    QList< QPair<QString, QString> > AddDirs_lst;
    if(!GetAfickDirsFromConf_v(Host_str, "/tmp/afick.conf", AddDirs_lst, IgnoreDirs_lst, Aliases_lst)) {
    }
    for(int i = 0; i < AddDirs_lst.size(); ++i) {
      Dirs_lst.push_back(AddDirs_lst[i].first);
    }
    IntegrityControlCreate * IntegrityControlCreate_po = new IntegrityControlCreate(Host_str, Dirs_lst);
    _Mutex_so.lock();
    _HostState_smap[Host_str] = ThreadHostState(_HRSProcess_en);
    _Mutex_so.unlock();
    QThread * TestThread_po = new QThread;
    IntegrityControlCreate_po->moveToThread(TestThread_po);
    connect(TestThread_po, SIGNAL(started()), IntegrityControlCreate_po, SLOT(process()));
    connect(IntegrityControlCreate_po, &IntegrityControlCreate::Output_sig, [=](QString Host_str, quint32 Errors_u32,
            QString Journal_str) {
      // Формирование отчета о сканировании хостов
      _Mutex_so.lock();
      _HostState_smap[Host_str] = ThreadHostState(Errors_u32 > 0 ? _HRSError_en : _HRSStop_en, Errors_u32, Journal_str);
      _Mutex_so.unlock();
    });
    connect(IntegrityControlCreate_po, SIGNAL(finished()), TestThread_po, SLOT(quit()));
    connect(IntegrityControlCreate_po, SIGNAL(finished()), IntegrityControlCreate_po, SLOT(deleteLater()));
    connect(TestThread_po, SIGNAL(finished()), TestThread_po, SLOT(deleteLater()));
    TestThread_po->start(); // Запуск сканирования хостов (в потоках)
    return TestThread_po;
  }

  void ControllerIntegrityControl::GetStateInstallPoint_v(const QString & Host_str, HttpResponse & response)
  {
    QJsonObject MainObject_o;
    QJsonArray DataArray_o = MainObject_o["data"].toArray();
    _Mutex_so.lock();
    ThreadHostState ThreadHostState_o = _HostState_smap[Host_str];
    _Mutex_so.unlock();
    MainObject_o["data"] = DataArray_o;
    MainObject_o["journal"] = ThreadHostState_o._Journal_str;
    QString Errors_str = QString::number(-static_cast<int>(ThreadHostState_o._ErrorsCount_u32));
    MainObject_o["error"] = Errors_str;
    QString Date_str = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    MainObject_o["date"] = Date_str;
    QString State_str;
    switch (ThreadHostState_o._RunState_en) {
    case _HRSStop_en:
      State_str = "stop";
      break;
    case _HRSProcess_en:
      State_str = "process";
      break;
    default:
      State_str = "error";
      break;
    }
    try {
      DatabaseWorker DatabaseWorker_o(this);
      DatabaseWorker_o.QueryExec_lst("INSERT INTO hosts_integrity_control VALUES(\"" + Host_str + "\", \" \", \" \", \" \", \" \", \" \", \" \")", false);
      int ExitCode_i = 0;
      DatabaseWorker_o.QueryExecWithCode_lst("UPDATE hosts_integrity_control SET ic_zero=\"" + Date_str + "\", ic_journal=\"" +
                                             ThreadHostState_o._Journal_str + "\", ic_errors=\"" + Errors_str + "\", ic_state=\"create_" +
                                             State_str + "\"  WHERE hostname = \"" + Host_str + "\" ", ExitCode_i);
      if(ExitCode_i != 0) { // Если запрос не выполнен
        MainObject_o["db_error"] = "No_Query_Exec";
      }
    } catch(OSDSException & e) {
      qWarning()<<"ControllerIntegrityControl::GetStateInstallPoint_v->"<<e.what();
      MainObject_o["db_error"] = "Not_Open_DB";
    }

    SetJsonPackageHead_v(MainObject_o, "point_install_state", State_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerIntegrityControl::ComparePoint_v(const QString & Host_str, HttpResponse & response)
  {
    if(!Host_str.isEmpty()) {
      StartComparePointThread_po(Host_str);
    }
    QJsonObject MainObject_o;
    SetJsonPackageHead_v(MainObject_o, "point_compare", "done");
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  QThread * ControllerIntegrityControl::StartComparePointThread_po(const QString & Host_str)
  {
    QStringList Aliases_lst, IgnoreDirs_lst, Dirs_lst;
    QList< QPair<QString, QString> > AddDirs_lst;
    if(!GetAfickDirsFromConf_v(Host_str, "/tmp/afick.conf", AddDirs_lst, IgnoreDirs_lst, Aliases_lst)) {
    }
    for(int i = 0; i < AddDirs_lst.size(); ++i) {
      Dirs_lst.push_back(AddDirs_lst[i].first);
    }
    bool IsAudit_b = false;
    try {
      DatabaseWorker DatabaseWorker_o;
      QSqlQuery Query_lst = DatabaseWorker_o.QueryExec_lst("SELECT * FROM hosts_integrity_control_settings WHERE hostname=\"" + Host_str + "\"");
      Query_lst.next();
      IsAudit_b = Query_lst.value("ic_is_audit").toString() == "true";
    } catch (OSDSException & e) {
      qWarning()<<"ControllerIntegrityControl::StartComparePointThread_po->"<<e.what();
    }
    IntegrityControlTest * IntegrityControlTest_po = new IntegrityControlTest(Host_str, Dirs_lst, IsAudit_b);
    _Mutex_so.lock();
    _HostState_smap[Host_str] = ThreadHostState(_HRSProcess_en);
    _Mutex_so.unlock();
    QThread * TestThread_po = new QThread;
    IntegrityControlTest_po->moveToThread(TestThread_po);
    connect(TestThread_po, SIGNAL(started()), IntegrityControlTest_po, SLOT(process()));
    connect(IntegrityControlTest_po, &IntegrityControlTest::Output_sig, [=](QString Host_str, quint32 Errors_u32,
            QString Journal_str, QMap<QString, QPair<QString, QString> > Audit_map) {
      // Формирование отчета о сканировании хостов
      _Mutex_so.lock();
      _HostState_smap[Host_str]= ThreadHostState(Errors_u32 > 0 ? _HRSError_en : _HRSStop_en, Errors_u32, Journal_str);
      QStringList Keys_lst = Audit_map.keys();
      QList<CIAuditData> FullAudit_lst;
      foreach(const QString & Key_str, Keys_lst) {
        const QPair<QString, QString> & Data_pair = Audit_map[Key_str];
        CIAuditData CIAuditData_o(Key_str, Data_pair.first, Data_pair.second);
        FullAudit_lst.push_back(CIAuditData_o);
      }
      _CIAuditData_smap[Host_str] = FullAudit_lst;
      _Mutex_so.unlock();
    });
    connect(IntegrityControlTest_po, SIGNAL(finished()), TestThread_po, SLOT(quit()));
    connect(IntegrityControlTest_po, SIGNAL(finished()), IntegrityControlTest_po, SLOT(deleteLater()));
    connect(TestThread_po, SIGNAL(finished()), TestThread_po, SLOT(deleteLater()));
    TestThread_po->start(); // Запуск сканирования хостов (в потоках)
    return TestThread_po;
  }

  void ControllerIntegrityControl::GetStateComparePoint_v(const QString & Host_str, HttpResponse & response)
  {
    QJsonObject MainObject_o;
    _Mutex_so.lock();
    const ThreadHostState & ThreadHostState_o = _HostState_smap[Host_str];
    const QList<CIAuditData> & CIAuditData_lst = _CIAuditData_smap[Host_str];
    _Mutex_so.unlock();
    QJsonArray DataArray_o = MainObject_o["data"].toArray();
    QString AuditData_str;

    AuditData_str += "<table width=\"100%\" class=\"a_jnl\"><thead><tr><th>Объект</th><th>Информация</th></tr></thead><tbody id=\"table_audit_body\">";
    foreach(const CIAuditData & CIAuditData_rc, CIAuditData_lst) {
      //AuditData_str += "<tr><td>" + CIAuditData_rc.GetName_str() + "</td><td>" + CIAuditData_rc.GetAudit_str() + "</td></tr>";
        AuditData_str += CIAuditData_rc.GetAudit_str();

    }
    AuditData_str += "</tbody></table>";
    MainObject_o["data"] = DataArray_o;
    MainObject_o["audit"] = AuditData_str.replace("\"", "");
    MainObject_o["journal"] = ThreadHostState_o._Journal_str;
    QString Errors_str = QString::number(ThreadHostState_o._ErrorsCount_u32);
    MainObject_o["error"] = Errors_str;
    QString Check_str = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    MainObject_o["date"] = Check_str;
    QString State_str;
    switch (ThreadHostState_o._RunState_en) {
    case _HRSStop_en:
      State_str = "stop";
      break;
    case _HRSProcess_en:
      State_str = "process";
      break;
    default:
      State_str = "error";
      break;
    }
    try {
      DatabaseWorker DatabaseWorker_o(this);
      DatabaseWorker_o.QueryExec_lst("INSERT INTO hosts_integrity_control VALUES(\"" + Host_str + "\", \" \", \" \", \" \", \" \", \" \", \" \")", false);
      int ExitCode_i = 0;
      DatabaseWorker_o.QueryExecWithCode_lst("UPDATE hosts_integrity_control SET ic_check=\"" + Check_str + "\", ic_journal=\"" +
                                             ThreadHostState_o._Journal_str + "\", ic_errors=\"" + Errors_str + "\", ic_state=\"compare_" +
                                             State_str + "\", ic_audit=\"" + AuditData_str + "\"  WHERE hostname = \"" + Host_str + "\" ", ExitCode_i);
      if(ExitCode_i != 0) { // Если запрос не выполнен
        MainObject_o["db_error"] = "No_Query_Exec";
      }
    } catch(OSDSException & e) {
      qWarning()<<"ControllerIntegrityControl::GetStateComparePoint_v->"<<e.what();
      MainObject_o["db_error"] = "Not_Open_DB";
    }

    SetJsonPackageHead_v(MainObject_o, "point_compare_state", State_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerIntegrityControl::GetAfickDirs_v(const QString & Host_str, HttpResponse & response)
  {
    QJsonObject MainObject_o;
    QString State_str = "done";

    QStringList Aliases_lst, IgnoreDirs_lst;
    QList< QPair<QString, QString> > AddDirs_lst;
    QJsonArray AliasArray_o = MainObject_o["aliases"].toArray();
    QJsonArray SetDirsArray_o = MainObject_o["set_dirs"].toArray();
    QJsonArray IgnoreDirsArray_o = MainObject_o["ignore_dirs"].toArray();
    if(!GetAfickDirsFromConf_v(Host_str, "/tmp/afick.conf", AddDirs_lst, IgnoreDirs_lst, Aliases_lst)) {
      State_str = "error";
      MainObject_o["error_state"] = _LastError_str;
    } else {
      foreach(const QString & Alias_str, Aliases_lst) {
        QJsonObject ArrayObject_o;
        ArrayObject_o["alias"] = Alias_str;
        AliasArray_o.append(ArrayObject_o);
      }
      foreach(const QString & IgnoreDir_str, IgnoreDirs_lst) {
        QJsonObject ArrayObject_o;
        ArrayObject_o["path"] = IgnoreDir_str;
        IgnoreDirsArray_o.append(ArrayObject_o);
      }
      for(int i = 0; i < AddDirs_lst.size(); ++i) {
        QJsonObject ArrayObject_o;
        ArrayObject_o["path"] = AddDirs_lst[i].first;
        ArrayObject_o["alias"] = AddDirs_lst[i].second;
        SetDirsArray_o.append(ArrayObject_o);
      }
      try {
        DatabaseWorker DatabaseWorker_o;
        QSqlQuery Query_lst = DatabaseWorker_o.QueryExec_lst("SELECT * FROM hosts_integrity_control_settings WHERE hostname=\"" + Host_str + "\"");
        Query_lst.next();
        MainObject_o["is_audit"] = Query_lst.value("ic_is_audit").toString();
      } catch (OSDSException & e) {
        qWarning()<<"ControllerIntegrityControl::GetAfickDirs_v->" << e.what();
      }
    }
    MainObject_o["aliases"] = AliasArray_o;
    MainObject_o["set_dirs"] = SetDirsArray_o;
    MainObject_o["ignore_dirs"] = IgnoreDirsArray_o;

    SetJsonPackageHead_v(MainObject_o, "get_afick_dirs", State_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerIntegrityControl::SetAfickDirs_v(const QString & Host_str , const QString & SetDirs_str, bool IsAudit_b, HttpResponse & response)
  {
    QString State_str = "done";
    QJsonObject ReadObject_o;
    QJsonDocument JSonSettings_o = QJsonDocument::fromJson(SetDirs_str.toUtf8());
    if(!JSonSettings_o.isNull()) {
      if(JSonSettings_o.isObject()) {
        ReadObject_o = JSonSettings_o.object();
      } else {
        qWarning()<<"ControllerIntegrityControl::JSON doc no correct object";
        State_str = "error";
      }
    } else {
      qWarning()<<"ControllerIntegrityControl::no correct JSON doc";
      State_str = "error";
    }
    if(State_str == "done") { // Только если JSON прочитан
      QString SetDirCorrect_str;
      QJsonArray arr = ReadObject_o["set"].toArray();
      foreach(QJsonValue Value_o, arr) {
        SetDirCorrect_str += Value_o.toObject()["path"].toString() + " ";
        SetDirCorrect_str += Value_o.toObject()["alias"].toString() + "\n";
      }
      arr = ReadObject_o["ignore"].toArray();
      foreach(QJsonValue Value_o, arr) {
        SetDirCorrect_str += "!" + Value_o.toString() + "\n";
      }
      QFile::remove("/tmp/afick.conf.bak");
      if(!QFile::rename("/tmp/afick.conf", "/tmp/afick.conf.bak")) {
        State_str = "error";
        qWarning("ControllerIntegrityControl::create bak file rom /tmp/afick.conf");
      } else {  // Продолжаем работу
        QString TempFileText_str;
        QFile BakFile_o("/tmp/afick.conf.bak");
        if(BakFile_o.open(QIODevice::ReadOnly)) {
          QTextStream Stream_o(&BakFile_o);
          bool IsFileSection_b = false;
          while( !Stream_o.atEnd() ) {
            QString Line_str = Stream_o.readLine();
            if(!IsFileSection_b) {  // Если мы вне файловой секции
              TempFileText_str += Line_str + '\n';
              if(Line_str.indexOf("file section") != -1) {
                IsFileSection_b = true;
                TempFileText_str += Stream_o.readLine() + "\n";
                TempFileText_str += SetDirCorrect_str;
              }
            } else {
              if(Line_str.indexOf("#####") != -1) {
                TempFileText_str += Line_str + '\n';
                IsFileSection_b = false;
              }
            }
          }
        } else {
          State_str = "error";
          qWarning("ControllerIntegrityControl::cannot open file /tmp/afick.conf.bak");
        }
        BakFile_o.close();
        QFile SettingsFile_o("/tmp/afick.conf");
        if(State_str != "error" && SettingsFile_o.open(QIODevice::WriteOnly)) {
          SettingsFile_o.write(TempFileText_str.toUtf8());
        } else {
          State_str = "error";
          qWarning("ControllerIntegrityControl::cannot open file /tmp/afick.conf");
        }
        SettingsFile_o.close();
      }
      SSHWorker SSHWorker_o(Host_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
      SSHWorker_o.SendFileToHost_b("/tmp/afick.conf", "/tmp/");
      SSHWorker_o.ExecCommandOnHost_b("sudo rm /etc/afick.conf.bak; sudo mv /etc/afick.conf /etc/afick.conf.bak; "
                                      "sudo mv /tmp/afick.conf /etc/afick.conf; sudo chmod 0600 /etc/afick.conf");
      try {
        DatabaseWorker DatabaseWorker_o;
        DatabaseWorker_o.QueryExec_lst("INSERT INTO hosts_integrity_control_settings VALUES(\"" + Host_str + "\", \"" + (IsAudit_b ? "true" : "false") + "\")", false);
        DatabaseWorker_o.QueryExec_lst("UPDATE hosts_integrity_control_settings SET ic_is_audit=\"" + QString((IsAudit_b ? "true" : "false")) + "\"  WHERE hostname = \"" + Host_str + "\" ");
      } catch (OSDSException & e) {
        qWarning()<<"ControllerIntegrityControl::SetAfickDirs_v->"<<e.what();
      }
    }
    QJsonObject MainObject_o;
    SetJsonPackageHead_v(MainObject_o, "set_afick_dirs", State_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  bool ControllerIntegrityControl::GetAfickDirsFromConf_v(const QString & Host_str, const QString & File_str, QList<QPair<QString, QString> > & AddDirs_lst, QStringList & IgnoreDirs_lst, QStringList & Aliases_lst)
  {
    try {
      SSHWorker SSHWorker_o(Host_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
      if(!SSHWorker_o.ExecCommandOnHost_b("sudo chmod 0666 /etc/afick.conf")) { // Делаем файл читабельным
        qWarning("ControllerIntegrityControl::command failed: sudo chmod 0600 /etc/afick.conf");
        _LastError_str = "Set_Afick_Conf_Chmod";
        return false;
      }
      if(!SSHWorker_o.RecvFileFromHost_b("/etc/afick.conf", File_str)) { // Копируем файл на сервер
        qWarning("ControllerIntegrityControl::Recive file is crashed! /etc/afick.conf");
        _LastError_str = "Recive_Afick_Conf";
        return false;
      }
      if(!SSHWorker_o.ExecCommandOnHost_b("sudo chmod 0600 /etc/afick.conf")) { // Возвращаем права на файл
        qWarning("ControllerIntegrityControl::command failed: sudo chmod 0600 /etc/afick.conf");
        _LastError_str = "Return_Afick_Conf_Chmod";
        return false;
      }
    } catch (OSDSException & e) {
      qWarning("ControllerIntegrityControl::SSH command not found \"sudo cat /etc/afick.conf\" : %s", e.what());
      _LastError_str = "Error_SSH_Connect";
      return false;
    }

    //----- Процесс парсинга файла -----
    QFile AfickFile_o(File_str);
    if(AfickFile_o.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QTextStream Stream_o(&AfickFile_o);
      int Section_i = 0; // Разбираемые секции (1 - alias section, 2 - file section)
      while( !Stream_o.atEnd() ) {
        QString Line_str = Stream_o.readLine().simplified();
        if(Section_i == 0 && Line_str.indexOf("alias section") != -1) {
          Section_i = 1;
          continue;
        } else if (Section_i == 0 && Line_str.indexOf("file section") != -1) {
          Section_i = 2;
          continue;
        } else if(Section_i == 1) { // Для алиасов
          if(Line_str.size() > 0) {
            if(Line_str[0] == '#') {
              if(Line_str.indexOf("file section") != -1) {
                Section_i = 2;
              } else if (Line_str.indexOf("section") != -1) {
                Section_i = 0;
              }
              continue;
            } else {
              Aliases_lst.push_back(Line_str.split("=")[0].simplified());
            }
          }
        } else if(Section_i == 2) { // Для директорий
          if(Line_str.size() > 0) {
            if(Line_str[0] == '#') {
              if(Line_str.indexOf("alias section") != -1) {
                Section_i = 1;
              } else if (Line_str.indexOf("section") != -1) {
                Section_i = 0;
              }
              continue;
            } else if (Line_str[0] == '!') {  // Для игнорируемых каталогов
              QStringList Data_lst = Line_str.split(" ");
              IgnoreDirs_lst.push_back(Data_lst[0].remove("!"));
            } else {  // Для проверяемых каталогов
              QStringList Data_lst = Line_str.split(" ");
              AddDirs_lst.push_back(QPair<QString, QString>(Data_lst[0], Data_lst[1]));
            }
          }
        }
      }
    } else {
      _LastError_str = "Read_Afick_Conf";
      return false;
    }
    return true;
  }
}
