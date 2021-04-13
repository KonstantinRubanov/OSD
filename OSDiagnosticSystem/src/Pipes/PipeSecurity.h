#ifndef PIPESECURITY_H
#define PIPESECURITY_H

#include <QObject>

#include "PipeAbstract.h"

namespace osds {
  QObject * CallbackCreatePipeSecurity_po();

  /** @class PipeSecurity Контроллер через pipe для управления СЗИ */
  class PipeSecurity : public PipeAbstract
  {
    Q_OBJECT
    Q_DISABLE_COPY(PipeSecurity)

  public:
    PipeSecurity(QObject * parent = Q_NULLPTR);
    virtual ~PipeSecurity() override;

    /** @brief service Упрощенный сервис с чистыми параметрами
     *  @param Parameters_lst Параметры: 0 - путь, 1 - тип (tests,...) (echo "/security;tests")*/
    void service(const QStringList & Parameters_lst) override;
  };
}
#endif // PIPESECURITY_H
