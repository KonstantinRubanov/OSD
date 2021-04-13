#ifndef OSDSAPI_H
#define OSDSAPI_H

// Упростим себе жизнь
#ifndef osdsapi
#define osdsapi osds::OSDSAPI::instance()
#endif

class QCoreApplication;
class QSettings;
class QString;

namespace stefanfrings {
  class FileLogger;
  class HttpSessionStore;
  class StaticFileController;
}

namespace osds {

  class Validation;

  /** @brief The EValidationLevel enum Уровни доступа к ресурсам ПО */
  enum EValidationLevel {
    VLNone_en,  ///< @brief Доступ запрещен
    VLUser_en, ///< @brief Пользовательский доступ
    VLAdmin_en ///< @brief Админский доступ
  };

  class OSDSAPI;

  /**
      * @class OSDSAPIDestroyer
      * @brief Класс-деструктор для OSDSAPI
      */
  class OSDSAPIDestroyer
  {
  private:
    OSDSAPI * _OSDSAPI_po;
  public:
    virtual ~OSDSAPIDestroyer();
    void InitDestoyer_v(OSDSAPI * OSDSAPI_po);
  };

  class OSDSAPI
  {
    static OSDSAPI * _OSDSAPI_po; ///< @brief Объект класса, с которым идет работа
    static OSDSAPIDestroyer _OSDSAPIDestroyer_o; ///< @brief Объект класса-деструктора

  protected:
    /** @brief Конструктор */
    OSDSAPI();
    /** @brief Деструктор */
    virtual ~OSDSAPI();

    friend class OSDSAPIDestroyer;  ///< @brief Друг-уничтожитель

  public:
    /** @brief Метод для получения объекта */
    static OSDSAPI* instance();
    /** @brief Инициализация свойств логера */
    void InitLogger_v(QSettings * Settings_po, QCoreApplication * Application_po);
    /** @brief Инициализация свойств логера */
    void InitSession_v(QSettings * Settings_po, QCoreApplication * Application_po);
    /** @brief Инициализация обработчика запросов браузера к файлам */
    void InitFileController_v(QSettings * Settings_po, QCoreApplication * Application_po);
    /** @brief Возвращает логгер */
    stefanfrings::FileLogger* GetLogger_po() const { return _Logger_po; }
    stefanfrings::HttpSessionStore* GetSession_po() const { return _SessionStore_po; }
    stefanfrings::StaticFileController* GetStatiFileController_po() const { return _FileController_po; }
    Validation * GetValidation_po(){ return _Validation_po; }
    /**
     * @brief GetDocroot_pstr Возвращает строку с папкой клиента
     * @return Указатель на строку
     */
    const QString & GetDocroot_str() const { return *_Docroot_pstr; }

  private:
    stefanfrings::FileLogger * _Logger_po = nullptr; ///< @brief Система ведения лога
    stefanfrings::HttpSessionStore * _SessionStore_po = nullptr;  ///< @brief Сессия соединения, включая куки
    stefanfrings::StaticFileController * _FileController_po = nullptr; ///< @brief Обработчик запросов браузера к файлам
    Validation * _Validation_po;  ///< @brief Параметры авторизации
    QString * _Docroot_pstr; ///< @brief Путь к папке с клиентом
  };
}

#endif // OSDSAPI_H
