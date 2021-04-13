#include "auditprocess.h"


namespace osds{
  AuditProcess::AuditProcess(AuditTask& pTask)
  {
   mTask = pTask;
  }
  void AuditProcess::doWork(){
      mTask.State = -1;
      //qWarning()<<"Audit task complete.";
      mTask.State = 0;
      emit resultReady(mTask.State);
  }
}
