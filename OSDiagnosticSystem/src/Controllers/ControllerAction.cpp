#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHostInfo>
#include <QFileInfo>
#include <QUuid>



#include "ControllerAction.h"
#include "httpsessionstore.h"
#include "osdsexception.h"
#include "osdsapi.h"
#include "validation.h"
#include "databaseworker.h"
#include "controllerprotectedfolder.h"
#include "auditprocess.h"

const QString rules_udev = "# Skeep not USB\r\n\
SUBSYSTEM!=\"usb\", SUBSYSTEM!=\"block\", GOTO=\"usb_end\"\r\n\
# Skeep remove actions\r\n\
ACTION!=\"add\", GOTO=\"usb_end\"\r\n\
# Linux Foundation\r\n\
# 2.0 root hub\r\n\
SUBSYSTEMS==\"usb\", ACTION==\"add\", ATTR{product}==\"EHCI Host Controller\", ATTR{bDeviceClass}==\"09\", GOTO=\"usb_end\"\r\n\
# 1.1 root hub\r\n\
SUBSYSTEMS==\"usb\", ACTION==\"add\", ATTR{product}==\"UHCI Host Controller\", ATTR{bDeviceClass}==\"09\", GOTO=\"usb_end\"\r\n\
# Hub\r\n\
SUBSYSTEMS==\"usb\", ACTION==\"add\", ATTR{product}==\"USB Hub\", ATTR{bDeviceClass}==\"09\"  GOTO=\"usb_end\"\r\n\
SUBSYSTEMS==\"usb\", ACTION==\"add\", ATTR{product}==\"USB Optical Mouse|USB Keyboard\", GOTO=\"usb_end\"\r\n\
# Rules\r\n\
# Disable all other USB devices\r\n\
ACTION==\"add\", SUBSYSTEMS==\"usb\", RUN+=\"/bin/sh /opt/asb/scripts/usb_unautorized.sh\"\r\n\
LABEL=\"usb_end\"\r\n";


using namespace stefanfrings;

namespace osds {
  QString ControllerAction::_Username_str = "";
  QString  ControllerAction::_Password_str = "";


  QObject * CallbackCreateControllerAction_po()
  {
    return new ControllerAction;
  }

  ControllerAction::ControllerAction(QObject * parent) : ControllerAbstract (parent)
  {
    _ControllerName_str = "ControllerAction";
  }

