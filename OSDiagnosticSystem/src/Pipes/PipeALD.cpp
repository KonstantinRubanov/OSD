#include "PipeALD.h"
#include "httpsessionstore.h"

namespace osds {

  QObject * CallbackCreatePipeALD_po()
  {
    return new PipeALD;
  }

  PipeALD::PipeALD(QObject * parent) : PipeAbstract(parent)
  {

  }

  PipeALD::~PipeALD()
  {

  }

  void PipeALD::service(const QStringList &Parameters_lst)
  {

  }

}
