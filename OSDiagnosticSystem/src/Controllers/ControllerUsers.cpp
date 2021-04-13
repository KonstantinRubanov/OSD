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
#include <QNetworkReply>
#include <QTcpServer>

#include "ControllerUsers.h"
#include "httpsessionstore.h"
#include "ControllerLogin.h"
#include "osdsapi.h"
#include "validation.h"
#include "constants.h"
#include "osdsexception.h"
#include "dbqueriesstrings.h"

using namespace stefanfrings;

namespace osds {

QString ControllerUsers::_InstalledSoftwarePath_str = "/home/user";
QString ControllerUsers::_UserOutputPath_str = "/tmp/user/";
QString ControllerUsers::_WebServerHostIP_str = "127.0.0.1";
QString ControllerUsers::_WebServerHostPort_str = "8080";

  QObject * CallbackCreateControllerUsers_po()
  {
    return new ControllerUsers;
  }

  ControllerUsers::ControllerUsers(QObject * parent) : ControllerAbstract(parent)
  {
    _ControllerName_str = "ControllerUsers";
    // Заполнение ключами списка с информацией о пользователях
    _UsersFirstLevelKeys_slist.append("UserName"); // пользователь
    _UsersFirstLevelKeys_slist.append("UID"); // идентификатор
    _UsersFirstLevelKeys_slist.append("FullUserName"); // полное имя
    _UsersFirstLevelKeys_slist.append("GUID"); // идентификатор в группе
    _UsersFirstLevelKeys_slist.append("Groups"); // группы (первичная, вторичные, локальные)
    _UsersFirstLevelKeys_slist.append("HomeFileSystemType"); // тип ФС домашнего каталога
    _UsersFirstLevelKeys_slist.append("HomeServer"); // сервер домашнего каталога
    _UsersFirstLevelKeys_slist.append("HomePath"); // домашний каталог
    _UsersFirstLevelKeys_slist.append("BashSrc"); // командная оболочка
    _UsersFirstLevelKeys_slist.append("GecosField"); // поле gecos
    _UsersFirstLevelKeys_slist.append("Description"); // описание
    _UsersFirstLevelKeys_slist.append("Policy"); // политика
    _UsersFirstLevelKeys_slist.append("TemporaryBlocked"); // временно заблокирован
    _UsersFirstLevelKeys_slist.append("PrincipalKerberos"); // принципал Kerberos
    _UsersFirstLevelKeys_slist.append("PasswordPolicy"); // политика пароля
    _UsersFirstLevelKeys_slist.append("ExpirationDate"); // окончание срока действия
    _UsersFirstLevelKeys_slist.append("LastPasswordChangeDate"); // дата последней смены пароля
    _UsersFirstLevelKeys_slist.append("PasswordExpirationDate"); // окончание срока действия пароля
    _UsersFirstLevelKeys_slist.append("LastChangeDate");//дата последнего изменения
    _UsersFirstLevelKeys_slist.append("LastSuccesfullAuthentificationDate"); // дата последней успешной аутентификации
    _UsersFirstLevelKeys_slist.append("LastFailedAuthentificationDate"); // дата последней неуспешной аутентификации
    _UsersFirstLevelKeys_slist.append("NumberOfFailedPasswordEnters"); // количество неуспешных попыток ввода пароля
    _UsersFirstLevelKeys_slist.append("BlockedForFailedEntries"); // заблокирован за неудачные попытки входа
    _UsersFirstLevelKeys_slist.append("ForceChangePasswordFlag"); //флаг принудительной смены пароля
    _UsersFirstLevelKeys_slist.append("StringAttributes"); // строковые атрибуты
    _UsersFirstLevelKeys_slist.append("AldAccount"); // ald_account
    // Заполнение подключей для ключа Groups
    _UsersGroupsLevelKeys_slist.append("PrimaryGroup"); // первичная группа
    _UsersGroupsLevelKeys_slist.append("SecondaryGroup"); // вторичные группы
    _UsersGroupsLevelKeys_slist.append("LocalGroup"); // локальные группы
    //для ведения истории смены пароля
    _DatabaseWorker_o.setParent(this);
    //для экспорта файлов
    QDir UserOutput_dir;
    UserOutput_dir.mkdir(_UserOutputPath_str);
//    _NetworkManager_po = new QNetworkAccessManager(this);
  }

  void ControllerUsers::service(HttpRequest& request, HttpResponse& response)
  {
    QByteArray Path_o = request.getPath();
    if (Path_o.endsWith("/users")){
        if (request.getParameterMap().contains("usersAld")){
            if (request.getParameter("usersAld") == "userlist"){
                UsersALD_v(response, ACSAnswer);
              }
            else if (request.getParameter("usersAld") == "unlockAll"){
                UnlockAllUsers_v(false, response);
              }
            else if (request.getParameter("usersAld") == "export_user_output"){
                UsersALD_v(response, ACSDoNotAnswer);
                PrintToFile_v(request.getParameter("user_name"), response);
              }
            else if(request.getParameter("usersAld") == "user_names"){
                UserNamesALD_v(response);
              }
          }
        else if (request.getParameterMap().contains("lock_user") && request.getParameterMap().contains("userName")) {
            LockUser_v((request.getParameter("lock_user") == "true"), request.getParameter("userName"), response);
          }
        else if (request.getParameterMap().contains("change_password")){
            ChangeUserPassword_v(request.getParameter("user_name"), request.getParameter("new_password"), response);
          }
        else if (request.getParameterMap().contains("print_passwords")){
            PrintAllPasswords_v(response);
          }
        else if (request.getParameterMap().contains("get")){
            if (request.getParameter("get") == "host-list"){
                HostListALD_v(response, ACSAnswer);
              }
            else if (request.getParameter("get") == "application-list"){
                GetApplicationsOnHost_v(request.getParameter("host_name"), request.getParameter("user_name"), response);
              }
            else if (request.getParameter("get") == "compare_with_local_users"){
                CheckUserAsLocalOnHost_v(response, request.getParameter("host_name"), request.getParameter("user_name"));
              }
          }
        else if (request.getParameterMap().contains("lock-access")){
            SetACLOnExecFiles_v(request.getParameter("host_name"), request.getParameter("user_name"), request.getParameter("file_name"), (request.getParameter("lock-access") == "true"), response);
          }
        else if (request.getParameterMap().contains("check_cgi_host_access")){
            CheckCgiHostAccess_v(response);
          }
      }
  }

  void ControllerUsers::SetSettings_sv(QSettings & Settings_ro)
  {
    ControllerUsers::_InstalledSoftwarePath_str = Settings_ro.value("installedSoftwarePath").toString();
    if (Settings_ro.value("userOutputPath").toString().endsWith("/"))
      ControllerUsers::_UserOutputPath_str = Settings_ro.value("userOutputPath").toString();
    else
      ControllerUsers::_UserOutputPath_str = QString (Settings_ro.value("userOutputPath").toString() + "/");
    ControllerUsers::_WebServerHostIP_str = Settings_ro.value("WebServerHostIP").toString();
    ControllerUsers::_WebServerHostPort_str = Settings_ro.value("WebServerHostPort").toString();
  }

