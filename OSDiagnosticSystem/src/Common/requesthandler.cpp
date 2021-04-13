/**
  @file
  @author Stefan Frings
*/

#include "requesthandler.h"
#include "osdsapi.h"
#include "filelogger.h"
#include "staticfilecontroller.h"
#include "httpsessionstore.h"
#include "FactoryController.h"
#include "ControllerLogin.h"

using namespace stefanfrings;

namespace osds {
  RequestHandler::RequestHandler(const QString & SettingsName_str, QObject* parent) : HttpRequestHandler(parent)
  {
    _FactoryController_po = new FactoryController(SettingsName_str, this);
  }

  void RequestHandler::service(HttpRequest& request, HttpResponse& response)
  {
    QByteArray path=request.getPath();
    qDebug("Conroller: path=%s",path.data());
    HttpSession Sesseion_o = osdsapi->GetSession_po()->getSession(request, response);
    qDebug("Session: id=%s", Sesseion_o.getId().data());

    stefanfrings::HttpRequestHandler * Controller_po = _FactoryController_po->BuildController_po(path);
    if(Controller_po) {
      if((Sesseion_o.contains("login") && Sesseion_o.get("login").toString() == "admin") || path == "/login") {
        Controller_po->service(request, response);
      }
      else {
        response.setStatus(401);
      }
    }
    else {
      osdsapi->GetStatiFileController_po()->service(request, response);
    }

    qDebug("Conroller: finished request");

    if (osdsapi->GetLogger_po()) {
      osdsapi->GetLogger_po()->clear();
    }
    delete Controller_po; // Закончили работу с контроллером
  }
}
