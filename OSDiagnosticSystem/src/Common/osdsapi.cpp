
#include <QCoreApplication>
#include <QDir>

#include "filelogger.h"
#include "httpsessionstore.h"
#include "staticfilecontroller.h"
#include "osdsapi.h"
#include "validation.h"

using namespace stefanfrings;

namespace osds
{
  OSDSAPI * OSDSAPI::_OSDSAPI_po = nullptr;

  OSDSAPIDestroyer OSDSAPI::_OSDSAPIDestroyer_o;

  OSDSAPIDestroyer::~OSDSAPIDestroyer()
  {
    delete OSDSAPI::_OSDSAPI_po;
  }

  void OSDSAPIDestroyer::InitDestoyer_v(OSDSAPI * OSDSAPI_po)
  {
    OSDSAPI::_OSDSAPI_po = OSDSAPI_po;
  }

  OSDSAPI::OSDSAPI() :
    _Validation_po(new Validation),
    _Docroot_pstr(new QString)
  {
  }

  OSDSAPI::~OSDSAPI()
  {
    delete _Logger_po;
    _Logger_po = nullptr;
    delete _SessionStore_po;
    _SessionStore_po = nullptr;
    delete _FileController_po;
    _FileController_po = nullptr;
    delete _Validation_po;
    _Validation_po = nullptr;
  }

  OSDSAPI* OSDSAPI::instance()
  {
    if(!_OSDSAPI_po) {
      _OSDSAPI_po = new OSDSAPI();
      _OSDSAPIDestroyer_o.InitDestoyer_v(_OSDSAPI_po);
    }
    return _OSDSAPI_po;
  }

  void OSDSAPI::InitLogger_v(QSettings * Settings_po, QCoreApplication * Application_po)
  {
    if(!_Logger_po) { // Только если еще не создан
      _Logger_po = new FileLogger(Settings_po, 10000, Application_po);
      _Logger_po->installMsgHandler();
    }
  }

  void OSDSAPI::InitSession_v(QSettings * Settings_po, QCoreApplication * Application_po)
  {
    if(!_SessionStore_po) { // Только если еще не создан
      _SessionStore_po = new HttpSessionStore(Settings_po, Application_po);
    }
  }

  void OSDSAPI::InitFileController_v(QSettings * Settings_po, QCoreApplication * Application_po)
  {
    *_Docroot_pstr=Settings_po->value("path",".").toString();
    if(!(_Docroot_pstr->startsWith(":/") || _Docroot_pstr->startsWith("qrc://"))) {
        #ifdef Q_OS_WIN32
            if (QDir::isRelativePath(docroot) && Settings_po->format()!=QSettings::NativeFormat)
        #else
            if (QDir::isRelativePath(*_Docroot_pstr))
        #endif
        {
            QFileInfo configFile(Settings_po->fileName());
            *_Docroot_pstr=QFileInfo(configFile.absolutePath(), *_Docroot_pstr).absoluteFilePath();
        }
    }
    if(!_FileController_po) { // Только если еще не создан
      _FileController_po = new StaticFileController(Settings_po, Application_po);
    }
  }
}
