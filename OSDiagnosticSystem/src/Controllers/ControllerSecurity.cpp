#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>
#include <QThread>
#include <QSqlQuery>
#include <QDateTime>

#include "sshworker.h"
#include "ControllerSecurity.h"
#include "ControllerLogin.h"
#include "databaseworker.h"
#include "httpsessionstore.h"
#include "osdsexception.h"
#include "osdsapi.h"
#include "validation.h"
#include "constants.h"
#include "testhostprocess.h"

using namespace stefanfrings;

namespace osds {

  QMap<QString, ThreadHostState> ControllerSecurity::_TestHostState_smap;
  QMutex ControllerSecurity::_Mutex_so;

  QObject * CallbackCreateControllerSecurity_po()
  {
    return new ControllerSecurity;
  }

  ControllerSecurity::ControllerSecurity(QObject * parent) : ControllerAbstract(parent)
  {
    _ControllerName_str = "ControllerSecurity";
  }

  void ControllerSecurity::service(HttpRequest& request, HttpResponse& response)
  {
    if(request.getParameterMap().contains("get") && request.getParameter("get") == "hosts") {
      GetHostList_v(response);
    } else if (request.getParameterMap().contains("test") && request.getParameter("test") == "start") {
      if(request.getParameterMap().contains("host")) {
        StartSingleTest_v(request.getParameter("host"), response);
      } else {
        qWarning()<<"ControllerSecurity::service->"<<"No host name in test start command";
      }
    } else if (request.getParameterMap().contains("test") && request.getParameter("test") == "state") {
      if(request.getParameterMap().contains("host")) {
        GetStateSingleTest_v(request.getParameter("host"), response);
      } else {
        qWarning()<<"ControllerSecurity::service->"<<"No host name in test state command";
      }
    }
  }

  void ControllerSecurity::SetSettings_sv(QSettings & /*Settings_ro*/)
  {

  }

  void ControllerSecurity::InitializeDataBase_sv()
  {
    try {
      DatabaseWorker DatabaseWorker_o;
      DatabaseWorker_o.QueryExec_lst("CREATE TABLE IF NOT EXISTS tests_state(hostname text primary key, journal text, last_start text, errors text)", false);
    } catch(OSDSException & e) {
      qWarning("ControllerSecurity::InitializeDataBase_sv->Don't crreate DB table: %s", e.what());
    }
  }