  void ControllerUsers::InitializeDataBase_sv()
  {
    try {
      DatabaseWorker DatabaseWorker_o;
      //Справочник пользователей
      DatabaseWorker_o.QueryExec_lst("CREATE TABLE IF NOT EXISTS users(user_id INTEGER primary key autoincrement, name UNIQUE constraint text not null)", false);
      //Справочник операций над паролями
      DatabaseWorker_o.QueryExec_lst("CREATE TABLE IF NOT EXISTS operations(operation_id INTEGER primary key, name UNIQUE constraint text not null)", false);
      DatabaseWorker_o.QueryExec_lst(InsertOperationQuery_str, QList<QPair<QString, QString>>() << qMakePair(QString(":p_name"), QString("change_by_admin")), false);
      DatabaseWorker_o.QueryExec_lst(InsertOperationQuery_str, QList<QPair<QString, QString>>() << qMakePair(QString(":p_name"), QString("change_by_user")), false);
      //Справочник статуса пароля
      DatabaseWorker_o.QueryExec_lst("CREATE TABLE IF NOT EXISTS passwords_states("
                    "passwords_states_id INTEGER primary key autoincrement,"
                    "user_id INTEGER not null,"
                    "operation_date datetime not null,"
                    "op_id INTEGER not null,"
                    "pswd text"
                    ")", false);
    }
    catch (OSDSException & e) {
      qWarning()<<e.what();
    }
  }

