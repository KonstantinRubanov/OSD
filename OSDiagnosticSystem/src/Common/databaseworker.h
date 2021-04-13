#ifndef DATABASEWORKER_H
#define DATABASEWORKER_H

#include <QObject>
#include <QMutex>
#include <QSqlDatabase>
#include <memory>
#include <mutex>

class QSqlQuery;

class QSettings;

namespace osds {
  /** @brief The DatabaseWorker class Позволяет работать с БД (SQLite) */
  class DatabaseWorker : public QObject
  {
    Q_OBJECT

    static QString _DataBaseName_str; ///< @brief Имя БД
    static QMutex _SQLMutex_so;  ///< @brief Блокировка в многопоточном режиме
    std::mutex _SQLMutex_sh;  ///< @brief Блокировка в многопоточном режиме

  public:
    /**
     * @brief DatabaseWorker Основной конструктор
     * @param parent Родитель
     */
    explicit DatabaseWorker(QObject *parent = nullptr);
    /**
     * @brief SetSettings_sv Установка данных из настроек
     * @param Settings_ro Настройки
     */
    static void SetSettings_sv(QSettings & Settings_ro);
    /**
     * @brief QueryExec_lst Выполняет запрос и выдает результат
     * @param Query_str Запрос в форме строки
     * @param IsExceptionQuery_b Прерывание по облому в запросе
     * @return Список строк ответа
     */
    QSqlQuery QueryExec_lst(QString Query_str , bool IsExceptionQuery_b = true);
    /**
     * @brief QueryExec_lst Выполняет запрос и выдает результат
     * @param Query_str Запрос в форме строки
     * @param ExitCode_ri Код выхода (0 - нормально отработано)
     * @param IsExceptionQuery_b Прерывание по облому в запросе
     * @return Список строк ответа
     */
    QSqlQuery QueryExecWithCode_lst(QString Query_str, int & ExitCode_ri, bool IsExceptionQuery_b = true);
    /**
     * @brief QueryExec_lst Выполняет запрос и выдает результат
     * @param Query_str Запрос в форме строки, строка может в себе содержать параметры. Параметр указывается именем через двоеточие.
     * @param BindList_list Список с параметрами для вставки в запрос.
     * QPair<QString,QString>: first - имя параметра через двоеточие(например, ":p_name"), second - значение параметра.
     * @param IsExceptionQuery_b Прерывание по облому в запросе
     * @param paramType Что требуется сделать со значением по параметру (записать в таблицу\удалить из таблицы)
     * @return Список строк ответа
     */
    QSqlQuery QueryExec_lst(QString Query_str, QList <QPair<QString,QString>> BindList_list, bool IsExceptionQuery_b = true, QSql::ParamType paramType = QSql::In);
    /**
     * @brief WriteAdministratorAction_v Записывает действия оператора в журнал
     * @param Controller_str Имя контроллера
     * @param Text_str Текст с записью
     */
    void WriteAdministratorAction_v(const QString & Controller_str, const QString & Text_str);

    bool QueryExec_lstExt(QString Query_str, bool IsExceptionQuery_b,  QSqlQuery& QSqlQuery_o);
    /**
     * @brief CreateUserTables_v Создает справочники пользователей и операций над паролями.
     * Их требуется создать один раз при первом создании БД.
     */
    void CreateUserTables_v();
  };
}

#endif // DATABASEWORKER_H
