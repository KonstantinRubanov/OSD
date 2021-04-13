#ifndef PIPEACTION_H
#define PIPEACTION_H

#include <QObject>

#include "httprequesthandler.h"
#include "PipeAbstract.h"

namespace osds {

  QObject * CallbackCreatePipeAction_po();

  class PipeAction : public PipeAbstract
  {
    Q_OBJECT
    Q_DISABLE_COPY(PipeAction)

  public:
    PipeAction(QObject * parent = Q_NULLPTR);
    virtual ~PipeAction() override;

    /** @brief service Упрощенный сервис с чистыми параметрами
   *  @param Parameters_lst */
    void service(const QStringList & Parameters_lst) override;
  };

}

#endif // PIPEACTION_H
