#include <QJsonObject>

#include "ControllerAbstract.h"
#include "constants.h"

namespace osds {

  ControllerAbstract::ControllerAbstract(QObject * parent) : stefanfrings::HttpRequestHandler(parent)
  {

  }

  ControllerAbstract::~ControllerAbstract()
  {

  }

  void ControllerAbstract::SetJsonPackageHead_v(QJsonObject & JsonObgect_ro, const QString & Operation_rc, const QString & State_rc)
  {
    JsonObgect_ro["version"] = OSDS_VERSION;
    JsonObgect_ro["object"] = this->GetControllerName_str();
    JsonObgect_ro["operation"] = Operation_rc;
    JsonObgect_ro["state"] = State_rc;
  }

} // namespace osds

