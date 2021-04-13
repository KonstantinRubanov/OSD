#ifndef CONTROLLERALD_H
#define CONTROLLERALD_H

#include <QObject>

#include "ControllerAbstract.h"
#include "sshworker.h"
#include "osdsexception.h"
#include "databaseworker.h"



const QString InitDBSave[] = {
        "CREATE TABLE IF NOT EXISTS hosts_save(\
        hostname text,\
        dir_to_save text,\
        PRIMARY KEY (hostname, dir_to_save));",

        "CREATE TABLE IF NOT EXISTS hosts_save_history(\
        id_history INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE,\
        hostname text,\
        dir_to_save text,\
        id_operation INTEGER DEFAULT 0,\
        uid_process text,\
        id_state_process INTEGER DEFAULT -1,\
        arc_file text,\
        errors text,\
        time_start TIMESTAMP DEFAULT (datetime('now','localtime')),\
        time_stop TIMESTAMP DEFAULT (datetime('now','localtime')),\
        FOREIGN KEY (hostname, dir_to_save) REFERENCES hosts_save(hostname, dir_to_save))",

        "CREATE TABLE IF NOT EXISTS save_server(\
         ftp_server text,\
         ftp_user text,\
         ftp_pass text,\
         tmp_hostname text,\
         PRIMARY KEY (ftp_server));",

        "CREATE TRIGGER IF NOT EXISTS delete_client BEFORE DELETE ON hosts_save FOR EACH ROW \
        BEGIN \
          DELETE FROM hosts_save_history WHERE hostname = OLD.hostname AND dir_to_save = OLD.dir_to_save; \
        END ",

        "CREATE TABLE IF NOT EXISTS hosts_ald_alive(\
            hostname text,\
            id_state INTEGER DEFAULT 0,\
            time_stop TIMESTAMP DEFAULT (datetime('now','localtime')),\
            PRIMARY KEY (hostname));"
        };


namespace osds {
  /** @brief CallbackCreateControllerALD_po Создает объект класса ControllerLogin
 *  @return указатель на ControllerALD (stefanfrings::HttpRequestHandler) */
  QObject * CallbackCreateControllerALD_po();

  class ControllerALD : public ControllerAbstract
  {
    Q_OBJECT
    Q_DISABLE_COPY(ControllerALD)

  public:
    ControllerALD(QObject * parent = Q_NULLPTR);

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

    static void SetSshParams(const QString _Username_, const QString _Password_);

  private:
    static QString _Username_str;  ///< @brief Имя для входа по SSH
    static QString _Password_str;  ///< @brief Пароль для входа по SSH

    bool RestartSSh();
    bool InitTables();
  };


}
#endif // CONTROLLERALD_H
