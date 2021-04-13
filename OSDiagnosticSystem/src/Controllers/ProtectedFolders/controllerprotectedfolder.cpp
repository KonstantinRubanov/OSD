#include "controllerprotectedfolder.h"


namespace osds {
  ControllerProtectedFolder::ControllerProtectedFolder()
  {
      //qWarning()<<"ControllerProtectedFolder init...";
      /*qRegisterMetaType <ObjProtect> ("ObjProtect");
      qRegisterMetaType <ret_thread> ("ret_thread");
      ProtectFolderWork *worker = new ProtectFolderWork;
      worker->moveToThread(&workerThread);
      connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
      connect(this, &ControllerProtectedFolder::operate, worker, &ProtectFolderWork::doWork);
      connect(worker, &ProtectFolderWork::resultReady, this, &ControllerProtectedFolder::handleResults);
      workerThread.start();
      //qWarning()<<"ControllerProtectedFolder init complete...";
      */

  }
   ControllerProtectedFolder::~ControllerProtectedFolder()
  {
     workerThread.quit();
     workerThread.wait();
  }
   void ControllerProtectedFolder::handleResults(const ret_thread &result){

   }
}

