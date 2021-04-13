#ifndef PIPEANTIVIRUS_H
#define PIPEANTIVIRUS_H

#include <QObject>

#include "PipeAbstract.h"
namespace osds {
  QObject * CallbackCreatePipeAntivirus_po();

  class PipeAntivirus : public PipeAbstract
  {
    Q_OBJECT
    Q_DISABLE_COPY(PipeAntivirus)

  public:
    PipeAntivirus(QObject * parent = Q_NULLPTR);
    virtual ~PipeAntivirus() override;

    /** @brief service Упрощенный сервис с чистыми параметрами
   *  @param Parameters_lst */
    void service(const QStringList & Parameters_lst) override;
  };
}
#endif // PIPEANTIVIRUS_H
