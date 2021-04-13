#include "PipeUsers.h"
#include "httpsessionstore.h"

namespace osds {

  QObject * CallbackCreatePipeUsers_po()
  {
    return new PipeUsers;
  }

  PipeUsers::PipeUsers(QObject * parent) : PipeAbstract(parent)
  {

  }

  PipeUsers::~PipeUsers()
  {

  }

  void PipeUsers::service(const QStringList &Parameters_lst)
  {

  }
}
