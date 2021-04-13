#ifndef PIPEALD_H
#define PIPEALD_H

#include <QObject>

#include "PipeAbstract.h"

namespace osds {

  QObject * CallbackCreatePipeALD_po();

  class PipeALD : public PipeAbstract
  {
    Q_OBJECT
    Q_DISABLE_COPY(PipeALD)

  public:
    PipeALD(QObject * parent = Q_NULLPTR);
    ~PipeALD() override;
    void service(const QStringList & Parameters_lst) override;

  };
}
#endif // PIPEALD_H
