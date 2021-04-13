#ifndef CONTROLLERINTEGRITYCONTROL_H
#define CONTROLLERINTEGRITYCONTROL_H

#include <QObject>
#include <QMutex>

#include "ControllerAbstract.h"
#include "constants.h"

namespace osds {

  /** @brief CallbackCreateControllerIntegrityControl_po Создает объект класса ControllerIntegrityControl
 *  @return указатель на ControllerIntegrityControl (stefanfrings::HttpRequestHandler) */
  QObject * CallbackCreateControllerIntegrityControl_po();
  /**
   * @brief The ControllerIntegrityControl class Контроллер для управления контролем целостности
   */
  class ControllerIntegrityControl : public ControllerAbstract
  {
    Q_OBJECT
    Q_DISABLE_COPY(ControllerIntegrityControl)

    static QMap<QString, ThreadHostState> _HostState_smap; // Вывод состояний сделан через map
    static QMutex _Mutex_so; ///< @brief Блокировка для _TestHostState_smap
    /** @brief Данные о прохождении аудита */
    class CIAuditData {
      QString _Name_str;  ///< @brief имя файла
      QString _Date_str;  ///< @brief Дата проведения
      QString _Audit_str; ///< @brief Строка аудита
      public :
      CIAuditData(const QString & Name_str, const QString & Date_str, const QString & Audit_str) :
        _Name_str(Name_str),
        _Date_str(Date_str),
        _Audit_str(Audit_str) {}
      /** @brief Возвращает имя файла/папки */
      const QString & GetName_str() const { return _Name_str; }
      /** @brief Возвращает дату/время */
      const QString & GetDate_str() const { return _Date_str; }
      /** @brief Возвращает строку аудита */
      const QString & GetAudit_str() const { return _Audit_str; }
    };
    static QMap<QString, QList<CIAuditData> > _CIAuditData_smap;  ///< @brief Набор данных прохождения аудита

  public:
    ControllerIntegrityControl(QObject * parent = Q_NULLPTR);

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
    QString _LastError_str; ///< @brief Последняя актуальная ошибка

    /**
     * @brief GetHostsList_v Возвращает список хостов
     * @param response Ответка на сервер
     */
    void GetHostsList_v(stefanfrings::HttpResponse & response);
    /**
     * @brief InstallPoint_v Установка контрольной точки КЦ для хоста
     * @param Host_str Имя хоста
     * @param response Ответка на сервер
     */
    void InstallPoint_v(const QString & Host_str, stefanfrings::HttpResponse & response);
    /**
     * @brief StartInstallPointThread_po Запуск потока установки контрольной точки КЦ
     * @param Host_str Имя хоста
     */
    QThread * StartInstallPointThread_po(const QString & Host_str);
    /**
     * @brief GetStateInstallPoint_v Запрос статуса установки КЦ
     * @param response Ответка на сервер
     */
    void GetStateInstallPoint_v(const QString & Host_str, stefanfrings::HttpResponse& response);
    /**
     * @brief ComparePoint_v Проверка контрольной точки КЦ
     * @param Host_str Имя хоста
     * @param response Ответка на сервер
     */
    void ComparePoint_v(const QString & Host_str, stefanfrings::HttpResponse & response);
    /**
     * @brief StartComparePointThread_po Запуск потока проверки КЦ
     * @param Host_str Имя хоста
     */
    QThread * StartComparePointThread_po(const QString & Host_str);
    /**
     * @brief GetStateInstallPoint_v Запрос статуса проведения КЦ
     * @param response Ответка на сервер
     */
    void GetStateComparePoint_v(const QString & Host_str, stefanfrings::HttpResponse& response);
    /**
     * @brief GetAfickDirs_v Запрос папок для проведения КЦ
     * @param response Ответка на сервер
     */
    void GetAfickDirs_v(const QString & Host_str, stefanfrings::HttpResponse& response);
    /**
     * @brief SetAfickDirs_v Установка элементов для проведения КЦ
     * @param Host_str Имя хоста
     * @param SetDirs_str Проверяемые каталоги
     * @param response Ответка на сервер
     */
    void SetAfickDirs_v(const QString & Host_str, const QString & SetDirs_str, bool IsAudit_b, stefanfrings::HttpResponse& response);
    /**
     * @brief GetAfickDirsFromConf_v Парсит файл конфигурации и формирует список проверяемых каталогов
     */
    bool GetAfickDirsFromConf_v(const QString & Host_str, const QString & File_str,
                                QList<QPair<QString, QString> > & AddDirs_str, QStringList & IgnoreDirs_lst,
                                QStringList & Aliases_lst);
  };
}

#endif // CONTROLLERINTEGRITYCONTROL_H
