#ifndef CONTROLLERUSERS_H
#define CONTROLLERUSERS_H

#include <QObject>
#include <QNetworkAccessManager>

#include "ControllerAbstract.h"
#include "sshworker.h"
#include "osdsexception.h"
#include "databaseworker.h"

namespace osds {
  /** @brief CallbackCreateControllerUsers_po Создает объект класса ControllerUsers
 *  @return указатель на ControllerUsers (stefanfrings::HttpRequestHandler) */
  QObject * CallbackCreateControllerUsers_po();

  class ControllerUsers : public ControllerAbstract
  {
    Q_OBJECT
    Q_DISABLE_COPY(ControllerUsers)

    static QString _InstalledSoftwarePath_str;  ///< @brief Пути к местоположению установленных программ на хосте, для управления доступом к ним(разделены пробелом)
    static QString _UserOutputPath_str;         ///< @brief Путь к файлам с информацией о пользователе, паролях и тд
    static QString _WebServerHostIP_str;        ///< @brief Адрес удаленного веб-сервера (для работы нашего клиента с generatorCgi)
    static QString _WebServerHostPort_str;      ///< @brief Порт удаленного веб-сервера (для мониторинга его состояния)
  public:
    ControllerUsers(QObject * parent = Q_NULLPTR);

    void service(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response) override;
    /**
     * @brief SetSettings_sv Установка данных из настроек
     * @param Settings_ro Настройки
     */
    static void SetSettings_sv(QSettings & Settings_ro);
    /**
     * @brief InitializeDataBase_sv Инициализация БД
     */
    static void InitializeDataBase_sv();
  private:
    enum AldCommandState {//Режим работы функции UsersALD_v, HostListALD_v
      ACSNone,
      ACSDoNotAnswer, //только сохраняем результат запроса информации о пользователях в мап
      ACSAnswer //отвечаем на клиент
    };

    enum DBManageInitializer {//кто инициализирует работу с БД хранения пользователей и паролей
      DBMINone,
      DBMIUserlist, //запрос для вывода списка пользователей и обновления информации в бд из user-get
      DBMIPasswordGenerator //запрос для записи в бд пароля после получения сгенерированных паролей
    };

    /**
     * @brief UserNamesALD_v возвращает только имена пользователей ALD (за счет этой функции можно будет оптимизировать следующие)
     * @param response ответ на клиент
     */
    void UserNamesALD_v(stefanfrings::HttpResponse& response);
    /**
     * @brief GetChangePasswordState_str получает из бд статус операции по смене пароля
     * @return "change_by_user"/"change_by_admin"
     */
    QString GetChangePasswordState_str(QString UserName_str);
    /**
     * @brief UsersALD_v возвращает информацию по всем пользователям ALD
     * @param response ответ на клиент
     * @param UACSState_en возвращаем ответ на клиент или только выполняем запрос алд
     */
    void UsersALD_v(stefanfrings::HttpResponse& response, AldCommandState UACSState_en);
    /**
     * @brief ManageDB_v заполнение таблиц с паролями и статусами
     * @param CurrentDateTime_str дата последней смены пароля из user-get
     * @param Password_str новое значение пароля
     * @param UserName_str имя пользователя для выполнения запросов к бд
     * @param DBMI_en кто инициатор смены пароля (если генерируем мы, то админ, во всех остальных случаях будет написан пользователь)
     */
    bool ManageDB_v(QString CurrentDateTime_str, QString Password_str, QString UserName_str, DBManageInitializer DBMI_en);
    /**
     * @brief ParseUserList_v раскладывает список с информацией о пользователе в json
     * @param JsonObject_o массив по данным одного пользователя под тегом data
     * @param list массив с данными одного пользователя
     */
    void ParseUserList_v(QJsonObject &JsonObject_o, QStringList & list);
    /**
     * @brief UsersALD_v разблокирование всех пользователей ALD
     * @param response ответ на клиент
     */
    void UnlockAllUsers_v(bool LockAllUsers_b,  stefanfrings::HttpResponse& response);

    void LockUser_v(bool Lock_b, QString UserName_str, stefanfrings::HttpResponse& response);

    bool IsUserLocked_b(QString UserName_str);

