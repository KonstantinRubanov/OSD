#ifndef PIPEJOURNALS_H
#define PIPEJOURNALS_H

#include <QObject>

#include "PipeAbstract.h"

namespace osds {

  QObject * CallbackCreatePipeJournals_po();

  class PipeJournals : public PipeAbstract
  {
    Q_OBJECT
    Q_DISABLE_COPY(PipeJournals)

  public:
    PipeJournals(QObject * parent = Q_NULLPTR);
    virtual ~PipeJournals() override;

    /** @brief service Упрощенный сервис с чистыми параметрами
   *  @param Parameters_lst */
    void service(const QStringList & Parameters_lst) override;
  };
}
#endif // ACTIONCONTROLLER_H
