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

#include "ControllerBackup.h"
#include "ControllerLogin.h"
#include "httpsessionstore.h"
#include "osdsapi.h"
#include "validation.h"
#include "constants.h"
#include "databaseworker.h"
#include "backupcommon.h"

using namespace stefanfrings;

namespace osds {

  QString ControllerBackup::BackupPath_str = "/tmp";
  QString ControllerBackup::TempBackupPath_str = "/tmp";
  QString ControllerBackup::_LastLoadedBackup_sstr = "";
  QString ControllerBackup::_CorrectState_str = "";

  QObject * CallbackCreateControllerBackup_po()
  {
    return new ControllerBackup;
  }

  ControllerBackup::ControllerBackup(QObject * parent) : ControllerAbstract(parent)
  {

  }

  void ControllerBackup::service(HttpRequest& request, HttpResponse& response)
  {
    QByteArray Path_o = request.getPath();
    if(request.getParameterMap().contains("action")) {
      if(request.getParameter("action") == "save") {  // Создание резервной копии ALD (пункт 13)
        BackupALDSave_v(response);
      } else if (request.getParameter("action") == "load") {  // Загрузка резервной копии ALD (пункт 13)
        if(request.getParameterMap().contains("dtime")) {
          BackupALDLoad_v(request.getParameter("dtime"), response);
        } else {  // Иначе ошибка запроса
          qWarning()<<"ControllerBackup::Error::"<<"action LOAD "<<request.getParameter("dtime");
        }
      } else if (request.getParameter("action") == "remove") {
        if(request.getParameterMap().contains("dtime")) {
          BackupALDRemove_v(request.getParameter("dtime"), response);
        } else {  // Иначе ошибка запроса
          qWarning()<<"ControllerBackup::Error::"<<"action REMOVE "<<request.getParameter("dtime");
        }
      } else if(request.getParameter("action") == "get_list"){
        BackupList_v(response);
      } else if(request.getParameter("action") == "backup_host") {
        if(request.getParameterMap().contains("host")) {
          CreateHostBackup_v(request.getParameter("host"), response);
        } else {
          qWarning()<<"ControllerBackup::Error::"<<"action BACKUP_HOST "<<request.getParameter("host");
        }
      } else if(request.getParameter("action") == "get_bh_list") {
        if(request.getParameterMap().contains("host")) {
          GetHostBackups_v(request.getParameter("host"), response);
        } else {
          qWarning()<<"ControllerBackup::Error::"<<"action GET_BH_LIST "<<request.getParameter("host");
        }
      } else if (request.getParameter("action") == "bh_load") {
        if(request.getParameterMap().contains("host") && request.getParameterMap().contains("datetime")) {
          LoadBackupToHost_v(request.getParameter("host"), request.getParameter("datetime"), response);
        } else {
          qWarning()<<"ControllerBackup::Error::"<<"action BH_LOAD "<<request.getParameter("host");
        }
      } else if (request.getParameter("action") == "bh_remove") {
        if(request.getParameterMap().contains("host") && request.getParameterMap().contains("datetime")) {
          RemoveBackupToHost_v(request.getParameter("host"), request.getParameter("datetime"), response);
        } else {
          qWarning()<<"ControllerBackup::Error::"<<"remove BH_LOAD "<<request.getParameter("host");
        }
      } else if(request.getParameter("action") == "install_curl") {
        if(request.getParameterMap().contains("host")) {
          InstallCurl_v(request.getParameter("host"), response);
        } else {
          qWarning()<<"ControllerBackup::Error::"<<"install curl on host: "<<request.getParameter("host");
        }
      }
    } else if(request.getParameter("get") == "host_settings") {
      if(request.getParameterMap().contains("host")) {
        GetHostSettings_v(request.getParameter("host"), response);
      } else {
        qWarning()<<"ControllerBackup::Error::"<<"get host_settings "<<request.getParameter("host");
      }
    } else if(request.getParameter("set") == "host_settings") {
      if(request.getParameterMap().contains("host") && request.getParameterMap().contains("settings")) {
        SetHostSettings_v(request.getParameter("host"), request.getParameter("settings"), response);
      } else {
        qWarning()<<"ControllerBackup::Error::"<<"set host_settings "<<request.getParameter("host");
      }
    }
  }

  void ControllerBackup::SetSettings_sv(QSettings & Settings_ro)
  {
    ControllerBackup::BackupPath_str = Settings_ro.value("backupPath").toString();
    ControllerBackup::TempBackupPath_str = Settings_ro.value("hostBackupServer").toString();
  }

