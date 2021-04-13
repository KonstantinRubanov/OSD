#include <QDebug>

#include "PipeLogin.h"
#include "httpsessionstore.h"
#include "osdsapi.h"
#include "validation.h"

namespace osds {

  QObject * CallbackCreatePipeLogin_po()
  {
    return new PipeLogin;
  }

  PipeLogin::PipeLogin(QObject * parent) : PipeAbstract(parent)
  {
  }

  PipeLogin::~PipeLogin()
  {

  }

  void PipeLogin::service(const QStringList &Parameters_lst)
  {

    if(Parameters_lst.size() >= 3) {
      qDebug()<<"Pipe login: "<<Parameters_lst[1]<<" password: "<<Parameters_lst[2];
      osdsapi->GetValidation_po()->SetAutorization_b(Parameters_lst[1], Parameters_lst[2]);
#if defined(__NO_ASTRA_LINUX__)
      QStringList Command_lst = osdsapi->GetValidation_po()->ALDProcess_lst("ls -la");  // Проверка отработки входа
      qDebug()<<Command_lst;
#endif
    }
  }
}
