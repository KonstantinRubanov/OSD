#include <QSettings>

#include "ControllerAction.h"
#include "ControllerALD.h"
#include "ControllerAntivirus.h"
#include "ControllerBackup.h"
#include "ControllerIntegrityControl.h"
#include "ControllerJournals.h"
#include "ControllerLogin.h"
#include "ControllerSecurity.h"
#include "ControllerSettings.h"
#include "ControllerUsers.h"
#include "FactoryController.h"
#include "httpsessionstore.h"
#include "controllerdomainpolicy.h"

using namespace stefanfrings;

namespace osds {

  template<class T>
  void SetControllerInitialize_v(QSettings & Settings_ro, const QString & Group_str) {
    Settings_ro.beginGroup(Group_str);
    T::SetSettings_sv(Settings_ro);
    T::InitializeDataBase_sv();
    Settings_ro.endGroup();
  }

  FactoryController::FactoryController(const QString & SettingsName_str, QObject * parent) : FactoryAbstract(parent)
  {
    AddController_v("/action", CallbackCreateControllerAction_po);
    AddController_v("/ald", CallbackCreateControllerALD_po);
    AddController_v("/policy", CallbackCreateControllerDomainPolicy_po);
    AddController_v("/antivirus", CallbackCreateControllerAntivirus_po);
    AddController_v("/backup", CallbackCreateControllerBackup_po);
    AddController_v("/integrity_control", CallbackCreateControllerIntegrityControl_po);
    AddController_v("/journals", CallbackCreateControllerJournals_po);
    AddController_v("/login", CallbackCreateControllerLogin_po);
    AddController_v("/security", CallbackCreateControllerSecurity_po);
    AddController_v("/settings", CallbackCreateControllerSettings_po);
    AddController_v("/users", CallbackCreateControllerUsers_po);


    //----- Передача настроек контролам -----
    QSettings Settings_o(SettingsName_str,QSettings::IniFormat, this);
    SetControllerInitialize_v<ControllerAction>(Settings_o, "action");
    SetControllerInitialize_v<ControllerALD>(Settings_o, "ald");
    SetControllerInitialize_v<ControllerDomainPolicy>(Settings_o, "policy");
    SetControllerInitialize_v<ControllerAntivirus>(Settings_o, "antivirus");
    SetControllerInitialize_v<ControllerBackup>(Settings_o, "backup");
    SetControllerInitialize_v<ControllerIntegrityControl>(Settings_o, "integrity_control");
    SetControllerInitialize_v<ControllerJournals>(Settings_o, "journals");
    SetControllerInitialize_v<ControllerSecurity>(Settings_o, "security");
    SetControllerInitialize_v<ControllerUsers>(Settings_o, "users");
    SetControllerInitialize_v<ControllerLogin>(Settings_o, "login");
    ControllerSettings::SetSettingsFileName_sv(Settings_o.fileName());
  }

  HttpRequestHandler * FactoryController::BuildController_po(const QString & Path_str)
  {
    if(!_CreateControllerMethod_map.contains(Path_str)) {
      return nullptr;
    }
    return dynamic_cast<HttpRequestHandler *>((*_CreateControllerMethod_map[Path_str])());
  }
}