  void ControllerBackup::InitializeDataBase_sv()
  {
    try {
      DatabaseWorker DatabaseWorker_o;
      DatabaseWorker_o.QueryExec_lst("CREATE TABLE IF NOT EXISTS hosts_backups(hostname text primary key, hb_server_type text, hb_server text, hb_login text, hb_password text, hb_dirs text)", false);
    } catch (OSDSException & e) {
      qWarning()<<"ControllerBackup::InitializeDataBase_sv->"<<e.what();
    }
  }

  void ControllerBackup::BackupALDSave_v(stefanfrings::HttpResponse& response)
  {
    QString Status_str("done");
    QString DTime_str;
    // Подключаемся по SSH на ALD-сервер (название bk_servname_date.***)
    try {
      if(!_CorrectState_str.isEmpty()) {  // Проверим на текущий процесс
        throw(BackupCommon::_CBEProcessAlreadyRun);
      }
      SSHWorker SSHWorker_o(ControllerLogin::GetALDServer_str(), ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
      SSHWorker_o.ExecCommandOnHost_b("pidof ald-init"); // Проверка на работу ald-init
      foreach (const QString & PidNumber_str, SSHWorker_o.GetLastOutput_lst()) {
        if(PidNumber_str.size() > 2) {  // Проверим корректность pid
          throw(BackupCommon::_CBEProcessAlreadyRun);
        }
      }
      // Запускаем создание резервной копии ALD и ключей в папку /tmp
      DTime_str = QString::number(QDateTime::currentSecsSinceEpoch());
      _CorrectState_str = "save " + DTime_str + " " + QString::number(QDateTime::currentDateTime().currentSecsSinceEpoch());
      QString BackupFile_str = "/tmp/data" + DTime_str + ".tar.gz";
      QString KeyFile_str = "/tmp/key" + DTime_str + ".tar.gz";
      QString Command_str = QString("sudo ald-init backup --data-archive=") + BackupFile_str + " --key-archive=" + KeyFile_str;
      int ExitStatus_i = 0;
      if(SSHWorker_o.ExecCommandOnHost_b(Command_str, ExitStatus_i) == false ) {
        throw(BackupCommon::_CBECreateALDBackup_en);
      }
      // Копируем к себе в указанную папку
      if(SSHWorker_o.ExecCommandOnHost_b("sudo chmod 0644 /tmp/key* /tmp/data*") == false ) {
        throw(BackupCommon::_CBEChmodALDBackup_en);
      }
      if(SSHWorker_o.RecvFileFromHost_b(BackupFile_str, BackupPath_str + "/" + QFileInfo(BackupFile_str).fileName()) == false ) {
        throw(BackupCommon::_CBEReciveDataALDBackup_en);
      }
      if( SSHWorker_o.RecvFileFromHost_b(KeyFile_str, BackupPath_str + "/" + QFileInfo(KeyFile_str).fileName()) == false ) {
        throw(BackupCommon::_CBEReciveKeyALDBackup_en);
      }
      // Удаляем старые файлы
      if(SSHWorker_o.ExecCommandOnHost_b("sudo rm -rf /tmp/key* /tmp/data*") == false ) {
        throw(BackupCommon::_CBEDeleteALDBackupOnTemp_en);
      }
    } catch (OSDSException & e) {
      qWarning()<<"ControllerBackup::BackupALDSave_v->"<<e.what();
      Status_str = "Ошибка подключения к ALD-серверу по SSH";
    } catch (BackupCommon::EControllerBackupErrors BackupError_en) {
      Status_str = BackupCommon().GetErrorText_str(BackupError_en);
      qWarning()<<"ControllerBackup::BackupALDSave_v->"<<Status_str;
    }
    //---- Только если закончился именно этот процесс -----
    if(!DTime_str.isEmpty() && _CorrectState_str.indexOf("save " + DTime_str) != -1) {
      _CorrectState_str = "";
    }
    QJsonObject MainObject_o;
    SetJsonPackageHead_v(MainObject_o, "ald_backup", Status_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerBackup::BackupALDLoad_v(const QString & DTime_str, HttpResponse& response)
  {
    QString Status_str("done");
    QString Epoch_str = QString::number(QDateTime::fromString(DTime_str, "yyyy-MM-dd_hh:mm:ss").toSecsSinceEpoch());
    QString DataName_str = BackupPath_str + "/data" + Epoch_str + ".tar.gz";
    QString KeyName_str = BackupPath_str + "/key" + Epoch_str + ".tar.gz";
    try {
      if(!_CorrectState_str.isEmpty()) {  // Проверим на текущий процесс
        throw(BackupCommon::_CBEProcessAlreadyRun);
      }
      // Копируем файлы с РК ALD на сервер ALD в папку tmp
      if(!QFile::exists(DataName_str)) { // Если не сущестыует, то бросаем исключение
        throw (BackupCommon::_CBENoDataFile_en);
      }
      if (!QFile::exists(KeyName_str)) {
        throw (BackupCommon::_CBENoKeyFile_en);
      }
      SSHWorker SSHWorker_o(ControllerLogin::GetALDServer_str(), ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
      SSHWorker_o.ExecCommandOnHost_b("pidof ald-init"); // Проверка на работу ald-init
      foreach (const QString & PidNumber_str, SSHWorker_o.GetLastOutput_lst()) {
        if(PidNumber_str.size() > 2) {  // Проверим корректность pid
          throw(BackupCommon::_CBEProcessAlreadyRun);
        }
      }
      _CorrectState_str = "load " + Epoch_str + " " + QString::number(QDateTime::currentDateTime().currentSecsSinceEpoch());
      if( SSHWorker_o.SendFileToHost_b(DataName_str, "/tmp") == false) {
        throw (BackupCommon::_CBESendDataALDBackup_en);
      }
      if(SSHWorker_o.SendFileToHost_b(KeyName_str, "/tmp") == false ) {
        throw (BackupCommon::_CBESendKeyALDBackup_en);
      }
      if(SSHWorker_o.ExecCommandOnHost_b("sudo chmod 0644 /tmp/key* /tmp/data*") == false ) {
        throw(BackupCommon::_CBEChmodALDBackup_en);
      }
      QString StopExec_str("sudo ald-init stop -f >/dev/null");
      QString InitExec_str("sudo ald-init init -f >/dev/null");
      QString RestoreExec_str("sudo ald-init restore-backup --data-archive=/tmp/" + QFileInfo(DataName_str).fileName() +
                              " --key-archive=/tmp/" + QFileInfo(KeyName_str).fileName() + " -f >/dev/null");
      QString StartExec_str("sudo ald-init start -f >/dev/null");
      int ExitCode_i = 0;
      if(SSHWorker_o.ExecCommandOnHost_b(StopExec_str + "; " + InitExec_str + "; " + RestoreExec_str + "; " + StartExec_str, ExitCode_i) == false ) {
        throw(BackupCommon::_CBEALDBackupLoad_en);
      }
      _LastLoadedBackup_sstr = DTime_str;
    } catch(OSDSException & e) {
      qWarning()<<"ControllerBackup::BackupALDLoad_v->"<<e.what();
      Status_str = "Ошибка подключения к ALD-серверу по SSH";
    } catch (BackupCommon::EControllerBackupErrors BackupError_en) {
      Status_str = BackupCommon().GetErrorText_str(BackupError_en);
      qWarning()<<"ControllerBackup::BackupALDLoad_v->"<<Status_str;
    }
    //---- Только если закончился именно этот процесс -----
    if(!Epoch_str.isEmpty() && _CorrectState_str.indexOf("load " + Epoch_str) != -1) {
      _CorrectState_str = "";
    }
    QJsonObject MainObject_o;
    SetJsonPackageHead_v(MainObject_o, "ald_backup", Status_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerBackup::BackupALDRemove_v(const QString & DTime_str, HttpResponse & response)
  {
    QString Epoch_str = QString::number(QDateTime::fromString(DTime_str, "yyyy-MM-dd_hh:mm:ss").toSecsSinceEpoch());
    QString DataName_str = "data" + Epoch_str + ".tar.gz";
    QString KeyName_str = "key" + Epoch_str + ".tar.gz";
    bool IsRemoveFiles_b = false;
    if(QFile::remove(BackupPath_str + "/" + DataName_str) && QFile::remove(BackupPath_str + "/" + KeyName_str)) {
      IsRemoveFiles_b = true;
    }
    QJsonObject MainObject_o;
    SetJsonPackageHead_v(MainObject_o, "action_remove", IsRemoveFiles_b ? "done" : "error");
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerBackup::BackupList_v(stefanfrings::HttpResponse& response)
  {
    QDateTime LoadingBackup_o;
    bool IsTimeout_b = false;
    if(!_CorrectState_str.isEmpty() && _CorrectState_str.left(4) == "load") {
      QStringList lst = _CorrectState_str.split(" ");
      if(QDateTime::currentDateTime().currentSecsSinceEpoch() > lst[2].toUInt() + 120) {
        _CorrectState_str = "";
        IsTimeout_b = true;
      } else {
        if(lst.size() == 3) {
          LoadingBackup_o = QDateTime::fromSecsSinceEpoch(lst[1].toUInt());
        } else {
          _CorrectState_str = "";
        }
      }
    }
    QDir dir(BackupPath_str);  //объявляем объект работы с папками
    dir.setFilter(QDir::Files | QDir::NoSymLinks);   ///устанавливаем фильтр выводимых файлов/папок (см ниже)
    QFileInfoList FilesInfo_lst = dir.entryInfoList();     //получаем список файлов директории
    QJsonObject MainObject_o;
    MainObject_o["is_timeout"] = IsTimeout_b ? "true" : "false";
    SetJsonPackageHead_v(MainObject_o, "ald_backup", "done");
    QJsonArray DataArray_o = MainObject_o["data"].toArray();
    foreach(const QFileInfo & FileInfo_rc, FilesInfo_lst) {
      if(FileInfo_rc.baseName().indexOf("data") >= 0) {
        QDateTime DateTime_o = QDateTime::fromSecsSinceEpoch(FileInfo_rc.baseName().remove("data").toUInt());
        QString Date_str = DateTime_o.date().toString("yyyy-MM-dd");
        QString Time_str = DateTime_o.time().toString("hh:mm:ss");
        QJsonObject ArrayObject_o;
        ArrayObject_o["date"] = Date_str;
        ArrayObject_o["time"] = Time_str;
        ArrayObject_o["size"] = QString::number(FileInfo_rc.size());
        ArrayObject_o["is_load"] = QString(Date_str + "_" + Time_str) == _LastLoadedBackup_sstr ? "true" : "false";
        ArrayObject_o["is_loading"] = LoadingBackup_o == DateTime_o ? "true" : "false";
        DataArray_o.append(ArrayObject_o);
      }
    }
    MainObject_o["data"] = DataArray_o;
    QStringList Hosts_lst = osdsapi->GetValidation_po()->ALDProcess_lst("host-list");
#if defined(__NO_ASTRA_LINUX__)
    Hosts_lst.clear();
    Hosts_lst.push_back("astra.vbox");
#endif //__NO_ASTRA_LINUX__
    DatabaseWorker DatabaseWorker_o(this);
    QJsonArray HostsArray_o = MainObject_o["hosts_list"].toArray();
    foreach(const QString & Name_str, Hosts_lst) {
      QJsonObject ArrayObject_o;
      QSqlQuery Query_lst = DatabaseWorker_o.QueryExec_lst("SELECT * FROM hosts_backups WHERE hostname=\"" + Name_str + "\"");
      Query_lst.next();
      ArrayObject_o["host"] = Name_str;
      ArrayObject_o["server_type"] = Query_lst.value("hb_server_type").toString();
      ArrayObject_o["server"] = Query_lst.value("hb_server").toString();
      HostsArray_o.append(ArrayObject_o);
    }
    MainObject_o["hosts_list"] = HostsArray_o;
    MainObject_o["ald_backup_path"] = BackupPath_str;
    MainObject_o["host_backup_path"] = TempBackupPath_str;
    _LastLoadedBackup_sstr = "";
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerBackup::CreateHostBackup_v(const QString & HostName_str, HttpResponse & response)
  {
    QString Result_str = "done";
    QString ErrorText_str = "";
    try {
      DatabaseWorker DatabaseWorker_o(this);
      QSqlQuery Query_lst = DatabaseWorker_o.QueryExec_lst("SELECT * FROM hosts_backups WHERE hostname=\"" + HostName_str + "\"");
      Query_lst.next();
      ERepositoryType ERepositoryType_en = static_cast<ERepositoryType>(Query_lst.value("hb_server_type").toUInt());
      QString LocalPath_str = Query_lst.value("hb_server").toString();
      QString Login_str = Query_lst.value("hb_login").toString();
      QString Password_str = Query_lst.value("hb_password").toString();
      QString Dirs_str = Query_lst.value("hb_dirs").toString().replace(";", " ");
      if(Dirs_str.isEmpty()) {
        throw(tr("Не заданы каталоги для РК"));
      }
      Dirs_str.replace(" -", " ");
      if(Dirs_str[0] == "-") {
        Dirs_str[0] = ' ';
      }
      SSHWorker SSHWorker_o(HostName_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
      SSHWorker_o.ExecCommandOnHost_b("sudo chmod -R 0777 " + TempBackupPath_str);  // Сделаем папку доступной
      // Только если удалось создать папку для временного хранилища
      if(SSHWorker_o.ExecCommandOnHost_b("sudo mkdir -p " + TempBackupPath_str)) {
        QString FileName_str = "bh_" + QString::number(QDateTime::currentSecsSinceEpoch()) + ".tgz ";
        int ExitStatus_i = 0;
        if(SSHWorker_o.ExecCommandOnHost_b("sudo tar --xattrs --acls -cvzf " + TempBackupPath_str + "/" + FileName_str + Dirs_str + " > /dev/null", ExitStatus_i)){
          if(ExitStatus_i != 0) {
            throw( tr("Невозможно создать РК" ));
          }
          switch (ERepositoryType_en) { // Для каждого типа хранилища разные свойства
          case _RTLocal_en: { // Для локального хранилища
              // Если локальная папка отличается от временной
              if(!LocalPath_str.isEmpty() && LocalPath_str != TempBackupPath_str && LocalPath_str != (TempBackupPath_str + "/")) {
                if(!SSHWorker_o.ExecCommandOnHost_b("sudo mkdir -p " + LocalPath_str)) {
                  qWarning()<<"ControllerBackup:: create local directory is failed!";
                }
                if(!SSHWorker_o.ExecCommandOnHost_b("sudo mv " + TempBackupPath_str + "/" + FileName_str + " " + LocalPath_str)) {
                  Result_str = "error";
                  ErrorText_str = "Move backup file to local directory is failed!";
                  qWarning()<<"ControllerBackup:: move backup file to local directory is failed!";
                } else if (!SSHWorker_o.ExecCommandOnHost_b("sudo ln -s " + LocalPath_str + "/" + FileName_str + " " + TempBackupPath_str + "/" + FileName_str)) {
                  Result_str = "error";
                  qWarning()<<"ControllerBackup:: create symbolic link to backup is failed!";
                  ErrorText_str = "Create symbolic link to backup is failed!";
                }
              }
            } break;
          case _RTFtp_en: {
              QString TextFTP_str = TempBackupPath_str + "/ftp_host_backups.txt";
              if(LocalPath_str[LocalPath_str.size() - 1] == "/") {
                LocalPath_str = LocalPath_str.left(LocalPath_str.size() - 1);
              }
              int ExitStatus_i = 0, IsDone_b = true;
              if(SSHWorker_o.ExecCommandOnHost_b("sudo curl -T " + TempBackupPath_str + "/" + FileName_str + " ftp://" + LocalPath_str +
                                                 " --user " + Login_str + ":" + Password_str + " 2>/dev/null", ExitStatus_i)) {
                if (ExitStatus_i == 1) { // curl не установлен
                  Result_str = "error_no_curl";
                  ErrorText_str = "curl is not installed!";
                  IsDone_b = false;
                } else if (ExitStatus_i == 6) { // Доступ к FTP невозможен
                  Result_str = "error_ftp_denied";
                  ErrorText_str = "ftp server: access denied!";
                  IsDone_b = false;
                } else if (ExitStatus_i == 67) {
                  Result_str = "error_ftp_pass";
                  ErrorText_str = "ftp server: access denied!";
                  IsDone_b = false;
                }
              } else {
                Result_str = "error";
                qWarning()<<"ControllerBackup:: send file to FTP is failed!";
                ErrorText_str = "Send file to FTP is failed!";
                IsDone_b = false;
              }
              if(IsDone_b) {
                if (!SSHWorker_o.ExecCommandOnHost_b("sudo touch " + TextFTP_str + "; sudo chmod 0666 " + TextFTP_str +
                                                     "; sudo echo \" -u " + Login_str + ":" + Password_str + " ftp://" + LocalPath_str + "/" +
                                                     FileName_str + " -o " + TempBackupPath_str + "/" + FileName_str + "\" >> " + TextFTP_str)) {
                  Result_str = "error";
                  qWarning()<<"ControllerBackup:: create string with ftp plus backup file is failed!";
                  ErrorText_str = "Create string with FTP plus backup file is failed!";
                } else if(!SSHWorker_o.ExecCommandOnHost_b("sudo rm -f " + TempBackupPath_str + "/" + FileName_str)) {
                  Result_str = "error";
                  qWarning()<<"ControllerBackup:: delete tmp backup file!";
                  ErrorText_str = "Delete tmp backup file is failed!";
                }
              } else {  //Иначе удаляем созданый бэкап
                SSHWorker_o.ExecCommandOnHost_b("sudo rm -rf " + TempBackupPath_str + "/" + FileName_str);
              }
            } break;
          case _RTSsh_en: {

            } break;
          }
        }
      }
    } catch (OSDSException & e) {
      qWarning()<<"ControllerBackup::CreateHostBackup_v->"<<e.what();
      Result_str = "abort";
    } catch(const QString & ExceptionText_str) {
      Result_str = "error";
      ErrorText_str =  ExceptionText_str;
      qWarning()<<"ControllerBackup::CreateHostBackup_v->"<<ExceptionText_str;
    }

    QJsonObject MainObject_o;
    MainObject_o["error_text"] = ErrorText_str;
    SetJsonPackageHead_v(MainObject_o, "host_backup", Result_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerBackup::GetHostBackups_v(const QString & HostName_str, HttpResponse & response)
  {
    QJsonObject MainObject_o;
    QString Result_str = "done";
    try {
      SSHWorker SSHWorker_o(HostName_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
      if(SSHWorker_o.ExecCommandOnHost_b("sudo ls -1 " + TempBackupPath_str + "/bh_*")) {
        QStringList FileNames_lst = SSHWorker_o.GetLastOutput_lst();
        FileNames_lst.removeAll("");
        QJsonArray DataArray_o = MainObject_o["data"].toArray();
        foreach(const QString & FileName_str, FileNames_lst) {
          QString Name_str = QFileInfo(FileName_str).baseName().remove("bh_");
          QJsonObject ArrayObject_o;
          ArrayObject_o["datetime"] = QDateTime::fromSecsSinceEpoch(Name_str.toLongLong()).toString("yyyy-MM-dd hh:mm:ss");
          DataArray_o.append(ArrayObject_o);
        }
        MainObject_o["data"] = DataArray_o;
      }
      if(SSHWorker_o.ExecCommandOnHost_b("sudo cat " + TempBackupPath_str + "/ftp_host_backups.txt")) {
        QStringList FileNames_lst = SSHWorker_o.GetLastOutput_lst();
        QJsonArray DataArray_o = MainObject_o["ftp"].toArray();
        foreach(QString FileName_str, FileNames_lst) {
          QStringList Names_lst = FileName_str.remove("ftp://").split("/");
          if(Names_lst.size() >= 2) {
            QString BHFileName_str = Names_lst[Names_lst.size() - 1];
            BHFileName_str.remove("bh_");
            BHFileName_str.remove(".tgz");
            Names_lst.removeLast();
            QString FTPPath_str = Names_lst.join("/");
            QJsonObject ArrayObject_o;
            ArrayObject_o["ftp_path"] = FTPPath_str;
            ArrayObject_o["datetime"] = QDateTime::fromSecsSinceEpoch(BHFileName_str.toLongLong()).toString("yyyy-MM-dd hh:mm:ss");
            DataArray_o.append(ArrayObject_o);
          }
        }
        MainObject_o["ftp"] = DataArray_o;
      }
    } catch(OSDSException & e) {
      qWarning()<<"ControllerBackup::"<<e.what();
      Result_str = "abort";
    }
    SetJsonPackageHead_v(MainObject_o, "get_bh_list", Result_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerBackup::LoadBackupToHost_v(const QString & HostName_str, const QString & DateTime_str, HttpResponse & response)
  {
    QString Result_str = "error";
    try {
      SSHWorker SSHWorker_o(HostName_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
      if(DateTime_str.indexOf("[ftp]") != -1) {
        QString CorrectDateTile_str = DateTime_str;
        CorrectDateTile_str.remove("[ftp]");
        QString CorrectFileName_str = "bh_" + QString::number(QDateTime::fromString(CorrectDateTile_str, "yyyy-MM-dd hh:mm:ss").toSecsSinceEpoch()) + ".tgz";
        if(SSHWorker_o.ExecCommandOnHost_b("sudo cat " + TempBackupPath_str + "/ftp_host_backups.txt")) {
          QStringList FileNames_lst = SSHWorker_o.GetLastOutput_lst();
          QString FTPCommandGet_str;
          foreach(QString FileName_str, FileNames_lst) {
            if(FileName_str.indexOf(CorrectFileName_str) != -1) {
              FTPCommandGet_str = FileName_str;
            }
          }
          if(!FTPCommandGet_str.isEmpty()) {  // Только если нашли файл в списке
            int ExitStatus_i = 0;
            if(SSHWorker_o.ExecCommandOnHost_b("sudo curl" + FTPCommandGet_str + " 2>/dev/null", ExitStatus_i)) {
              if(ExitStatus_i == 1) { // CURL не установлен
                Result_str = "error_no_curl";
              } else if (ExitStatus_i == 6) {
                Result_str = "error_ftp_denied";
              } else if (ExitStatus_i == 67) {
                Result_str = "error_ftp_pass";
              }
              else if(SSHWorker_o.ExecCommandOnHost_b("sudo execaps -c 0x1000 -- tar --xattrs --xattrs-include=security.{PDPL,AUDIT,DEF_AUDIT} --acls -xvf " + TempBackupPath_str + "/" +
                                                 CorrectFileName_str + " -C / > /dev/null")) {
                if(SSHWorker_o.ExecCommandOnHost_b("sudo rm -f " + TempBackupPath_str + "/" + CorrectFileName_str)) {
                  Result_str = "done";
                }
              }
            }
          }
        }
      } else if (DateTime_str.indexOf("[ssh]") != -1) {

      } else {
        QString SecsSinceEpoch_str = QString::number(QDateTime::fromString(DateTime_str, "yyyy-MM-dd hh:mm:ss").toSecsSinceEpoch());
        if(SSHWorker_o.ExecCommandOnHost_b("sudo execaps -c 0x1000 -- tar --xattrs --xattrs-include=security.{PDPL,AUDIT,DEF_AUDIT} --acls -xvf " + TempBackupPath_str + "/bh_" +
                                           SecsSinceEpoch_str + ".tgz -C / > /dev/null")) {
          Result_str = "done";
        }
      }
    } catch(OSDSException & e) {
      Result_str = "abort";
      qWarning()<<"ControllerBackup::"<<e.what();
    }
    QJsonObject MainObject_o;
    SetJsonPackageHead_v(MainObject_o, "bh_load", Result_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerBackup::RemoveBackupToHost_v(const QString & HostName_str, const QString & DateTime_str, HttpResponse & response)
  {
    QString Result_str = "error";
    try {
      SSHWorker SSHWorker_o(HostName_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
      if(DateTime_str.indexOf("[ftp]") != -1) {
        QString CorrectDateTile_str = DateTime_str;
        CorrectDateTile_str.remove("[ftp]");
        QString CorrectFileName_str = "bh_" + QString::number(QDateTime::fromString(CorrectDateTile_str, "yyyy-MM-dd hh:mm:ss").toSecsSinceEpoch()) + ".tgz";
        if(SSHWorker_o.ExecCommandOnHost_b("sudo cat " + TempBackupPath_str + "/ftp_host_backups.txt")) {
          QStringList FileNames_lst = SSHWorker_o.GetLastOutput_lst();
          QString FTPCommandGet_str;
          QString NewFileData_str = "";
          foreach(QString FileName_str, FileNames_lst) {
            if(FileName_str.indexOf(CorrectFileName_str) != -1) {
              FTPCommandGet_str = FileName_str;
            } else {
              NewFileData_str += FileName_str + "\n";
            }
          }
          if(!FTPCommandGet_str.isEmpty()) {  // Только если нашли файл в списке
            FTPCommandGet_str.replace("  ", " ");
            FTPCommandGet_str = FTPCommandGet_str.simplified();
            QStringList Command_lst = FTPCommandGet_str.split(" ");
            QString FullName_str = Command_lst[2].remove("ftp://");
            FullName_str = FullName_str.right(FullName_str.size() - (FullName_str.indexOf('/') + 1));
            QString CurlCommand_str = "sudo curl " + Command_lst[0] + " " + Command_lst[1] + " ftp://" + Command_lst[2] + " -Q \"DELE " + FullName_str + "\" 2>/dev/null";
            int ExitStatus_i = 0;
            if(SSHWorker_o.ExecCommandOnHost_b(CurlCommand_str, ExitStatus_i)) {
              if(ExitStatus_i == 1) { // На хосте нет curl
                Result_str = "error_no_curl";
              } else if (ExitStatus_i == 6) {
                Result_str = "error_ftp_denied";
              } else if (ExitStatus_i == 67) {
                Result_str = "error_ftp_pass";
              } else if(ExitStatus_i == 78) {  // Успех (ошибка, но успех)
                SSHWorker_o.ExecCommandOnHost_b("sudo echo \"" + NewFileData_str + "\" > " + TempBackupPath_str + "/ftp_host_backups.txt");
                Result_str = "done";
              }
            }
          }
        }

      } else if(DateTime_str.indexOf("[ssh]") != -1) {

      } else {
        QString FileName_str = TempBackupPath_str + "/bh_" + QString::number(QDateTime::fromString(DateTime_str, "yyyy-MM-dd hh:mm:ss").toSecsSinceEpoch()) + ".tgz";
        if(SSHWorker_o.ExecCommandOnHost_b("sudo rm -f `readlink -f " + FileName_str + "`; sudo rm -f " + FileName_str)) {
          Result_str = "done";
        }
      }
    } catch (OSDSException & e) {
      Result_str = "abort";
      qWarning()<<"ControllerBackup::"<<e.what();
    }

    QJsonObject MainObject_o;
    SetJsonPackageHead_v(MainObject_o, "bh_remove", Result_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerBackup::InstallCurl_v(const QString & HostName_str, HttpResponse & response)
  {
    QString Result_str = "done";
    try {
      SSHWorker SSHWorker_o(HostName_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
      SSHWorker_o.SendFileToHost_b(":/curl/curl_7.52.1-5+deb9u6_amd64.deb", "/tmp/");
      SSHWorker_o.SendFileToHost_b(":/curl/libcurl3_7.52.1-5+deb9u6_amd64.deb", "/tmp/");
      if(!SSHWorker_o.ExecCommandOnHost_b("cd /tmp; sudo dpkg -i *curl*.deb")) {
        qWarning()<<"ControllerBackup::Curl not installed on host!";
        Result_str = "error";
      }
    } catch (OSDSException & e) {
      Result_str = "abort";
      qWarning()<<"ControllerBackup::"<<e.what();
    }
    QJsonObject MainObject_o;
    SetJsonPackageHead_v(MainObject_o, "install_curl", Result_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerBackup::GetHostSettings_v(const QString & HostName_str, HttpResponse & response)
  {
    QString State_str = "done";
    QJsonObject MainObject_o;
    try {
      DatabaseWorker DatabaseWorker_o(this);
      QSqlQuery Query_lst = DatabaseWorker_o.QueryExec_lst("SELECT * FROM hosts_backups WHERE hostname=\"" + HostName_str + "\"");
      Query_lst.next();
      MainObject_o["host"] = HostName_str;
      QString ServerType_str = Query_lst.value("hb_server_type").toString();
      MainObject_o["server_type"] = ServerType_str;
      QString ServerPath_str = Query_lst.value("hb_server").toString();
      if(ServerType_str == "0" && ServerPath_str.isEmpty()) {
        ServerPath_str = TempBackupPath_str;
      }
      MainObject_o["server"] = ServerPath_str;
      MainObject_o["login"] = Query_lst.value("hb_login").toString();
      MainObject_o["password"] = Query_lst.value("hb_password").toString();
      MainObject_o["dirs"] = Query_lst.value("hb_dirs").toString();
    } catch (OSDSException & e) {
      qWarning("ControllerBackup:: set host %s settings error: %s", HostName_str.toStdString().c_str(), e.what());
      State_str = "error";
    }
    SetJsonPackageHead_v(MainObject_o, "bh_load", State_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerBackup::SetHostSettings_v(const QString & HostName_str, const QString & Settings_str, HttpResponse & response)
  {
    QString State_str = "done";
    QJsonObject ReadObject_o;
    QJsonDocument JSonSettings_o = QJsonDocument::fromJson(Settings_str.toUtf8());
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
    QString ServerType_str = ReadObject_o["server_type"].toString();
    QString Server_str = ReadObject_o["server"].toString();
    QString Login_str = ReadObject_o["login"].toString();
    QString Password_str = ReadObject_o["password"].toString();
    QString Dirs_str;
    QJsonArray arr = ReadObject_o["dirs"].toArray();
    foreach(QJsonValue Value_o, arr) {
      bool IsDir_b = Value_o.toObject()["type"].toString() == "dir";
      Dirs_str += (IsDir_b ? "" : "-") + Value_o.toObject()["path"].toString() + ";";
    }
    try {
      DatabaseWorker DatabaseWorker_o(this);
      DatabaseWorker_o.QueryExec_lst("INSERT INTO hosts_backups VALUES(\"" + HostName_str + "\", \" \", \" \", \" \", \" \", \" \")", false);
      DatabaseWorker_o.QueryExec_lst("UPDATE hosts_backups SET hb_server_type=\"" + ServerType_str + "\", hb_server=\"" +
                                     Server_str + "\", hb_login=\"" + Login_str + "\", hb_password=\"" +
                                     Password_str + "\", hb_dirs=\"" + Dirs_str + "\" WHERE hostname=\"" + HostName_str + "\"");
    } catch (OSDSException & e) {
      qWarning("ControllerBackup:: set host %s settings error: %s", HostName_str.toStdString().c_str(), e.what());
      State_str = "error";
    }
    QJsonObject MainObject_o;
    SetJsonPackageHead_v(MainObject_o, "bh_save", State_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

}
