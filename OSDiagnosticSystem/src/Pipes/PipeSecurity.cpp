#include <QDebug>

#include "PipeSecurity.h"
#include "httpsessionstore.h"
#include "osdsapi.h"
#include "validation.h"

namespace osds {
  QObject * CallbackCreatePipeSecurity_po()
  {
    return new PipeSecurity;
  }

  PipeSecurity::PipeSecurity(QObject * parent) : PipeAbstract(parent)
  {

  }

  PipeSecurity::~PipeSecurity()
  {

  }

  void PipeSecurity::service(const QStringList &Parameters_lst)
  {
    if(Parameters_lst.size() >= 2) {
      if(Parameters_lst[1] == "tests") {
        QStringList TestOut_lst = osdsapi->GetValidation_po()->ExecProcess_lst("/usr/lib/parsec/tests/audit_file.sh");
        qDebug()<<TestOut_lst;
      }
    }
  }
}
