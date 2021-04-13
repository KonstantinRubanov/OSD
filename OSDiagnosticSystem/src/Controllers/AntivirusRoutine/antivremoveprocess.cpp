#include <QFileInfo>

#include "sshworker.h"
#include "osdsexception.h"
#include "ControllerAntivirus.h"
#include "ControllerLogin.h"
#include "antivremoveprocess.h"

namespace osds {

  AntivRemoveProcess::AntivRemoveProcess(const QString & Host_str, const QString & Distrib_str, QObject * parent) : QObject(parent),
    _HostName_str(Host_str),
    _Distrib_str(Distrib_str)
  {

  }

  void AntivRemoveProcess::process()
  {
    try {
      SSHWorker SSHWorker_o(_HostName_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
      if(_Distrib_str.indexOf("drweb") != -1) {
        RemoveDrWeb_v(SSHWorker_o);
      } else if (_Distrib_str.indexOf("kesl") != -1) {
        RemoveKaspersky_v(SSHWorker_o);
      } else {
        emit Output_sig(false);
      }
    } catch(OSDSException & e) {
      qWarning()<<e.what();
      emit Output_sig(false);
    }
    emit finished();
  }

  void AntivRemoveProcess::stop()
  {

  }

  void AntivRemoveProcess::RemoveDrWeb_v(SSHWorker & SSHWorker_ro)
  {
    qDebug()<<"AntivRemoveProcess::remove packge dr.Web";
    if(!SSHWorker_ro.ExecCommandOnHost_b("yes | sudo /opt/drweb.com/bin/uninst.sh")) {
      qWarning()<<"AntivRemoveProcess::remove distributiv crashed! " + _Distrib_str;
      emit Output_sig(false);
      return;
    }
    emit Output_sig(true);
  }

  void AntivRemoveProcess::RemoveKaspersky_v(SSHWorker & SSHWorker_ro)
  {
    qDebug()<<"ControllerAntivirus::Remove packge kesl";
    if(!SSHWorker_ro.ExecCommandOnHost_b("sudo dpkg -r kesl-astra > /dev/null")) {
      qWarning()<<"ControllerAntivirus::remove distributiv crashed! " + _Distrib_str;
      emit Output_sig(false);
      return;
    }
    emit Output_sig(true);
  }
}