  void ControllerAction::service(HttpRequest& request, HttpResponse& response)
  {
      QByteArray Path_o = request.getPath();      
      if(request.getParameterMap().contains("resourcebyhost"))
      {
          QString ResDir  = "/";
          QString ResHost;
          QString ResIP;
          QString ResCommand;
          QString mOpt = "";
          int CntRule = 0;
          int cntObj  = 0;
          /*--------------------------resources-------------------------*/
           if (request.getParameter("resourcebyhost") == "resourcelist"){
              QVariantMap map;
              QVariantMap resourceNode;              
              /**/
             if(request.getParameterMap().contains("resourcehost"))
               ResHost = request.getParameter("resourcehost");

             QHostInfo info = QHostInfo::fromName(ResHost);
             const auto addresses = info.addresses();
             QHostAddress address = info.addresses().first();
             ResIP = address.toString();

             if(request.getParameterMap().contains("resourcepath"))
               ResDir = request.getParameter("resourcepath");
             if(request.getParameterMap().contains("resourcetype")){
                 if(request.getParameter("resourcetype") == "dir")
                     mOpt = " | grep drw ";
             }
             //QFileInfo finfo(ResDir);
             //if(!finfo.isDir()) return;
             /*SSH*/
             SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
             ResCommand = "sudo ls -Lla -d '" + ResDir + "'* " + mOpt + " | awk '{out=\"\"; for(i=9;i<=NF;i++){out=out\" \"$i}; print $1\":\"out\":\"$3\":\"$4\"\"}'";
             qWarning()<<"SSH start command.";
             if(SSH_action(SSHWorkerObj, ResCommand)){
                 QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
                 SSHWorkerObjlst.removeAll(QString(""));                 

                 QVariantMap mapZero;
                 mapZero.insert("res_base", "/");
                 mapZero.insert("res_hame", "..");
                 mapZero.insert("res_attr", "/");
                 mapZero.insert("res_owner", "root");
                 mapZero.insert("res_group", "root");
                 mapZero.insert("res_type", "d");
                 resourceNode.insert("-1",mapZero);

                 for (auto& ObjectSshlst: SSHWorkerObjlst) {
                   //qWarning()<<ObjectSshlst;
                   QStringList Temp_lst = ObjectSshlst.split(":");
                   /*attribute + name resource*/
                   if(Temp_lst.size() >= 2) {
                       QVariantMap mapNode;
                       QString field = Temp_lst[1];
                       //field = field.left(field.lastIndexOf(" "));
                       mapNode.insert("res_base", field.toUtf8());
                       QString nameOnly = field.toUtf8();
                       nameOnly =  nameOnly.mid(nameOnly.lastIndexOf("/")+1);
                       mapNode.insert("res_hame", nameOnly.toUtf8());
                       mapNode.insert("res_attr", Temp_lst[0]);
                       mapNode.insert("res_owner", Temp_lst[2]);
                       mapNode.insert("res_group", Temp_lst[3]);
                       QString psrType = Temp_lst[0];
                       if(psrType[0] == 'd')
                         mapNode.insert("res_type", "d");
                       else
                         mapNode.insert("res_type", "f");
                       resourceNode.insert(QString::number(cntObj).toUtf8(), mapNode);
                       cntObj++;
                   }
                   map.insert("data", resourceNode);
                 }
               }
               else{
                 resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                 map.insert("data", resourceNode);
               }
             //if(cntObj || QDir(ResDir).exists()){
               map.insert("version", "1.1.1.1");
               map.insert("object", 1);
               map.insert("state",  0);
               map.insert("host_name", ResHost);
               map.insert("host_ip",  ResIP);
               map.insert("action", ResDir);

               QJsonObject json = QJsonObject::fromVariantMap(map);
               QJsonDocument JsonDocument_o(json);
               //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
               if (!JsonDocument_o.isNull()) {
                 response.setHeader("Content-Type", "text/html; charset=UTF-8");
                 response.write(JsonDocument_o.toJson());
               }
             //}
           }

           /*-------------------audit read----------------------------------*/         
                    if (request.getParameter("resourcebyhost") == "auditlist"){
                       QVariantMap map;
                       QVariantMap resourceNode;

                       /**/
                      if(request.getParameterMap().contains("resourcehost"))
                        ResHost = request.getParameter("resourcehost");

                      QHostInfo info = QHostInfo::fromName(ResHost);
                      const auto addresses = info.addresses();
                      QHostAddress address = info.addresses().first();
                      ResIP = address.toString();

                      if(request.getParameterMap().contains("resourcepath"))
                        ResDir = request.getParameter("resourcepath");

                      /*SSH*/
                      if(ResDir == "/"){
                        /*
                        AuditTask mTask;
                        QString RAudit = "-Rps";
                        if(ResDir == "/")
                          RAudit = "-Rps";
                        else
                          RAudit = "-ps";
                        QString Command = "sudo timeout 60 sudo getfaud " + RAudit + " '" + ResDir + "'";

                        mTask.UID = QString(SequentialGUID::GetSequentialGUID(SequentialGUID::SequentialAsString).toString());
                        mTask.AuditHost = ResHost;
                        mTask.AuditPath = ResDir;
                        mTask.SSHHost   = ResIP;
                        mTask.SSHUser   = ControllerAction::_Username_str;
                        mTask.SSHPass   = ControllerAction::_Password_str;
                        mTask.Command   = Command;
                        mTask.State     = -1;

                        qWarning()<<"AuditTask move to thread..."<<ResHost<<" "<<ResDir;;
                        QThreadPool::globalInstance()->start(new AuditProcess(mTask));
                        qWarning()<<"AuditTask move to thread complete..."<<ResHost<<" "<<ResDir;
                        */
                      }

                      SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
                      QString RecAudit = "-Rps";
                      if(ResDir == "/")
                        RecAudit = "-Rps";
                      else
                        RecAudit = "-ps";
                      ResCommand = "sudo timeout 30 sudo getfaud " + RecAudit + " '" + ResDir + "'";
                      //qWarning()<<"SSH start command."<<ResCommand<<endl;
                      int NextRule = 0;
                      int PosRule  = 0;
                      CntRule  = 0;
                      QString Temp_obj;
                      QString Temp_rule;
                      if(SSH_action(SSHWorkerObj, ResCommand)){
                          QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
                          //qWarning()<<"Result."<<SSHWorkerObjlst<<endl;
                          for (auto& ObjectSshlst: SSHWorkerObjlst) {
                             if(ObjectSshlst == "" && PosRule >= 2){
                                if(PosRule){
                                  QVariantMap mapNodeRule;
                                  QVariantMap mapNode;
                                  mapNode.insert("res_hame", Temp_obj);
                                  mapNode.insert("res_attr", Temp_rule);

                                  QStringList Temp_lst_rules = Temp_rule.split(",");
                                  if(Temp_lst_rules.size() == 0){
                                      Temp_lst_rules.append(Temp_rule);
                                  }
                                  int CntNextInnerRule = 0;
                                  for (auto& ObjectRule: Temp_lst_rules){
                                    QStringList Temp_rules_params = ObjectRule.split(":");
                                    if(Temp_rules_params.size() > 0){
                                        QVariantMap mapNodeNextRule;
                                        QString access_map_sucess, access_map_fail;

                                        if(Temp_rules_params.size() == 3){
                                            mapNodeNextRule.insert("full_rule", ObjectRule);
                                            mapNodeNextRule.insert("subject_rule", Temp_rules_params[0]);
                                            mapNodeNextRule.insert("name_rule", "-");
                                            mapNodeNextRule.insert("success_labels", Temp_rules_params[1]);
                                            mapNodeNextRule.insert("fail_labels", Temp_rules_params[2]);
                                            access_map_sucess = Temp_rules_params[1];
                                            access_map_fail   = Temp_rules_params[2];
                                        }
                                        if(Temp_rules_params.size() == 4){
                                            mapNodeNextRule.insert("full_rule", ObjectRule);
                                            mapNodeNextRule.insert("subject_rule", Temp_rules_params[0]);
                                            mapNodeNextRule.insert("name_rule", Temp_rules_params[1]);
                                            mapNodeNextRule.insert("success_labels", Temp_rules_params[2]);
                                            mapNodeNextRule.insert("fail_labels", Temp_rules_params[3]);
                                            access_map_sucess = Temp_rules_params[2];
                                            access_map_fail   = Temp_rules_params[3];
                                        };
                                        if(Temp_rules_params.size()){
                                          QVariantMap mapNodeNextRuleSucess;
                                          QVariantMap mapNodeNextRuleFail;
                                          for(int r = 0; r < access_map_sucess.length(); r++)
                                            mapNodeNextRuleSucess.insert(QString::number(r).toUtf8(),access_map_sucess.mid(r, 1));
                                          for(int r = 0; r < access_map_fail.length(); r++)
                                            mapNodeNextRuleFail.insert(QString::number(r).toUtf8(),access_map_fail.mid(r, 1));
                                          mapNodeNextRule.insert("map_success",mapNodeNextRuleSucess);
                                          mapNodeNextRule.insert("map_fail",mapNodeNextRuleFail);
                                        }
                                        mapNodeRule.insert(QString::number(CntNextInnerRule).toUtf8(),mapNodeNextRule);
                                        CntNextInnerRule++;
                                    }
                                  }
                                  mapNode.insert("rules", mapNodeRule);
                                  resourceNode.insert(QString::number(CntRule).toUtf8(), mapNode);
                                  map.insert("data", resourceNode);
                                }
                                CntRule++;
                                PosRule = 0;
                                Temp_obj = "";
                                Temp_rule = "";
                                continue;
                             }
                            //qWarning()<<ObjectSshlst;
                            if(PosRule == 0){
                              QStringList Temp_lst = ObjectSshlst.split(": ");
                              if(Temp_lst[0] != "# file"){
                                  Temp_obj = "";
                                  Temp_rule = "";
                                  continue;
                              }
                              if(Temp_lst.size() >= 2)
                                Temp_obj = Temp_lst[1];
                            }
                            if(PosRule == 1){
                              Temp_rule =  ObjectSshlst;
                            }
                            PosRule++;
                          }
                        }
                        else{
                          resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                          map.insert("data", resourceNode);
                        }
                      //if(CntRule){
                        map.insert("version", "1.1.1.1");
                        map.insert("object", 2);
                        map.insert("state",  0);
                        map.insert("host_name", ResHost);
                        map.insert("host_ip",  ResIP);
                        map.insert("action", ResDir);

                        QJsonObject json = QJsonObject::fromVariantMap(map);
                        QJsonDocument JsonDocument_o(json);
                        //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
                        if (!JsonDocument_o.isNull()) {
                          response.setHeader("Content-Type", "text/html; charset=UTF-8");
                          response.write(JsonDocument_o.toJson());
                        }
                      //}
                    }
           /*-------------------audit write---------------------------------*/
           if (request.getParameter("resourcebyhost") == "auditmodify"){
              QVariantMap map;
              QVariantMap resourceNode;                           
              QString     AuditRule;
              QString     ResRec = "";
              /**/
             if(request.getParameterMap().contains("resourcehost"))
               ResHost = request.getParameter("resourcehost");
             QHostInfo info = QHostInfo::fromName(ResHost);
             const auto addresses = info.addresses();
             QHostAddress address = info.addresses().first();
             ResIP = address.toString();
             if(request.getParameterMap().contains("resourcepath"))
               ResDir = request.getParameter("resourcepath");
             else{
               /*no resource audit*/
               qWarning()<<"SSH no resource found.";
               return;
             }
            if(request.getParameterMap().contains("auditrule"))
               AuditRule = request.getParameter("auditrule"); 
            else{
              /*no resource audit*/
              qWarning()<<"SSH no rule found.";
              return;
            }

            /*
            QFileInfo finfo(ResDir);
            if(finfo.isDir()) {
              if(request.getParameterMap().contains("attrrec"))
                ResRec = "-R";
              else
                ResRec = "";
            }
            */
            if(request.getParameterMap().contains("attrrec"))
              ResRec = "-R";
            else
              ResRec = "";
             /*SSH*/
             SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
             qWarning()<<"SSH start command.";
             if(AuditRule != "-")  // no delete rule
               ResCommand = "sudo setfaud " + ResRec + " -m " + AuditRule + " '" + ResDir +  "'";
             else
               ResCommand = "sudo setfaud " + ResRec + " -b '" + ResDir + "'";
               if(SSH_action(SSHWorkerObj,ResCommand)) {
                 resourceNode.insert("update", "OK");
                 map.insert("data", resourceNode);
               }
               else{
                 resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                 map.insert("data", resourceNode);
               }
            // if(CntRule){
               map.insert("version", "1.1.1.1");
               map.insert("object", 2);
               map.insert("state",  1);
               map.insert("host_name", ResHost);
               map.insert("host_ip",  ResIP);
               map.insert("action", ResDir);
               QJsonObject json = QJsonObject::fromVariantMap(map);
               QJsonDocument JsonDocument_o(json);
               //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
               if (!JsonDocument_o.isNull()) {
                 response.setHeader("Content-Type", "text/html; charset=UTF-8");
                 response.write(JsonDocument_o.toJson());
               }
             //}
           }

           /*-------------ACL----------------*/
          if (request.getParameter("resourcebyhost") == "getaclattr"){
             QVariantMap map;
             QString     AuditRule;
             QVariantMap mapNodeAttr;
             QVariantMap resourceNode;
             QVariantMap mapValues;
             QVariantMap mapNode;
                         /**/
             if(request.getParameterMap().contains("resourcehost"))
               ResHost = request.getParameter("resourcehost");

              QHostInfo info = QHostInfo::fromName(ResHost);
              const auto addresses = info.addresses();
              QHostAddress address = info.addresses().first();
              ResIP = address.toString();

              if(request.getParameterMap().contains("resourcepath"))
                ResDir = request.getParameter("resourcepath");
              else{
                /*no resource audit*/
                qWarning()<<"SSH no resource found.";
                return;
              }
               /*SSH*/
                 SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
                 qWarning()<<"SSH start command.";                 
                 QJsonObject jsonDef;
                 int NextRule = 0;
                 int PosRule  = 0;
                 int CntRuleAttr = 0;
                 CntRule  = 0;
                 QString Temp_file;
                 QString Temp_owner;
                 QString Temp_group;
                 QString Temp_flags;
                 ResCommand = "sudo getfacl --no-effective -p " + ResDir;
                 if(SSH_action(SSHWorkerObj, ResCommand)){
                     QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
                     /*DEFAULT*/
                     QVariantMap mapDef;
                     QVariantMap mapDefNode;
                     QVariantMap resourceNodeDefAttr;
                     int cntDef = 0;
                     for (auto& ObjectSshlst: SSHWorkerObjlst) {
                       if(ObjectSshlst == ""){
                         if(cntDef){
                           mapDef.insert("data", mapDefNode);
                         }
                         continue;
                       }
                       QStringList Temp_attr = ObjectSshlst.split(":");
                       resourceNodeDefAttr.clear();
                       if(Temp_attr[0] == "default"){
                         if(Temp_attr.size() > 3){
                           //:default resourceNodeDefAttr.insert("def_obj",  Temp_attr[0]);
                           resourceNodeDefAttr.insert("res_obj",  Temp_attr[1]);
                           resourceNodeDefAttr.insert("res_what", Temp_attr[2]);
                           resourceNodeDefAttr.insert("res_mask", Temp_attr[3]);
                           QString defInc = QString(Temp_attr[0] + "_" + Temp_attr[1] + "_" + Temp_attr[2]).toUtf8();
                           mapDefNode.insert(defInc, resourceNodeDefAttr);
                         }
                       }
                       cntDef++;
                   }
                   if(cntDef){
                     jsonDef = QJsonObject::fromVariantMap(mapDef);
                     //qWarning()<<jsonDef;
                   }
                     /*DEFAULT*/
                   for (auto& ObjectSshlst: SSHWorkerObjlst) {
                     if(ObjectSshlst == ""){
                       if(PosRule){
                           map.insert("version", "1.1.1.1");
                           map.insert("object", 3);
                           map.insert("state",  0);
                           map.insert("host_name", ResHost);
                           map.insert("host_ip",  ResIP);
                           map.insert("action", ResDir);
                           /**/
                           mapNode.insert("res_hame", Temp_file);
                           mapNode.insert("res_owner", Temp_owner);
                           mapNode.insert("res_group", Temp_group);
                           mapNode.insert("res_flags", Temp_flags);
                           mapNode.insert("acl_node",  mapValues);
                           resourceNode.insert("acl_attributes", mapNode);
                           map.insert("data", resourceNode);
                       }
                       CntRule++;
                       PosRule = 0;
                       Temp_file  = "";
                       Temp_owner = "";
                       Temp_group = "";
                       Temp_flags = "";
                       mapValues.clear();
                       mapNode.clear();
                       resourceNode.clear();
                       continue;
                     }
                     QStringList Temp_lst = ObjectSshlst.split(": ");
                     //qWarning()<<ObjectSshlst;
                     if(Temp_lst[0] == "# file"){
                       if(Temp_lst.size() >= 2)
                           Temp_file = Temp_lst[1];
                     }
                     else if(Temp_lst[0] == "# owner"){
                       if(Temp_lst.size() >= 2)
                         Temp_owner = Temp_lst[1];
                     }
                     else if(Temp_lst[0] == "# group"){
                       if(Temp_lst.size() >= 2)
                         Temp_group = Temp_lst[1];
                     }
                     else if(Temp_lst[0] == "# flags"){
                       if(Temp_lst.size() >= 2)
                         Temp_flags = Temp_lst[1];
                     }
                     else{
                         QStringList Temp_attr = ObjectSshlst.split(":");
                         mapNodeAttr.clear();
                         if(Temp_attr[0] == "user"  ||
                            Temp_attr[0] == "group" ||
                            Temp_attr[0] == "other" ||
                            Temp_attr[0] == "mask"){
                             if(Temp_attr.size() == 3){
                               QVariantMap mapNodeAttrDefault;
                               QJsonValue jData = jsonDef.value("data");
                               QJsonObject subtree = jData.toObject();
                               QString defInc = QString("default_" + Temp_attr[0] + "_" + Temp_attr[1]).toUtf8();
                               QJsonValue jDataDef = subtree.value(defInc);
                               QJsonObject subtreeDef = jDataDef.toObject();
                               int DefCnt = 0;
                               foreach(const QString& key, subtreeDef.keys()) {
                                 QJsonValue value = subtreeDef.value(key);
                                 mapNodeAttrDefault.insert(key,value.toString());
                                 DefCnt++;
                                 //qWarning()<< "Key = " << key << ", Value = " << value.toString();
                               }
                               mapNodeAttr.insert("res_obj",  Temp_attr[0]);
                               mapNodeAttr.insert("res_what", Temp_attr[1]);
                               mapNodeAttr.insert("res_mask", Temp_attr[2]);
                               if(DefCnt)
                                 mapNodeAttr.insert("default",  mapNodeAttrDefault);
                               mapValues.insert(QString::number(CntRuleAttr).toUtf8(), mapNodeAttr);
                             }
                          }
                          CntRuleAttr++;
                      }
                      PosRule++;
                   }
                 }
                 else{
                   resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                   map.insert("data", resourceNode);
                 }
               //if(CntRule){
                 QJsonObject json = QJsonObject::fromVariantMap(map);
                 QJsonDocument JsonDocument_o(json);
                 //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
                  if (!JsonDocument_o.isNull()) {
                    response.setHeader("Content-Type", "text/html; charset=UTF-8");
                    response.write(JsonDocument_o.toJson());
                  }
                //}
              }

          /*mandats*/

          if (request.getParameter("resourcebyhost") == "mandatlist"){
             QVariantMap map;
             QVariantMap resourceNode;
             QVariantMap mapNodeCatMain;
             QVariantMap mapNodeLevalsMain;
             QVariantMap mapNodeSessionMain;
             QVariantMap mapNodeCurMain;
             int NextRule = 0;
             int PosRule  = 0;
             /**/
            if(request.getParameterMap().contains("resourcehost"))
              ResHost = request.getParameter("resourcehost");

            QHostInfo info = QHostInfo::fromName(ResHost);
            const auto addresses = info.addresses();
            QHostAddress address = info.addresses().first();
            ResIP = address.toString();

            if(request.getParameterMap().contains("resourcepath"))
              ResDir = request.getParameter("resourcepath");
            /*SSH*/
            SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
            CntRule  = 0;
            QString ResCommandMndRules = "sudo pdp-ls -d  -Mna '" + ResDir + "' | awk '{print $6\"|\"$5}'";
            QString ResCommandMndConfLevals = "sudo userlev";
            QString ResCommandMndConCateg   = "sudo usercat";
            QString ResCommandMndSession     = "sudo pdpl-user '" + ControllerAction::_Username_str + "'";
            QString  ResCommandMndSessionCur = "sudo pdp-id '" + ControllerAction::_Username_str + "'";
            //qWarning()<<"SSH start command.";
            if(SSH_action(SSHWorkerObj, ResCommandMndRules)){
              QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
              SSHWorkerObjlst.removeAll(QString(""));
              for (auto& ObjectSshlst: SSHWorkerObjlst) {
                QStringList Temp_lst = ObjectSshlst.split("|");
                //qWarning()<<Temp_lst;
                if(Temp_lst.size() >= 2){
                  QVariantMap mapNode;
                  mapNode.insert("res_hame",   Temp_lst[0]);
                  mapNode.insert("res_mandat", Temp_lst[1]);
                  QStringList Temp_mac = Temp_lst[1].split(":");
                  if(Temp_mac.size() >= 4){
                    mapNode.insert("res_mandat_1", Temp_mac[0]);
                    mapNode.insert("res_mandat_2", Temp_mac[1]);
                    mapNode.insert("res_mandat_3", Temp_mac[2]);
                    mapNode.insert("res_mandat_4", Temp_mac[3]);
                  }
                  resourceNode.insert("mandat_rules", mapNode);
                  map.insert("data", resourceNode);
                  //qWarning()<<ObjectSshlst;
                  PosRule++;
                }
              }
            }

            if(SSH_action(SSHWorkerObj, ResCommandMndConCateg)){
              QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
              SSHWorkerObjlst.removeAll(QString(""));
              int CatConfidencial = 0;
              for (auto& ObjectSshlst: SSHWorkerObjlst) {
                QStringList Temp_lst = ObjectSshlst.split(" ");
                QVariantMap mapNode;
                if(Temp_lst.size() >= 2){
                  mapNode.insert("res_cat_id",   Temp_lst[0]);
                  mapNode.insert("res_cat_name", Temp_lst[1]);
                  mapNodeCatMain.insert(QString::number(CatConfidencial).toUtf8(),  mapNode);
                }
                resourceNode.insert("res_cat", mapNodeCatMain);
                map.insert("data", resourceNode);
                CatConfidencial++;
              }
            }

            if(SSH_action(SSHWorkerObj,  ResCommandMndSessionCur)){
              QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
              SSHWorkerObjlst.removeAll(QString(""));
              int SesCyrConfidencial = 0;
              for (auto& ObjectSshlst: SSHWorkerObjlst) {
                QStringList Temp_lst = ObjectSshlst.split(", ");
                QVariantMap mapNode;
                if(Temp_lst.size() >= 3){
                  mapNode.insert("res_cat_0", Temp_lst[0]);
                  mapNode.insert("res_cat_1", Temp_lst[1]);
                  mapNode.insert("res_cat_2", Temp_lst[2]);
                  mapNodeCurMain.insert(QString::number(SesCyrConfidencial).toUtf8(),  mapNode);
                }
                resourceNode.insert("session_cur", mapNodeCurMain);
                map.insert("data", resourceNode);
                SesCyrConfidencial++;
              }
            }

            if(SSH_action(SSHWorkerObj, ResCommandMndConfLevals)){
              QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
              SSHWorkerObjlst.removeAll(QString(""));
              int LevalConfidencial = 0;
              for (auto& ObjectSshlst: SSHWorkerObjlst) {
                QStringList Temp_lst = ObjectSshlst.split("  ");
                QVariantMap mapNode;
                if(Temp_lst.size() >= 2){
                  mapNode.insert("res_lev_id",   Temp_lst[0]);
                  mapNode.insert("res_lev_name", Temp_lst[1]);
                  mapNodeLevalsMain.insert(QString::number(LevalConfidencial).toUtf8(),  mapNode);
                }
                resourceNode.insert("res_lev", mapNodeLevalsMain);
                map.insert("data", resourceNode);
                LevalConfidencial++;
              }
            }

            if(SSH_action(SSHWorkerObj, ResCommandMndSession)){
              QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
              SSHWorkerObjlst.removeAll(QString(""));
              QString minLabel, maxLabel;
              int SesConfidencial = 0;
              for (auto& ObjectSshlst: SSHWorkerObjlst) {
                QStringList Temp_lst;
                QStringList Temp_lst_params;
                if(SesConfidencial ==  0 || SesConfidencial == 2)
                    Temp_lst = ObjectSshlst.split(": ");
                if(SesConfidencial ==  1 || SesConfidencial == 3)
                    Temp_lst = ObjectSshlst.split("   ");

                QVariantMap mapNode;
                if( Temp_lst.size() >= 1){
                  if(SesConfidencial == 0){
                    mapNode.clear();
                    minLabel = Temp_lst[1];
                  }
                  else if(SesConfidencial == 1){
                    mapNode.insert("res_ses_min_lab",  minLabel);
                    mapNode.insert("res_ses_min_data", Temp_lst[1]);
                    mapNodeSessionMain.insert("min_label",  mapNode);
                  }
                  else if(SesConfidencial == 2){
                    mapNode.clear();
                    maxLabel = Temp_lst[1];
                  }
                  else if(SesConfidencial == 3){
                    mapNode.insert("res_ses_max_lab",  maxLabel);
                    mapNode.insert("res_ses_max_data", Temp_lst[1]);
                    mapNodeSessionMain.insert("max_label",  mapNode);
                    mapNodeSessionMain.insert("user_session",  ControllerAction::_Username_str.toUtf8());
                    resourceNode.insert("res_session", mapNodeSessionMain);

                  }
                }
                map.insert("data", resourceNode);
                SesConfidencial++;
              }
            }

            if(PosRule){
              map.insert("version", "1.1.1.1");
              map.insert("object", 4);
              map.insert("state",  0);
              map.insert("host_name", ResHost);
              map.insert("host_ip",  ResIP);
              map.insert("action", ResDir);

              QJsonObject json = QJsonObject::fromVariantMap(map);
              QJsonDocument JsonDocument_o(json);
              //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
              if (!JsonDocument_o.isNull()) {
                response.setHeader("Content-Type", "text/html; charset=UTF-8");
                response.write(JsonDocument_o.toJson());
              }
            }
          }


          /*-------------------------resourcebyhost=setdiscret-------------------------*/
           if (request.getParameter("resourcebyhost") == "setdiscrete"){
              QVariantMap map;
              QVariantMap resourceNode;
              QString ResDiscreteVal = "";
              QString ResChownRule = "";
              QString ResRec = "";
              QString ChownUserVal="";
              QString ChownGroupVal="";
              /**/
             if(request.getParameterMap().contains("resourcehost"))
               ResHost = request.getParameter("resourcehost");

             QHostInfo info = QHostInfo::fromName(ResHost);
             const auto addresses = info.addresses();
             QHostAddress address = info.addresses().first();
             ResIP = address.toString();

             if(request.getParameterMap().contains("resourcepath"))
               ResDir = request.getParameter("resourcepath");
             else
               return;
             if(request.getParameterMap().contains("attrvalue"))
               ResDiscreteVal = request.getParameter("attrvalue");             

             /*QFileInfo finfo(ResDir);
             if(finfo.isDir()) {
               if(request.getParameterMap().contains("attrrec"))
                 ResRec = request.getParameter("attrrec");
             }*/
             if(request.getParameterMap().contains("attrrec"))
               ResRec = request.getParameter("attrrec");
             bool state_ssh = true;

             if(request.getParameterMap().contains("chown_owner")  &&
                request.getParameterMap().contains("chown_group")){
                 ChownUserVal = request.getParameter("chown_owner");
                 ChownGroupVal = request.getParameter("chown_group");
                 ResChownRule = "sudo chown "  + ResRec + "  \"" +  ChownUserVal + ":" + ChownGroupVal + "\" \"" + ResDir + "\"; ";
              }

             QString Command_bits_update = "";
             if(request.getParameterMap().contains("s_bit_suid")){
               int s_bit = QString(request.getParameter("s_bit_suid")).toInt();
               QString Command_s_bit;
               if(s_bit == 1)
                 Command_s_bit  = "sudo chmod " + ResRec + " u+s '" + ResDir + "'";
               else
                 Command_s_bit  = "sudo chmod " + ResRec + " u-s '" + ResDir + "'";
               Command_bits_update += Command_s_bit + "; ";
             }
             if(request.getParameterMap().contains("s_bit_sguid")){
               int s_bit = QString(request.getParameter("s_bit_sguid")).toInt();
               QString Command_s_bit;
               if(s_bit == 1)
                 Command_s_bit  = "sudo chmod " + ResRec + " g+s '" + ResDir + "'";
               else
                 Command_s_bit  = "sudo chmod " + ResRec + " g-s '" + ResDir + "'";
               Command_bits_update += Command_s_bit  + "; ";
             }
             if(request.getParameterMap().contains("s_bit_sticky")){
               int s_bit = QString(request.getParameter("s_bit_sticky")).toInt();
               QString Command_s_bit;
               if(s_bit == 1)
                 Command_s_bit  = "sudo chmod " + ResRec + " +t '" + ResDir + "'";
               else
                 Command_s_bit  = "sudo chmod " + ResRec + " -t '" + ResDir + "'";
               Command_bits_update += Command_s_bit + "; ";
             }

             /*SSH*/
             SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
             if(!ControllerAction::CheckSSHDiscreteRules(SSHWorkerObj, ResIP, ResDir, ResRec, ResDiscreteVal, ResChownRule, Command_bits_update)){
               state_ssh = false;
             }

             if(state_ssh){
               resourceNode.insert("update", "OK");
               map.insert("data", resourceNode);
             }
             else{                 
                 resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                 map.insert("data", resourceNode);
             }
             map.insert("version", "1.1.1.1");
             map.insert("object", 5);
             map.insert("state",  1);
             map.insert("host_name", ResHost);
             map.insert("host_ip",   ResIP);
             map.insert("action",    ResDir);

             QJsonObject json = QJsonObject::fromVariantMap(map);
             QJsonDocument JsonDocument_o(json);
             //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
             if (!JsonDocument_o.isNull()) {
               response.setHeader("Content-Type", "text/html; charset=UTF-8");
               response.write(JsonDocument_o.toJson());
             }
           }

           /*Audit Journale*/

           if (request.getParameter("resourcebyhost") == "getauditjournal"){
              QVariantMap map;
              QVariantMap resourceNode;
              /**/
             if(request.getParameterMap().contains("resourcehost"))
               ResHost = request.getParameter("resourcehost");
             if(request.getParameterMap().contains("resourcepath"))
               ResDir = request.getParameter("resourcepath");

             QHostInfo info = QHostInfo::fromName(ResHost);
             const auto addresses = info.addresses();
             QHostAddress address = info.addresses().first();
             ResIP = address.toString();
             /*SSH*/
             SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
             if(!request.getParameterMap().contains("resourcepath"))
               ResCommand = "sudo kernlog";
             else
               ResCommand = "sudo kernlog | grep '" + ResDir + "'";
             qWarning()<<"SSH start command...";
             if(SSH_action(SSHWorkerObj, ResCommand)){
                 QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
                 SSHWorkerObjlst.removeAll(QString(""));
                 cntObj = 0;
                 for (auto& ObjectSshlst: SSHWorkerObjlst) {
                   resourceNode.insert(QString::number(cntObj).toUtf8(), ObjectSshlst);
                   /*
                   QString s(ObjectSshlst);
                   QStringList sl = s.split(QRegularExpression("(.*?\'|\<.*?\:)"));
                   qWarning()<<sl;
                   */;
                   map.insert("data", resourceNode);
                   cntObj++;
                 }
               }
               else{
                 resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                 map.insert("data", resourceNode);
               }
             //if(cntObj){
               map.insert("version", "1.1.1.1");
               map.insert("object", 12);
               map.insert("state",  0);
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
             //}
           }

           /*--------------------------LOCAL USERS-------------------------*/
           //filter users
           if (request.getParameter("resourcebyhost") == "localonlyuserlist"){
               QVariantMap map;
               QVariantMap resourceNode;
               if(request.getParameterMap().contains("resourcehost"))
                 ResHost = request.getParameter("resourcehost");
               QHostInfo info = QHostInfo::fromName(ResHost);
               const auto addresses = info.addresses();
               QHostAddress address = info.addresses().first();
               ResIP = address.toString();
              /*SSH*/
              SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
              ResCommand = "sudo getent passwd | awk -F: '{print $1\":\"$3}' | sort";
              qWarning()<<"SSH start command."<<ResCommand;
              if(SSH_action(SSHWorkerObj, ResCommand)){
                  QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
                  SSHWorkerObjlst.removeAll(QString(""));
                  cntObj = 0;
                  for (auto& ObjectSshlst: SSHWorkerObjlst){
                    QStringList Temp_lst = ObjectSshlst.split(":");
                    if(request.getParameterMap().contains("domainfilter")){
                      if(!Temp_lst[1].isEmpty() && Temp_lst[1].toInt() < 2500){
                        resourceNode.insert(QString::number(cntObj).toUtf8(), Temp_lst[0].toUtf8());
                        map.insert("data", resourceNode);
                        cntObj++;
                      }
                    }
                    else{
                      if(!Temp_lst[1].isEmpty()){
                        resourceNode.insert(QString::number(cntObj).toUtf8(), Temp_lst[0].toUtf8());
                        map.insert("data", resourceNode);
                        cntObj++;
                      }
                    }
                  }
                }
                else{
                  map.insert("state",  -1);
                  resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                  map.insert("data", resourceNode);
                }
              //if(cntObj){
                map.insert("version", "1.1.1.1");
                map.insert("object", 14);
                map.insert("state",  0);
                map.insert("action", "");
                QJsonObject json = QJsonObject::fromVariantMap(map);
                QJsonDocument JsonDocument_o(json);
                //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
                if (!JsonDocument_o.isNull()) {
                  response.setHeader("Content-Type", "text/html; charset=UTF-8");
                  response.write(JsonDocument_o.toJson());
                }
              //}
            }
           //filter groups
           if (request.getParameter("resourcebyhost") == "localonlygrouplist"){
               QVariantMap map;
               QVariantMap resourceNode;
               if(request.getParameterMap().contains("resourcehost"))
                 ResHost = request.getParameter("resourcehost");

               QHostInfo info = QHostInfo::fromName(ResHost);
               const auto addresses = info.addresses();
               QHostAddress address = info.addresses().first();
               ResIP = address.toString();
               /**/
              SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
              ResCommand = "sudo getent group | awk -F: '{print $1\":\"$3}'|sort";
              qWarning()<<"SSH start command."<<ResCommand;
              if(SSH_action(SSHWorkerObj, ResCommand)){
                  QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
                  SSHWorkerObjlst.removeAll(QString(""));
                  cntObj = 0;
                  for (auto& ObjectSshlst: SSHWorkerObjlst){
                    QStringList Temp_lst = ObjectSshlst.split(":");
                    if(!Temp_lst[1].isEmpty() && Temp_lst[1].toInt() < 2500){
                      resourceNode.insert(QString::number(cntObj).toUtf8(), Temp_lst[0].toUtf8());
                      map.insert("data", resourceNode);
                      cntObj++;
                    }
                  }
                }
                else{
                  map.insert("state",  -1);
                  resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                  map.insert("data", resourceNode);
                }
              //if(cntObj){
                map.insert("version", "1.1.1.1");
                map.insert("object", 15);
                map.insert("state",  0);
                map.insert("action", "");
                QJsonObject json = QJsonObject::fromVariantMap(map);
                QJsonDocument JsonDocument_o(json);
                //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
                if (!JsonDocument_o.isNull()) {
                  response.setHeader("Content-Type", "text/html; charset=UTF-8");
                  response.write(JsonDocument_o.toJson());
                }
              //}
            }

           //no filter users and groups
            if (request.getParameter("resourcebyhost") == "localuserlist"){
               QVariantMap map;
               QVariantMap resourceNode;
               /**/
              if(request.getParameterMap().contains("resourcehost"))
                ResHost = request.getParameter("resourcehost");

              QHostInfo info = QHostInfo::fromName(ResHost);
              const auto addresses = info.addresses();
              QHostAddress address = info.addresses().first();
              ResIP = address.toString();
              /*SSH*/
              SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
              ResCommand = "sudo cut -d: -f1 /etc/passwd | sort";
              qWarning()<<"SSH start command."<<ResCommand;
              if(SSH_action(SSHWorkerObj, ResCommand)){
                  QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
                  SSHWorkerObjlst.removeAll(QString(""));
                  cntObj = 0;
                  for (auto& ObjectSshlst: SSHWorkerObjlst) {
                    resourceNode.insert(QString::number(cntObj).toUtf8(), ObjectSshlst);
                    map.insert("data", resourceNode);
                    cntObj++;
                  }
                }
                else{
                  resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                  map.insert("data", resourceNode);
                }
              //if(cntObj){
                map.insert("version", "1.1.1.1");
                map.insert("object", 6);
                map.insert("state",  0);
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
              //}
            }
            /*--------------------------LOCAL GROUPS-------------------------*/
             if (request.getParameter("resourcebyhost") == "localgroupslist"){
                QVariantMap map;
                QVariantMap resourceNode;
                /**/
               if(request.getParameterMap().contains("resourcehost"))
                 ResHost = request.getParameter("resourcehost");
               QHostInfo info = QHostInfo::fromName(ResHost);
               const auto addresses = info.addresses();
               QHostAddress address = info.addresses().first();
               ResIP = address.toString();
               /*SSH*/
               SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
               ResCommand = "sudo cut -d: -f1 /etc/group | sort";
               qWarning()<<"SSH start command.";               
               if(SSH_action(SSHWorkerObj, ResCommand)){
                   QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
                   SSHWorkerObjlst.removeAll(QString(""));
                   cntObj = 0;
                   for (auto& ObjectSshlst: SSHWorkerObjlst) {
                     resourceNode.insert(QString::number(cntObj).toUtf8(), ObjectSshlst);
                     map.insert("data", resourceNode);
                     cntObj++;
                   }
                 }
                 else{
                   resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                   map.insert("data", resourceNode);
                 }
               //if(cntObj){
                 map.insert("version", "1.1.1.1");
                 map.insert("object", 7);
                 map.insert("state",  0);
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
               //}
             }

             /*--------------------------ADD ACL-------------------------*/
              if (request.getParameter("resourcebyhost") == "addacllist"){
                 QVariantMap map;
                 QVariantMap resourceNode;
                 QString ResRule;
                 QString TypeRule;
                 QString RuleRec;
                 QString ResRec = "";
                 /**/
                if(request.getParameterMap().contains("resourcehost"))
                  ResHost = request.getParameter("resourcehost");
                if(request.getParameterMap().contains("resourcepath"))
                  ResDir = request.getParameter("resourcepath");
                else
                  return;
                if(request.getParameterMap().contains("resourcerule"))
                  ResRule = request.getParameter("resourcerule");
                else
                  return;

                /*QFileInfo finfo(ResDir);
                if(finfo.isDir()) {
                  if(request.getParameterMap().contains("attrrec"))
                    ResRec = request.getParameter("attrrec");
                }*/
                if(request.getParameterMap().contains("attrrec"))
                  ResRec = request.getParameter("attrrec");
                QHostInfo info = QHostInfo::fromName(ResHost);
                const auto addresses = info.addresses();
                QHostAddress address = info.addresses().first();
                ResIP = address.toString();
                /*SSH*/
                SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
                ResCommand = "sudo setfacl " + ResRec + " -m '" + ResRule + "' '" + ResDir + "'";
                qWarning()<<"SSH start command.";
                if(SSH_action(SSHWorkerObj, ResCommand)){
                  resourceNode.insert("update", "OK");
                  map.insert("data", resourceNode);
                }
                else{
                  resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                  map.insert("data", resourceNode);
                }
                map.insert("version", "1.1.1.1");
                map.insert("object", 8);
                map.insert("state",  1);
                map.insert("host_name", ResHost);
                map.insert("host_ip",  ResIP);
                map.insert("action", ResDir);

                QJsonObject json = QJsonObject::fromVariantMap(map);
                QJsonDocument JsonDocument_o(json);
                //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
                if (!JsonDocument_o.isNull()) {
                  response.setHeader("Content-Type", "text/html; charset=UTF-8");
                  response.write(JsonDocument_o.toJson());
                }
              }
              /*--------------------------DELETE 1 ACL-------------------------*/
               if (request.getParameter("resourcebyhost") == "deleteacllist"){
                  QVariantMap map;
                  QVariantMap resourceNode;
                  QString ResRule;
                  QString TypeRule;
                  QString RuleRec;
                  QString ResRec = "";
                  /**/
                 if(request.getParameterMap().contains("resourcehost"))
                   ResHost = request.getParameter("resourcehost");
                 if(request.getParameterMap().contains("resourcepath"))
                   ResDir = request.getParameter("resourcepath");
                 else
                   return;
                 if(request.getParameterMap().contains("resourcerule"))
                   ResRule = request.getParameter("resourcerule");
                 else
                   return;
                 /*
                 QFileInfo finfo(ResDir);
                 if(finfo.isDir()) {
                   if(request.getParameterMap().contains("attrrec"))
                     ResRec = request.getParameter("attrrec");
                 }*/
                 if(request.getParameterMap().contains("attrrec"))
                   ResRec = request.getParameter("attrrec");
                 QHostInfo info = QHostInfo::fromName(ResHost);
                 const auto addresses = info.addresses();
                 QHostAddress address = info.addresses().first();
                 ResIP = address.toString();
                 /*SSH*/
                 SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
                 ResCommand = "sudo setfacl " + ResRec + " -x '" + ResRule + "' '" + ResDir + "'";
                 qWarning()<<"SSH start command.";
                 if(SSH_action(SSHWorkerObj, ResCommand)){
                   resourceNode.insert("update", "OK");
                   map.insert("data", resourceNode);
                 }
                 else{
                   resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                   map.insert("data", resourceNode);
                 }
                 map.insert("version", "1.1.1.1");
                 map.insert("object", 8);
                 map.insert("state",  1);
                 map.insert("host_name", ResHost);
                 map.insert("host_ip",  ResIP);
                 map.insert("action", ResDir);

                 QJsonObject json = QJsonObject::fromVariantMap(map);
                 QJsonDocument JsonDocument_o(json);
                 //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
                 if (!JsonDocument_o.isNull()) {
                   response.setHeader("Content-Type", "text/html; charset=UTF-8");
                   response.write(JsonDocument_o.toJson());
                 }
               }
               /*--------------------------DELETE 1 ACL-------------------------*/
                if (request.getParameter("resourcebyhost") == "clearacllist"){
                   QVariantMap map;
                   QVariantMap resourceNode;
                   QString ResRule;
                   QString TypeRule;
                   QString RuleRec;
                   QString ResRec = "";
                   /**/
                  if(request.getParameterMap().contains("resourcehost"))
                    ResHost = request.getParameter("resourcehost");
                  if(request.getParameterMap().contains("resourcepath"))
                    ResDir = request.getParameter("resourcepath");
                  else
                    return;
                  /*
                  QFileInfo finfo(ResDir);
                  if(finfo.isDir()) {
                    if(request.getParameterMap().contains("attrrec"))
                      ResRec = request.getParameter("attrrec");
                  }*/
                  if(request.getParameterMap().contains("attrrec"))
                    ResRec = request.getParameter("attrrec");
                  QHostInfo info = QHostInfo::fromName(ResHost);
                  const auto addresses = info.addresses();
                  QHostAddress address = info.addresses().first();
                  ResIP = address.toString();
                  /*SSH*/
                  SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
                  QString ResCommand_k = "sudo setfacl " + ResRec + " -k '" + ResDir + "'";
                  QString ResCommand_b = "sudo setfacl " + ResRec + " -b '" + ResDir + "'";
                  qWarning()<<"SSH start command.";
                  if(SSH_action(SSHWorkerObj, ResCommand_k) && SSH_action(SSHWorkerObj, ResCommand_b)){
                    resourceNode.insert("update", "OK");
                    map.insert("data", resourceNode);
                  }
                  else{
                    resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                    map.insert("data", resourceNode);
                  }
                  map.insert("version", "1.1.1.1");
                  map.insert("object", 9);
                  map.insert("state",  1);
                  map.insert("host_name", ResHost);
                  map.insert("host_ip",  ResIP);
                  map.insert("action", ResDir);

                  QJsonObject json = QJsonObject::fromVariantMap(map);
                  QJsonDocument JsonDocument_o(json);
                  //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
                  if (!JsonDocument_o.isNull()) {
                    response.setHeader("Content-Type", "text/html; charset=UTF-8");
                    response.write(JsonDocument_o.toJson());
                  }
                }

                /*--------------------------MANDAT PDPL_FILE-------------------------*/
                 if (request.getParameter("resourcebyhost") == "setmandatrule"){
                    QVariantMap map;
                    QVariantMap resourceNode;
                    QString ResRuleMandat;
                    QString ResRec = "";
                    /**/
                   if(request.getParameterMap().contains("resourcehost"))
                     ResHost = request.getParameter("resourcehost");
                   if(request.getParameterMap().contains("resourcepath"))
                     ResDir = request.getParameter("resourcepath");
                   else
                     return;
                   if(request.getParameterMap().contains("mandatrule"))
                     ResRuleMandat = request.getParameter("mandatrule");
                   else
                     return;

                   /*QFileInfo finfo(ResDir);
                   if(finfo.isDir()) {
                     if(request.getParameterMap().contains("attrrec"))
                       ResRec = request.getParameter("attrrec");
                    }*/
                   if(request.getParameterMap().contains("attrrec"))
                     ResRec = request.getParameter("attrrec");
                   QHostInfo info = QHostInfo::fromName(ResHost);
                   const auto addresses = info.addresses();
                   QHostAddress address = info.addresses().first();
                   ResIP = address.toString();
                   /*SSH*/
                   SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
                   QString ResCommand_rule = "sudo pdpl-file " + ResRec + " '" + ResRuleMandat + "' '" + ResDir + "'";
                   qWarning()<<"SSH start command pdpl-file."<<ResCommand_rule;
                   if(SSH_action(SSHWorkerObj, ResCommand_rule)){                      
                     QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
                     qWarning()<<"SSH result:"<<SSHWorkerObjlst;
                     qWarning()<<"SSH answer:"<<SSHWorkerObj.GetLastError_str();
                     SSHWorkerObjlst.removeAll(QString(""));
                     if(!SSHWorkerObjlst.isEmpty()){
                       QString labels="";
                       for(auto& message: SSHWorkerObjlst)
                         labels += message;
                       resourceNode.insert("update", labels);
                     }
                     else{
                       resourceNode.insert("update", "OK");
                     }
                     map.insert("data", resourceNode);
                   }
                   else{
                     resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                     map.insert("data", resourceNode);
                   }
                   map.insert("version", "1.1.1.1");
                   map.insert("object", 10);
                   map.insert("state", 1);
                   map.insert("host_name", ResHost);
                   map.insert("host_ip",  ResIP);
                   map.insert("action", ResDir);

                   QJsonObject json = QJsonObject::fromVariantMap(map);
                   QJsonDocument JsonDocument_o(json);
                   //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
                   if (!JsonDocument_o.isNull()) {
                     response.setHeader("Content-Type", "text/html; charset=UTF-8");
                     response.write(JsonDocument_o.toJson());
                   }
                 }

                 /*dev bus list and update*/

                 if (request.getParameter("resourcebyhost") == "devbusdriver"){
                    QVariantMap map;
                    QVariantMap resourceNode;
                    QString ResRuleMandat;
                    QString Dev = "";
                    QString Rule = "";
                    int bus_dev = 0;
                    bool enable_dev = false;
                    /**/
                   if(request.getParameterMap().contains("resourcehost"))
                     ResHost = request.getParameter("resourcehost");
                   if(request.getParameterMap().contains("dev"))
                     Dev = request.getParameter("dev");
                   if(request.getParameterMap().contains("rule"))
                     Rule = request.getParameter("rule");


                   QHostInfo info = QHostInfo::fromName(ResHost);
                   const auto addresses = info.addresses();
                   QHostAddress address = info.addresses().first();
                   ResIP = address.toString();
                   /*SSH*/
                   SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
                   QString ResCommand_rule;

                   if(Rule == "list") ResCommand_rule = "sudo lsmod | grep '" +  Dev +  "'";
                   else if(Rule == "enable")  ResCommand_rule = "sudo modprobe '" +  Dev +  "'";
                   else if(Rule == "disable") ResCommand_rule = "sudo modprobe -r '" +  Dev +  "'";


                   if(SSH_action(SSHWorkerObj, ResCommand_rule)){
                     QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
                     qWarning()<<"SSH result:"<<SSHWorkerObjlst;
                     qWarning()<<"SSH answer:"<<SSHWorkerObj.GetLastError_str();
                     SSHWorkerObjlst.removeAll(QString(""));
                     if((Rule == "disable" || Rule == "enable") && !SSHWorkerObjlst.isEmpty()){
                       QString labels="";
                       for(auto& message: SSHWorkerObjlst)
                         labels += message;
                       resourceNode.insert("update", labels);
                       map.insert("state", 1);
                     }
                     else{
                       if(Rule == "disable" || Rule == "enable"){
                         map.insert("state", 0);
                         resourceNode.insert("update", "OK");
                       }
                       if(Rule == "list"){
                           for(auto& message: SSHWorkerObjlst){
                             QStringList dev_split =  message.split(" ");
                             dev_split.removeAll(QString(""));
                             if(dev_split.size())
                                resourceNode.insert(QString::number(bus_dev).toUtf8(), dev_split[0]);
                             bus_dev++;
                          }
                       }
                     }
                     if(Rule == "list"){
                       if(bus_dev)
                         map.insert("state", "0");
                       else
                         map.insert("state", "1");
                     }
                     map.insert("data", resourceNode);
                   }
                   else{
                     resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                     map.insert("data", resourceNode);
                   }
                   map.insert("version", "1.1.1.1");
                   map.insert("object", 20);
                   map.insert("host_name", ResHost);
                   map.insert("host_ip",  ResIP);
                   map.insert("action", ResDir);

                   QJsonObject json = QJsonObject::fromVariantMap(map);
                   QJsonDocument JsonDocument_o(json);
                   //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
                   if (!JsonDocument_o.isNull()) {
                     response.setHeader("Content-Type", "text/html; charset=UTF-8");
                     response.write(JsonDocument_o.toJson());
                   }
                 }


                 /*list dev group update*/
                 if (request.getParameter("resourcebyhost") == "setusertodev"){
                    QVariantMap map;
                    QVariantMap resourceNode;
                    QString ResDev;
                    QString ResRec = "";
                    QString UserDev;
                    QString RuleGroupDev="";
                    /**/
                   if(request.getParameterMap().contains("resourcehost"))
                     ResHost = request.getParameter("resourcehost");
                   if(request.getParameterMap().contains("user_dev"))
                     UserDev = request.getParameter("user_dev");
                   if(request.getParameterMap().contains("device"))
                     ResDev = request.getParameter("device");
                   if(request.getParameterMap().contains("rule"))
                     RuleGroupDev = request.getParameter("rule");

                   /*QFileInfo finfo(ResDir);
                   if(finfo.isDir()) {
                     if(request.getParameterMap().contains("attrrec"))
                       ResRec = request.getParameter("attrrec");
                    }*/
                   if(request.getParameterMap().contains("attrrec"))
                     ResRec = request.getParameter("attrrec");
                   QHostInfo info = QHostInfo::fromName(ResHost);
                   const auto addresses = info.addresses();
                   QHostAddress address = info.addresses().first();
                   ResIP = address.toString();
                   /*SSH*/
                   SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
                   QString ResCommand_rule = "sudo gpasswd -" + RuleGroupDev + " '" + UserDev + "' '" + ResDev + "'";
                   if(SSH_action(SSHWorkerObj, ResCommand_rule)){
                     QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
                     resourceNode.insert("update", "OK");
                     map.insert("data", resourceNode);
                   }
                   else{
                     resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                     map.insert("data", resourceNode);
                   }
                   map.insert("version", "1.1.1.1");
                   map.insert("object", 15);
                   map.insert("state", 1);
                   map.insert("host_name", ResHost);
                   map.insert("host_ip",  ResIP);
                   map.insert("action", ResDir);

                   QJsonObject json = QJsonObject::fromVariantMap(map);
                   QJsonDocument JsonDocument_o(json);
                   //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
                   if (!JsonDocument_o.isNull()) {
                     response.setHeader("Content-Type", "text/html; charset=UTF-8");
                     response.write(JsonDocument_o.toJson());
                   }
                 }



                 //list users by dev mount
                  if (request.getParameter("resourcebyhost") == "gropdevlist"){
                     QVariantMap map;
                     QVariantMap resourceNode;
                     QString ResDevice;
                     /**/
                    if(request.getParameterMap().contains("resourcehost"))
                      ResHost = request.getParameter("resourcehost");
                    if(request.getParameterMap().contains("device"))
                      ResDevice = request.getParameter("device");

                    QHostInfo info = QHostInfo::fromName(ResHost);
                    const auto addresses = info.addresses();
                    QHostAddress address = info.addresses().first();
                    ResIP = address.toString();
                    /*SSH*/
                    SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
                    ResCommand = "cat  /etc/group | grep '" + ResDevice + "'";
                    qWarning()<<"SSH start command."<<ResCommand;
                    if(SSH_action(SSHWorkerObj, ResCommand)){
                        QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
                        SSHWorkerObjlst.removeAll(QString(""));
                        cntObj = 0;

                        for (auto& ObjectSshlst: SSHWorkerObjlst) {
                           QStringList users_cd = ObjectSshlst.split(":");
                           QStringList users_list;
                          if(users_cd.size() > 3){
                             users_list = users_cd[3].split(",");
                             for (auto& ObjectUsersDev: users_list){
                                 resourceNode.insert(QString::number(cntObj).toUtf8(), ObjectUsersDev.toUtf8());
                                 cntObj++;
                              }
                           }
                          map.insert("data", resourceNode);
                        }
                      }
                      else{
                        resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                        map.insert("data", resourceNode);
                      }
                    //if(cntObj){
                      map.insert("version", "1.1.1.1");
                      map.insert("object", 15);
                      map.insert("state",  0);
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
                    //}
                  }


                  //list users by dev mount
                   if (request.getParameter("resourcebyhost") == "dev_disk_serial"){
                      QVariantMap map;
                      QVariantMap resourceNode;
                      QVariantMap driveNode;
                      /**/
                     if(request.getParameterMap().contains("resourcehost"))
                       ResHost = request.getParameter("resourcehost");

                     QHostInfo info = QHostInfo::fromName(ResHost);
                     const auto addresses = info.addresses();
                     QHostAddress address = info.addresses().first();
                     ResIP = address.toString();
                     /*SSH*/
                     SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);                     

                     ResCommand = "rules=$(sudo cat /etc/udev/rules.d/99-usb.rules); (for  x in  `sudo find  /sys/bus/usb/devices/  -name  \"*\" -print | sudo grep \"[1-99]-[1-99]\"`; do SER=$(sudo udevadm info $x | sudo grep -Po  '(?<=ID_SERIAL_SHORT=).*'); if [ \"$SER\" == '' ]; then continue; fi; grep_tag=$(echo $rules |  grep  $SER);  SER_UDEV=$(echo \"$grep_tag\" | sudo grep -o '{ID_SERIAL_SHORT}==\"[^\"]*\"' |  cut -f2 -d '\"');   o_tag=$(echo $grep_tag | grep -Po \"(?<=OWNER:=).*\" | cut -d ',' -f1 ); g_tag=$(echo $grep_tag | grep -Po \"(?<=GROUP:=).*\" | cut -d ',' -f1 ); m_tag=$(echo $grep_tag | grep -Po \"(?<=MODE:=).*\" | cut -d ',' -f1 ); mode_r=\"\";  if [ \"$o_tag\" != \"\" -o \"$g_tag\" != \"\" -o \"$m_tag\" != \"\" ]; then mode_r=$(echo \"Владелец:$o_tag Группа:$g_tag  Атрибуты:$m_tag\"); fi; if [ \"$SER_UDEV\" != \"\" ];  then mode_r=$mode_r' ID:'$SER_UDEV; fi;   echo $x'|'$SER'|'$mode_r; done)|sort;  dev_rule=$(echo \"$rules\" | sudo grep -o '{ID_SERIAL_SHORT}==\"[^\"]*\"' |  cut -f2 -d '\"'); for xd in $dev_rule; do cnt=0; SER=\"\";   (for  x in  `sudo find  /sys/bus/usb/devices/  -name  \"*\" -print | sudo grep \"[1-99]-[1-99]\"`; do SER=$(sudo udevadm info $x | sudo grep -Po  '(?<=ID_SERIAL_SHORT=).*'); if [ \"$SER\" == $xd ]; then   cnt=1;  fi;  done;  if [  \"$cnt\" == 0 ]; then  echo UNKNOWN'|'$xd'|1'; fi; )  done;";
                     //ResCommand = "rules=$(sudo cat /etc/udev/rules.d/99-usb.rules); (for  x in  `sudo find  /sys/bus/usb/devices/  -name  \"*\" -print | sudo grep \"[1-99]-[1-99]\"`; do SER=$(sudo udevadm info $x | sudo grep -Po  '(?<=ID_SERIAL_SHORT=).*'); if [ \"$SER\" == '' ]; then continue; fi;  echo $x'|'$SER'|'$(echo \"$rules\" | grep  $SER); done)|sort;  dev_rule=$(echo \"$rules\" | sudo grep -o '{ID_SERIAL_SHORT}==\"[^\"]*\"' |  cut -f2 -d '\"'); for xd in $dev_rule; do cnt=0; SER=\"\";   (for  x in  `sudo find  /sys/bus/usb/devices/  -name  \"*\" -print | sudo grep \"[1-99]-[1-99]\"`; do SER=$(sudo udevadm info $x | sudo grep -Po  '(?<=ID_SERIAL_SHORT=).*'); if [ \"$SER\" == $xd ]; then   cnt=1;  fi;  done;  if [  \"$cnt\" == 0 ]; then  echo UNKNOWN'|'$xd'|1'; fi; )  done;";
                     qWarning()<<"SSH start command."<<ResCommand;

                     if(SSH_action(SSHWorkerObj, ResCommand)){
                         QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
                         SSHWorkerObjlst.removeAll(QString(""));
                         qWarning()<<"SSH result:"<<SSHWorkerObjlst;
                         cntObj = 0;
                         for (auto& ObjectSshlst: SSHWorkerObjlst) {
                            QStringList drive = ObjectSshlst.split("|");
                           if(drive.size() >= 3 && drive[1].length() > 0){
                               driveNode.clear();
                               driveNode.insert("drive", drive[0]);
                               driveNode.insert("serial",drive[1]);
                               driveNode.insert("rule",drive[2]);
                               //driveNode.insert("info",drive[3]);
                               resourceNode.insert(QString::number(cntObj).toUtf8(), driveNode);
                               cntObj++;
                            }
                           map.insert("data", resourceNode);
                         }
                       }
                       else{
                         resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                         map.insert("data", resourceNode);
                       }
                     //if(cntObj){
                       map.insert("version", "1.1.1.1");
                       map.insert("object", 21);
                       map.insert("state",  0);
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
                     //}
                   }





                   /*fstab mount*/

                  if (request.getParameter("resourcebyhost") == "fstabpubliclist"){
                              QVariantMap map;
                              QVariantMap resourceNode;
                              QVariantMap driveNode;
                              /**/
                             if(request.getParameterMap().contains("resourcehost"))
                               ResHost = request.getParameter("resourcehost");

                             QHostInfo info = QHostInfo::fromName(ResHost);
                             const auto addresses = info.addresses();
                             QHostAddress address = info.addresses().first();
                             ResIP = address.toString();
                             /*SSH*/
                             SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
                             /*test*/

                             ResCommand = "sudo cat /etc/fstab | grep \"^.\dev/sr[0-9].*\\buser\,\" | grep -o -P '^.*(?<=sr[0-9])'";
                             //qWarning()<<"SSH start command."<<ResCommand;

                             if(SSH_action(SSHWorkerObj, ResCommand)){
                                 QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
                                 SSHWorkerObjlst.removeAll(QString(""));
                                 //qWarning()<<"SSH result:"<<SSHWorkerObjlst;
                                 cntObj = 0;
                                 for (auto& ObjectSshlst: SSHWorkerObjlst) {
                                   resourceNode.insert(QString::number(cntObj).toUtf8(), ObjectSshlst);
                                   map.insert("data", resourceNode);
                                   cntObj++;
                                 }
                               }
                               else{
                                 resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                                 map.insert("data", resourceNode);
                               }
                             //if(cntObj){
                               map.insert("version", "1.1.1.1");
                               map.insert("object", 45);
                               map.insert("state",  0);
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
                             //}
                           }

                   //list users by dev mount
                    if (request.getParameter("resourcebyhost") == "dirlist"){
                       QVariantMap map;
                       QVariantMap resourceNode;
                       QVariantMap driveNode;
                       /**/
                      if(request.getParameterMap().contains("resourcehost"))
                        ResHost = request.getParameter("resourcehost");
                      if(request.getParameterMap().contains("resourcepath"))
                        ResDir = request.getParameter("resourcepath");

                      QHostInfo info = QHostInfo::fromName(ResHost);
                      const auto addresses = info.addresses();
                      QHostAddress address = info.addresses().first();
                      ResIP = address.toString();
                      /*SSH*/
                      SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
                      /*test*/

                      ResCommand = "echo \"..\";sudo ls -Lla -d '" + ResDir + "'/* | grep drw | awk '{out=\"\"; for(i=9;i<=NF;i++){out=out\" \"$i}; print \"\"out\"\"}'";

                      qWarning()<<"SSH start command."<<ResCommand;

                      if(SSH_action(SSHWorkerObj, ResCommand)){
                          QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
                          SSHWorkerObjlst.removeAll(QString(""));
                          //qWarning()<<"SSH result:"<<SSHWorkerObjlst;
                          cntObj = 0;
                          for (auto& ObjectSshlst: SSHWorkerObjlst) {
                            resourceNode.insert(QString::number(cntObj).toUtf8(), ObjectSshlst);
                            map.insert("data", resourceNode);
                            cntObj++;
                          }
                        }
                        else{
                          resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                          map.insert("data", resourceNode);
                        }
                      //if(cntObj){
                        map.insert("version", "1.1.1.1");
                        map.insert("object", 22);
                        map.insert("state",  0);
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
                      //}
                    }


                    //list save path  to clear by host
                     if (request.getParameter("resourcebyhost") == "savelist"){
                        QVariantMap map;
                        QVariantMap resourceNode;
                        QVariantMap driveNode;
                        /**/
                       if(request.getParameterMap().contains("resourcehost"))
                         ResHost = request.getParameter("resourcehost");
                       if(request.getParameterMap().contains("resourcepath"))
                         ResDir = request.getParameter("resourcepath");
                       /*
                       AddHostFolderSave("vm-l174.ex.local", "/home/use/test1");
                       AddHostFolderSave("vm-l174.ex.local", "/home/use/test2");
                       AddHostFolderSave("vm-l174.ex.local", "/home/use/test3");
                       AddHostFolderSaveHistory("vm-l174.ex.local", "/home/use/test1", 0);
                       */

                       QHostInfo info = QHostInfo::fromName(ResHost);
                       const auto addresses = info.addresses();
                       QHostAddress address = info.addresses().first();
                       ResIP = address.toString();
                       int cntRow = 0;
                       try {
                         qWarning()<<"tables db save query...";
                         DatabaseWorker DatabaseWorker_o(this);
                         QSqlQuery Query_lst;
                         DatabaseWorker_o.QueryExec_lstExt(QString("SELECT dir_to_save  FROM  hosts_save WHERE hostname='" +ResHost + "'"), false, Query_lst);
                         Query_lst.first();
                         do{
                             if(!Query_lst.value("dir_to_save").toString().isEmpty()){
                               resourceNode.insert(QString::number(cntRow).toUtf8(), Query_lst.value("dir_to_save").toString());
                               cntRow++;
                             }
                         } while(Query_lst.next());
                         map.insert("data", resourceNode);
                         qWarning()<<"tables db save query end."<<endl;
                       }
                       catch (OSDSException & e) {
                         qWarning()<<"tables db save add errors..."<<endl;
                         qWarning()<<""<<e.what();
                         resourceNode.insert("update",e.what());
                          map.insert("data", resourceNode);
                       }
                       //if(cntObj){
                         map.insert("version", "1.1.1.1");
                         map.insert("object", 24);
                         if(cntRow > 9)
                           map.insert("state",  0);
                         else
                           map.insert("state",  1);
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
                       //}
                     }



                     //list save settings  to clear by host
                      if (request.getParameter("resourcebyhost") == "savesettings"){
                         QVariantMap map;
                         QVariantMap resourceNode;
                         QVariantMap driveNode;
                        int cntRow = 0;
                        try {
                          qWarning()<<"tables db save query...";
                          DatabaseWorker DatabaseWorker_o(this);
                          QSqlQuery Query_lst;

                          DatabaseWorker_o.QueryExec_lstExt(QString("SELECT ftp_server, ftp_user, ftp_pass  FROM  save_server "), false, Query_lst);
                          Query_lst.first();
                          do{
                              if(!Query_lst.value("ftp_server").toString().isEmpty()){
                                resourceNode.insert("ftp_server", Query_lst.value("ftp_server").toString());
                                resourceNode.insert("ftp_user", Query_lst.value("ftp_user").toString());
                                resourceNode.insert("ftp_pass", Query_lst.value("ftp_pass").toString());
                                cntRow++;
                              }
                          } while(Query_lst.next());
                          map.insert("data", resourceNode);
                          qWarning()<<"tables db save query end."<<endl;
                        }
                        catch (OSDSException & e) {
                          qWarning()<<"tables db save add errors..."<<endl;
                          qWarning()<<""<<e.what();
                          resourceNode.insert("update",e.what());
                           map.insert("data", resourceNode);
                        }
                        //if(cntObj){
                          map.insert("version", "1.1.1.1");
                          map.insert("object", 35);
                          if(cntRow > 9)
                            map.insert("state",  0);
                          else
                            map.insert("state",  1);
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
                        //}
                      }


                      if (request.getParameter("resourcebyhost") == "replacesavesettings"){
                         QVariantMap map;
                         QVariantMap resourceNode;
                         QVariantMap driveNode;
                         QString ftp_server="";
                         QString ftp_user="";
                         QString ftp_pass="";

                         if(request.getParameterMap().contains("ftp_server"))
                           ftp_server = request.getParameter("ftp_server");
                         if(request.getParameterMap().contains("ftp_user"))
                           ftp_user = request.getParameter("ftp_user");
                         if(request.getParameterMap().contains("ftp_pass"))
                           ftp_pass = request.getParameter("ftp_pass");

                        int cntRow = 0;
                        try {
                          qWarning()<<"tables db save history add...";
                          DatabaseWorker DatabaseWorker_o(this);
                          QSqlQuery Query_lst;
                          DatabaseWorker_o.QueryExec_lstExt("DELETE FROM save_server", false, Query_lst);
                          if(!DatabaseWorker_o.QueryExec_lstExt(QString("REPLACE INTO save_server (ftp_server, ftp_user, ftp_pass) VALUES('" +ftp_server + "','" + ftp_user + "','" + ftp_pass + "');"), false, Query_lst)){
                            resourceNode.insert("update","ERROR");
                            qWarning()<<"tables db settings save errors..."<<endl;
                          }
                          else{
                            resourceNode.insert("update","OK");
                          }
                           map.insert("data", resourceNode);

                        } catch (OSDSException & e) {
                            qWarning()<<"tables db settings save errors..."<<endl;
                            qWarning()<<""<<e.what();
                            resourceNode.insert("update",e.what());
                            map.insert("data", resourceNode);
                        }
                        //if(cntObj){
                          map.insert("version", "1.1.1.1");
                          map.insert("object", 36);
                          map.insert("state",  0);
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
                        //}
                      }

                     //backupssavelist"
                      if (request.getParameter("resourcebyhost") == "backupssavelist"){
                         QVariantMap map;
                         QVariantMap resourceNode;
                         QVariantMap driveNode;
                         /**/
                        if(request.getParameterMap().contains("resourcehost"))
                          ResHost = request.getParameter("resourcehost");
                        if(request.getParameterMap().contains("resourcepath"))
                          ResDir = request.getParameter("resourcepath");

                        QHostInfo info = QHostInfo::fromName(ResHost);
                        const auto addresses = info.addresses();
                        QHostAddress address = info.addresses().first();
                        ResIP = address.toString();
                        int cntRow = 0;
                        try {
                          //qWarning()<<"tables db save query...";
                          DatabaseWorker DatabaseWorker_o(this);
                          QSqlQuery Query_lst;
                          if(!DatabaseWorker_o.QueryExec_lstExt(QString("SELECT id_operation, uid_process, id_state_process, arc_file, time_start, time_stop  FROM  hosts_save_history WHERE hostname='" +ResHost + "' AND dir_to_save='" + ResDir + "'  ORDER BY id_history DESC"), false, Query_lst)){
                            qWarning()<<"db errors backuplist..."<<endl;
                          }
                          Query_lst.first();
                          do{
                              driveNode.clear();
                              if(!Query_lst.value("uid_process").toString().isEmpty()){
                                driveNode.insert("id_operation", Query_lst.value("id_operation").toString().toUtf8());
                                driveNode.insert("uid_process", Query_lst.value("uid_process").toString().toUtf8());
                                driveNode.insert("id_state_process", Query_lst.value("id_state_process").toString().toUtf8());
                                driveNode.insert("arc_file", Query_lst.value("arc_file").toString().toUtf8());
                                driveNode.insert("time_start", Query_lst.value("time_start").toString().toUtf8());
                                driveNode.insert("time_stop", Query_lst.value("time_stop").toString().toUtf8());

                                resourceNode.insert(QString::number(cntRow).toUtf8(), driveNode);
                                cntRow++;
                              }
                          } while(Query_lst.next());
                          map.insert("data", resourceNode);
                          //qWarning()<<"tables db save query end."<<endl;
                        }
                        catch (OSDSException & e) {
                          qWarning()<<"db errors..."<<endl;
                          qWarning()<<""<<e.what();
                          resourceNode.insert("update",e.what());
                           map.insert("data", resourceNode);
                        }
                        //if(cntObj){
                          map.insert("version", "1.1.1.1");
                          map.insert("object", 34);
                          if(cntRow > 9)
                            map.insert("state",  0);
                          else
                            map.insert("state",  1);
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
                        //}
                      }



                     if (request.getParameter("resourcebyhost") == "savelistmod"){
                        QVariantMap map;
                        QVariantMap resourceNode;
                        QVariantMap driveNode;
                        QString Mode = "";
                        bool recClear = false;
                        bool fail     = false;
                        /**/
                       if(request.getParameterMap().contains("resourcehost"))
                         ResHost = request.getParameter("resourcehost");
                       if(request.getParameterMap().contains("resourcepath"))
                         ResDir = request.getParameter("resourcepath");
                       if(request.getParameterMap().contains("resourcemode"))
                         Mode = request.getParameter("resourcemode");
                       if(request.getParameterMap().contains("recursive"))
                         recClear = true;
                       if(Mode != "d" && Mode != "i" && Mode != "c") return;

                       QHostInfo info = QHostInfo::fromName(ResHost);
                       const auto addresses = info.addresses();
                       QHostAddress address = info.addresses().first();
                       ResIP = address.toString();
                       int cntRow = 0;
                       try {
                         //qWarning()<<"tables db save modify query...";
                         if(Mode == "d"){
                           resourceNode.insert("update", RemoveHostFolderSaveHistory(ResHost, ResDir)?"OK":"ERROR");
                         }
                         if(Mode == "i"){
                           resourceNode.insert("update", AddHostFolderSave(ResHost, ResDir)?"OK":"ERROR");
                         }
                         if(Mode == "c"){              
                           if(recClear){
                               qWarning()<<"tables db save query clear all...";
                               DatabaseWorker DatabaseWorker_o(this);
                               QSqlQuery Query_lst;
                               if(!DatabaseWorker_o.QueryExec_lstExt(QString("SELECT hostname, dir_to_save FROM  hosts_save WHERE hostname='" + ResHost + "'"), false, Query_lst)){
                                 qWarning()<<"db errors backuplist..."<<endl;
                                 resourceNode.insert("update", "ERROR");
                               }
                               else{
                                 Query_lst.first();
                                 do{
                                   if(!Query_lst.value("hostname").toString().isEmpty() &&
                                     !Query_lst.value("dir_to_save").toString().isEmpty()){
                                       if(ClearHostFolderSave(Query_lst.value("hostname").toString().toUtf8(), Query_lst.value("dir_to_save").toString().toUtf8()) != 0){
                                           resourceNode.insert("update", "ERROR");
                                           fail = true;
                                           break;
                                       }
                                    }
                                 } while(Query_lst.next());
                                 if(!fail) resourceNode.insert("update", "OK");
                               }
                           }
                           else{
                             resourceNode.insert("update", (ClearHostFolderSave(ResHost, ResDir) == 0)?"OK":"ERROR");
                           }
                         }

                         resourceNode.insert("update","OK");
                         map.insert("data", resourceNode);
                         //qWarning()<<"tables db save modify end."<<endl;
                       }
                       catch (OSDSException & e) {
                         qWarning()<<"tables db save modify errors..."<<endl;
                         qWarning()<<""<<e.what();
                         resourceNode.insert("update",e.what());
                          map.insert("data", resourceNode);
                       }
                       //if(cntObj){
                         map.insert("version", "1.1.1.1");
                         map.insert("object", 25);
                         map.insert("state",  0);
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
                       //}
                     }




                     if (request.getParameter("resourcebyhost") == "dev_need_touch"){
                        QVariantMap map;
                        QVariantMap resourceNode;
                        QVariantMap driveNode;
                        /**/
                       if(request.getParameterMap().contains("resourcehost"))
                         ResHost = request.getParameter("resourcehost");

                       QHostInfo info = QHostInfo::fromName(ResHost);
                       const auto addresses = info.addresses();
                       QHostAddress address = info.addresses().first();
                       ResIP = address.toString();
                       /*SSH*/
                       SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
                       QString ResCommand = QString("if [[ -e /etc/udev/rules.d/99-usb.rules ]]; then  echo 1; else echo 0; fi;");
                       if(SSH_action(SSHWorkerObj, ResCommand)){
                           QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
                           SSHWorkerObjlst.removeAll(QString(""));
                           qWarning()<<"SSH result:"<<SSHWorkerObjlst;
                           if(SSHWorkerObjlst[0] == "1"){
                             resourceNode.insert("update", "FILE_EXIST");
                           }
                           else
                             resourceNode.insert("update", "FILE_NO_EXIST");                                                                                ;
                           map.insert("data", resourceNode);
                         }
                         else{
                           resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                           map.insert("data", resourceNode);
                         }
                       //if(cntObj){
                         map.insert("version", "1.1.1.1");
                         map.insert("object", 51);
                         map.insert("state",  0);
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
                       //}
                     }

                    if (request.getParameter("resourcebyhost") == "alive_host"){
                      QVariantMap map;
                      QVariantMap resourceNode;
                      QVariantMap driveNode;;
                      int mapNodeCnt = 0;
                      try{
                        DatabaseWorker DatabaseWorker_o(this);
                        QSqlQuery Query_lst;
                        if(!DatabaseWorker_o.QueryExec_lstExt(QString("SELECT hostname, id_state, time_stop FROM hosts_ald_alive"), false, Query_lst)){
                          qWarning()<<"trace alive hosts....db errors..."<<endl;
                           map.insert("state",  1);
                        }
                        else{
                           Query_lst.first();
                           do{
                             QVariantMap mapDev;
                             mapDev.insert("hostname",Query_lst.value("hostname").toString().toUtf8());
                             mapDev.insert("id_state",Query_lst.value("id_state").toString().toUtf8());
                             mapDev.insert("time_stop",Query_lst.value("time_stop").toString().toUtf8());
                             resourceNode.insert(QString::number(mapNodeCnt).toUtf8(), mapDev);
                             mapNodeCnt++;
                          } while(Query_lst.next());
                         if(mapNodeCnt)
                           map.insert("data", resourceNode);
                          map.insert("state",  0);
                       }
                      }
                      catch(OSDSException & e){
                        map.insert("state",  2);
                      }
                      map.insert("version", "1.1.1.1");
                      map.insert("object", 99);
                      map.insert("host_name", "");
                      map.insert("host_ip",  "");
                      map.insert("action", "");
                      QJsonObject json = QJsonObject::fromVariantMap(map);
                      QJsonDocument JsonDocument_o(json);
                      //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
                      if (!JsonDocument_o.isNull()) {
                        response.setHeader("Content-Type", "text/html; charset=UTF-8");
                        response.write(JsonDocument_o.toJson());
                       }
                     }

                     //touch usb rules
                      if (request.getParameter("resourcebyhost") == "fstab_public_touch"){
                         QVariantMap map;
                         QVariantMap resourceNode;
                         QVariantMap driveNode;
                         QString     mState;
                         /**/
                        if(request.getParameterMap().contains("resourcehost"))
                          ResHost = request.getParameter("resourcehost");
                        if(request.getParameterMap().contains("state"))
                          mState = request.getParameter("state");

                        QHostInfo info = QHostInfo::fromName(ResHost);
                        const auto addresses = info.addresses();
                        QHostAddress address = info.addresses().first();
                        ResIP = address.toString();
                        /*SSH*/
                        SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);                                                
                        //scripts files sh
                        QString _nouser = "sudo /bin/sh /opt/asb/scripts/nouser_fstab.sh; sudo /bin/sh /opt/asb/scripts/cd_unmount_remove.sh";
                        QString _user   = "sudo /bin/sh /opt/asb/scripts/user_fstab.sh";

                        //full strings
                        //QString _nouser = "filefstab=\"/etc/fstab\"; while IFS=  read -r line_par; do  res_str=\"$(echo $line_par |  sudo cut -d : -f1)s\";  dev_par=\"$(echo $line_par |  sudo grep -Po \".dev/sr[0-9]\")\"; dev_par=$(echo \"$dev_par\" | sudo sed 's/\//\\\//g'); dev_mnt=\"$(echo $line_par | sudo awk '{print $2}'  )\"; dev_mnt=$(echo \"$dev_mnt\" | sudo sed 's/\//\\\//g'); if [ $dev_par != \"\" ]; then  sudo sed -i \"$res_str/.*/$dev_par        $dev_mnt   udf,iso9660 nouser,noauto     0       0/\" $filefstab; fi;  done <<< $(sudo cat $filefstab | sudo grep -n  \"^.\dev/sr[0-9].*\");";
                        //QString _user="filefstab=\"/etc/fstab\"; while IFS=  read -r line_par; do  res_str=\"$(echo $line_par |  sudo cut -d : -f1)s\";  dev_par=\"$(echo $line_par |  sudo grep -Po \".dev/sr[0-9]\")\"; dev_par=$(echo \"$dev_par\" | sudo sed 's/\//\\\//g'); dev_mnt=\"$(echo $line_par | sudo awk '{print $2}'  )\"; dev_mnt=$(echo \"$dev_mnt\" | sudo sed 's/\//\\\//g'); if [ $dev_par != \"\" ]; then  sudo sed -i \"$res_str/.*/$dev_par        $dev_mnt   udf,iso9660 user,noauto     0       0/\" $filefstab; fi;  done <<< $(sudo cat $filefstab | sudo grep -n  \"^.\dev/sr[0-9].*\");";

                        //QString ResCommand = (mState == "0")?
                        //  "filefstab=\"/etc/fstab\"; while IFS=  read -r line_par; do  sudo sed -i ''$line_par's/user/nouser/' $filefstab;     done <<< $(sudo cat $filefstab | sudo grep -n  \"^.\dev/sr[0-9].*\\buser.*\" | sudo cut -d : -f1); ":
                        //  "filefstab=\"/etc/fstab\"; while IFS=  read -r line_par; do  sudo sed -i ''$line_par's/nouser/user/' $filefstab;     done <<< $(sudo cat $filefstab | sudo grep -n  \"^.\dev/sr[0-9].*\\bnouser.*\" | sudo cut -d : -f1); ";
                        QString ResCommand = (mState == "0")?_nouser:_user;
                        //qWarning()<<"SSH start command."<<ResCommand;
                        if(SSH_action(SSHWorkerObj, ResCommand)){
                            QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
                            SSHWorkerObjlst.removeAll(QString(""));
                            if(!SSHWorkerObjlst.size())
                              resourceNode.insert("update", "OK");
                            else
                              resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                            map.insert("data", resourceNode);
                          }
                          else{
                            resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                            map.insert("data", resourceNode);
                          }
                        //if(cntObj){
                          map.insert("version", "1.1.1.1");
                          map.insert("object", 47);
                          map.insert("state",  0);
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
                        //}
                      }

