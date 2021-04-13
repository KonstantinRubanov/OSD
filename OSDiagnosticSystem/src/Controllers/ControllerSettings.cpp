#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>
#include <QThread>
#include <QCoreApplication>
#include <QProcess>
#include <QDir>

#include "ControllerSettings.h"
#include "httpsessionstore.h"
#include "osdsapi.h"
#include "validation.h"
#include "constants.h"

using namespace stefanfrings;

namespace osds {

  QString ControllerSettings::SettingsFileName_sstr = "";

  QObject * CallbackCreateControllerSettings_po()
  {
    return new ControllerSettings;
  }

  ControllerSettings::ControllerSettings(QObject * parent) : ControllerAbstract(parent)
  {
    _ControllerName_str = "ControllerSettings";
  }

  void ControllerSettings::InitializeDataBase_sv()
  {

  }

  void ControllerSettings::service(HttpRequest& request, HttpResponse& response)
  {
    if(request.getParameterMap().contains("get") && request.getParameter("get") == "list") {
      GetSettingsList_v(response);
    } else if (request.getParameterMap().contains("set") && request.getParameter("set") == "list") {
      if(request.getParameterMap().contains("list")) {
        SetSettingsList_v(request.getParameter("list"), response);
      } else {
        qWarning("ControllerSettings::Error! No correct tag!");
      }
    } else if (request.getParameterMap().contains("action") && request.getParameter("action") == "reboot") {
      QFile::remove(QDir::temp().absoluteFilePath("osdssingle.lock"));
      qApp->exit(0); // Просто закрываем ПО, должно перезапуститься когда работает как сервис
    }
  }

  void ControllerSettings::SetSettingsFileName_sv(const QString & FileName_str)
  {
    ControllerSettings::SettingsFileName_sstr = FileName_str;
  }

  void ControllerSettings::GetSettingsList_v(HttpResponse & response)
  {
    QJsonObject MainObject_o;
    SetJsonPackageHead_v(MainObject_o, "settings_list", "done");
    QJsonArray DataArray_o = MainObject_o["data"].toArray();
    QSettings Settings_o(SettingsFileName_sstr, QSettings::IniFormat);
    foreach(const QString & GroupName_str, Settings_o.childGroups()) {
      {
        QJsonObject ArrayObject_o;
        ArrayObject_o["type"] = "title";
        ArrayObject_o["name"] = GroupName_str;
        ArrayObject_o["val"] = "";
        DataArray_o.append(ArrayObject_o);
      }
      Settings_o.beginGroup(GroupName_str);
      foreach(const QString & KeyName_str, Settings_o.childKeys()) {
        QJsonObject ArrayObject_o;
        ArrayObject_o["type"] = "td";
        ArrayObject_o["name"] = KeyName_str;
        ArrayObject_o["val"] = Settings_o.value(KeyName_str).toString();
        DataArray_o.append(ArrayObject_o);
      }
      Settings_o.endGroup();
    }
    MainObject_o["data"] = DataArray_o;
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerSettings::SetSettingsList_v(const QString & SettingsList_str, HttpResponse & response)
  {
    QString Status_str = "done";
    QSettings Settings_o(SettingsFileName_sstr, QSettings::IniFormat);

    QJsonObject ReadObject_o;
    QJsonDocument JSonSettings_o = QJsonDocument::fromJson(SettingsList_str.toUtf8());
    if(!JSonSettings_o.isNull()) {
      if(JSonSettings_o.isObject()) {
        ReadObject_o = JSonSettings_o.object();
        QStringList Sections_lst = ReadObject_o.keys();
        foreach(const QString & SectionName_str, Sections_lst) {
          Settings_o.beginGroup(SectionName_str);
          QJsonArray Parameters_o = ReadObject_o[SectionName_str].toArray();
          foreach(QJsonValue ObjValue_o, Parameters_o) {
            QJsonObject Object_o = ObjValue_o.toObject();
            QStringList ObjKeys_lst = Object_o.keys();
            foreach(const QString & ObjKey_str, ObjKeys_lst) {
              QString ObjValue_str = Object_o[ObjKey_str].toString();
              Settings_o.setValue(ObjKey_str, ObjValue_str);
            }
          }
          Settings_o.endGroup();
        }
      } else {
        Status_str = "error";
      }
    } else {
      Status_str = "error";
    }

    QJsonObject MainObject_o;
    SetJsonPackageHead_v(MainObject_o, "set_settings_list", Status_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }
}