    void ClearUserFaillog_v(QString UserName_str);
    /**
      * @brief ChangeUserPassword_v смена пароля пользователя ALD
      * @param UserName_str имя пользователя
      * @param NewPasswd_str новый пароль
      * @param response ответ на клиент
      */
    void ChangeUserPassword_v(QString UserName_str, QString NewPasswd_str, stefanfrings::HttpResponse& response);
    /**
      * @brief GeneratePassword_v генерация пароля
      */
    void GeneratePassword_v();
    /**
      * @brief PrintToFile_v вывод информации по выбранному пользователю в файл.
      * @param UserName_str имя пользователя
      * @param response ответ на клиент
      */
    void PrintToFile_v(QString UserName_str, stefanfrings::HttpResponse& response);
    /**
      * @brief PrintAllPasswords_v печать списка пользователей домена с паролями.
      * @param response ответ на клиент
      */
    void PrintAllPasswords_v(stefanfrings::HttpResponse& response);
    /**
      * @brief HostListALD_v возвращает список хостов ALD, сохраняет результат запроса в _SavedHostList_slist
      * @param UACSState_en возвращаем ответ на клиент или только выполняем запрос алд
      * @param response ответ на клиент
      */
    void HostListALD_v(stefanfrings::HttpResponse& response, AldCommandState UACSState_en);
    /**
      * @brief GetApplicationsOnHost_v возвращает список исполняемых файлов из папок, указанных в настройках OSDiagnosticSystem.ini,
      * со статусом для данного пользователя(заблокировано или нет)
      * @param HostName_str имя хоста
      * @param UserName_str имя пользователя
      * @param response ответ на клиент
      */
    void GetApplicationsOnHost_v(QString HostName_str, QString UserName_str, stefanfrings::HttpResponse& response);
    /**
      * @brief GetFileAccessState_str состояние доступа исполняемого файла для пользователя
      * @param HostName_str имя хоста
      * @param UserName_str имя пользователя
      * @param FullPathToFile_str полный путь к файлу
      * @param response ответ на клиент
      */
    QPair<QString, QString> GetFileAccessState_str(QString UserName_str, QStringList CommandGetFAcl_slist);
    /**
      * @brief SetACLOnExecFiles_v блокирует доступ пользователя к исполняемому файлу через ACL
      * @param HostName_str имя хоста, на котором находится файл
      * @param UserName_str имя пользователя
      * @param FileName_str имя файла
      * @param LockAccess_b true - заблокировать доступ, false - разблокировать
      * @param response ответ на клиент
      */
    void SetACLOnExecFiles_v(QString HostName_str, QString UserName_str, QString FileName_str, bool LockAccess_b, stefanfrings::HttpResponse& response);
    /**
      * @brief ReadFromFile_slist чтение из файла
      * @param FileName_str имя файла
      * @return построчный вывод в QStringList
      */
    QStringList ReadFromFile_slist(QString FileName_str);
    /**
      * @brief CheckCgiHostAccess_v проверка доступа к удаленному веб-серверу
      * @param response ответ на клиент
      */
    void CheckCgiHostAccess_v(stefanfrings::HttpResponse& response);
    /**
      * @brief CheckUserAsLocalOnHost_v проверка, заходил ли пользователь на хост
      * @param response ответ на клиент
      */
    void CheckUserAsLocalOnHost_v(stefanfrings::HttpResponse& response, QString HostName_str, QString UserName_str);

    QStringList _UsersFirstLevelKeys_slist; ///<@brief Первый уровень ключей для json с информацией о пользователях
    QStringList _UsersGroupsLevelKeys_slist; ///<@brief Вложения для ключа Groups в json с информацией о пользователях

    static QString _Username_str;  ///< @brief Имя для входа по SSH
    static QString _Password_str;  ///< @brief Пароль для входа по SSH
    DatabaseWorker _DatabaseWorker_o; ///<@brief Управление БД
    QStringList _SavedUserlist_slist; ///<@brief Сохранение списка пользователей
    QMap<QString,QStringList> _UserGetOutput_map; ///<@brief Сохранение всей информации по всем пользователям, входящим в домен АЛД
    QStringList _SavedHostList_slist; ///<@brief Сохранение списка хостов
    QNetworkAccessManager *_NetworkManager_po;
//    QNetworkReply *_NetworkReply_po;

  private slots:
    void replyFinished_slt();
  };
}
#endif // CONTROLLERUSERS_H
