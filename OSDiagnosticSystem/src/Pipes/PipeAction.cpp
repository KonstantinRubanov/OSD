#include "PipeAction.h"
#include "httpsessionstore.h"

namespace osds {

  QObject * CallbackCreatePipeAction_po()
  {
    return new PipeAction;
  }


  PipeAction::PipeAction(QObject * parent) : PipeAbstract(parent)
  {

  }

  PipeAction::~PipeAction()
  {

  }

  void PipeAction::service(const QStringList &Parameters_lst)
  {

  }
}
