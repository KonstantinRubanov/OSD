#ifndef CONTROLLERLOGIN_H
#define CONTROLLERLOGIN_H

#include <QObject>

#include "osdsapi.h"
#include "ControllerAbstract.h"

namespace osds {
  /** @brief CallbackCreateControllerLogin_po Создает объект класса ControllerLogin
 *  @return указатель на ControllerLogin (stefanfrings::HttpRequestHandler) */
  QObject * CallbackCreateControllerLogin_po();

  class ControllerLogin : public ControllerAbstract
  {
    Q_OBJECT
    Q_DISABLE_COPY(ControllerLogin)

    static QString _Username_str;  ///< @brief Имя для входа по SSH
    static QString _Password_str;  ///< @brief Пароль для входа по SSH
    static QString _ALDServer_str;  ///< @brief Имя или адрес сервера ALD

    QMap<QByteArray, QByteArray> _Users_map;  ///< @brief Все пользователи

  public:
    ControllerLogin(QObject * parent = Q_NULLPTR);

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
    /**
     * @brief GetUserName_str Возвращает имя пользователя */
    static QString GetUserName_str() { return _Username_str; }
    /**
     * @brief GetUserName_str Возвращает пароль */
    static QString GetPassword_str() { return _Password_str; }
    /**
     * @brief GetALDServer_str Возвращает имя ALD сервера или путь к нему */
    static QString GetALDServer_str() { return _ALDServer_str; }

  private:
    /**
     * @brief ValidatePassword_b Проверка данных админа на корректность
     * @param Login_str Имя (admin)
     * @param Password_str Пароль (password)
     * @return Состояние проверки на доступ
     */
    EValidationLevel ValidatePassword_en(const QString & Login_str, const QString & Password_str);
  };

}

#endif // CONTROLLERLOGIN_H
