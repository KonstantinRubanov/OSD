#include "sshworker.h"
#include "osdsexception.h"
#include "ControllerAntivirus.h"
#include "ControllerLogin.h"
#include "testhostprocess.h"

namespace osds {

  TestHostProcess::TestHostProcess(const QString & Host_str, QObject * parent) : QObject(parent),
    _HostName_str(Host_str)
  {

  }

  void TestHostProcess::process()
  {
    try {
      SSHWorker SSHWorker_o(_HostName_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
      if(SSHWorker_o.ExecCommandOnHost_b("cd /usr/lib/parsec/tests; sudo ./run.sh")) {
        QStringList Output_lst = SSHWorker_o.GetLastOutput_lst();//, InfectOutput_lst;
        quint32 ErrorsCount_u32 = 0;
        QString CurrentDT_str = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
        QString Journal_str   = "Дата и время проведения тестирования: " + CurrentDT_str + ". <br>\r\n";
        foreach(const QString & OutLine_str, Output_lst) {
          Journal_str += OutLine_str + ";";
          QString EndErrorsRU_str = "Количество ошибок: ", EndErrorsENG_str = "Number of errors: ";
          if(OutLine_str.indexOf(EndErrorsRU_str) != -1) {
            ErrorsCount_u32 = OutLine_str.right(OutLine_str.size() - EndErrorsRU_str.size()).toUInt();
          } else if (OutLine_str.indexOf(EndErrorsENG_str) != -1) {
            ErrorsCount_u32 = OutLine_str.right(OutLine_str.size() - EndErrorsENG_str.size()).toUInt();
          }
        }
        emit Output_sig(_HostName_str, ErrorsCount_u32, Journal_str);
        emit finished();
      }
    } catch (OSDSException & e) {
      qWarning()<<"TestHostProcess::"<< e.what();
      emit Output_sig(_HostName_str, 1, "Error_SSH_Connect");
      emit finished();
    }

  }

  void TestHostProcess::stop()
  {

  }
}
