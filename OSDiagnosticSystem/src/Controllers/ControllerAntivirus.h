#ifndef CONTROLLERANTIVIRUS_H
#define CONTROLLERANTIVIRUS_H

#include <QObject>
#include <QMap>
#include <QMutex>

#include "ControllerAbstract.h"
#include "constants.h"

namespace osds {
  /** @brief CallbackCreateControllerAntivirus_po Создает объект класса ControllerLogin
    * @return указатель на ControllerAntivirus (stefanfrings::HttpRequestHandler) */
  QObject * CallbackCreateControllerAntivirus_po();

  class ControllerAntivirus : public ControllerAbstract
  {
    Q_OBJECT
    Q_DISABLE_COPY(ControllerAntivirus)

    static QString _DrWebPath_str;  ///< @brief Путь к папке с дистрибутивом Dr.Web
    static QString _DrWebLicenseKey_str;  ///< @brief Путь к файлу лицензии
    static QString _DrWebUpdateUrl_str;  ///< @brief Путь к обновлениям DrWeb
    static QString _KasperskyPath_str;  ///< @brief Путь к папке с дистрибутивом Kaspersky
    static QString _KasperskyLicenseKey_str;  ///< @brief Путь к файлу лицензии для каспера
    static QString _KasperskyAutoinstall_str;  ///< @brief Путь к файлу тихой установки
    static QString _KasperskyUpdateUrl_str;  ///< @brief Путь к обновлениям kaspersky

    static QMap<QString, EnHostRunState> _HostScanState_smap; // Вывод состояний сделан через map
    static QMap<QString, EnHostRunState> _AntivirusUpdateState_smap; // Вывод состояний сделан через map
    static QMap<QString, EnHostRunState> _AntivirusInstallState_smap; // Вывод состояний сделан через map
    static QMap<QString, EnHostRunState> _AntivirusRemoveState_smap; // Вывод состояний сделан через map
    static QMutex _Mutex_so; ///< @brief Блокировка для *_smap

    /** @brief Структура состояний антивируса */
    struct SAntivirusState {
      QString _Version_str; ///< @brief Версия антивируса
      bool _IsAntivirusStateOk_b; ///< @brief Состояние работоспособности ПО
    };

  public:
    ControllerAntivirus(QObject * parent = Q_NULLPTR);

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
    static void InitializeDataBase_settings();

  private:
    /**
     * @brief FormationHostsList Формирует и отправляет JSON со списком хостов, антивирусов
     * @param response Ответка на клиент
     */
    void GetHostsList_v(stefanfrings::HttpResponse& response);

    void ListSaveFullDirs(stefanfrings::HttpResponse& response);

    void ListSaveFullAppendDirs(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response);

    void ListSaveFullRemoveDirs(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response);

    void ListSaveFullUpdateSettings(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response);


