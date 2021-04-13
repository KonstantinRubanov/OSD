#include "PipeJournals.h"
#include "httpsessionstore.h"

namespace osds {

  QObject * CallbackCreatePipeJournals_po()
  {
    return new PipeJournals;
  }

  PipeJournals::PipeJournals(QObject * parent) : PipeAbstract(parent)
  {

  }

  PipeJournals::~PipeJournals()
  {

  }

  void PipeJournals::service(const QStringList &Parameters_lst)
  {

  }

}
