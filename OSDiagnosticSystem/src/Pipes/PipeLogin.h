#ifndef PIPELOGIN_H
#define PIPELOGIN_H

#include <QObject>

#include "PipeAbstract.h"

namespace osds {

  QObject * CallbackCreatePipeLogin_po();

  /**
   * @brief The PipeLogin class Для получения эфекта, нужно сделать echo "/login;LOGIN;PASSWORD" > /tmp/pipe_osds.txt
   */
  class PipeLogin : public PipeAbstract
  {
    Q_OBJECT
    Q_DISABLE_COPY(PipeLogin)

  public:
    PipeLogin(QObject * parent = Q_NULLPTR);
    virtual ~PipeLogin() override;

    /** @brief service Упрощенный сервис с чистыми параметрами
     *  @param Parameters_lst */
    void service(const QStringList & Parameters_lst) override;
  };
}
#endif // PIPELOGIN_H