    void ListSaveFullSettings(stefanfrings::HttpResponse& response);
    /**
     * @brief InstallAntivirusOnHost_v Устанавливает дистрибутив антивируса на хост
     * @param Host_str Имя хоста
     * @param Distrib_str Имя дистрибутива
     * @param response Ответка на клиент
     */
    void InstallAntivirusOnHost_v(const QString & Host_str, const QString & Distrib_str, stefanfrings::HttpResponse& response);
    /**
     * @brief RemoveAntivirus_v Удаление антивируса с хоста
     * @param Host_str Имя хоста
     * @param response Ответка на клиент
     */
    void RemoveAntivirus_v(const QString & Host_str, stefanfrings::HttpResponse& response);
    /**
     * @brief StartAntivirusRemoveInThread_po Удаление выбранного антивируса на хост
     * @param Host_str Имя хоста
     * @param Distrib_str Название дистрибутива
     * @return Созданный поток
     */
    QThread * StartAntivirusRemoveInThread_po(const QString & Host_str, const QString & Distrib_str);
    /**
     * @brief GetStateAntivirusRemove_v Статус удаления антивируса с хоста
     * @param Host_str Имя хоста
     * @param response Ответка на сервер
     */
    void GetStateAntivirusRemove_v(const QString & Host_str, stefanfrings::HttpResponse& response);
    /**
     * @brief ScanDirsOnHost_v Проведение сканирования выбранных каталогов на хосте
     * @param Host_str Имя хоста
     * @param Dirs_lst Набор каталогов
     * @param response Ответка на клиента
     */
    void ScanDirsOnHost_v(const QString & Host_str, QStringList Dirs_lst, stefanfrings::HttpResponse& response);
    /**
     * @brief SetHostType_v Установка типа проверки для хоста
     * @param Host_str Имя хоста
     * @param Type_str  Тип проверки
     * @param DirList_str Список директорий для проверки (только для выборочной проверки)
     * @param response Ответка на клиента
     */
    void SetHostType_v(const QString & Host_str, const QString & Type_str, const QString & DirList_str, stefanfrings::HttpResponse& response);
    /**
     * @brief GetHostDirList_v Запрос на формирование списка директорий на хосте
     * @param Host_str Имя хоста
     * @param response Ответка на клиента
     */
    void GetHostDirList_v(const QString & Host_str, const QString & Dir_str, stefanfrings::HttpResponse& response);
    /**
     * @brief ScanHost_v  Скагирует выбранный хост выбранным антивирусом и типом
     * @param Host_str Имя хоста
     * @param response Ответка на клиента
     */
    void ScanHost_v(const QString & Host_str, stefanfrings::HttpResponse& response, bool is_full_lan);
    /**
     * @brief GetStateScanHost_v Возвращает текущий статус сканирования
     * @param Host_str Имя хоста
     * @param response Ответка на клиента
     */
    void GetStateScanHost_v(const QString & Host_str, stefanfrings::HttpResponse& response);
    /**
     * @brief StopHostScanned_v Останавливает процесс сканирования хоста
     * @param Host_str Имя хоста
     * @param response Ответка на клиента
     */
    void StopHostScanned_v(const QString & Host_str, stefanfrings::HttpResponse& response);
    /**
     * @brief GetScanJournal_v Возвращает журнал с последней проверки
     * @param Host_str Имя хоста
     * @param response Ответка на клиента
     */
    void GetScanJournal_v(const QString & Host_str, stefanfrings::HttpResponse& response);
    /**
     * @brief AntivirusUpdate_v Запуск обновления антивируса
     * @param Host_str Имя хоста
     * @param response Ответка на клиента
     */
    void AntivirusUpdate_v(const QString & Host_str, stefanfrings::HttpResponse& response);
    /**
     * @brief GetStateAntivirusUpdate_v Проверка состояния обновления антивируса
     * @param Host_str Имя хоста
     * @param response Ответка на клиента
     */
    void GetStateAntivirusUpdate_v(const QString & Host_str, stefanfrings::HttpResponse& response);
    /**
     * @brief GetLicenseState_v Проверяет состояние лицензии
     * @param Host_str Имя хоста
     * @param response Ответка на клиента
     */
    void GetLicenseState_v(const QString & Host_str, stefanfrings::HttpResponse& response);
    /**
     * @brief GetDrWebVersionOnHost_str Узнает версию установленного Dr.Web на хосте
     * @param Host_str Имя хоста
     * @return Номер версии
     */
    SAntivirusState GetDrWebVersionOnHost_str(const QString & Host_str);
    /**
     * @brief GetKasperskyVersionOnHost_str Узнает версию установленного Dr.Web на хосте
     * @param Host_str Имя хоста
     * @return Номер версии
     */
    SAntivirusState GetKasperskyVersionOnHost_str(const QString & Host_str);
    /**
     * @brief GetScanCommand_str Формирует команду для сканирования хоста
     * @param Host_str Имя хоста
     * @return Строка с командой
     */
    QString GetScanCommand_str(const QString & Host_str, bool is_full_lan);
    /**
     * @brief StartScanHostInThread_v Запускает сканирование хоста в отдельном потоке
     * @param Host_str Имя хоста
     * @param ScanCommand_str Команда сканирования
     * @return Созданный поток
     */
    QThread * StartScanHostInThread_po(const QString & Host_str, const QString & ScanCommand_str);
    /**
     * @brief StartAntivirusUpdateInThread_po Запускает обновление антивируса на определенном хосте
     * @param Host_str Имя хоста
     * @param AntivirusType_str Тип антивируса
     * @return Созданный поток
     */
    QThread * StartAntivirusUpdateInThread_po(const QString & Host_str, const QString & AntivirusType_str, const QString & UpdateUrl_str);
    /**
     * @brief StartAntivirusInstallInThread_po Установка выбранного антивируса на хост
     * @param Host_str Имя хоста
     * @param Distrib_str Название дистрибутива
     * @return Созданный поток
     */
    QThread * StartAntivirusInstallInThread_po(const QString & Host_str, const QString & Distrib_str);
    /**
     * @brief GetStateAntivirusInstall_v Статус установки антивируса на хост
     * @param Host_str Имя хоста
     * @param response Ответка на сервер
     */
    void GetStateAntivirusInstall_v(const QString & Host_str, stefanfrings::HttpResponse& response);
  };

}
#endif // CONTROLLERANTIVIRUS_H
