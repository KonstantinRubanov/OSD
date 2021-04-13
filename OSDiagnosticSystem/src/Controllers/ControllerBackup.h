#ifndef CONTROLLERBACKUP_H
#define CONTROLLERBACKUP_H

#include <QObject>

#include "ControllerAbstract.h"
#include "sshworker.h"
#include "osdsexception.h"

namespace osds {
  /** @brief CallbackCreateControllerBackup_po Создает объект класса ControllerLogin
 *  @return указатель на ControllerBackup (stefanfrings::HttpRequestHandler) */
  QObject * CallbackCreateControllerBackup_po();

  class ControllerBackup : public ControllerAbstract
  {
    Q_OBJECT
    Q_DISABLE_COPY(ControllerBackup)

    static QString BackupPath_str;  ///< @brief Папка для записи бэкапов
    static QString TempBackupPath_str;  ///< @brief Папка (если локальная или адрес сервера) для записи бэкапов хоста
    static QString _LastLoadedBackup_sstr; ///< @brief Последнее загруженное обновление
    static QString _CorrectState_str; ///< @brief Текущее состояние
    /**
     * @brief The ERepositoryType enum Тип хранилища (должен совпадать с клиентской частью)
     */
    enum ERepositoryType {
      _RTLocal_en,  ///< @brief Локальное хранилище хоста
      _RTFtp_en,  ///< @brief Файловый сервер  с доступом по FTP
      _RTSsh_en ///< @brief Файловый сервер  с доступом по SSH
    };

  public:
    ControllerBackup(QObject * parent = Q_NULLPTR);

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
    /**
     * @brief BackupSave_v Сделать резервную копию ALD
     * @param response Ответка на клиента
     */
    void BackupALDSave_v(stefanfrings::HttpResponse& response);
    /**
     * @brief BackupLoad_v Загрузить резервную копию ALD
     * @param DTime_str Дата и время резервной копии
     * @param response Ответка на сервер
     */
    void BackupALDLoad_v(const QString & DTime_str, stefanfrings::HttpResponse& response);
    /**
     * @brief BackupALDRemove_v Удаление резервной копии и ключа
     * @param DTime_str Дата и время резервной копии
     * @param response Ответка на сервер
     */
    void BackupALDRemove_v(const QString & DTime_str, stefanfrings::HttpResponse& response);
    /**
     * @brief BackupList_v Сформировать лист всех резервных копий
     * @param response Ответка на сервер
     */
    void BackupList_v(stefanfrings::HttpResponse& response);
    /**
     * @brief CreateHostBackup_v Создание резервной копии носителей информации хоста
     * @param HostName_str Имя хоста
     * @param response Ответка на сервер
     */
    void CreateHostBackup_v(const QString & HostName_str, stefanfrings::HttpResponse& response);
    /**
     * @brief GetHostBackups_v Возвращает список существующих бэкапов для хоста
     * @param HostName_str Имя хоста
     * @param response Ответка на сервер
     */
    void GetHostBackups_v(const QString & HostName_str, stefanfrings::HttpResponse& response);
    /**
     * @brief LoadBackupToHost_v Восстановление HDD из бэкапа
     * @param HostName_str Имя хоста
     * @param DateTime_str Дата и время загружаемого бэкапа
     * @param response Ответка на сервер
     */
    void LoadBackupToHost_v(const QString & HostName_str, const QString & DateTime_str, stefanfrings::HttpResponse& response);
    /**
     * @brief RemoveBackupToHost_v Удаление бэкапа хоста
     * @param HostName_str Имя хоста
     * @param DateTime_str Дата и время загружаемого бэкапа
     * @param response Ответка на сервер
     */
    void RemoveBackupToHost_v(const QString & HostName_str, const QString & DateTime_str, stefanfrings::HttpResponse& response);
    /**
     * @brief InstallCurl_v Установка curl на хост
     * @param HostName_str Имя хоста
     */
    void InstallCurl_v(const QString & HostName_str, stefanfrings::HttpResponse& response);
    /**
     * @brief GetHostSettings_v Возвращает настройки РК выбранного хоста
     * @param HostName_str Имя хоста
     * @param response Ответка на сервер
     */
    void GetHostSettings_v(const QString & HostName_str, stefanfrings::HttpResponse& response);
    /**
     * @brief SetHostSettings_v Устанавливаетнастройки РК выбранного хоста
     * @param HostName_str Имя хоста
     * @param Settings_str Строка с настройками
     * @param response Ответка на сервер
     */
    void SetHostSettings_v(const QString & HostName_str, const QString & Settings_str, stefanfrings::HttpResponse& response);
  };
}
#endif // CONTROLLERBACKUP_H
