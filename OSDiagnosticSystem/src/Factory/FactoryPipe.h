#ifndef FACTRYPIPE_H
#define FACTRYPIPE_H

#include <QObject>

#include "FactoryAbstract.h"

namespace osds {

  class PipeAbstract;

  class FactoryPipe : public FactoryAbstract
  {
    Q_OBJECT
    Q_DISABLE_COPY(FactoryPipe)

  public:
    FactoryPipe(QObject * parent = Q_NULLPTR);
    PipeAbstract * BuildController_po(const QString & );
  private:

  };
}
#endif // FACTRYPIPE_H
