#ifndef CONTROLLERJOURNAL_H
#define CONTROLLERJOURNAL_H

#include <QObject>
#include <QFileInfoList>

#include "ControllerAbstract.h"

namespace osds {

  /** @brief CallbackCreateControllerJournals_po Создает объект класса ControllerLogin
 *  @return указатель на ControllerJournals (stefanfrings::HttpRequestHandler) */
  QObject * CallbackCreateControllerJournals_po();

  class ControllerJournals : public ControllerAbstract
  {
    Q_OBJECT
    Q_DISABLE_COPY(ControllerJournals)

    static QString _BackupPath_str;  ///< @brief Папка для записи бэкапов
    static QString _TmpPath_str;  ///< @brief Папка для записи временных файлов

#if !defined(__NO_ASTRA_LINUX__)
    const char * _AuthLogsPath = "/var/log";
#else //__NO_ASTRA_LINUX__
    const char * _AuthLogsPath = "/tmp/tmplog";  ///< @todo : Убрать после отработки
#endif  //__NO_ASTRA_LINUX__

    QMap<QString, int> _MonthNamesByNumber_map; ///< @brief Связь номеров месяца и номера (согдасно Astra)
  public:
    /**
     * @brief ControllerAbstract Основной конструктор
     * @param parent Родительский объект
     */
    ControllerJournals(QObject * parent = Q_NULLPTR);
    /**
     * @brief service Метод сервлета для контроллера
     * @param request Запрос
     * @param response  Ответ
     */
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
     * @brief Разбор файлов /var/log/auth.log
     * @return Возвращает пару start_date, stop_date
     */
    QPair<QString, QString> ParseAuthLogs_pair();
    /**
     * @brief GetArchiveFiles_v Собирает все архивные файлы и формирует массив
     * @param response Ответка на сервер
     */
    void GetListLogs_v(stefanfrings::HttpResponse& response);
    /**
     * @brief SaveLogArchive_v Сохранение данных из лога в архив
     * @param StartDate_str Начало записи
     * @param StopDate_str Конец записи
     * @param response Ответка на сервер
     */
    void SaveLogArchive_v(const QString & StartDate_str, const QString & StopDate_str, stefanfrings::HttpResponse& response);
    /**
     * @brief PrintLogData_v Вывод данных из лога на печать
     * @param StartDate_str Дата начала лога данных
     * @param StopDate_str Дата конца лога данных
     * @param response Ответка на сервер
     */
    void PrintLogData_v(const QString & StartDate_str, const QString & StopDate_str, stefanfrings::HttpResponse& response);
    /**
     * @brief DeleteArchive_v Удаляет архив с заданным диапазоном
     * @param ArchiveRange_str Диапазон архива
     * @param response Ответка на сервер
     */
    void DeleteArchive_v(const QString & ArchiveRange_str, stefanfrings::HttpResponse& response);
    /**
     * @brief PrintArchive_v Вывод архива на печать
     * @param ArchiveRange_str Диапазон архива
     * @param response Ответка на сервер
     */
    void PrintArchive_v(const QString & ArchiveRange_str, stefanfrings::HttpResponse& response);
    /**
     * @brief GetFileInfoList_lst Собирает список файлов в директории
     * @param Path Директория, где осуществляется поиск файлов
     * @param Filters_lst Фильтры поиска
     * @return Набор QFileInfo
     */
    QFileInfoList GetFileInfoList_lst(const QString & Path, QStringList Filters_lst = QStringList());
  };
}
#endif // CONTROLLERJOURNAL_H
