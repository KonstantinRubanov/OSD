#include "ControllerLogin.h"
#include "logger.h"
#include "httpsessionstore.h"
#include "staticfilecontroller.h"
#include "osdsapi.h"
#include "validation.h"
#include "ControllerAction.h"
#include "ControllerALD.h"
#include "controllerdomainpolicy.h"



using namespace stefanfrings;

namespace osds {

  QString ControllerLogin::_Username_str = "";
  QString ControllerLogin::_Password_str = "";
  QString ControllerLogin::_ALDServer_str = "";

  QObject * CallbackCreateControllerLogin_po()
  {
    return new ControllerLogin;
  }

  ControllerLogin::ControllerLogin(QObject * parent) : ControllerAbstract(parent)
  {
    _ControllerName_str = "ControllerLogin";
  }

  void ControllerLogin::service(HttpRequest& request, HttpResponse& response)
  {
    HttpSession Sesseion_o = osdsapi->GetSession_po()->getSession(request, response);
    qDebug("Session: id=%s", Sesseion_o.getId().data());
    if(request.getParameterMap().contains("username") && request.getParameterMap().contains("password")) {
      EValidationLevel Level_en = ValidatePassword_en(request.getParameter("username"), request.getParameter("password"));
      if(Level_en == VLAdmin_en) {
        Sesseion_o.set("login", QVariant("admin"));
        response.redirect("../main.html");
      }
      else if(Level_en == VLUser_en) {
        Sesseion_o.set("login", QVariant("user"));
        response.redirect("../main.html");
      }
      else {
        Sesseion_o.remove("login");
        response.redirect("../");
      }
    }
    else {
        response.redirect("../");
    }
  }

  void ControllerLogin::SetSettings_sv(QSettings & Settings_ro)
  {
    ControllerLogin::_Username_str = Settings_ro.value("username").toString();
    ControllerLogin::_Password_str = Settings_ro.value("password").toString();
    ControllerLogin::_ALDServer_str = Settings_ro.value("ald_server").toString();
  }

  void ControllerLogin::InitializeDataBase_sv()
  {

  }

  EValidationLevel ControllerLogin::ValidatePassword_en(const QString & Login_str, const QString & Password_str)
  {
    if(osdsapi->GetValidation_po()->SetAutorization_b(Login_str, Password_str)) {

      ControllerAction::SetSshParams(_Username_str, _Password_str);
      ControllerALD::SetSshParams(_Username_str, _Password_str);
      ControllerDomainPolicy::SetSshParams(_Username_str, _Password_str);

      return VLAdmin_en;
    }
    else if(Login_str == "user") {  // Пока оставлю для отработки
      return VLUser_en;
    }
    return VLNone_en;
  }

}
