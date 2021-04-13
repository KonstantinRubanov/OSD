#ifndef PIPEUSERS_H
#define PIPEUSERS_H

#include <QObject>

#include "PipeAbstract.h"

namespace osds {

  QObject * CallbackCreatePipeUsers_po();

  class PipeUsers : public PipeAbstract
  {
    Q_OBJECT
    Q_DISABLE_COPY(PipeUsers)

  public:
    PipeUsers(QObject * parent = Q_NULLPTR);
    virtual ~PipeUsers() override;

    /** @brief service Упрощенный сервис с чистыми параметрами
   *  @param Parameters_lst */
    void service(const QStringList & Parameters_lst) override;
  };
}
#endif // PIPEUSERS_H