                      //touch usb rules
                       if (request.getParameter("resourcebyhost") == "test_ssh_connect"){
                          QVariantMap map;
                          QVariantMap resourceNode;
                          QString     mState;
                          /**/
                         if(request.getParameterMap().contains("resourcehost"))
                           ResHost = request.getParameter("resourcehost");
                           if(SSH_test_host(ResHost) == 0)
                             resourceNode.insert("update", "OK");
                           else
                             resourceNode.insert("update", "ERR");
                           map.insert("data", resourceNode);

                           map.insert("version", "1.1.1.1");
                           map.insert("object", 84);
                           map.insert("state",  0);
                           map.insert("host_name", ResHost);
                           map.insert("host_ip",  "");
                           map.insert("action", "");
                           QJsonObject json = QJsonObject::fromVariantMap(map);
                           QJsonDocument JsonDocument_o(json);
                           //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
                           if (!JsonDocument_o.isNull()) {
                             response.setHeader("Content-Type", "text/html; charset=UTF-8");
                             response.write(JsonDocument_o.toJson());
                           }
                       }


                      //touch usb rules
                       if (request.getParameter("resourcebyhost") == "delete_udev_touch"){
                          QVariantMap map;
                          QVariantMap resourceNode;
                          QVariantMap driveNode;
                          /**/
                         if(request.getParameterMap().contains("resourcehost"))
                           ResHost = request.getParameter("resourcehost");
                         QHostInfo info = QHostInfo::fromName(ResHost);
                         const auto addresses = info.addresses();
                         QHostAddress address = info.addresses().first();
                         ResIP = address.toString();
                         /*SSH*/
                         SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
                         QString ResCommand = "sudo rm /etc/udev/rules.d/99-usb.rules; sudo udevadm control --reload-rules; sudo /bin/sh /opt/asb/scripts/usb_autorized_default.sh";
                         //qWarning()<<"SSH start command."<<ResCommand;
                         if(SSH_action(SSHWorkerObj, ResCommand)){
                             QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
                             SSHWorkerObjlst.removeAll(QString(""));
                             if(!SSHWorkerObjlst.size())
                               resourceNode.insert("update", "OK");
                             else
                               resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                             map.insert("data", resourceNode);
                           }
                           else{
                             resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                             map.insert("data", resourceNode);
                           }
                         //if(cntObj){
                           map.insert("version", "1.1.1.1");
                           map.insert("object", 77);
                           map.insert("state",  0);
                           map.insert("host_name", ResHost);
                           map.insert("host_ip",  ResIP);
                           map.insert("action", "");
                           QJsonObject json = QJsonObject::fromVariantMap(map);
                           QJsonDocument JsonDocument_o(json);
                           if (!JsonDocument_o.isNull()) {
                             response.setHeader("Content-Type", "text/html; charset=UTF-8");
                             response.write(JsonDocument_o.toJson());
                           }
                         //}
                       }