  void ControllerSecurity::GetHostList_v(HttpResponse & response)
  {
    QStringList Hosts_lst = osdsapi->GetValidation_po()->ALDProcess_lst("host-list");
    QString State_str = "done";
    QJsonObject MainObject_o;
    try {
      DatabaseWorker DatabaseWorker_o(this);
#if defined(__NO_ASTRA_LINUX__)
      Hosts_lst.clear();
      Hosts_lst.push_back("astra.vbox");
      Hosts_lst.push_back("astra1.vbox");
#endif  //__NO_ASTRA_LINUX__
      QJsonArray DataArray_o = MainObject_o["data"].toArray();
      foreach(const QString & Name_str, Hosts_lst) {
        QSqlQuery Query_lst = DatabaseWorker_o.QueryExec_lst("SELECT * FROM tests_state WHERE hostname=\"" + Name_str + "\"");
        Query_lst.next();
        QJsonObject ArrayObject_o;
        ArrayObject_o["host"] = Name_str;
        ArrayObject_o["last_start"] = Query_lst.value("last_start").toString();
        ArrayObject_o["journal"] = Query_lst.value("journal").toString();
        ArrayObject_o["errors"] = Query_lst.value("errors").toString();
        ArrayObject_o["tst_state"] = TestRunState_str(Name_str);
        DataArray_o.append(ArrayObject_o);
      }
      MainObject_o["data"] = DataArray_o;
    } catch (OSDSException & e) {
      qWarning()<<"ControllerSecurity::GetHostList_v->"<<e.what();
      State_str = "error_db";
    }
    SetJsonPackageHead_v(MainObject_o, "test_hosts", State_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerSecurity::StartSingleTest_v(const QString & Host_str, HttpResponse & response)
  {
    _TestHostState_smap.remove(Host_str);
    StartTestHostInThread_po(Host_str);
    QJsonObject MainObject_o;
    SetJsonPackageHead_v(MainObject_o, "test_started", "done");
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerSecurity::GetStateSingleTest_v(const QString & Host_str, HttpResponse & response)
  {
    QJsonObject MainObject_o;
    _Mutex_so.lock();
    ThreadHostState ThreadHostState_o = _TestHostState_smap[Host_str];
    _Mutex_so.unlock();
    MainObject_o["host"] = Host_str;
    MainObject_o["last_start"] = ThreadHostState_o._CurrentDate_str;
    MainObject_o["result"] = ThreadHostState_o._ErrorsCount_u32 == 0 ? "done" : "err";
    QString TestState_str = TestRunState_str(Host_str);
    if(TestState_str != "run") {
      ThreadHostState_o._RunState_en = _HRSStop_en;
    }
    if(TestState_str == "Error_SSH_Connect" && ThreadHostState_o._Journal_str.isEmpty()) {
      ThreadHostState_o._Journal_str = "Error_SSH_Connect";
      ThreadHostState_o._ErrorsCount_u32 = 1;
    }
    MainObject_o["journal"] = ThreadHostState_o._Journal_str;
    MainObject_o["errors"] = QString::number(ThreadHostState_o._ErrorsCount_u32);
    MainObject_o["tst_state"] = TestState_str;
    if(ThreadHostState_o._CurrentDate_str.isEmpty() && ThreadHostState_o._RunState_en == _HRSStop_en) {
      ThreadHostState_o._RunState_en = _HRSProcess_en;
    }
    SetJsonPackageHead_v(MainObject_o, "test_state", ThreadHostState_o._RunState_en != _HRSProcess_en ? "stop" : "process");
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  QThread * ControllerSecurity::StartTestHostInThread_po(const QString & Host_str)
  {
    _TestHostState_smap[Host_str] = ThreadHostState(_HRSProcess_en);
    if(TestRunState_str(Host_str) == "run") {
      return nullptr;
    }
    TestHostProcess * TestHostProcess_po = new TestHostProcess(Host_str);

    QThread * TestThread_po = new QThread;
    TestHostProcess_po->moveToThread(TestThread_po);
    connect(TestThread_po, SIGNAL(started()), TestHostProcess_po, SLOT(process()));
    connect(TestHostProcess_po, &TestHostProcess::Output_sig, [=](QString Host_str, quint32 Errors_u32, QString Journal_str){
      // Формирование отчета о сканировании хостов
      QString CurrentDT_str = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
      _Mutex_so.lock();
      _TestHostState_smap[Host_str] = ThreadHostState(_HRSStop_en, Errors_u32, Journal_str, CurrentDT_str);
      _Mutex_so.unlock();
      try {
        DatabaseWorker DatabaseWorker_o;
        DatabaseWorker_o.QueryExec_lst("INSERT INTO tests_state VALUES(\"" + Host_str + "\", \"\", \"\", \"\")", false);
        DatabaseWorker_o.QueryExec_lst("UPDATE tests_state SET journal=\"" + Journal_str + "\", last_start=\"" + CurrentDT_str +
                                       "\", errors=\"" + QString::number(Errors_u32) + "\"" + " WHERE hostname = \"" + Host_str + "\"");
      } catch (OSDSException & e) {
        qWarning()<<"ControllerSecurity::StartTestHostInThread_po->"<<e.what();
      }
    });
    connect(TestHostProcess_po, SIGNAL(finished()), TestThread_po, SLOT(quit()));
    connect(TestHostProcess_po, SIGNAL(finished()), TestHostProcess_po, SLOT(deleteLater()));
    connect(TestThread_po, SIGNAL(finished()), TestThread_po, SLOT(deleteLater()));
    TestThread_po->start(); // Запуск сканирования хостов (в потоках)
    return TestThread_po;
  }

  QString ControllerSecurity::TestRunState_str(const QString & Host_str)
  {
    QString TestState_str = "not run";
    try {//----- Тут можно рассмотреть текущее состояние тестирования -----
      SSHWorker SSHWorker_o(Host_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
      if(SSHWorker_o.ExecCommandOnHost_b("pgrep -x run.sh >/dev/null && echo \"true\" || echo \"false\"")) {
        if(SSHWorker_o.GetLastOutput_lst().join("\n").indexOf("true") != -1) {
          TestState_str = "run";
        }
      }
    } catch (OSDSException & e) {
      TestState_str = "Error_SSH_Connect";
      qWarning()<<"ControllerSecurity::GetHostList_v->"<<e.what();
    }
    return TestState_str;
  }
}
