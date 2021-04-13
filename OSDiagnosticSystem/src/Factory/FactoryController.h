#ifndef FACTORYCONTROLLER_H
#define FACTORYCONTROLLER_H

#include <QObject>

#include "FactoryAbstract.h"

namespace osds {
  class FactoryController : public FactoryAbstract
  {
    Q_OBJECT
    Q_DISABLE_COPY(FactoryController)

  public:
    FactoryController(const QString & SettingsName_str, QObject * parent = Q_NULLPTR);
    stefanfrings::HttpRequestHandler * BuildController_po(const QString & );

  };
} // namespace osds


#endif // FACTORYCONTROLLER_H
