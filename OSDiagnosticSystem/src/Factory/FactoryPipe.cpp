
#include "PipeAction.h"
#include "PipeALD.h"
#include "PipeAntivirus.h"
#include "PipeJournals.h"
#include "PipeLogin.h"
#include "PipeSecurity.h"
#include "PipeUsers.h"
#include "FactoryPipe.h"
#include "controllerdomainpolicy.h"

namespace osds {

  FactoryPipe::FactoryPipe(QObject * parent) : FactoryAbstract(parent)
  {
    AddController_v("/action", CallbackCreatePipeAction_po);
    AddController_v("/ald", CallbackCreatePipeALD_po);
    AddController_v("/policy", CallbackCreateControllerDomainPolicy_po);
    AddController_v("/antivirus", CallbackCreatePipeAntivirus_po);
    AddController_v("/journals", CallbackCreatePipeJournals_po);
    AddController_v("/login", CallbackCreatePipeLogin_po);
    AddController_v("/security", CallbackCreatePipeSecurity_po);
    AddController_v("/users", CallbackCreatePipeUsers_po);
  }

  PipeAbstract * FactoryPipe::BuildController_po(const QString & Path_str)
  {
    if(!_CreateControllerMethod_map.contains(Path_str)) {
      return nullptr;
    }
    return dynamic_cast<PipeAbstract *>((*_CreateControllerMethod_map[Path_str])());
  }
}