  void ControllerUsers::UserNamesALD_v(stefanfrings::HttpResponse& response)
  {
    QJsonObject MainObject_o;
    SetJsonPackageHead_v(MainObject_o,"list_users", "done");
    QJsonArray DataArray_o = MainObject_o["data"].toArray();
    QString Command_str = "user-list";
#if !defined(__NO_ASTRA_LINUX__)
    QStringList Users_list = osdsapi->GetValidation_po()->ALDProcess_lst(Command_str);
    //если список user-list пуст, то отправляем ответ на клиент со статусом
    if(Users_list.isEmpty()){
        qDebug() << "User-list is empty!";
        QString AnswerOperation = QString("list_users");
        SetJsonPackageHead_v(MainObject_o,AnswerOperation, "failed");
        response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
        return;
      }
#else //__NO_ASTRA_LINUX__
    QStringList Users_list;
    Users_list.append("user1");
    Users_list.append("user2");
    Users_list.append("user3");
    Users_list.append("user4");
    Users_list.append("user5");
    Users_list.append("user6");
#endif //__NO_ASTRA_LINUX__
    for (auto i : Users_list){
        QJsonObject OneUserObject_o;
        OneUserObject_o["UserName"] = i;
        DataArray_o.append(OneUserObject_o);
      }
    MainObject_o["data"] = DataArray_o;
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  QString ControllerUsers::GetChangePasswordState_str(QString UserName_str)
  {
    QSqlQuery QSqlQuery_o;
    //получим id пользователя для дальнейших запросов
    QSqlQuery_o = _DatabaseWorker_o.QueryExec_lst(GetUserIdQuery_str,  QList<QPair<QString, QString>>()
                                                  << qMakePair(QString(":p_name"), UserName_str), false);
    QSqlQuery_o.next();
    QString UserId_str = QSqlQuery_o.value("user_id").toString();
    if (UserId_str == "")
      return QString("");
    //какая дата последней смены пароля по пользователю записана в БД
    QSqlQuery_o = _DatabaseWorker_o.QueryExec_lst(GetLastOperationQuery_str,  QList<QPair<QString, QString>>()
                                                  << qMakePair(QString(":p_user_id"), UserId_str), false);
    QSqlQuery_o.next();
    QString OperationId_str = QSqlQuery_o.value("op_id").toString();
    if (OperationId_str == "")
      return QString("");
    //получим id операции из справочника для дальнейших запросов
    QSqlQuery_o = _DatabaseWorker_o.QueryExec_lst(GetOperationIdQuery_str, QList<QPair<QString, QString>>()
                                                  << qMakePair(QString(":p_name"), OperationId_str), false);
    QSqlQuery_o.next();
    QString OperationName_str = QSqlQuery_o.value("name").toString();
    if (OperationName_str == "")
      return QString("");
    else
      return OperationName_str;
  }

  void ControllerUsers::UsersALD_v(stefanfrings::HttpResponse &response, AldCommandState UACSState_en)
   {
     QJsonObject MainObject_o;
     SetJsonPackageHead_v(MainObject_o,"list_users", "done");
     QJsonArray DataArray_o = MainObject_o["data"].toArray();
     QString Command_str = "user-list";
#if !defined(__NO_ASTRA_LINUX__)
     QStringList Users_list = osdsapi->GetValidation_po()->ALDProcess_lst(Command_str);
     //если список user-list пуст, то отправляем ответ на клиент со статусом
     if(Users_list.isEmpty()){
         qDebug() << "User-list is empty!";
         QString AnswerOperation = (UACSState_en == ACSAnswer) ? "list_users" : "print_user_output";
         SetJsonPackageHead_v(MainObject_o,AnswerOperation, "failed");
         if (UACSState_en == ACSAnswer) {
             MainObject_o["user_list"] = "failed";
           }
         response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
         return;
       }
     for (auto i: Users_list){
         QJsonObject OneUserObject_o;
         Command_str = QString("user-get " + i + " --krb");
         //вся информация по 1 пользователю
         QStringList UserInformation_list = osdsapi->GetValidation_po()->ALDProcess_lst(Command_str);
         //если список user-get пуст, то отправляем ответ на клиент со статусом и именем пользователя
         if(UserInformation_list.isEmpty()){
             qDebug() << "User-get list is empty for user: " + i;
             if (UACSState_en == ACSAnswer){
                 SetJsonPackageHead_v(MainObject_o,"list_users", "failed");
                 MainObject_o["user_get"] = "failed";
                 MainObject_o["userName"] = i;
                 response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
               }
             return;
           }
         _UserGetOutput_map[i] = UserInformation_list;
         _DatabaseWorker_o.QueryExec_lst(InsertUserQuery_str, QList<QPair<QString, QString>>() << qMakePair(QString(":p_name"), i), false);
         //текущая дата последней смены пароля из запроса user-get
         int PosComma_i = _UserGetOutput_map[i].at(_UsersFirstLevelKeys_slist.indexOf("LastPasswordChangeDate") + 2).lastIndexOf(",");
         QString Value_str = QString(_UserGetOutput_map[i].at(_UsersFirstLevelKeys_slist.indexOf("LastPasswordChangeDate") + 2)).remove(0,PosComma_i).remove(", ");
         Value_str.chop(6);
         QString Format_str = "dd MMM yyyy hh:mm:ss";
         QLocale EnglishLocale_o(QLocale::English);
         QDateTime CurrentDateTime_o = EnglishLocale_o.toDateTime(Value_str, Format_str);
         QString CurrentDateTime_str = CurrentDateTime_o.toString(Format_str/*"dd.MM.yyyy hh:mm:ss"*/); //дата последней смены пароля в user-get
         ManageDB_v(CurrentDateTime_str, "********", i, DBMIUserlist);
         if (UACSState_en == ACSAnswer){
             ParseUserList_v(OneUserObject_o, UserInformation_list);
             OneUserObject_o["ChangePasswordState"] = GetChangePasswordState_str(i);
             //    OneUserObject_o["ChangePasswordState"] = //выгрузить из бд
             DataArray_o.append(OneUserObject_o);
           }
       }
     if (UACSState_en == ACSAnswer){
         MainObject_o["data"] = DataArray_o;
         response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
       }
#else
     //для отладки
     QStringList UserInformation_list = ReadFromFile_slist("user-get2.txt");
     _UserGetOutput_map["user1"] = UserInformation_list;
     _UserGetOutput_map["user2"] = UserInformation_list;
     _UserGetOutput_map["user3"] = UserInformation_list;
     QStringList Users_list(QStringList() << "user1" << "user2" << "user3");

     foreach(const QString& UserName_str, Users_list){
         _DatabaseWorker_o.QueryExec_lst(InsertUserQuery_str, QList<QPair<QString, QString>>() << qMakePair(QString(":p_name"), UserName_str), false);
         //текущая дата последней смены пароля из запроса user-get
         int PosComma_i = _UserGetOutput_map[UserName_str].at(_UsersFirstLevelKeys_slist.indexOf("LastPasswordChangeDate") + 2).lastIndexOf(",");
         QString Value_str = QString(_UserGetOutput_map[UserName_str].at(_UsersFirstLevelKeys_slist.indexOf("LastPasswordChangeDate") + 2)).remove(0,PosComma_i).remove(", ");
         Value_str.chop(6);
         QString Format_str = "dd MMM yyyy hh:mm:ss";
         QLocale EnglishLocale_o(QLocale::English);
         QDateTime CurrentDateTime_o = EnglishLocale_o.toDateTime(Value_str, Format_str);
         QString CurrentDateTime_str = CurrentDateTime_o.toString(Format_str/*"dd.MM.yyyy hh:mm:ss"*/); //дата последней смены пароля в user-get
         ManageDB_v(CurrentDateTime_str, "********", UserName_str, DBMIUserlist);
       }
     if (UACSState_en == ACSAnswer){
         QJsonObject OneUserObject_o;
         ParseUserList_v(OneUserObject_o, UserInformation_list);
         OneUserObject_o["ChangePasswordState"] = GetChangePasswordState_str("user1");
         DataArray_o.append(OneUserObject_o);
         OneUserObject_o["UserName"] = QString("user2");
         OneUserObject_o["ChangePasswordState"] = GetChangePasswordState_str("user2");
         DataArray_o.append(OneUserObject_o);//
         OneUserObject_o["UserName"] = QString("user3");
         OneUserObject_o["ChangePasswordState"] = GetChangePasswordState_str("user3");
         DataArray_o.append(OneUserObject_o);//
         MainObject_o["data"] = DataArray_o;
         response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
       }
//конец отладки
#endif
     //для отладки в ALD
     QFile jsonFile("json.txt");
     jsonFile.open(QFile::WriteOnly);
     jsonFile.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
     jsonFile.close();
     //
   }

  bool ControllerUsers::ManageDB_v(QString CurrentDateTime_str,QString Password_str, QString UserName_str, DBManageInitializer DBMI_en)
  {
    QSqlQuery QSqlQuery_o;
    QString OperationName_str;//имя операции
    QString OperationId_str;//id операции в справочнике
    QString Format_str = "dd MMM yyyy hh:mm:ss";
    QLocale EnglishLocale_o(QLocale::English);

    //получим id пользователя для дальнейших запросов
    QSqlQuery_o = _DatabaseWorker_o.QueryExec_lst(GetUserIdQuery_str,  QList<QPair<QString, QString>>()
                                                  << qMakePair(QString(":p_name"), UserName_str), false);
    QSqlQuery_o.next();
    QString UserId_str = QSqlQuery_o.value("user_id").toString();
    if (UserId_str == "")
      return false;
    //из основного запроса получаем тип операции и последнюю дату смены пароля
    QSqlQuery_o = _DatabaseWorker_o.QueryExec_lst(GetLastOperationQuery_str,  QList<QPair<QString, QString>>()
                                                  << qMakePair(QString(":p_user_id"), UserId_str), false);
    QSqlQuery_o.next();
    OperationId_str = QSqlQuery_o.value("op_id").toString();
    //какая дата последней смены пароля по пользователю записана в БД
    QString LastChangePasswordDate_str = QSqlQuery_o.value("operation_date").toString();
    //определим, кто менял пароль
    if (DBMI_en == DBMIPasswordGenerator){
       OperationName_str = "change_by_admin";
       QSqlQuery_o = _DatabaseWorker_o.QueryExec_lst(GetOperationIdQuery_str, QList<QPair<QString, QString>>()
                                                     << qMakePair(QString(":p_name"), OperationName_str), false);
       QSqlQuery_o.next();
       OperationId_str = QSqlQuery_o.value("operation_id").toString();
      }
    else {
        //получим имя операции из справочника для дальнейших запросов
        QSqlQuery OperationNameQuery_o = _DatabaseWorker_o.QueryExec_lst(GetOperationNameQuery_str, QList<QPair<QString, QString>>()
                                                      << qMakePair(QString(":p_name"), OperationId_str), false);
        OperationNameQuery_o.next();
        OperationName_str = OperationNameQuery_o.value("name").toString();
      }

    if (LastChangePasswordDate_str == ""){
        if (OperationId_str == ""){
            OperationId_str = QString("2");
          }
        _DatabaseWorker_o.QueryExec_lst(InsertUserPasswordQuery_str, QList<QPair<QString, QString>>()
                                        << qMakePair(QString(":p_user_id"),  UserId_str)
                                        << qMakePair(QString(":p_operation_date"), CurrentDateTime_str)
                                        << qMakePair(QString(":p_op_id"), OperationId_str)
                                        << qMakePair(QString(":p_pswd"),Password_str), false);
        return true;
      }
    else {
        QDateTime DateFromUserGet_o = QDateTime::fromString(CurrentDateTime_str, Format_str);
        QDateTime DateFromDB_o = QDateTime::fromString(LastChangePasswordDate_str, Format_str);
        if (DateFromUserGet_o > DateFromDB_o) {//если дата из user-get новее, то в бд записывается она
            _DatabaseWorker_o.QueryExec_lst(UpdateOperationDateQuery_str, QList<QPair<QString, QString>>()
                                            << qMakePair(QString(":p_op_date"), CurrentDateTime_str)
                                            << qMakePair(QString(":p_op_id"), OperationId_str)
                                            << qMakePair(QString(":p_pswd"),Password_str)
                                            << qMakePair(QString(":p_user_id"),  UserId_str), false);
            return true;
          }
        else
          return false;
      }
  }

   void ControllerUsers::ParseUserList_v(QJsonObject & JsonObject_o, QStringList & list)
   {
     int ListCounter_i = 0;
     list.removeAll(QString(""));
     QString Value_str;
     if (list.isEmpty()){
         qDebug() << "ParseUserList_v: user-get list is empty!";
         return;
     }
     for (int i = 0; i < _UsersFirstLevelKeys_slist.count(); i++ ){
         if (_UsersFirstLevelKeys_slist.at(i) == "LastPasswordChangeDate" ||
         _UsersFirstLevelKeys_slist.at(i) == "PasswordExpirationDate" ||
         _UsersFirstLevelKeys_slist.at(i) == "LastChangeDate" ||
         _UsersFirstLevelKeys_slist.at(i) == "LastSuccesfullAuthentificationDate" ||
         _UsersFirstLevelKeys_slist.at(i) == "LastFailedAuthentificationDate" ||
         _UsersFirstLevelKeys_slist.at(i) == "ExpirationDate"){
             QString ReturnString_str;
             if (list.at(ListCounter_i).contains("[")){
                 ReturnString_str = "-";
               }
             else {
                 int PosComma_i = list.at(ListCounter_i).lastIndexOf(",");
                 Value_str = QString(list.at(ListCounter_i)).remove(0,PosComma_i).remove(", ");
                 Value_str.chop(6);
                 QString Format_str = "dd MMM yyyy hh:mm:ss";
                 QLocale EnglishLocale_o(QLocale::English);
                 QDateTime DateTime_o = EnglishLocale_o.toDateTime(Value_str, Format_str);
                 ReturnString_str = DateTime_o.toString("dd.MM.yyyy hh:mm:ss");
               }
             JsonObject_o[_UsersFirstLevelKeys_slist.at(i)] = ReturnString_str;
             ListCounter_i++;
           }
         else if(_UsersFirstLevelKeys_slist.at(i) != "LastPasswordChangeDate" &&
         _UsersFirstLevelKeys_slist.at(i) != "PasswordExpirationDate" &&
         _UsersFirstLevelKeys_slist.at(i) != "LastChangeDate" &&
         _UsersFirstLevelKeys_slist.at(i) != "LastSuccesfullAuthentificationDate" &&
         _UsersFirstLevelKeys_slist.at(i) != "LastFailedAuthentificationDate" &&
         _UsersFirstLevelKeys_slist.at(i) != "ExpirationDate"){             
             Value_str = QStringList(list.at(ListCounter_i).split(':')).at(1);
             if (_UsersFirstLevelKeys_slist.at(i) == "Groups"){
                 QJsonObject GroupsObject_o;//key "Groups"
                 QJsonObject GroupObject_o;//key "PrimaryGroup"/"SecondaryGroup"/"LocalGroups"
                 for (int j = 0; j < _UsersGroupsLevelKeys_slist.count(); j++){
                     Value_str = QStringList(list.at(ListCounter_i).split(':')).at(1);
                     QJsonArray Array_o = GroupObject_o[_UsersGroupsLevelKeys_slist.at(j)].toArray();
                     QStringList SubValue_slist = Value_str.split(',', QString::SkipEmptyParts);//если значений у ключа несколько
                     for (auto SubValueName_str: SubValue_slist){
                         QJsonObject GroupName_o;
                         GroupName_o["GroupName"] = SubValueName_str.trimmed();
                         Array_o.append(GroupName_o);
                       }
                     GroupObject_o[_UsersGroupsLevelKeys_slist.at(j)] = Array_o;
                     ListCounter_i++;
                   }
                 GroupsObject_o[list.at(ListCounter_i)] = GroupObject_o;
                 JsonObject_o[_UsersFirstLevelKeys_slist.at(i)] = GroupsObject_o[list.at(ListCounter_i)];
               }
             else if(_UsersFirstLevelKeys_slist.at(i) == "GecosField"){
                 QJsonArray Array_o = JsonObject_o[_UsersFirstLevelKeys_slist.at(i)].toArray();
                 QStringList SubValue_slist = Value_str.split(',');//если значений у ключа несколько
                 for (auto SubValueName_str: SubValue_slist){
                     QJsonObject GroupName_o;
                     GroupName_o["GecosName"] = SubValueName_str.trimmed();
                     Array_o.append(GroupName_o);
                   }
                 JsonObject_o[_UsersFirstLevelKeys_slist.at(i)] = Array_o;
                 ListCounter_i++;
               }
             else {
                 JsonObject_o[_UsersFirstLevelKeys_slist.at(i)] = Value_str.trimmed();
                 ListCounter_i++;
               }
           }
       }
   }

   void ControllerUsers::UnlockAllUsers_v(bool LockAllUsers_b,  stefanfrings::HttpResponse& response)
   {
     QString Command_str = "user-list";
     QStringList Users_list = osdsapi->GetValidation_po()->ALDProcess_lst(Command_str);
     for (auto i : Users_list){
       QString Locked_str = (LockAllUsers_b) ? QString::number(1) : QString::number(0);
       #if !defined(__NO_ASTRA_LINUX__)
       osdsapi->GetValidation_po()->ALDProcess_lst("user-ald-cap " + i + " --c-locked=" + Locked_str);
       if (!LockAllUsers_b){
         ClearUserFaillog_v(i);
       }
//       osdsapi->GetValidation_po()->ALDProcess_lst("admin-apply-f");
       #else //__NO_ASTRA_LINUX__
       qDebug() << QString("user-ald-cap " + i + "--c-locked=" + Locked_str);
       qDebug() << "admin-apply-f";
       #endif //__NO_ASTRA_LINUX__
     }
     QJsonObject MainObject_o;
     SetJsonPackageHead_v(MainObject_o,"lock_all_users","done");
     response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
   }

   void ControllerUsers::LockUser_v(bool Lock_b, QString UserName_str, stefanfrings::HttpResponse& response)
   {
     QJsonObject MainObject_o;
     QJsonArray DataArray_o = MainObject_o["data"].toArray();
     if (IsUserLocked_b(UserName_str) == Lock_b){
       SetJsonPackageHead_v(MainObject_o,"lock_user_status","failed");
       QJsonObject OneUserObject_o;
       OneUserObject_o["lock_user"] = (Lock_b) ? QString("locked") : QString("unlocked");
       OneUserObject_o["userName"] = UserName_str;
       DataArray_o.append(OneUserObject_o);
       MainObject_o["data"] = DataArray_o;
     }
     else {
       //пользователь блокируется в ALD
       QString Locked_str = (Lock_b) ? QString::number(1) : QString::number(0);
#if !defined(__NO_ASTRA_LINUX__)
       osdsapi->GetValidation_po()->ALDProcess_lst("user-ald-cap " + UserName_str + " --c-locked=" + Locked_str);
       if (!Lock_b){
         ClearUserFaillog_v(UserName_str);
       }
//       osdsapi->GetValidation_po()->ALDProcess_lst("admin-apply-f");
#else //__NO_ASTRA_LINUX__
       qDebug() << QString("user-ald-cap -f " + UserName_str + " --c-locked=" + Locked_str);
       qDebug() << "admin-apply-f";
#endif //__NO_ASTRA_LINUX__
       //здесь должна быть новая проверка IsUserBlocked_b(UserName_str), чтоб результат вернуть в фронт-энд
       SetJsonPackageHead_v(MainObject_o,"lock_user_status","done");///???
       QJsonObject OneUserObject_o;
       OneUserObject_o["lock_user"] = (IsUserLocked_b(UserName_str)) ? QString("locked") : QString("unlocked");
       OneUserObject_o["userName"] = UserName_str;
       DataArray_o.append(OneUserObject_o);
       MainObject_o["data"] = DataArray_o;
     }
     response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
   }

   bool ControllerUsers::IsUserLocked_b(QString UserName_str)
   {
     bool IsBlocked_b = false;
     QStringList Value_lst;
     //Проверка, заблокирован ли пользователь
      QString LockedStatus_str = QString("Временно заблокирован: Да");
#if !defined(__NO_ASTRA_LINUX__)
     Value_lst = osdsapi->GetValidation_po()->ALDProcess_lst("user-get " + UserName_str);
#else //__NO_ASTRA_LINUX__
     qDebug() << QString("user-get " + UserName_str);
#endif //__NO_ASTRA_LINUX__
     IsBlocked_b = (!Value_lst.empty() && Value_lst.contains(LockedStatus_str)) ? true : false;
     qDebug() << UserName_str << " locked=" << IsBlocked_b;
     return IsBlocked_b;
   }

   void ControllerUsers::ClearUserFaillog_v(QString /*UserName_str*/)
   {
//     QStringList Hosts_lst = osdsapi->GetValidation_po()->ALDProcess_lst("host-list");
//     if (Hosts_lst.empty()){
//       qDebug() << "ClearUserFaillog_v : empty host list!";
//       return;
//     }
//     try {
//       foreach(const QString & Name_str, Hosts_lst) {
//         QHostInfo info = QHostInfo::fromName(Name_str);
//         foreach(const QHostAddress & addr, info.addresses()) {
//           QString Command_str = QString("echo \"sudo faillog -r -u\" " + UserName_str);
//           QString HostIP = addr.toString();
//           SSHWorker SSHWorkerObj(HostIP,ControllerALD::_Username_str,ControllerALD::_Password_str);
////           SSH_action(SSHWorkerObj, Command_str);
//           QString AldCommand_str = QString("user-mod " + UserName_str + " --clr-failcnt");
//           osdsapi->GetValidation_po()->ALDProcess_lst(AldCommand_str);
//         }
//       }
//     }
//     catch(OSDSException & e) {
//       qWarning()<<e.what();
//     }
   }

   void ControllerUsers::ChangeUserPassword_v(QString UserName_str, QString NewPasswd_str, stefanfrings::HttpResponse &response)
   {
     //объект заполнения ответа на клиент
     QJsonObject MainObject_o;
     //далее идут действия по смене пароля
     QString PasswdFile_str = QString("/tmp/pwd_" + UserName_str + ".txt");//сменить права этого файла на 0600
//#if !defined(__NO_ASTRA_LINUX__)
//     osdsapi->GetValidation_po()->ExecProcess_lst("rm -f " + PasswdFile_str);//сотрем предыдущий, прежде чем создадим новый
//#endif
     QFile File_o(PasswdFile_str);
     if(!File_o.open(QIODevice::ReadWrite))
       return;
     QTextStream NewPasswdStream_o(&File_o);
     QFile file("/tmp/osdsfile.pas");//пока взято из validation
     if (!file.open(QIODevice::ReadOnly))
       return;
     QTextStream Stream_o(&file);

     Stream_o.setCodec(QTextCodec::codecForName("UTF-8"));
     NewPasswdStream_o.setCodec(QTextCodec::codecForName("UTF-8"));
     NewPasswdStream_o << QString(UserName_str + ":" + NewPasswd_str);
     while( !Stream_o.atEnd() ) {
         NewPasswdStream_o << "\n";
         NewPasswdStream_o << Stream_o.readLine();
       }
     file.close();
     File_o.close();

     QString LastError_str;
     QString Output_str;
#if !defined(__NO_ASTRA_LINUX__)
     qWarning() << "test ald-admin user-passw";
     osdsapi->GetValidation_po()->ExecProcess_lst("sudo chmod 0600 " + PasswdFile_str);
     QString Command_str = QString("sudo ald-admin user-passwd " + UserName_str + " -f --pass-file=" + PasswdFile_str);
     qWarning() << Command_str;

     {
       SSHWorker SSHWorker_o("127.0.0.1", ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
       QStringList pSSHWorkerObjlst;
       if(SSHWorker_o.ExecCommandOnHost_b(QString(Command_str + " 2>&1"))){
          pSSHWorkerObjlst = SSHWorker_o.GetLastOutput_lst();
          pSSHWorkerObjlst.removeAll(QString(""));
          Output_str = (QString)pSSHWorkerObjlst.join("\n");
        }
     }
     /*
     QProcess Process_o(this);
     Process_o.start(Command_str + " 2>&1");
     if(!Process_o.waitForStarted(20000) || !Process_o.waitForFinished(-1)) {
         LastError_str = "Process run timeout";
         qWarning() << LastError_str;
         MainObject_o["pswd_state"] = "failed";
       }
     */
     qWarning() << "test ald-admin user-passw complete";
     //Output_str = QString::fromUtf8(Process_o.readAllStandardOutput());
     qWarning() << Output_str;
#else //__NO_ASTRA_LINUX__
     osdsapi->GetValidation_po()->ExecProcess_lst("chmod 0600 " + PasswdFile_str);//без этого астра не поменяет пароль
     Output_str = ReadFromFile_slist(QString("passwd_output.txt")).join("\n");
#endif //__NO_ASTRA_LINUX__
     if(Output_str.indexOf(QObject::trUtf8("ОШИБКА:")) != -1) {  // Если ошибка найдена
         if (Output_str.indexOf(QObject::trUtf8("Password does not contain enough character classes")) != -1){
             LastError_str = "Password does not contain enough character classes";
             qWarning()<<LastError_str;
             MainObject_o["pswd_state"] = "not_enough_character_classes";
           }
         else if (Output_str.indexOf(QObject::trUtf8("Cannot reuse password")) != -1){
             LastError_str = "Cannot reuse password";
             qWarning()<<LastError_str;
             MainObject_o["pswd_state"] = "cannot_reuse_password";
           }
         else if (Output_str.indexOf(QObject::trUtf8("Password is too short")) != -1){
             LastError_str = "Password is too short";
             qWarning()<<LastError_str;
             MainObject_o["pswd_state"] = "password_is_too_short";
           }
         else{
             LastError_str = "Error run command!";
             qWarning()<<LastError_str;
             MainObject_o["pswd_state"] = "error_run_command";
           }
       }
     else if (Output_str.indexOf(QObject::trUtf8("rpc-handshake")) != -1){
         LastError_str = "RPC-handshake error";
         qWarning()<<LastError_str;
         MainObject_o["pswd_state"] = "rpc_handshake";
       }
     else {//статус "выполнено", если нет ошибок
         //заполнение таблицы password_states с паролями пользователей,если при смене пароля не возникло ошибок
         QStringList Value_lst;
#if !defined(__NO_ASTRA_LINUX__)
    Value_lst = osdsapi->GetValidation_po()->ALDProcess_lst("user-get --krb " + UserName_str);
#else //__NO_ASTRA_LINUX__
    Value_lst = ReadFromFile_slist("user-get1.txt");
#endif
         if (!Value_lst.isEmpty()){
             //текущая дата последней смены пароля из запроса user-get
             int PosComma_i = Value_lst.at(_UsersFirstLevelKeys_slist.indexOf("LastPasswordChangeDate") + 2).lastIndexOf(",");
             QString Value_str = QString(Value_lst.at(_UsersFirstLevelKeys_slist.indexOf("LastPasswordChangeDate") + 2)).remove(0,PosComma_i).remove(", ");
             Value_str.chop(6);
             QString Format_str = "dd MMM yyyy hh:mm:ss";
             QLocale EnglishLocale_o(QLocale::English);
             QDateTime CurrentDateTime_o = EnglishLocale_o.toDateTime(Value_str, Format_str);
             QString CurrentDateTime_str = CurrentDateTime_o.toString(Format_str/*"dd.MM.yyyy hh:mm:ss"*/); //дата последней смены пароля в user-get
             if (ManageDB_v(CurrentDateTime_str, NewPasswd_str, UserName_str, DBMIPasswordGenerator)){
//                 if (MainObject_o["pswd_state"] == ""){
//                     MainObject_o["pswd_state"] = "failed";
//                   }
//               }
//             else {
                 MainObject_o["pswd_state"] = "done";
               }
           }
         else {
             MainObject_o["pswd_state"] = "user_get_failed";
           }
       }
     MainObject_o["user_name"] = UserName_str;
     MainObject_o["new_pswd"] = NewPasswd_str;
     response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
     osdsapi->GetValidation_po()->ExecProcess_lst("sudo rm -f " + PasswdFile_str);//удалим временный файл после установки пароля
   }

   void ControllerUsers::GeneratePassword_v()
   {

   }

   void ControllerUsers::PrintToFile_v(QString UserName_str, HttpResponse &response)
   {
     QString Contains_str = (_UserGetOutput_map.contains(UserName_str))?"true":"false";
     qDebug() << "PrintToFile_v::_UserGetOutput_map.contains = " + Contains_str;
     QJsonObject MainObject_o;
     if (!_UserGetOutput_map.contains(UserName_str)){
         SetJsonPackageHead_v(MainObject_o,"print_user_output","failed");
         response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
         return;
       }

     QFile UserOutputFile_o(QString(_UserOutputPath_str + UserName_str + "_user_output.txt"));
     if (UserOutputFile_o.open(QIODevice::WriteOnly | QIODevice::Text)){
         QTextStream Stream_o (&UserOutputFile_o);
         Stream_o.setCodec(QTextCodec::codecForName("UTF-8"));
         for (auto i : _UserGetOutput_map.value(UserName_str)){
             Stream_o << i << "\n";
           }
         UserOutputFile_o.close();
         //ответ на клиент
         SetJsonPackageHead_v(MainObject_o,"print_user_output","done");
         response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
       }
     else {
         SetJsonPackageHead_v(MainObject_o,"print_user_output","failed");
         response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
       }
   }

   void ControllerUsers::PrintAllPasswords_v(stefanfrings::HttpResponse& response){
     QJsonObject MainObject_o;
     SetJsonPackageHead_v(MainObject_o,"passwords_list","done");
     QJsonArray DataArray_o = MainObject_o["data"].toArray();
     QStringList Users_list;
#if !defined(__NO_ASTRA_LINUX__)
     QString Command_str = "user-list";
     Users_list = osdsapi->GetValidation_po()->ALDProcess_lst(Command_str);
#else
     Users_list.append(QString("user1"));
     Users_list.append(QString("user2"));
     Users_list.append(QString("user3"));
#endif
     //если список user-list пуст, то отправляем ответ на клиент со статусом
     if(Users_list.isEmpty()){
         qDebug() << "PrintAllPasswords_v :: User-list is empty!";
         SetJsonPackageHead_v(MainObject_o,"passwords_list", "failed");
         response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
         return;
       }

     for (auto It : Users_list){
         QString Name_str = It;
         QSqlQuery Query_lst;
         //получим id пользователя для дальнейших запросов
         Query_lst = _DatabaseWorker_o.QueryExec_lst(GetUserIdQuery_str,  QList<QPair<QString, QString>>() << qMakePair(QString(":p_name"), Name_str), false);
         Query_lst.next();
         QString UserId_str = Query_lst.value("user_id").toString();
         Query_lst = _DatabaseWorker_o.QueryExec_lst(GetLastOperationQuery_str, QList<QPair<QString, QString>>() << qMakePair(QString(":p_user_id"),UserId_str), false, QSql::InOut);
         Query_lst.next();
         QJsonObject OneUserObject_o;
         OneUserObject_o["UserName"] = Name_str;
         QString Password_str = Query_lst.value("pswd").toString();
         OneUserObject_o["Password"] = Password_str;
         DataArray_o.append(OneUserObject_o);
       }
     MainObject_o["data"] = DataArray_o;
     response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
   }

   void ControllerUsers::HostListALD_v(stefanfrings::HttpResponse& response, AldCommandState UACSState_en)
   {
#if !defined(__NO_ASTRA_LINUX__)
     _SavedHostList_slist = osdsapi->GetValidation_po()->ALDProcess_lst("host-list");
#else //отладка команды не на астре (будет удалено)
     QFile file("host-list.txt");
     if (!file.open(QIODevice::ReadOnly))
       return;
     QTextStream Stream_o ( &file );
     Stream_o.setCodec(QTextCodec::codecForName("UTF-8"));
     while( !Stream_o.atEnd() ) {
         _SavedHostList_slist.append(Stream_o.readLine());
       }
     file.close();
#endif
     if (UACSState_en == ACSAnswer){
         QJsonObject MainObject_o;
         SetJsonPackageHead_v(MainObject_o, "get_hosts", "done");
         QJsonArray DataArray_o = MainObject_o["data"].toArray();
         for(auto Name_str : _SavedHostList_slist) {
             QJsonObject ArrayObject_o;
             ArrayObject_o["host"] = Name_str;
             QHostInfo HostInfo_o = QHostInfo::fromName(Name_str);
             foreach(const QHostAddress & address, HostInfo_o.addresses()) {
                 ArrayObject_o["ip"] = address.toString();
               }
             DataArray_o.append(ArrayObject_o);
           }
         MainObject_o["data"] = DataArray_o;
         //----- Добавим к ответу папки из Settings, для которых будет проводиться поиск приложений -----
         MainObject_o["path"] = QString(_InstalledSoftwarePath_str.split(" ").join(", "));
         response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
       }
   }

   void ControllerUsers::GetApplicationsOnHost_v(QString HostName_str, QString UserName_str, stefanfrings::HttpResponse& response)
   {
     QJsonObject MainObject_o;
     QList<QPair<QString,QString>> ResultOutput_list;//список пар: название программы, статус(заблокирована/разблокирована)
#if defined(__NO_ASTRA_LINUX__)
     HostName_str = QString("127.0.0.1");
#endif
     try{
       SSHWorker SSHWorker_o(HostName_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());

       QStringList PathFile_slist = _InstalledSoftwarePath_str.split(" ");
       //запишем результат во временный файл, чтобы использовать его для формирования команды на setfacl
       QFile OutputFile_o(QString(_UserOutputPath_str + "executable_files_list.txt"));
       if (OutputFile_o.open(QIODevice::WriteOnly | QIODevice::Text)){
         QTextStream Stream_o (&OutputFile_o);
         Stream_o.setCodec(QTextCodec::codecForName("UTF-8"));
         QString Command_str;//формируем команду find
         QString Path_str;//соберем все пути в одну строку через пробел
         for (auto i : PathFile_slist){
             QString CheckDirectory_str = "sudo [ -d \"" + i + "\" ] && echo \"Directory exists.\"";
             if(SSHWorker_o.ExecCommandOnHost_b(CheckDirectory_str)){
                 if (SSHWorker_o.GetLastOutput_lst().at(0) != ""){
                     Path_str = Path_str + i + " ";
                 }
                 else {
                     SetJsonPackageHead_v(MainObject_o, "get_applications", "no_such_path");//не существует указанного пути
                     MainObject_o["path"] = i;
                     response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
                     return;
                 }
             }
         }
         Command_str = QString("sudo find " + Path_str + "-executable -type f > /tmp/find_output.txt;");
         if(SSHWorker_o.ExecCommandOnHost_b(Command_str)) {
           if(!SSHWorker_o.RecvFileFromHost_b("/tmp/find_output.txt", QString(_UserOutputPath_str +"find_output.txt"))) { // Копируем файл на сервер
             qWarning("ControllerUsers::Recieve file is crashed! /tmp/find_output.txt");
           }
         }
         QStringList Result_lst = ReadFromFile_slist(QString(_UserOutputPath_str +"find_output.txt"));//список всех абсолютных путей исполняемых файлов
         Result_lst.removeAll(QString(""));
         QString FilesGetFAcl_str;//запишем результат find в строку через пробел
         for (auto j: Result_lst){
           QString NoSpaces_str = j.replace(QString(" "),QString("\\ "));
           FilesGetFAcl_str.append(NoSpaces_str + " ");//если в названиях файлов есть пробелы - экранируем
           Stream_o << NoSpaces_str << "\n";
         }
         OutputFile_o.close();

         if (FilesGetFAcl_str == ""){
           SetJsonPackageHead_v(MainObject_o, "get_applications", "empty_directory");//в указанных путях ничего нет
           response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
           return;
         }

         QString OutputCommand_str = "sudo getfacl " + FilesGetFAcl_str + "> /tmp/getfacl_file.txt;";
         QStringList ResultOutputCommand_lst;//список с данными команды getfacl по всем файлам
         if (SSHWorker_o.ExecCommandOnHost_b(OutputCommand_str)){
           if(!SSHWorker_o.RecvFileFromHost_b("/tmp/getfacl_file.txt", QString(_UserOutputPath_str +"getfacl_file.txt"))) { // Копируем файл на сервер
             qWarning("ControllerUsers::Recieve file is crashed! /tmp/getfacl_file.txt");
           }
           ResultOutputCommand_lst = ReadFromFile_slist(QString(_UserOutputPath_str +"getfacl_file.txt"));
         }
         //разбиваем огромный вывод на выводы для каждого файла
         QList <QStringList> OutPutOneFile_list;
         QStringList OneList_slist;
         for (auto i : ResultOutputCommand_lst){
           if( i != ""){
             OneList_slist.append(i);
           }
           else {
             OutPutOneFile_list.append(OneList_slist);
             OneList_slist.clear();
           }
         }

         if (OutPutOneFile_list.empty()){
           SetJsonPackageHead_v(MainObject_o, "get_applications", "no_exec_files");//не нашлось исполняемых файлов
         }
         else {
           //парсим каждый маленький вывод для поиска прав
           for (auto StringList_o : OutPutOneFile_list){
             if (StringList_o.isEmpty())
               break;
             QPair<QString,QString> State_pair = GetFileAccessState_str(UserName_str, StringList_o);
             ResultOutput_list.append(State_pair);
           }
           //ответ на клиент
           SetJsonPackageHead_v(MainObject_o, "get_applications", "done");
           QJsonArray DataArray_o = MainObject_o["data"].toArray();
           for (auto i : ResultOutput_list){
             QJsonObject ArrayObject_o;
             ArrayObject_o["app_name"] = i.first;
             ArrayObject_o["app_state"] = i.second;
             DataArray_o.append(ArrayObject_o);
           }
           MainObject_o["data"] = DataArray_o;
         }
       }
       else {
         SetJsonPackageHead_v(MainObject_o, "get_applications", "failed");
       }
     }
     catch(OSDSException & e) {
       qWarning()<<e.what();
       SetJsonPackageHead_v(MainObject_o, "get_applications", "ssh_error");
     }
     response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
   }

   QPair<QString,QString> ControllerUsers::GetFileAccessState_str(QString UserName_str, QStringList CommandGetFAcl_slist)
   {
     CommandGetFAcl_slist.removeAll(QString(""));
//     QString FullPath_str;
//     if (CommandGetFAcl_slist.at(0).indexOf("# file: ") > -1){
//       FullPath_str = CommandGetFAcl_slist.at(0).right(CommandGetFAcl_slist.at(0).size() - QString("# file: ").size());
//     }
     QString FileName;
     if (!CommandGetFAcl_slist.isEmpty() && CommandGetFAcl_slist.at(0).indexOf("# file: ") > -1){
       QString String_o = CommandGetFAcl_slist.at(0);
       FileName = String_o.right(String_o.size() - String_o.lastIndexOf("/") - 1);
     }
     QString GetFACLTemplate_str = QString("user:" + UserName_str + ":");
     QString Result_str;
     for(auto i : CommandGetFAcl_slist){
       if (i.indexOf(GetFACLTemplate_str) > -1){
         QString Access_str = i.right(3);
         Result_str = (Access_str == "---") ? "locked" : "unlocked";
         break;
       }
       Result_str = "unlocked";
     }
     return qMakePair(FileName, Result_str);
   }

   void ControllerUsers::SetACLOnExecFiles_v(QString HostName_str, QString UserName_str, QString FileName_str, bool LockAccess_b, stefanfrings::HttpResponse& response)
   {
     QFile DebugACLFile(QString(_UserOutputPath_str + "GetFaclDebug.txt"));
     QJsonObject MainObject_o;
     QString CommandState_str;
     //откроем временный файл со списком исполняемых файлов, запрошенных ранее,чтобы получить абс путь к исполняемому файлу
     QStringList FilePath_list = ReadFromFile_slist(QString(_UserOutputPath_str + "executable_files_list.txt"));

     if (FilePath_list.isEmpty()){
       SetJsonPackageHead_v(MainObject_o, "lock_access", "failed");
       MainObject_o["file_name"] = FileName_str;
     }
     else {
       QString FullPathToFile_str;
       for (auto i : FilePath_list){
         if (i.indexOf(FileName_str) > -1){
           FullPathToFile_str = i;
           break;
         }
       }
       //формирование команды на хост
       QString LockAccess_str = (LockAccess_b) ? "---" : "rwx";
       QString CommandSetFAcl_str = QString("sudo setfacl -m u:" + UserName_str + ":" + LockAccess_str + " " + FullPathToFile_str);//установка новых прав
       QString CommandGetFAcl_str = QString("sudo getfacl " + FullPathToFile_str);//для проверки установленных прав
       QStringList Result_lst;
#if defined(__NO_ASTRA_LINUX__)
       HostName_str = QString("127.0.0.1");
#endif
       //#if !defined(__NO_ASTRA_LINUX__)
       try{
         SSHWorker SSHWorker_o(HostName_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
         if(SSHWorker_o.ExecCommandOnHost_b(CommandSetFAcl_str)) {
             if (SSHWorker_o.ExecCommandOnHost_b(CommandGetFAcl_str)){
                 Result_lst = SSHWorker_o.GetLastOutput_lst();
               }
           }
         Result_lst.removeAll(QString(""));

         if (DebugACLFile.open(QIODevice::WriteOnly | QIODevice::Text)){
             QTextStream Stream_o (&DebugACLFile);
             Stream_o.setCodec(QTextCodec::codecForName("UTF-8"));
             CommandState_str = "unlocked";
             Stream_o << "sudo log/pass:" << ControllerLogin::GetUserName_str() << " " << ControllerLogin::GetPassword_str();
             Stream_o << "\n";
             Stream_o << "----------------------------------------------------------------------";
             Stream_o << "\n";
             Stream_o << "SetACLCommand arguments";
             Stream_o << "\n";
             Stream_o << "HostName_str = " << HostName_str;
             Stream_o << "\n";
             Stream_o << "UserName_str = " << UserName_str;
             Stream_o << "\n";
             Stream_o << "FileName_str = " << FileName_str;
             Stream_o << "\n";
             Stream_o << "LockAccess_b = " << LockAccess_b;
             Stream_o << "\n";
             Stream_o << "----------------------------------------------------------------------";
             Stream_o << "\n";
             Stream_o << "GetACLCommand:";
             for (auto i : Result_lst){
                 Stream_o << "\n";
                 Stream_o << i;
                 if (i.indexOf(QString("user:" + UserName_str + ":" + LockAccess_str)) > -1){
                     CommandState_str = (i.right(3) == "---") ? "locked" : "unlocked";
                     break;
                   }
               }
             Stream_o << "\n";
             Stream_o << "----------------------------------------------------------------------";
             Stream_o << "\n";
             Stream_o << "SetACLCommand:";
             Stream_o << "\n";
             Stream_o << CommandSetFAcl_str;
             DebugACLFile.close();
           }
         SetJsonPackageHead_v(MainObject_o, "lock_access", "done");
         MainObject_o["setfacl_command"] = CommandSetFAcl_str;
         MainObject_o["command_state"] = CommandState_str;
         MainObject_o["app_name"] = FileName_str;
       }
       catch(OSDSException & e) {
         qWarning()<<e.what();
         SetJsonPackageHead_v(MainObject_o, "lock_access", "failed");
         MainObject_o["setfacl"] = QString("failed");
       }
     }
     response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
   }

   QStringList ControllerUsers::ReadFromFile_slist(QString FileName_str)
   {
     QFile file(FileName_str);
     if (!file.open(QIODevice::ReadOnly))
       return QStringList();
     QStringList Data_slist;
     QTextStream Stream_o ( &file );
     Stream_o.setCodec(QTextCodec::codecForName("UTF-8"));
     while( !Stream_o.atEnd() ) {
         Data_slist.append(Stream_o.readLine());
       }
     file.close();
     return Data_slist;
   }

   void ControllerUsers::CheckCgiHostAccess_v(stefanfrings::HttpResponse& response)
   {
     QJsonObject MainObject_o;
     SetJsonPackageHead_v(MainObject_o, "check_cgi_host_access", "done");
     QTcpServer *QTcpServer_po = new QTcpServer(this);

     if (!QTcpServer_po->listen(QHostAddress(_WebServerHostIP_str))){
         MainObject_o["host_state"] = "unreachable";
       }
     else{
         MainObject_o["host_state"] = "connected";
         MainObject_o["host_ip"] = _WebServerHostIP_str;
       }
     response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
//под комментарием доработка для проерки состояния веб-сервиса без помощи клиента
//     QString WebService_str = QString("http://" + _WebServerHostIP_str + "/cgi-bin/generatorCgi");
//     // создаем объект для запроса
//     QNetworkRequest NetworkRequest_o(WebService_str);
//     NetworkRequest_o.setRawHeader("Accept","text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
//     NetworkRequest_o.setRawHeader("Accept-Language", "ru-RU,ru;q=0.8,en-US;q=0.5,en;q=0.3");
//     NetworkRequest_o.setRawHeader("Connection","keep-alive");
//     NetworkRequest_o.setRawHeader("Access-Control-Allow-Headers","origin, x-requested-with, content-type");
//     NetworkRequest_o.setHeader(QNetworkRequest::ContentTypeHeader, "text/html; charset=UTF-8");
////     QNetworkReply* QNetworkAccessManager::head(NetworkRequest_o);
//     _NetworkManager_po->get(NetworkRequest_o);
//     QNetworkReply* reply = _NetworkManager_po->head(NetworkRequest_o);
//     QFile file("/tmp/NetworkAccess.txt");
//     if (!file.open(QIODevice::ReadWrite))
//       return;
//     QTextStream Stream_o ( &file );
//     Stream_o.setCodec(QTextCodec::codecForName("UTF-8"));
//     Stream_o << reply->header(QNetworkRequest::ContentTypeHeader).toByteArray();
//     file.close();
//     // Подписываемся на сигнал о готовности загрузки
//     QObject::connect(reply, SIGNAL(finished()), this, SLOT(replyFinished_slt()));
   }

   void ControllerUsers::CheckUserAsLocalOnHost_v(stefanfrings::HttpResponse& response, QString HostName_str, QString UserName_str)
   {
     QJsonObject MainObject_o;
     QStringList Result_lst;
#if defined(__NO_ASTRA_LINUX__)
     HostName_str = QString("127.0.0.1");
#endif
     try {
       SSHWorker SSHWorker_o(HostName_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
       QString Command_str = QString("sudo getent passwd");
       if(SSHWorker_o.ExecCommandOnHost_b(Command_str)) {
           Result_lst = SSHWorker_o.GetLastOutput_lst();
//       Result_lst = ReadFromFile_slist("getent.txt");
           Result_lst.removeAll(QString(""));
           for (auto String : Result_lst){
               if (String.indexOf(UserName_str) != -1){
                   SetJsonPackageHead_v(MainObject_o, "compare_with_local_users", "done");
                   response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
                   return;
                 }
               else {
                   SetJsonPackageHead_v(MainObject_o, "compare_with_local_users", "failed");
                 }
             }
         }
     }
     catch(OSDSException & e) {
       qWarning()<<e.what();
       SetJsonPackageHead_v(MainObject_o, "compare_with_local_users", "exception");
     }
     response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
   }

   void ControllerUsers::replyFinished_slt()
   {
////     if (_NetworkReply_po == nullptr)
////       return;

//     QFile file("/tmp/NetworkAccess.txt");
//     if (!file.open(QIODevice::ReadWrite))
//       return;
//     QTextStream Stream_o ( &file );
//     Stream_o.setCodec(QTextCodec::codecForName("UTF-8"));

////     if (_NetworkReply_po->error() == QNetworkReply::NoError){
////         QByteArray content= _NetworkReply_po->readAll();// Получаем содержимое ответа
////         QTextCodec *codec = QTextCodec::codecForName("utf8");// Реализуем преобразование кодировки
////         Stream_o << codec->toUnicode(content.data());
////       }
////     else {
////         // Выводим описание ошибки, если она возникает.
////         Stream_o << _NetworkReply_po->errorString();
////       }
//     file.close();
   }
}
