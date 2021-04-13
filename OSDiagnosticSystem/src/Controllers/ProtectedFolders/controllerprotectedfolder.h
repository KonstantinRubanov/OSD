#ifndef CONTROLLERPROTECTEDFOLDER_H
#define CONTROLLERPROTECTEDFOLDER_H

#include <QObject>
#include <QThread>
#include <QMetaType>

#include "protectfolderwork.h"
#include "ControllerAction.h"

namespace osds {
  class ControllerProtectedFolder : public QObject
  {
    Q_OBJECT
    QThread workerThread;
   public:
      ControllerProtectedFolder ();
      ~ControllerProtectedFolder ();
   public slots:
    void handleResults(const ret_thread &);
  signals:
    void operate(const ObjProtect&);
  };
}

#endif // CONTROLLERPROTECTEDFOLDER_H
