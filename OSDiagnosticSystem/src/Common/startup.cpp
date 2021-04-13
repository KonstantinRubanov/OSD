/**
  @file
  @author Stefan Frings
*/

#include <QDir>
#include <QFile>
#include <QString>

#if defined(Q_OS_LINUX)
#include <unistd.h>
#endif

#include "startup.h"
#include "osdsapi.h"
#include "requesthandler.h"
#include "pipelistener.h"
#include "httplistener.h"
#include "databaseworker.h"

/** Name of this application */
#define APPNAME "asb"

/** Publisher of this application */
#define ORGANISATION "KROM-C"

/** Short description of the Windows service */
#define DESCRIPTION "Astra Linux monitoring system"

using namespace stefanfrings;

namespace osds
{

  /** Search the configuration file */
  QString searchConfigFile()
  {
    QString binDir=QCoreApplication::applicationDirPath();
    QString appName=QCoreApplication::applicationName();
    QString fileName(appName+".ini");

    QStringList searchList;
    searchList.append(binDir);
    searchList.append(binDir+"/etc");
    searchList.append(binDir+"/../etc");
    searchList.append(binDir+"/../../etc"); // for development without shadow build
    searchList.append(binDir+"/../"+appName+"/etc"); // for development with shadow build
    searchList.append(binDir+"/../../"+appName+"/etc"); // for development with shadow build
    searchList.append(binDir+"/../../../"+appName+"/etc"); // for development with shadow build
    searchList.append(binDir+"/../../../../"+appName+"/etc"); // for development with shadow build
    searchList.append(binDir+"/../../../../../"+appName+"/etc"); // for development with shadow build
    searchList.append(QDir::rootPath()+"etc/opt");
    searchList.append(QDir::rootPath()+"etc");

    foreach (QString dir, searchList)
    {
      QFile file(dir+"/"+fileName);
      if (file.exists())
      {
        // found
        fileName=QDir(file.fileName()).canonicalPath();
        qDebug("Using config file %s", qPrintable(fileName));
        return fileName;
      }
    }

    // not found
    foreach (QString dir, searchList)
    {
      qWarning("%s/%s not found",qPrintable(dir),qPrintable(fileName));
    }
    qFatal("Cannot find config file %s",qPrintable(fileName));
    //    return QString();
  }

  void Startup::start()
  {
#if !defined (__NO_ASTRA_LINUX__)
    if (getuid()) {
      qWarning()<<"Run program from NOT! sudo user!!!";
      exit(-1);
    }
#endif  //__NO_ASTRA_LINUX__
    // Initialize the core application
    QCoreApplication* CoreApp_po = application();
    CoreApp_po->setApplicationName(APPNAME);
    CoreApp_po->setOrganizationName(ORGANISATION);

    // Find the configuration file
    QString configFileName=searchConfigFile();

    // Configure logging into a file
    QSettings* LogSettings_po=new QSettings(configFileName,QSettings::IniFormat,CoreApp_po);
    LogSettings_po->beginGroup("logging");
    osdsapi->InitLogger_v(LogSettings_po, CoreApp_po);

    //----- Задание настроек для работы с БД -----
    QSettings* SQLSettings_po = new QSettings(configFileName, QSettings::IniFormat,CoreApp_po);
    SQLSettings_po->beginGroup("sql");
    DatabaseWorker::SetSettings_sv(*SQLSettings_po);

    // Configure and start the TCP listener
    qDebug("ServiceHelper: Starting service");
    QSettings* listenerSettings=new QSettings(configFileName,QSettings::IniFormat,CoreApp_po);
    listenerSettings->beginGroup("listener");
    _Listener_po=new HttpListener(listenerSettings, new RequestHandler(configFileName, CoreApp_po), CoreApp_po);

    // Configure and start PiPe listener
    QSettings* PipeSettings_po = new QSettings(configFileName,QSettings::IniFormat,CoreApp_po);
    PipeSettings_po->beginGroup("pipe");
    _PipeListener_po = new PipeListener(PipeSettings_po, CoreApp_po);
    //----- Задание настроек для работы с контроллером файлов -----
    QSettings* StaticSettings_po=new QSettings(configFileName,QSettings::IniFormat,CoreApp_po);
    StaticSettings_po->beginGroup("docroot");
    osdsapi->InitFileController_v(StaticSettings_po, CoreApp_po);
    //----- Задание настроек для работы с сессиями -----
    QSettings* SessionsSettings_po = new QSettings(configFileName, QSettings::IniFormat,CoreApp_po);
    SessionsSettings_po->beginGroup("sessions");
    osdsapi->InitSession_v(SessionsSettings_po, CoreApp_po);

    qWarning("Startup: Service has started");
  }

  void Startup::stop()
  {
    // Note that this method is only called when the application exits itself.
    // It is not called when you close the window, press Ctrl-C or send a kill signal.

    delete _Listener_po;
    qWarning("Startup: Service has been stopped");
  }


  Startup::Startup(int argc, char *argv[]) : QtService<QCoreApplication>(argc, argv, APPNAME)
  {
    setServiceDescription(DESCRIPTION);
    setStartupType(QtServiceController::AutoStartup);
  }

}

