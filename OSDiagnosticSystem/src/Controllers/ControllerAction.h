#ifndef CONTROLLERACTION_H
#define CONTROLLERACTION_H

#include <QObject>
#include <QDateTime>
#include <QDir>
#include <QString>
#include <QSqlQuery>
#include <QPair>
#include <QThreadPool>
#include <memory>
#include <mutex>


#include "ControllerAbstract.h"
#include "sshworker.h"
#include "sequentialguid.h"

class QSettings;

namespace osds {
  /** @brief CallbackCreateControllerAction_po Создает объект класса ControllerAction
 *  @return указатель на ControllerAction (stefanfrings::HttpRequestHandler) */
  QObject * CallbackCreateControllerAction_po();

  class ControllerAction : public ControllerAbstract
  {
    Q_OBJECT
    Q_DISABLE_COPY(ControllerAction)



  public:
    ControllerAction(QObject * parent = Q_NULLPTR);

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
    bool CheckSSHDiscreteRules(SSHWorker& SSHWorkerObj, QString& ResIP, QString& ResDir, QString& ResRec, QString& ResDiscreteVal, QString&  ResChownRuleVal, QString& Command_bits_updateVal);
    bool SSH_action(SSHWorker& SSHWorkerObj, QString& mCOmmand);
    int AddHostFolderSave(const QString& mHost, const QString& mFolder);
    int RemoveHostFolderSaveHistory(const QString& mHost, const QString& mFolder);
    int AddHostFolderSaveHistory(const QString& mHost, const QString& mFolder, const int& mId);
    int ClearHostFolderSave(const QString& mHost, const QString& mFolder);
    int SSH_test_host(const QString& mHost);


    static QString _Username_str;  ///< @brief Имя для входа по SSH
    static QString _Password_str;  ///< @brief Пароль для входа по SSH
    std::mutex mMutexAction;
    std::mutex locker;



  };
}
#endif // CONTROLLERACTION_H
