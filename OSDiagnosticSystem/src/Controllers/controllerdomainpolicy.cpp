#include <QObject>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHostInfo>
#include <QFileInfo>
#include <QUuid>

#include "ControllerAbstract.h"
#include "osdsexception.h"
#include "databaseworker.h"

#include "controllerdomainpolicy.h"

using namespace stefanfrings;

namespace osds {

  QString ControllerDomainPolicy::_Username_str = "";
  QString ControllerDomainPolicy::_Password_str = "";

  QObject * CallbackCreateControllerDomainPolicy_po()
  {
    return new ControllerDomainPolicy;
  }

  ControllerDomainPolicy::ControllerDomainPolicy(QObject * parent) : ControllerAbstract (parent)
  {
    _ControllerName_str = "ControllerDomainPolicy";
  }

  void ControllerDomainPolicy::service(HttpRequest& request, HttpResponse& response)
  {
      QByteArray Path_o = request.getPath();


      if(request.getParameterMap().contains("domainpolicy")) {

          if (request.getParameter("domainpolicy") == "domaindevlist"){
            //qWarning()<<"ControllerDomainPolicy::service...";
            QVariantMap resourceNode;
            QString Command_str = QString("pass_dev='/tmp/osdsfile.pas'; rule=$(sudo ald-admin device-list --pass-file=$pass_dev);  (for x in  $rule; do  result=''; IFS=': ';  read -ra my_array <<< \"$x\"; if [ \"$(sudo grep \":\" <<< $x)\" == '' ]; then  continue; fi;  counter=0; result='';  unit=$(sudo ald-admin device-get ${my_array[0]}  --pass-file=$pass_dev);  res=$(awk '/.: /{print $0}' <<< $unit | sudo cut -f2 -d ':');  while IFS=  read -r line_par; do result+=\"|$line_par\";  done <<< $res; dev_str=\"\";  cnt_dev=0; while IFS=  read -r line_dev;  do if [ $cnt_dev !=  0 ]; then  dev_str+=\"#$line_dev\"; else dev_str=$line_dev; fi;   cnt_dev=$[$cnt_dev+1];  done <<< $(sudo grep -o 'ENV{ID_SERIAL_SHORT}==\"[^\"]*\"' <<< \"$unit\"  | sudo cut -f2 -d '\"');      echo \"$dev_str$result\";  done;)").toUtf8();
            //qWarning()<<Command_str;
            int RetCode = -1;
            int cntRules = 0;
            int cnt_usb_prop = 0;
            QVariantMap map;
            QVariantMap hostNode;
            QVariantMap deviceNode;
            QVariantMap deviceNodeGroup;

           // qWarning()<<"ControllerDomainPolicy::service... SSHWorkerObj...";
            SSHWorker SSHWorkerObj("127.0.0.1",ControllerDomainPolicy::_Username_str,ControllerDomainPolicy::_Password_str);
            map.insert("version", "1.1.1.1");
            map.insert("object", 30);
            map.insert("action", "");
           // qWarning()<<"SSH_action(SSHWorkerObj, Command_str))";
            if(SSH_action(SSHWorkerObj, Command_str)){
              RetCode = 0;
              QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
              SSHWorkerObjlst.removeAll(QString(""));
              qWarning()<<SSHWorkerObjlst;
              for (auto pKey: SSHWorkerObjlst){
                QStringList Temp_lst = pKey.split("| ");
                QVariantMap deviceNodeProp;
                cnt_usb_prop = 0;
                if(Temp_lst.size() >= 6){                
                  QStringList dev_split = Temp_lst[0].split("#");
                  dev_split.removeAll(QString(""));
                  if(dev_split.size() > 1){
                     for(auto &p : dev_split){
                         deviceNodeProp.insert(QString::number(cnt_usb_prop).toUtf8(), p.toUtf8());
                         cnt_usb_prop++;
                      }
                  }
                  else
                    deviceNodeProp.insert(QString::number(cnt_usb_prop).toUtf8(), Temp_lst[0].toUtf8());
                  deviceNode.insert("name",Temp_lst[1].toUtf8());
                  deviceNode.insert("serial",deviceNodeProp);
                  deviceNode.insert("owner",Temp_lst[3].toUtf8());
                  deviceNode.insert("state",Temp_lst[2].toUtf8());
                  deviceNode.insert("group",Temp_lst[4].toUtf8());
                  deviceNode.insert("info",Temp_lst[5].toUtf8());
                  deviceNode.insert("rule",Temp_lst[6].toUtf8());
                  deviceNodeGroup.insert(QString::number(cntRules).toUtf8(), deviceNode);
                }
                cntRules++;
              }
            }
            if(RetCode == 0){
              map.insert("state", 0);
              if(cntRules)
                map.insert("data", deviceNodeGroup);
            }
            QJsonObject json = QJsonObject::fromVariantMap(map);
            QJsonDocument JsonDocument_o(json);
            //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
            if (!JsonDocument_o.isNull()) {
              response.setHeader("Content-Type", "text/html; charset=UTF-8");
              //qWarning()<<"ControllerDomainPolicy::service...response.write";
              response.write(JsonDocument_o.toJson());
              response.flush();
            }
            if(RetCode != 0)
              qWarning()<<"Command:"<<Command_str<<" unsuccess ret code:"<<RetCode;
          }


          if (request.getParameter("domainpolicy") == "appendrule"){
            QVariantMap resourceNode;
            QString DevName="";
            QString DevSerial="";
            QString DevOwner="";
            QString DevGroup="";
            QString DevDescr="";
            QString DevMode="";
            QString DevState="";
            QString AllRules="";
            bool reload = false;

            if(request.getParameterMap().contains("devname"))
              DevName = request.getParameter("devname");

            if(request.getParameterMap().contains("devserial")){
              DevSerial = request.getParameter("devserial");
              QStringList dev_split = DevSerial.split("|");
              dev_split.removeAll(QString(""));
              if(dev_split.size() > 1){
                 for(auto &p : dev_split){
                   AllRules += " --attr=ENV{ID_SERIAL_SHORT}==\\\"" + QString(p).remove("\"") + "\\\" ";
                 }
              }
              else
                AllRules = " --attr=ENV{ID_SERIAL_SHORT}==\\\"" + DevSerial + "\\\" ";
             }

            if(request.getParameterMap().contains("devowner"))
              DevOwner = request.getParameter("devowner");

            if(request.getParameterMap().contains("devgroup"))
              DevGroup = request.getParameter("devgroup");

            if(request.getParameterMap().contains("devdescr"))
              DevDescr = " --desc=\"" + request.getParameter("devdescr") + "\"";
            else
              DevDescr = " --desc=\" \"";

            if(request.getParameterMap().contains("devmode"))
              DevMode = request.getParameter("devmode");

            if(request.getParameterMap().contains("devstate"))
              DevState = request.getParameter("devstate");

            if(request.getParameterMap().contains("reload"))
              reload = true;
            //QString Command_str("yes | sudo ald-admin device-add " + DevName + " --attr=ENV{ID_SERIAL_SHORT}==\"" + DevSerial + "\"  --owner=\"" + DevOwner + "\" --group=\"" + DevGroup + "\" --desc=\"" + DevDescr + "\" --mode=" + DevMode + " --status=" + DevState + " --pass-file=/tmp/osdsfile.pas");
            QString Command_str_reload = "";

            if(reload)
              Command_str_reload = QString("sudo echo 'yes' | sudo ald-admin  device-rm  %1 --pass-file=/tmp/osdsfile.pas; ").arg(
                    DevName
                    ).toUtf8();

            QString Command_str = QString("%1 sudo echo 'yes' | sudo ald-admin device-add %2 %3 --owner=\"%4\" --group=\"%5\"  %6 --mode=%7 --status=%8 --pass-file=/tmp/osdsfile.pas").arg(
                    Command_str_reload,
                    DevName,
                    AllRules,
                    DevOwner,
                    DevGroup,
                    DevDescr,
                    DevMode,
                    DevState).toUtf8();
            qWarning()<<Command_str;
            int RetCode = -1;
            QVariantMap map;
            QVariantMap hostNode;
            QVariantMap deviceNode;
            QVariantMap deviceNodeGroup;
            QString errSsh="";
           // qWarning()<<"ControllerDomainPolicy::service... SSHWorkerObj...";
            SSHWorker SSHWorkerObj("127.0.0.1",ControllerDomainPolicy::_Username_str,ControllerDomainPolicy::_Password_str);
            map.insert("version", "1.1.1.1");
            map.insert("object", 30);
            map.insert("action", "");
           // qWarning()<<"SSH_action(SSHWorkerObj, Command_str))";
            if(SSH_action(SSHWorkerObj, Command_str)){
               RetCode = 0;
               QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
               SSHWorkerObjlst.removeAll(QString(""));
               qWarning()<<SSHWorkerObjlst;
               QString strData = SSHWorkerObjlst.join("");
               QRegExp rx("\\b(ОШИБКА:)\\b");
               if(rx.indexIn(strData) > 0){
                   RetCode = 1;
                 errSsh = strData;
               }
               else
                 errSsh = "";
            }
            if(RetCode == 0){
              map.insert("state", 0);
              deviceNode.insert("update","OK");
              map.insert("data", deviceNode);
            }
            else if(RetCode == 1){
               map.insert("state", 1);
               deviceNode.insert("update", errSsh.toUtf8());
            }
            else{
               map.insert("state", -1);
               deviceNode.insert("update", SSHWorkerObj.GetLastError_str());
            }
             map.insert("data", deviceNode);
            QJsonObject json = QJsonObject::fromVariantMap(map);
            QJsonDocument JsonDocument_o(json);
            //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
            if (!JsonDocument_o.isNull()) {
              response.setHeader("Content-Type", "text/html; charset=UTF-8");
              //qWarning()<<"ControllerDomainPolicy::service...response.write";
              response.write(JsonDocument_o.toJson());
              response.flush();
            }
            if(RetCode != 0)
              qWarning()<<"Command:"<<Command_str<<" unsuccess ret code:"<<RetCode;
          }


          if (request.getParameter("domainpolicy") == "rmdev"){
            QVariantMap resourceNode;
            QString DevName="";

            if(request.getParameterMap().contains("devname"))
              DevName = request.getParameter("devname");

            QString Command_str = QString("sudo echo 'yes' | sudo ald-admin  device-rm  %1 --pass-file=/tmp/osdsfile.pas").arg(
                    DevName
                    ).toUtf8();
            qWarning()<<Command_str;
            int RetCode = -1;
            QVariantMap map;
            QVariantMap hostNode;
            QVariantMap deviceNode;
            QVariantMap deviceNodeGroup;
            QString errSsh="";
           // qWarning()<<"ControllerDomainPolicy::service... SSHWorkerObj...";
            SSHWorker SSHWorkerObj("127.0.0.1",ControllerDomainPolicy::_Username_str,ControllerDomainPolicy::_Password_str);
            map.insert("version", "1.1.1.1");
            map.insert("object", 30);
            map.insert("action", "");
           // qWarning()<<"SSH_action(SSHWorkerObj, Command_str))";
            if(SSH_action(SSHWorkerObj, Command_str)){
               RetCode = 0;
               QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
               SSHWorkerObjlst.removeAll(QString(""));
               qWarning()<<SSHWorkerObjlst;
               QString strData = SSHWorkerObjlst.join("");
               QRegExp rx("\\b(ОШИБКА:)\\b");
               if(rx.indexIn(strData) > 0){
                   RetCode = 1;
                 errSsh = strData;
               }
               else
                 errSsh = "";
            }
            if(RetCode == 0){
              map.insert("state", 0);
              deviceNode.insert("update","OK");
              map.insert("data", deviceNode);
            }
            else if(RetCode == 1){
               map.insert("state", 1);
               deviceNode.insert("update", errSsh.toUtf8());
            }
            else{
               map.insert("state", -1);
               deviceNode.insert("update", SSHWorkerObj.GetLastError_str());
            }
             map.insert("data", deviceNode);
            QJsonObject json = QJsonObject::fromVariantMap(map);
            QJsonDocument JsonDocument_o(json);
            //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
            if (!JsonDocument_o.isNull()) {
              response.setHeader("Content-Type", "text/html; charset=UTF-8");
              //qWarning()<<"ControllerDomainPolicy::service...response.write";
              response.write(JsonDocument_o.toJson());
              response.flush();
            }
            if(RetCode != 0)
              qWarning()<<"Command:"<<Command_str<<" unsuccess ret code:"<<RetCode;
          }

          if (request.getParameter("domainpolicy") == "rmproperty"){
            QVariantMap resourceNode;
            QString DevName="";
            QString DevRule="";
            if(request.getParameterMap().contains("devname"))
              DevName = request.getParameter("devname");
            if(request.getParameterMap().contains("devproperty"))
              DevRule = request.getParameter("devproperty");
            QString Command_str = QString("sudo ald-admin device-mod  %1 --attr-rm=ENV{ID_SERIAL_SHORT}==\\\"%2\\\" --pass-file=/tmp/osdsfile.pas").arg(
                    DevName,
                    DevRule
                    ).toUtf8();
            qWarning()<<Command_str;
            int RetCode = -1;
            QVariantMap map;
            QVariantMap hostNode;
            QVariantMap deviceNode;
            QVariantMap deviceNodeGroup;
            QString errSsh="";
           // qWarning()<<"ControllerDomainPolicy::service... SSHWorkerObj...";
            SSHWorker SSHWorkerObj("127.0.0.1",ControllerDomainPolicy::_Username_str,ControllerDomainPolicy::_Password_str);
            map.insert("version", "1.1.1.1");
            map.insert("object", 41);
            map.insert("action", "");
           // qWarning()<<"SSH_action(SSHWorkerObj, Command_str))";
            if(SSH_action(SSHWorkerObj, Command_str)){
               RetCode = 0;
               QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
               SSHWorkerObjlst.removeAll(QString(""));
               qWarning()<<SSHWorkerObjlst;
               QString strData = SSHWorkerObjlst.join("");
               QRegExp rx("\\b(ОШИБКА:)\\b");
               if(rx.indexIn(strData) > 0){
                   RetCode = 1;
                 errSsh = strData;
               }
               else
                 errSsh = "";
            }
            if(RetCode == 0){
              map.insert("state", 0);
              deviceNode.insert("update","OK");
              map.insert("data", deviceNode);
            }
            else if(RetCode == 1){
               map.insert("state", 1);
               deviceNode.insert("update", errSsh.toUtf8());
            }
            else{
               map.insert("state", -1);
               deviceNode.insert("update", SSHWorkerObj.GetLastError_str());
            }
             map.insert("data", deviceNode);
            QJsonObject json = QJsonObject::fromVariantMap(map);
            QJsonDocument JsonDocument_o(json);
            //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
            if (!JsonDocument_o.isNull()) {
              response.setHeader("Content-Type", "text/html; charset=UTF-8");
              //qWarning()<<"ControllerDomainPolicy::service...response.write";
              response.write(JsonDocument_o.toJson());
              response.flush();
            }
            if(RetCode != 0)
              qWarning()<<"Command:"<<Command_str<<" unsuccess ret code:"<<RetCode;
          }

          if (request.getParameter("domainpolicy") == "addproperty"){
            QVariantMap resourceNode;
            QString DevName="";
            QString DevRule="";
            if(request.getParameterMap().contains("devname"))
              DevName = request.getParameter("devname");
            if(request.getParameterMap().contains("devproperty"))
              DevRule = request.getParameter("devproperty");
            QString Command_str = QString("sudo ald-admin device-mod  %1  --attr-add=ENV{ID_SERIAL_SHORT}==\\\"%2\\\" --pass-file=/tmp/osdsfile.pas").arg(
                    DevName,
                    DevRule
                    ).toUtf8();
            qWarning()<<Command_str;
            int RetCode = -1;
            QVariantMap map;
            QVariantMap hostNode;
            QVariantMap deviceNode;
            QVariantMap deviceNodeGroup;
            QString errSsh="";
           // qWarning()<<"ControllerDomainPolicy::service... SSHWorkerObj...";
            SSHWorker SSHWorkerObj("127.0.0.1",ControllerDomainPolicy::_Username_str,ControllerDomainPolicy::_Password_str);
            map.insert("version", "1.1.1.1");
            map.insert("object", 42);
            map.insert("action", "");
           // qWarning()<<"SSH_action(SSHWorkerObj, Command_str))";
            if(SSH_action(SSHWorkerObj, Command_str)){
               RetCode = 0;
               QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
               SSHWorkerObjlst.removeAll(QString(""));
               qWarning()<<SSHWorkerObjlst;
               QString strData = SSHWorkerObjlst.join("");
               QRegExp rx("\\b(ОШИБКА:)\\b");
               if(rx.indexIn(strData) > 0){
                   RetCode = 1;
                 errSsh = strData;
               }
               else
                 errSsh = "";
            }
            if(RetCode == 0){
              map.insert("state", 0);
              deviceNode.insert("update","OK");
              map.insert("data", deviceNode);
            }
            else if(RetCode == 1){
               map.insert("state", 1);
               deviceNode.insert("update", errSsh.toUtf8());
            }
            else{
               map.insert("state", -1);
               deviceNode.insert("update", SSHWorkerObj.GetLastError_str());
            }
             map.insert("data", deviceNode);
            QJsonObject json = QJsonObject::fromVariantMap(map);
            QJsonDocument JsonDocument_o(json);
            //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
            if (!JsonDocument_o.isNull()) {
              response.setHeader("Content-Type", "text/html; charset=UTF-8");
              //qWarning()<<"ControllerDomainPolicy::service...response.write";
              response.write(JsonDocument_o.toJson());
              response.flush();
            }
            if(RetCode != 0)
              qWarning()<<"Command:"<<Command_str<<" unsuccess ret code:"<<RetCode;
          }




          if (request.getParameter("domainpolicy") == "autorizeproperty"){
            QVariantMap resourceNode;
            QString DevName="";
            QString DevHost="";
            QString DevRule="";
            if(request.getParameterMap().contains("devhost"))
              DevHost = request.getParameter("devhost");
            if(request.getParameterMap().contains("devname"))
              DevName = request.getParameter("devname");
            QString Command_str = QString("echo 1 | sudo tee %1/authorized; sudo udevadm control --reload-rules").arg(
                    DevName
                    ).toUtf8();
            qWarning()<<Command_str;
            int RetCode = -1;
            QVariantMap map;
            QVariantMap hostNode;
            QVariantMap deviceNode;
            QVariantMap deviceNodeGroup;
            QString errSsh="";
           // qWarning()<<"ControllerDomainPolicy::service... SSHWorkerObj...";
            SSHWorker SSHWorkerObj(DevHost,ControllerDomainPolicy::_Username_str,ControllerDomainPolicy::_Password_str);
            map.insert("version", "1.1.1.1");
            map.insert("object", 42);
            map.insert("action", "");
           // qWarning()<<"SSH_action(SSHWorkerObj, Command_str))";
            if(SSH_action(SSHWorkerObj, Command_str)){
               RetCode = 0;
               QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
               SSHWorkerObjlst.removeAll(QString(""));
            }
            if(RetCode == 0){
              map.insert("state", 0);
              deviceNode.insert("update","OK");
              map.insert("data", deviceNode);
            }
            else if(RetCode == 1){
               map.insert("state", 1);
               deviceNode.insert("update", errSsh.toUtf8());
            }
            else{
               map.insert("state", -1);
               deviceNode.insert("update", SSHWorkerObj.GetLastError_str());
            }
             map.insert("data", deviceNode);
            QJsonObject json = QJsonObject::fromVariantMap(map);
            QJsonDocument JsonDocument_o(json);
            //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
            if (!JsonDocument_o.isNull()) {
              response.setHeader("Content-Type", "text/html; charset=UTF-8");
              //qWarning()<<"ControllerDomainPolicy::service...response.write";
              response.write(JsonDocument_o.toJson());
              response.flush();
            }
            if(RetCode != 0)
              qWarning()<<"Command:"<<Command_str<<" unsuccess ret code:"<<RetCode;
          }

          if (request.getParameter("domainpolicy") == "viewdev"){
            QVariantMap resourceNode;
            QString DevName="";

            if(request.getParameterMap().contains("devname"))
              DevName = request.getParameter("devname");

            QString Command_str = QString("x='%1'; pass_dev='/tmp/osdsfile.pas'; rule=$(sudo ald-admin device-get $x --pass-file=$pass_dev);  IFS=': ';  read -ra my_array <<< \"$x\";  result='';  unit=$(sudo ald-admin device-get ${my_array[0]}  --pass-file=$pass_dev);  res=$(awk '/.: /{print $0}' <<< $unit | sudo cut -f2 -d ':');  while IFS=  read -r line_par; do result+=\"|$line_par\";  done <<< $res; dev_str=\"\"; cnt_dev=0;  while IFS=  read -r line_dev;  do if [ $cnt_dev !=  0 ]; then  dev_str+=\"#$line_dev\"; else dev_str=$line_dev; fi;   cnt_dev=$[$cnt_dev+1];   done <<< $(sudo grep -o 'ENV{ID_SERIAL_SHORT}==\"[^\"]*\"' <<< \"$unit\"  | sudo cut -f2 -d '\"'); echo \"$dev_str$result\";").arg(
                    DevName
                    ).toUtf8();
            qWarning()<<Command_str;
            int RetCode = -1;
            QVariantMap map;
            QVariantMap hostNode;
            QVariantMap deviceNode;
            QVariantMap deviceNodeGroup;
            int cntRules = 0;
            int cnt_usb_prop = 0;
            QString errSsh="";
           // qWarning()<<"ControllerDomainPolicy::service... SSHWorkerObj...";
            SSHWorker SSHWorkerObj("127.0.0.1",ControllerDomainPolicy::_Username_str,ControllerDomainPolicy::_Password_str);
            map.insert("version", "1.1.1.1");
            map.insert("object", 30);
            map.insert("action", "");

           if(SSH_action(SSHWorkerObj, Command_str)){
               RetCode = 0;
               QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
               SSHWorkerObjlst.removeAll(QString(""));
               qWarning()<<SSHWorkerObjlst;

               QString strData = SSHWorkerObjlst.join("");
               QRegExp rx("\\b(ОШИБКА:)\\b");
               if(rx.indexIn(strData) > 0){
                   RetCode = 1;
                   errSsh = strData;
               }
               else
                 errSsh = "";

               for (auto pKey: SSHWorkerObjlst){
                 QStringList Temp_lst = pKey.split("| ");
                 if(Temp_lst.size() >= 6){
                     QVariantMap deviceNodeProp;
                     cnt_usb_prop = 0;
                     QStringList dev_split = Temp_lst[0].split("#");
                       dev_split.removeAll(QString(""));
                       if(dev_split.size() > 1){
                          for(auto &p : dev_split){
                              deviceNodeProp.insert(QString::number(cnt_usb_prop).toUtf8(), p.toUtf8());
                              cnt_usb_prop++;
                           }
                       }
                       else
                         deviceNodeProp.insert(QString::number(cnt_usb_prop).toUtf8(), Temp_lst[0].toUtf8());

                   deviceNode.insert("name",Temp_lst[1].toUtf8());
                   deviceNode.insert("serial",deviceNodeProp);
                   deviceNode.insert("owner",Temp_lst[3].toUtf8());
                   deviceNode.insert("state",Temp_lst[2].toUtf8());
                   deviceNode.insert("group",Temp_lst[4].toUtf8());
                   deviceNode.insert("info",Temp_lst[5].toUtf8());
                   deviceNode.insert("rule",Temp_lst[6].toUtf8());
                   deviceNodeGroup.insert(QString::number(cntRules).toUtf8(), deviceNode);
                 }
                 cntRules++;
               }
             }
             if(RetCode == 0){
               map.insert("state", 0);
               if(cntRules)
                 map.insert("data", deviceNodeGroup);
             }
             if(RetCode == 1){
                  map.insert("state", 1);
                  deviceNode.insert("update", errSsh.toUtf8());
             }

            QJsonObject json = QJsonObject::fromVariantMap(map);
            QJsonDocument JsonDocument_o(json);
            //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
            if (!JsonDocument_o.isNull()) {
              response.setHeader("Content-Type", "text/html; charset=UTF-8");
              //qWarning()<<"ControllerDomainPolicy::service...response.write";
              response.write(JsonDocument_o.toJson());
              response.flush();
            }
            if(RetCode != 0)
              qWarning()<<"Command:"<<Command_str<<" unsuccess ret code:"<<RetCode;
          }


          if (request.getParameter("domainpolicy") == "modrule"){
            QVariantMap resourceNode;
            QString DevName="";
            QString DevSerial="";
            QString DevOwner="";
            QString DevGroup="";
            QString DevDescr="";
            QString DevMode="";
            QString DevState="";

            QString DevModRule="";

            if(request.getParameterMap().contains("devname"))
              DevName = request.getParameter("devname");

            if(request.getParameterMap().contains("devowner")){
               DevOwner = request.getParameter("devowner");
               DevModRule +=" --owner=\"" + DevOwner + "\" ";
             }

            if(request.getParameterMap().contains("devgroup")){
               DevGroup = request.getParameter("devgroup");
               DevModRule +=" --group=\"" + DevGroup + "\" ";
             }

            if(request.getParameterMap().contains("devdescr")){
              DevDescr = request.getParameter("devdescr");
              DevModRule +=" --desc=\"" + DevDescr + "\" ";
             }

            if(request.getParameterMap().contains("devmode")){
               DevMode = request.getParameter("devmode");
               DevModRule +=" --mode=\"" + DevMode + "\" ";
             }

            if(request.getParameterMap().contains("devstate")){
              DevState = request.getParameter("devstate");
              DevModRule +=" --status=\"" + DevState + "\" ";
            }

            QString Command_str = QString("sudo ald-admin device-mod  %1 %2 --pass-file=/tmp/osdsfile.pas").arg(
                    DevName,
                    DevModRule
                    ).toUtf8();
            qWarning()<<Command_str;
            int RetCode = -1;
            QVariantMap map;
            QVariantMap hostNode;
            QVariantMap deviceNode;
            QVariantMap deviceNodeGroup;
            QString errSsh="";
           // qWarning()<<"ControllerDomainPolicy::service... SSHWorkerObj...";
            SSHWorker SSHWorkerObj("127.0.0.1",ControllerDomainPolicy::_Username_str,ControllerDomainPolicy::_Password_str);
            map.insert("version", "1.1.1.1");
            map.insert("object", 30);
            map.insert("action", "");
           // qWarning()<<"SSH_action(SSHWorkerObj, Command_str))";
            if(SSH_action(SSHWorkerObj, Command_str)){
               RetCode = 0;
               QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
               SSHWorkerObjlst.removeAll(QString(""));
               qWarning()<<SSHWorkerObjlst;
               QString strData = SSHWorkerObjlst.join("");
               QRegExp rx("\\b(ОШИБКА:)\\b");
               if(rx.indexIn(strData) > 0){
                   RetCode = 1;
                 errSsh = strData;
               }
               else
                 errSsh = "";
            }
            if(RetCode == 0){
              map.insert("state", 0);
              deviceNode.insert("update","OK");
              map.insert("data", deviceNode);
            }
            else if(RetCode == 1){
               map.insert("state", 1);
               deviceNode.insert("update", errSsh.toUtf8());
            }
            else{
               map.insert("state", -1);
               deviceNode.insert("update", SSHWorkerObj.GetLastError_str());
            }
             map.insert("data", deviceNode);
            QJsonObject json = QJsonObject::fromVariantMap(map);
            QJsonDocument JsonDocument_o(json);
            //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
            if (!JsonDocument_o.isNull()) {
              response.setHeader("Content-Type", "text/html; charset=UTF-8");
              //qWarning()<<"ControllerDomainPolicy::service...response.write";
              response.write(JsonDocument_o.toJson());
              response.flush();
            }
            if(RetCode != 0)
              qWarning()<<"Command:"<<Command_str<<" unsuccess ret code:"<<RetCode;
          }



          //list users by dev mount
           if (request.getParameter("domainpolicy") == "list_disk_serial"){
              QVariantMap map;
              QVariantMap resourceNode;
              QVariantMap driveNode;
              QString ResHost;
              QString ResIP;
              int cntObj = 0;
              /**/
             if(request.getParameterMap().contains("resourcehost"))
               ResHost = request.getParameter("resourcehost");

             QHostInfo info = QHostInfo::fromName(ResHost);
             const auto addresses = info.addresses();
             QHostAddress address = info.addresses().first();
             ResIP = address.toString();
             /*SSH*/
             SSHWorker SSHWorkerObj(ResIP,ControllerDomainPolicy::_Username_str,ControllerDomainPolicy::_Password_str);
             QString ResCommand = "for  x in  `sudo find  /sys/bus/usb/devices/  -name  \"[1-9]-[1-9]\"  -print`; do SER=$(sudo udevadm info $x | grep -Po  '(?<=ID_SERIAL_SHORT=).*'); if [ \"$SER\" == '' ]; then continue; fi; echo \"$x|$SER\";  done;";
             qWarning()<<"SSH start command."<<ResCommand;

             if(SSH_action(SSHWorkerObj, ResCommand)){
                 QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
                 SSHWorkerObjlst.removeAll(QString(""));
                 qWarning()<<"SSH result:"<<SSHWorkerObjlst;
                 cntObj = 0;
                 for (auto& ObjectSshlst: SSHWorkerObjlst) {
                    QStringList drive = ObjectSshlst.split("|");
                   if(drive.size() >= 2 && drive[1].length() > 0){
                       driveNode.clear();
                       driveNode.insert("drive", drive[0]);
                       driveNode.insert("serial",drive[1]);
                       resourceNode.insert(QString::number(cntObj).toUtf8(), driveNode);
                       cntObj++;
                    }
                   map.insert("data", resourceNode);
                   map.insert("state",  0);
                 }
               }
               else{
                 map.insert("state",  1);
                 resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                 map.insert("data", resourceNode);
               }
               map.insert("version", "1.1.1.1");
               map.insert("object", 40);
               map.insert("host_name", ResHost);
               map.insert("host_ip",  ResIP);
               map.insert("action", "");
               QJsonObject json = QJsonObject::fromVariantMap(map);
               QJsonDocument JsonDocument_o(json);
               //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
               if (!JsonDocument_o.isNull()) {
                 response.setHeader("Content-Type", "text/html; charset=UTF-8");
                 response.write(JsonDocument_o.toJson());
               }
           }



      }


  }


  void  ControllerDomainPolicy::SetSettings_sv(QSettings & Settings_ro)
  {
    QString hh = Settings_ro.value("none").toString();
    SetSshParams(Settings_ro.value("username").toString(), Settings_ro.value("password").toString());
  }

  void  ControllerDomainPolicy::InitializeDataBase_sv()
  {

  }

  void  ControllerDomainPolicy::SetSshParams(const QString _Username_, const QString _Password_){
     ControllerDomainPolicy::_Username_str = _Username_;
     ControllerDomainPolicy::_Password_str = _Password_;
  }

  bool ControllerDomainPolicy::SSH_action(SSHWorker& SSHWorkerObj, QString& mCOmmand){
    qWarning()<<"SSH start command.";
    try{
      if(SSHWorkerObj.ExecCommandOnHost_b(QString(mCOmmand + " 2>&1")))
        return true;
      else
        return false;
    }
    catch(OSDSException & e) {
      qWarning()<<"SSH ERROR = "<< e.what();
      return false;
    }
  }
}
