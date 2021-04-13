#ifndef CONTROLLERSECURITY_H
#define CONTROLLERSECURITY_H

#include <QObject>
#include <QMutex>

#include "ControllerAbstract.h"
#include "constants.h"

namespace osds {

  /** @brief CallbackCreateControllerSecurity_po Создает объект класса ControllerSecurity
 *  @return указатель на ControllerSecurity (stefanfrings::HttpRequestHandler) */
  QObject * CallbackCreateControllerSecurity_po();

  class ControllerSecurity : public ControllerAbstract
  {
    Q_OBJECT
    Q_DISABLE_COPY(ControllerSecurity)

    static QMap<QString, ThreadHostState> _TestHostState_smap; // Вывод состояний сделан через map
    static QMutex _Mutex_so; ///< @brief Блокировка для _TestHostState_smap

  public:
    ControllerSecurity(QObject * parent = Q_NULLPTR);

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
     * @brief GetHostList_v Возвращает список хостов
     * @param response Ответка на клиента
     */
    void GetHostList_v(stefanfrings::HttpResponse& response);
    /**
     * @brief StartSingleTest_v Запусе теста на одном хосте
     * @param Host_str Имя хоста
     * @param response Ответка на клиента
     */
    void StartSingleTest_v(const QString & Host_str, stefanfrings::HttpResponse& response);
    /**
     * @brief GetStateSingleTest_v Запрос состояния единичного теста
     * @param Host_str Имя хоста
     * @param response Ответка на клиента
     */
    void GetStateSingleTest_v(const QString & Host_str, stefanfrings::HttpResponse& response);
    /**
     * @brief StartTestHostInThread_po Запуск тестирования одного хоста в потоке
     * @param Host_str Имя хоста
     * @return Поток
     */
    QThread * StartTestHostInThread_po(const QString & Host_str);
    /**
     * @brief TestRunState_b Определяет состояние работы теста на хосте
     * @param Host_str Имя хоста
     * @return состояние run|not run|Error_SSH_Connect
     */
    QString TestRunState_str(const QString & Host_str);
  };
}

#endif // CONTROLLERSECURITY_H