                   //touch usb rules
                    if (request.getParameter("resourcebyhost") == "dev_usb_rules_touch"){
                       QVariantMap map;
                       QVariantMap resourceNode;
                       QVariantMap driveNode;
                       /**/
                      if(request.getParameterMap().contains("resourcehost"))
                        ResHost = request.getParameter("resourcehost");

                      QHostInfo info = QHostInfo::fromName(ResHost);
                      const auto addresses = info.addresses();
                      QHostAddress address = info.addresses().first();
                      ResIP = address.toString();
                      /*SSH*/
                      SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
                      QString ResCommand = QString("msize=$(sudo stat --format=%s \"/etc/udev/rules.d/99-usb.rules\"); if [[ ! -e /etc/udev/rules.d/99-usb.rules ]]; then  sudo touch /etc/udev/rules.d/99-usb.rules; sudo chmod 755 /etc/udev/rules.d/99-usb.rules; sudo echo '" + rules_udev + "' | sudo tee  /etc/udev/rules.d/99-usb.rules;  else  if [[ \"$msize\" == '0' ]]; then sudo echo '" + rules_udev + "' | sudo tee  /etc/udev/rules.d/99-usb.rules;  fi; echo \"FILE_EXIST\";  fi; sudo udevadm control --reload-rules; sudo /bin/sh /opt/asb/scripts/usb_autorized_off_default.sh");
                      qWarning()<<"SSH start command."<<ResCommand;
                      QString ResCommandReload = "sudo udevadm control --reload-rules";
                      if(SSH_action(SSHWorkerObj, ResCommand)){
                          QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
                          SSHWorkerObjlst.removeAll(QString(""));
                          qWarning()<<"SSH result:"<<SSHWorkerObjlst;
                          if(SSHWorkerObjlst[0] == "FILE_EXIST"){
                            resourceNode.insert("update", "FILE_EXIST");
                          }
                          else
                            resourceNode.insert("update", "OK");                                                                                ;
                          map.insert("data", resourceNode);
                        }
                        else{
                          resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                          map.insert("data", resourceNode);
                        }
                      //if(cntObj){
                        map.insert("version", "1.1.1.1");
                        map.insert("object", 21);
                        map.insert("state",  0);
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
                      //}
                    }


