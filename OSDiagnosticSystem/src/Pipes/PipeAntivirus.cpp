#include "PipeAntivirus.h"
#include "httpsessionstore.h"

namespace osds {
  QObject * CallbackCreatePipeAntivirus_po()
  {
    return new PipeAntivirus;
  }

  PipeAntivirus::PipeAntivirus(QObject * parent) : PipeAbstract(parent)
  {

  }
  PipeAntivirus::~PipeAntivirus()
  {

  }

  void PipeAntivirus::service(const QStringList &Parameters_lst)
  {

  }

}
