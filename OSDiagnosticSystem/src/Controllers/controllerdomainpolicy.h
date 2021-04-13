#ifndef CONTROLLERDOMAINPOLICY_H
#define CONTROLLERDOMAINPOLICY_H

#include <QObject>
#include <memory>
#include <mutex>

#include "sshworker.h"
#include "ControllerAbstract.h"

namespace osds {
  /** @brief CallbackCreateControllerAction_po Создает объект класса ControllerAction
  *  @return указатель на ControllerAction (stefanfrings::HttpRequestHandler) */

  QObject * CallbackCreateControllerDomainPolicy_po();

  class ControllerDomainPolicy : public ControllerAbstract
  {
    Q_OBJECT
    Q_DISABLE_COPY(ControllerDomainPolicy)
   public:
    ControllerDomainPolicy(QObject * parent = Q_NULLPTR);

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

    bool SSH_action(SSHWorker& SSHWorkerObj, QString& mCOmmand);
  };
}
#endif // CONTROLLERDOMAINPOLICY_H