                    //touch usb rules
                     if (request.getParameter("resourcebyhost") == "dev_scp"){
                        QVariantMap map;
                        QVariantMap resourceNode;
                        QVariantMap driveNode;
                        /**/
                       if(request.getParameterMap().contains("resourcehost"))
                         ResHost = request.getParameter("resourcehost");

                       QHostInfo info = QHostInfo::fromName(ResHost);
                       const auto addresses = info.addresses();
                       QHostAddress address = info.addresses().first();
                       ResIP = address.toString();
                       /*SSH*/
                       bool isNeedScripts = false;

                      {
                       SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
                       QString ResCommand = "if [[ ! -e /opt/asb ]]; then sudo mkdir -p  /opt/asb; sudo chmod -R 0700 /opt/asb;  sudo chown " + ControllerAction::_Username_str +  ":"  + ControllerAction::_Username_str + " /opt/asb; echo 0; else  if [ ! -e /opt/asb/scripts/usb_unautorized.sh  -o  ! -e /opt/asb/scripts/usb_autorized.sh   -o   ! -e /opt/asb/scripts/nouser_fstab.sh  -o  ! -e /opt/asb/scripts/user_fstab.sh ]; then echo 0;  else  echo 1; fi; fi;";
                       //QString ResCommand = "if [[ ! -e /opt/asb/scripts ]]; then sudo mkdir -p  /opt/asb/scripts; sudo chmod -R 0700 /opt/asb/scripts;  sudo chown " + ControllerAction::_Username_str +  ":"  + ControllerAction::_Username_str + " /opt/asb/scripts; echo 0; else  if [ ! -e /opt/asb/scripts/usb_unautorized.sh  -o  ! -e /opt/asb/scripts/usb_autorized.sh   -o   ! -e /opt/asb/scripts/nouser_fstab.sh  -o  ! -e /opt/asb/scripts/user_fstab.sh  -o ! -e /opt/asb/scripts/cd_unmount_remove.sh -o ! -e /opt/asb/scripts/usb_autorized_default.sh  ]; then echo 0;  else  echo 1; fi; fi;";
                       qWarning()<<"SSH start command."<<ResCommand;
                       if(SSH_action(SSHWorkerObj, ResCommand)){
                           QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
                           SSHWorkerObjlst.removeAll(QString(""));
                           qWarning()<<SSHWorkerObjlst;
                           if(SSHWorkerObjlst[0] == "0"){
                             isNeedScripts = true;
                             //SFTP
                             /*
                             SSHWorkerObj.SendFileToHost_b("/opt/asb/scripts/usb_unautorized.sh", "/opt/asb/scripts");
                             SSHWorkerObj.SendFileToHost_b("/opt/asb/scripts/usb_autorized.sh", "/opt/asb/scripts");
                             SSHWorkerObj.SendFileToHost_b("/opt/asb/scripts/nouser_fstab.sh", "/opt/asb/scripts");
                             SSHWorkerObj.SendFileToHost_b("/opt/asb/scripts/user_fstab.sh", "/opt/asb/scripts");
                             */
                             //ResCommand  = "if [[ -e /opt/asb/scripts ]]; then  sudo chmod -R 0700 /opt/asb/scripts; fi;";
                             //SSH_action(SSHWorkerObj, ResCommand);
                           }
                         }
                         else{
                           resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                           map.insert("data", resourceNode);
                         }
                     }

                       //SCP SSH
                      if(isNeedScripts){
                           QString ResCommandScp = "#!/usr/bin/expect\r\n\
                           spawn scp -r /opt/asb/scripts " + ControllerAction::_Username_str + "@" + ResIP +":/opt/asb/scripts;\r\n\
                           set pass \"""" +  ControllerAction::_Password_str + """\"\r\n\
                           expect {\r\n\
                           password: {send \"""\''$pass\''\\r\"""; exp_continue }\r\n\
                           }";
                           {
                             SSHWorker SSHWorkerObjScp("127.0.0.1", ControllerAction::_Username_str,ControllerAction::_Password_str);
                             qWarning()<<"Copy scripts..."<<ResCommand;
                             QString ScpLocal = ResIP + "_scp.exp";
                             QString ScpFile = "sudo rm /tmp/" + ScpLocal + "; if [[ ! -e /tmp/" + ScpLocal + " ]]; then sudo touch /tmp/" + ScpLocal + "; sudo chmod 755  /tmp/" + ScpLocal + "; echo '" + ResCommandScp + "' | sudo tee /tmp/" + ScpLocal + "; fi;";
                             QString ResExpect = QString("sudo expect /tmp/" + ScpLocal + "");
                             if(SSH_action(SSHWorkerObjScp, ScpFile) &&
                                SSH_action(SSHWorkerObjScp, ResExpect)){
                             }
                             else{
                                 resourceNode.insert("update", SSHWorkerObjScp.GetLastError_str());
                                 map.insert("data", resourceNode);
                             }
                           }
                       }

                       //if(cntObj){
                         map.insert("version", "1.1.1.1");
                         map.insert("object", 81);
                         map.insert("state",  0);
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
                       //}
                     }



                    if (request.getParameter("resourcebyhost") == "mod_usb_rules_serial"){
                       QVariantMap map;
                       QVariantMap resourceNode;
                       QVariantMap driveNode;
                       QString ResSerial;
                       QString mTypeRule;
                       QString mUser="";
                       QString mGroup="";
                       QString mModeRule = "";
                       QString mDiscrete = "0";
                       /**/
                      if(request.getParameterMap().contains("resourcehost"))
                        ResHost = request.getParameter("resourcehost");

                      if(request.getParameterMap().contains("rule"))
                        ResSerial = request.getParameter("rule");
                      else
                        return;
                      if(!ResSerial.length()) return;

                      if(request.getParameterMap().contains("typerule"))
                        mTypeRule = request.getParameter("typerule");

                      if(request.getParameterMap().contains("user"))
                        mUser = request.getParameter("user");

                      if(request.getParameterMap().contains("group"))
                        mGroup = request.getParameter("group");

                      if(request.getParameterMap().contains("discrete"))
                        mDiscrete = request.getParameter("discrete");

                      if(mTypeRule != "del" && mTypeRule != "add") return;

                      QHostInfo info = QHostInfo::fromName(ResHost);
                      const auto addresses = info.addresses();
                      QHostAddress address = info.addresses().first();
                      ResIP = address.toString();
                      /*SSH*/
                      QString ResCommand;
                      SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
                      if(mTypeRule == "del")
                        ResCommand = QString("rules=$(sudo cat /etc/udev/rules.d/99-usb.rules); SER_FIND=$(echo \"$rules\" | sudo grep  \"" + ResSerial + "\"); if [ \"$SER_FIND\" != '' ]; then echo \"$(echo \"$rules\" | sudo grep -v \"$SER_FIND\")\"; else echo \"$rules\"; fi;");
                      else if(mTypeRule == "add"){
                        QString mUserRule="";
                        QString mGroupRule="";
                        if(request.getParameterMap().contains("discrete") && mDiscrete != "0"){
                         if(mUser.length()){
                            mUserRule += " OWNER:=\"" + mUser + "\", ";
                          }
                          if(mGroup.length() && mDiscrete!= "0"){
                             mGroupRule += " GROUP:=\"" + mGroup + "\", ";
                           }
                           mModeRule += " MODE:=\"" + mDiscrete + "\", ";
                        }
                        ResCommand = QString("rules=$(sudo cat /etc/udev/rules.d/99-usb.rules); SER_FIND=$(sudo echo \"$rules\" | sudo grep  \"" + ResSerial + "\"); if [ \"$SER_FIND\" != '' ]; then echo \"$(sudo echo \"$rules\"  | sudo grep -v \"$SER_FIND\") \"| sed -e '/^\# Rules.*/a ACTION==\"add\", ENV{ID_SERIAL_SHORT}==\"" + ResSerial + "\", " + mUserRule + mGroupRule + mModeRule + " RUN+=\"/bin/sh /opt/asb/scripts/usb_autorized.sh 1 /sys%p/authorized\", GOTO=\"usb_end\"\r\n'; else  echo \"$(echo \"$rules\" | sed -e '/^\# Rules.*/a ACTION==\"add\", ENV{ID_SERIAL_SHORT}==\"" + ResSerial + "\", " + mUserRule + mGroupRule + mModeRule + " RUN+=\"/bin/sh /opt/asb/scripts/usb_autorized.sh 1 /sys%p/authorized\", GOTO=\"usb_end\"\r\n')\"; fi;");
                       //ResCommand = QString("sudo sed '/^\# Rules.*/a ACTION==\"add\", ENV{ID_SERIAL_SHORT}==\"" + ResSerial + "\", " + mUserRule + mGroupRule + mModeRule + " RUN+=\"/bin/sh /opt/asb/scripts/usb_autorized.sh 1 /sys%p/authorized\", GOTO=\"usb_end\"\r\n' /etc/udev/rules.d/99-usb.rules");
                      }
                      qWarning()<<"SSH start command."<<ResCommand;
                      if(SSH_action(SSHWorkerObj, ResCommand)){
                          QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
                          SSHWorkerObjlst.removeAll(QString(""));
                          qWarning()<<"SSH result:"<<SSHWorkerObjlst;                
                          QString WriteRule;

                          if(!SSHWorkerObjlst.size())
                            WriteRule = "sudo echo '" + rules_udev + "' | sudo tee /etc/udev/rules.d/99-usb.rules";
                          else
                            WriteRule = "sudo echo '" + (QString)SSHWorkerObjlst.join("\n") + "' | sudo tee /etc/udev/rules.d/99-usb.rules";

                          //
                         if(SSH_action(SSHWorkerObj, WriteRule)){
                          qWarning()<<"SSH result:"<<SSHWorkerObj.GetLastOutput_lst();
                          QString ResCommandReload = "sudo udevadm control --reload-rules";
                          if(SSH_action(SSHWorkerObj, ResCommandReload)){
                            resourceNode.insert("update", "OK");
                          }
                         }
                         else
                          resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                          map.insert("data", resourceNode);
                        }
                        else{
                          resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
                          map.insert("data", resourceNode);
                        }
                      if(mTypeRule == "add"){
                          ResCommand = QString("(for  x in  `sudo find  /sys/bus/usb/devices/  -name  \"*\" -print | sudo grep \"[1-99]-[1-99]\"`; do SER=$(sudo udevadm info $x | sudo grep -Po  '(?<=ID_SERIAL_SHORT=).*'); if [ \"$SER\" == \"" + ResSerial + "\" ]; then  sudo echo 0 | sudo tee $x'/authorized';  break; fi;  done)");
                          SSH_action(SSHWorkerObj, ResCommand);
                          ResCommand = QString("(for  x in  `sudo find  /sys/bus/usb/devices/  -name  \"*\" -print | sudo grep \"[1-99]-[1-99]\"`; do SER=$(sudo udevadm info $x | sudo grep -Po  '(?<=ID_SERIAL_SHORT=).*'); if [ \"$SER\" == \"" + ResSerial + "\" ]; then  sudo echo 1 | sudo tee $x'/authorized';  break; fi;  done)");
                          SSH_action(SSHWorkerObj, ResCommand);
                      }
                      if(mTypeRule == "del"){
                          ResCommand = QString("(for  x in  `sudo find  /sys/bus/usb/devices/  -name  \"*\" -print | sudo grep \"[1-99]-[1-99]\"`; do SER=$(sudo udevadm info $x | sudo grep -Po  '(?<=ID_SERIAL_SHORT=).*'); if [ \"$SER\" == \"" + ResSerial + "\" ]; then  sudo echo 0 | sudo tee $x'/authorized';  break; fi;  done)");
                          SSH_action(SSHWorkerObj, ResCommand);
                      }
                      //if(cntObj){
                        map.insert("version", "1.1.1.1");
                        map.insert("object", 21);
                        map.insert("state",  0);
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
                      //}
                    }


                    /*save pack tar gz*/
                    if (request.getParameter("resourcebyhost") == "saveresourceDev"){                       
                       QVariantMap map;
                       QVariantMap resourceNode;
                       QString resourceOpearation;
                       QString ftpServer;
                       QString ftpUser;
                       QString ftpPass;

                       bool recursive = false;
                       bool fail      = false;
                       int  state     = 0;
                       /**/
                      if(request.getParameterMap().contains("resourcehost"))
                        ResHost = request.getParameter("resourcehost");
                      if(request.getParameterMap().contains("resourcepath"))
                        ResDir = request.getParameter("resourcepath");
                      if(request.getParameterMap().contains("operation"))
                        resourceOpearation = request.getParameter("operation");
                      if(request.getParameterMap().contains("recursive"))
                        recursive=true;

                      DatabaseWorker DatabaseWorker_o(this);
                      QSqlQuery Query_lst;

                      if(DatabaseWorker_o.QueryExec_lstExt(QString("SELECT ftp_server, ftp_user, ftp_pass  FROM  save_server "), false, Query_lst)){
                          Query_lst.first();
                          if(!Query_lst.value("ftp_server").toString().isEmpty()){
                            ftpServer = Query_lst.value("ftp_server").toString();
                          }
                          if(!Query_lst.value("ftp_user").toString().isEmpty()){
                            ftpUser = Query_lst.value("ftp_user").toString();
                          }
                          if(!Query_lst.value("ftp_pass").toString().isEmpty()){
                            ftpPass = Query_lst.value("ftp_pass").toString();
                          }
                      }

                       qWarning()<<"load recursive list save...";
                      {
                       std::unique_lock<std::mutex> lc(locker);
                       if(!DatabaseWorker_o.QueryExec_lstExt(QString("SELECT hostname, dir_to_save FROM hosts_save WHERE hostname='"  +  ResHost  +  "'"), false, Query_lst)){
                          qWarning()<<"db errors recursive backuplist..."<<endl;
                          fail = true;
                       }
                       else{
                         Query_lst.first();
                         do{
                           if(recursive){
                             qWarning()<<"load recursive dirs...";
                             ResHost = Query_lst.value("hostname").toString();
                             ResDir  = Query_lst.value("dir_to_save").toString();
                             qWarning()<<ResHost;
                             qWarning()<<ResDir;
                           }
                           ObjProtect pObjProtect;
                           pObjProtect.mFtpSavePath = ftpServer;
                           pObjProtect.mFtpUser     = ftpUser;
                           pObjProtect.mFtpPass     = ftpPass;
                           pObjProtect.mSShUser   = ControllerAction::_Username_str;
                           pObjProtect.mSShPass   = ControllerAction::_Password_str;
                           pObjProtect.mOperation = (resourceOpearation.isEmpty())?0:resourceOpearation.toInt();
                           pObjProtect.mTempSavePath = "/tmp/osd_save_resources";
                           //---------------TASK---------------
                           QHostInfo info = QHostInfo::fromName(ResHost);
                           const auto addresses = info.addresses();
                           QHostAddress address = info.addresses().first();
                           ResIP = address.toString();
                           QString mUID = QString(SequentialGUID::GetSequentialGUID(SequentialGUID::SequentialAsString).toString());
                           std::vector<std::pair<QString, QString>> replacements {
                               { "}", "" },
                               { "{", ""  }
                           };
                           for ( auto const &r : replacements) {
                               mUID.replace(r.first, r.second);
                           }
                           QString ArcName = QString("protected_" + ResHost + "_" + mUID  + ".tar.gz"); //  QString(ResHost + "_" + QDateTime::currentDateTime().toString("dd-MM-yyyy_HH:mm:ss") + "_" + mUID + ".tar.gz");
                           pObjProtect.mIP      = ResIP;
                           pObjProtect.mHost    = ResHost;
                           pObjProtect.mFolder  = ResDir;
                           pObjProtect.mUid     = mUID;
                           pObjProtect.mFileArc = ArcName;

                           QSqlQuery Query_lstIns;
                           if(DatabaseWorker_o.QueryExec_lstExt(QString("INSERT INTO hosts_save_history (hostname, dir_to_save, id_operation, uid_process, arc_file, time_stop) VALUES('" + pObjProtect.mHost + "','" + pObjProtect.mFolder + "','" + (QString::number(pObjProtect.mOperation).toUtf8()) + "','" + pObjProtect.mUid + "', '" + pObjProtect.mFileArc + "', (datetime('now','localtime')));"), false, Query_lstIns)){
                             qWarning()<<"ControllerProtectedFolder move to thread...";
                             //ControllerProtectedFolder pControllerProtectedFolder;
                             //emit pControllerProtectedFolder.operate(pObjProtect);
                             QThreadPool::globalInstance()->start(new ProtectFolderWork(pObjProtect));
                             qWarning()<<"ControllerProtectedFolder move to thread complete...";
                             state = 1;
                           }
                           else{
                             state = 2;
                             fail = true;
                           }
                           //---------------TASK---------------
                           if(!recursive) break;
                         } while(Query_lst.next());
                       }

                      }
                      resourceNode.insert("update", (!fail)?state:0);
                      map.insert("state",(!fail)?1:-1);

                      map.insert("data", resourceNode);
                      map.insert("version", "1.1.1.1");
                      map.insert("object", 32);
                      map.insert("host_name", ResHost);
                      map.insert("host_ip",  ResIP);
                      map.insert("action", ResDir);

                      QJsonObject json = QJsonObject::fromVariantMap(map);
                      QJsonDocument JsonDocument_o(json);
                      //qDebug() << JsonDocument_o.toJson(QJsonDocument::Indented);
                      if (!JsonDocument_o.isNull()) {
                        response.setHeader("Content-Type", "text/html; charset=UTF-8");
                        response.write(JsonDocument_o.toJson());
                        response.setStatus(200,"OK");
                        response.flush();
                      }
                    }

         }
  }

  void ControllerAction::SetSettings_sv(QSettings & Settings_ro)
  {
    QString hh = Settings_ro.value("none").toString();
    SetSshParams(Settings_ro.value("username").toString(), Settings_ro.value("password").toString());
  }

  void ControllerAction::InitializeDataBase_sv()
  {

  }

  void ControllerAction::SetSshParams(const QString _Username_, const QString _Password_){
    ControllerAction::_Username_str = _Username_;
    ControllerAction::_Password_str = _Password_;
  }


  int ControllerAction::ClearHostFolderSave(const QString& mHost, const QString& mFolder){
      try {                      
          QHostInfo info = QHostInfo::fromName(mHost);
          const auto addresses = info.addresses();
          QHostAddress address = info.addresses().first();
          QString ResIP = address.toString();
          /*SSH*/
           SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
           QString ResCommand = "sudo rm -r '" + mFolder + "'/*";
           qWarning()<<"SSH start command..."<<mHost<<"  "<<ResCommand;
           if(SSH_action(SSHWorkerObj, ResCommand)){
             QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
             return 0;
           }
           else{
             return 1;
           }
      }
      catch (OSDSException & e) {
          qWarning()<<"clear folder errors..."<<endl;
          qWarning()<<""<<e.what();
          return 2;
      }
  }


  int ControllerAction::SSH_test_host(const QString& mHost){
      try {
          QHostInfo info = QHostInfo::fromName(mHost);
          const auto addresses = info.addresses();
          QHostAddress address = info.addresses().first();
          QString ResIP = address.toString();
          /*SSH*/
          SSHWorker SSHWorkerObj(ResIP,ControllerAction::_Username_str,ControllerAction::_Password_str);
          return 0;
      }
      catch (OSDSException & e) {
          qWarning()<<"test SSH connect fail..."<<mHost<<endl;
          qWarning()<<""<<e.what();
          return 2;
      }
  }

  int ControllerAction::AddHostFolderSave(const QString& mHost, const QString& mFolder){
      try {
        qWarning()<<"tables db save add...";
        DatabaseWorker DatabaseWorker_o(this);
        QSqlQuery Query_lst;
        if(!DatabaseWorker_o.QueryExec_lstExt(QString("SELECT COUNT(*) FROM  hosts_save WHERE hostname='" +mHost + "' AND dir_to_save='" + mFolder + "'"), false, Query_lst)){
            qWarning()<<"::AddHostFolderSave  errors"<<endl;
            return 1;
        }
        Query_lst.next();
        int cntRow = 0;
        if(!Query_lst.value("COUNT(*)").toString().isEmpty())
          cntRow = Query_lst.value("COUNT(*)").toInt();
        if(cntRow > 0) return 1;
        if(!DatabaseWorker_o.QueryExec_lstExt(QString("REPLACE INTO hosts_save VALUES('" +mHost + "','" + mFolder + "');"), false, Query_lst)){
            qWarning()<<"::AddHostFolderSave  insert errors"<<endl;
            return 2;
        }
        qWarning()<<"tables db save add end"<<endl;
        return 0;
      } catch (OSDSException & e) {
          qWarning()<<"tables db save add errors..."<<endl;
          qWarning()<<""<<e.what();
          return 2;
      }
  }

  int ControllerAction::AddHostFolderSaveHistory(const QString& mHost, const QString& mFolder, const int& mId){
      try {
        qWarning()<<"tables db save history add...";
        DatabaseWorker DatabaseWorker_o(this);
        QSqlQuery Query_lst;
        if(!DatabaseWorker_o.QueryExec_lstExt(QString("REPLACE INTO hosts_save_history (hostname, dir_to_save, id_operation) VALUES('" +mHost + "','" + mFolder + "','" + (QString::number(mId).toUtf8()) + "');"), false, Query_lst)){
          qWarning()<<"::AddHostFolderSaveHistory errors"<<endl;
          return 2;
        }
        qWarning()<<"tables db save history add end"<<endl;
        return 0;
      } catch (OSDSException & e) {
          qWarning()<<"tables db save history add errors..."<<endl;
          qWarning()<<""<<e.what();
          return 2;
      }
  }


  int ControllerAction::RemoveHostFolderSaveHistory(const QString& mHost, const QString& mFolder){
      try {
        qWarning()<<"tables db save history del...";
        DatabaseWorker DatabaseWorker_o(this);
        QSqlQuery Query_lst;
        if(!DatabaseWorker_o.QueryExec_lstExt(QString("DELETE FROM  hosts_save WHERE hostname='" +mHost + "' AND dir_to_save='" + mFolder + "'"), false, Query_lst)){
          qWarning()<<"::RemoveHostFolderSaveHistory  errors"<<endl;
          return 1;
        }
         qWarning()<<"tables db save history del end"<<endl;
        return 0;
      } catch (OSDSException & e) {
          qWarning()<<"tables db save history del errors..."<<endl;
          qWarning()<<""<<e.what();
          return 2;
      }
  }


  bool ControllerAction::CheckSSHDiscreteRules(SSHWorker& SSHWorkerObj, QString& ResIP, QString& ResDir, QString& ResRec, QString& ResDiscreteVal, QString&  ResChownRuleVal, QString& Command_bits_updateVal){
      QString ResCommand_user  = "";
      QString ResCommand_group = "";
      QString ResCommand_other = "";

      if(ResDiscreteVal.length()){
        int mOwner = ResDiscreteVal.mid(0, 1).toInt();
        int mGroup = ResDiscreteVal.mid(1, 1).toInt();
        int mOther = ResDiscreteVal.mid(2, 1).toInt();
        QString sUser  = "user::";
        QString sGroup = "group::";
        QString sOther = "other::";

        sUser += (mOwner&(1<<2))?"r":"-";
        sUser += (mOwner&(1<<1))?"w":"-";
        sUser += (mOwner&(1<<0))?"x":"-";
        sGroup += (mGroup&(1<<2))?"r":"-";
        sGroup += (mGroup&(1<<1))?"w":"-";
        sGroup += (mGroup&(1<<0))?"x":"-";
        sOther += (mOther&(1<<2))?"r":"-";
        sOther += (mOther&(1<<1))?"w":"-";
        sOther += (mOther&(1<<0))?"x":"-";

        ResCommand_user  = "sudo setfacl " + ResRec + " -m '" + sUser + "' '" + ResDir + "'; ";
        ResCommand_group = "sudo setfacl " + ResRec + " -m '" + sGroup + "' '" + ResDir + "'; ";
        ResCommand_other = "sudo setfacl " + ResRec + " -m '" + sOther + "' '" + ResDir + "'; ";
      }
      QString DiscreteSSH = QString(ResChownRuleVal + ResCommand_user + ResCommand_group  + ResCommand_other + Command_bits_updateVal);
      if(SSH_action(SSHWorkerObj, DiscreteSSH))
        return true;
      else
        return false;
  }

  bool ControllerAction::SSH_action(SSHWorker& SSHWorkerObj, QString& mCOmmand){
    qWarning()<<"SSH start command:"<<mCOmmand;
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



