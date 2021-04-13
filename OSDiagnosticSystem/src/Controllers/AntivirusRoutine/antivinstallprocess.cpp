#include <QFileInfo>

#include "sshworker.h"
#include "osdsexception.h"
#include "ControllerAntivirus.h"
#include "ControllerLogin.h"
#include "antivinstallprocess.h"

namespace osds {

  AntivInstallProcess::AntivInstallProcess(const QString & Host_str, const QString & Distrib_str,
                                           const QString & DrWebPath_str, const QString & KasperskyPath_str,
                                           const QString & DrWebLicenseKey_str, const QString & KasperskyLicenseKey_str,
                                           const QString & KasperskyAutoinstall_str, QObject * parent) : QObject(parent),
    _HostName_str(Host_str),
    _Distrib_str(Distrib_str),
    _DrWebPath_str(DrWebPath_str),
    _KasperskyPath_str(KasperskyPath_str),
    _DrWebLicenseKey_str(DrWebLicenseKey_str),
    _KasperskyLicenseKey_str(KasperskyLicenseKey_str),
    _KasperskyAutoinstall_str(KasperskyAutoinstall_str)
  {

  }

  void AntivInstallProcess::process()
  {
    try {

      qWarning()<<"AntivInstallProcess::process()-> " + _Distrib_str;
      QString Path_str = _Distrib_str.contains("drweb") ? _DrWebPath_str : _KasperskyPath_str;
      qWarning()<<"AntivInstallProcess::process()->"<<Path_str;
      qWarning()<<"AntivInstallProcess::process()->SSHWorker...";
      SSHWorker SSHWorker_o(_HostName_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());      
      qWarning()<<"AntivInstallProcess::process()->SSHWorker start. SendFileToHost...";
      if(!SSHWorker_o.SendFileToHost_b(Path_str + "/" + _Distrib_str, "/tmp/")) {
        qWarning()<<"ControllerAntivirus::Send file crashed! " + _Distrib_str;
        emit Output_sig(false);
        emit finished();
        return;
      }
      qWarning()<<"AntivInstallProcess::process()->SendFileToHost...complete";
      if(_Distrib_str.indexOf("drweb") != -1) {
        qWarning()<<"AntivInstallProcess::process()->SendFileToHost...InstallDrWeb_v...";
        InstallDrWeb_v(SSHWorker_o);        
      } else if (_Distrib_str.indexOf("kesl") != -1) {
        InstallKaspersky_v(SSHWorker_o);
      } else {
        qWarning()<<"AntivInstallProcess::process()... no distributive name";
        emit Output_sig(false);
      }
    } catch(OSDSException & e) {
      qWarning()<<"OSDSException AntivInstallProcess::process()..."<<e.what();
      emit Output_sig(false);
    }
    qWarning()<<"AntivInstallProcess::process()->finished...";
    emit finished();
  }

  void AntivInstallProcess::stop()
  {

  }

  void AntivInstallProcess::InstallDrWeb_v(SSHWorker & SSHWorker_ro)
  {

   try{
    qWarning()<<"AntivInstallProcess::install mode " + _Distrib_str;
    if(!SSHWorker_ro.ExecCommandOnHost_b("sudo chmod 0777 /tmp/" + _Distrib_str)) {
       qWarning()<<"AntivInstallProcess::install mode crashed! " + _Distrib_str;
    }
    qWarning()<<"AntivInstallProcess::Install packge dr.Web";
    if(!SSHWorker_ro.ExecCommandOnHost_b("sudo /tmp/" + _Distrib_str + " -- -n")) {
      qWarning()<<"AntivInstallProcess::install distributiv crashed! " + _Distrib_str;
      emit Output_sig(false);
      return;
    }
    qWarning()<<"AntivInstallProcess::Copy license file dr.web to temp";
    if(!SSHWorker_ro.SendFileToHost_b(_DrWebLicenseKey_str, "/tmp/")) {
      qWarning()<<"AntivInstallProcess::Send file crashed! " + _Distrib_str;
      emit Output_sig(false);
      return;
    }
    qWarning()<<"AntivInstallProcess::Move license file dr.web to opt";
    if(!SSHWorker_ro.ExecCommandOnHost_b("sudo mv /tmp/" + QFileInfo(_DrWebLicenseKey_str).fileName() + " /opt/")) {
      qWarning()<<"AntivInstallProcess::Install key file failed! " + _Distrib_str;
      emit Output_sig(false);
      return;
    }
    qWarning()<<"AntivInstallProcess::Install license file dr.web";
    if(!SSHWorker_ro.ExecCommandOnHost_b("sudo drweb-ctl cfset Root.KeyPath /opt/drweb32.key")) {
      qWarning()<<"AntivInstallProcess::Install key file failed! " + _Distrib_str;
      emit Output_sig(false);
      return;
    }
    qWarning()<<"AntivInstallProcess::InstallDrWeb_v->drweb end";
    emit Output_sig(true);
      }
    catch(OSDSException & e) {
       qWarning()<<"OSDSException AntivInstallProcess::InstallDrWeb_v->"<<e.what();
    }
  }

  void AntivInstallProcess::InstallKaspersky_v(SSHWorker & SSHWorker_ro)
  {
    qDebug()<<"ControllerAntivirus::Install packge kesl";
    if(!SSHWorker_ro.ExecCommandOnHost_b("sudo dpkg -i /tmp/" + _Distrib_str)) {
      qWarning()<<"ControllerAntivirus::install distributiv crashed! " + _Distrib_str;
      emit Output_sig(false);
      return;
    }
    qDebug()<<"ControllerAntivirus::Copy kesl license key" ;
    if(!SSHWorker_ro.SendFileToHost_b(_KasperskyLicenseKey_str, "/tmp/")) {
      qWarning()<<"ControllerAntivirus::Send file crashed! " + _Distrib_str;
      emit Output_sig(false);
      return;
    }
    qDebug()<<"ControllerAntivirus::Copy autoinstall kesl file";
    if(!SSHWorker_ro.SendFileToHost_b(_KasperskyAutoinstall_str, "/tmp/")) {
      qWarning()<<"ControllerAntivirus::Send file crashed! " + _Distrib_str;
      emit Output_sig(false);
      return;
    }
    qDebug()<<"ControllerAntivirus::Post install kesl script run";
    if(!SSHWorker_ro.ExecCommandOnHost_b("sudo /opt/kaspersky/kesl/bin/kesl-setup.pl --autoinstall=/tmp/" + QFileInfo(_KasperskyAutoinstall_str).fileName())) {
      qWarning()<<"ControllerAntivirus::install distributiv crashed! " + _Distrib_str;
      emit Output_sig(false);
      return;
    }
    qDebug()<<"ControllerAntivirus::Install license key for Kaspersky";
    if(!SSHWorker_ro.ExecCommandOnHost_b("sudo kesl-control --install-active-key /tmp/" + QFileInfo(_KasperskyLicenseKey_str).fileName())) {
      qWarning()<<"ControllerAntivirus::install license key for Kaspersky crashed! " + _Distrib_str;
      emit Output_sig(false);
      return;
    }
    emit Output_sig(true);
  }
}
