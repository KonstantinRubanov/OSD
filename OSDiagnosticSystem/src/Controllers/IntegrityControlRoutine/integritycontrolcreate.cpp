#include "sshworker.h"
#include "osdsexception.h"
#include "ControllerAntivirus.h"
#include "ControllerLogin.h"
#include "integritycontrolcreate.h"

namespace osds {

  IntegrityControlCreate::IntegrityControlCreate(const QString & Host_str, const QStringList & Dirs_lst, QObject * parent) : QObject(parent),
    _HostName_str(Host_str),
    _AddedDirs_str(Dirs_lst)
  {

  }

  void IntegrityControlCreate::process()
  {
    try {
      quint32 ErrorsCount_u32 = 0;
      QString Journal_str;

      qWarning()<<"IntegrityControlCreate::  dirlist = "<< _AddedDirs_str;

      SSHWorker SSHWorker_o(_HostName_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
      //----- Добавление каталогов в аудит -----
      /*
       if(!_AddedDirs_str.isEmpty() && SSHWorker_o.ExecCommandOnHost_b("sudo setfaud -RPs o:udnrmcy:udnrmcy '" + _AddedDirs_str.join(" ") + "'" + " >/dev/null; sudo systemctl restart parlogd; ")) {
        //Journal_str += "Added dirs to audit IC";
      }*/

      for(auto &p_str : _AddedDirs_str){
          SSHWorker_o.ExecCommandOnHost_b("sudo setfaud -RPs o:udnrmcy:udnrmcy '" + p_str + "'" + " >/dev/null ");
      }

      SSHWorker_o.ExecCommandOnHost_b("sudo systemctl restart parlogd; ");
      //----- Создание точки КЦ -----
      qWarning()<<"IntegrityControlCreate::  afick create point!!!";
      if(SSHWorker_o.ExecCommandOnHost_b("sudo afick -i > /tmp/afick_create.log; tail -n30 /tmp/afick_create.log; if [ \"$(systemctl status parlogd.service | grep \"active (running)\")\" == "" ]; then sudo systemctl restart parlogd; fi; ")) {
        QStringList Output_lst = SSHWorker_o.GetLastOutput_lst();//, InfectOutput_lst;
        foreach(const QString & OutLine_str, Output_lst) {
          //Journal_str += OutLine_str + "<br>";
        }
      } else {
        ErrorsCount_u32 = 1;
        Journal_str = SSHWorker_o.GetLastError_str();
      }
      emit Output_sig(_HostName_str, ErrorsCount_u32, Journal_str);
      emit finished();
    } catch (OSDSException & e) {
      qWarning()<<"IntegrityControlCreate::"<< e.what();
      emit Output_sig(_HostName_str, 1, "abort");
      emit finished();
    }

  }

  void IntegrityControlCreate::stop()
  {

  }
}
