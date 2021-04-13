/**
  @file
  @author Stefan Frings
*/

#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include "httprequesthandler.h"

namespace stefanfrings { class HttpRequest; class HttpResponse; }

namespace osds {

  class FactoryController;

  /** @brief The RequestHandler class Класс служащий для приема запроса по HTTP и отправки ответа */
  class RequestHandler : public stefanfrings::HttpRequestHandler {
    Q_OBJECT
    Q_DISABLE_COPY(RequestHandler)
  public:

    /** @brief RequestHandler Конструктор с параметром
     * @param SettingsName_str Имя файла настроек
     * @param parent Родительский объект
     */
    RequestHandler(const QString & SettingsName_str, QObject* parent = nullptr);

    /** @brief Процесс приема запроса по HTTP
      * @param request Запрос
      * @param response Возможный ответ */
    void service(stefanfrings::HttpRequest & request, stefanfrings::HttpResponse & response);

  private:
    FactoryController * _FactoryController_po = nullptr;  ///< @brief Фабрика контроллеров
  };

} // namespace osds

#endif // REQUESTHANDLER_H
