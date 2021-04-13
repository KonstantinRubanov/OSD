#include "sshworker.h"
#include "osdsexception.h"
#include "ControllerAntivirus.h"
#include "ControllerLogin.h"
#include "antivupdateprocess.h"

namespace osds {

  AntivUpdateProcess::AntivUpdateProcess(const QString & Host_str, const QString & AntivirusType_str, const QString UpdateUrl_str, QObject * parent) : QObject(parent),
    _HostName_str(Host_str),
    _AntivirusType_str(AntivirusType_str),
    _UpdateUrl_str(UpdateUrl_str)
  {

  }

  void AntivUpdateProcess::process()
  {
    try {
      AntivirusCodeErrors AntivirusCodeErrors_en = ACENone_en;
      SSHWorker SSHWorker_o(_HostName_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
      if(_AntivirusType_str.indexOf("kesl") != -1) {  // для каспера
        if(_UpdateUrl_str.isEmpty()) {
          emit Output_sig(false, AntivirusCodeErrors_en);
          emit finished();
          return;
        }
        if(SSHWorker_o.ExecCommandOnHost_b("sudo kesl-control --set-settings 6 SourceType=Custom CustomSources.item_0000.URL="
                                           + _UpdateUrl_str +
                                           " CustomSources.item_0000.Enabled=Yes ApplicationUpdateMode=DownloadAndInstall") == false ) {
          throw(ACESetUpdateSettings);
        }
        if (SSHWorker_o.ExecCommandOnHost_b("sudo kesl-control --start-task 6 -W | grep \"UpdateError\"") == false) {
          throw(ACESetUpdateProcess);
        }
        QStringList Output_lst = SSHWorker_o.GetLastOutput_lst();
        bool IsOk_b = true;
        foreach(const QString & Text_str, Output_lst) {
          if(Text_str.indexOf("UpdateError") != -1) {
            IsOk_b = false;
            AntivirusCodeErrors_en = ACENoUpdates_en;
            break;
          }
        }
        emit Output_sig(IsOk_b, AntivirusCodeErrors_en);
      } else if (_AntivirusType_str.indexOf("drweb") != -1) { // для dr.Web
        if(_UpdateUrl_str.isEmpty()) {
          emit Output_sig(false, AntivirusCodeErrors_en);
          emit finished();
          return;
        }
        if(SSHWorker_o.ExecCommandOnHost_b("sudo drweb-ctl update 2>&1")) {
          QStringList Output_lst = SSHWorker_o.GetLastOutput_lst();
          bool IsOk_b = true;
          foreach(const QString & Text_str, Output_lst) {
            if(Text_str.indexOf("Error: No update servers available") != -1) {
              IsOk_b = false;
              AntivirusCodeErrors_en = ACENoUpdates_en;
              break;
            } else if(Text_str.indexOf("Error: Invalid license") != -1) {
              IsOk_b = false;
              AntivirusCodeErrors_en = ACENoLicense_en;
              break;
            }
          }
          emit Output_sig(IsOk_b, AntivirusCodeErrors_en);
        } else {
          emit Output_sig(false, AntivirusCodeErrors_en);
        }
      }
      else {
        emit Output_sig(false, AntivirusCodeErrors_en);
      }
    } catch(OSDSException & e) {
       qWarning()<<"AntivUpdateProcess::process->"<<e.what();
       emit Output_sig(false, ACENone_en);
    } catch (AntivirusCodeErrors Error_en) {
      qWarning()<<"AntivUpdateProcess::process->AntivirusCodeError::"<<Error_en;
      emit Output_sig(false, Error_en);
    }
    emit finished();
  }

  void AntivUpdateProcess::stop()
  {

  }
}
