#include <atomic>
#include "auditprocess.h"
#include "ControllerAction.h"


namespace osds{
  std::vector<AuditTask> listTask;

  AuditProcess::AuditProcess(AuditTask& pTask)
  {
   mTask = pTask;
  }
  void AuditProcess::doWork(){
      mTask.State = -1;
      qWarning()<<"AuditTask pool complete..."<<mTask.AuditHost<<" "<<mTask.AuditPath;      

      int NextRule = 0;
      int PosRule  = 0;
      int CntRule  = 0;
      QVariantMap resourceNode;
      QString Temp_obj;
      QString Temp_rule;
      try{
       SSHWorker SSHWorkerObj(mTask.SSHHost, mTask.SSHUser, mTask.SSHPass);
       if(SSHWorkerObj.ExecCommandOnHost_b(QString(mTask.Command + " 2>&1"))){
          QStringList SSHWorkerObjlst = SSHWorkerObj.GetLastOutput_lst();
         // qWarning()<<"Result."<<SSHWorkerObjlst<<endl;
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
                  mTask.map.insert("data", resourceNode);
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
          qWarning()<<"Task error result = " << SSHWorkerObj.GetLastError_str();
          resourceNode.insert("update", SSHWorkerObj.GetLastError_str());
          mTask.map.insert("data", resourceNode);
          mTask.State = 1;
        }
      }
      catch(OSDSException & e) {
         qWarning()<<"Task error result = " << e.what();
         resourceNode.insert("update", e.what());
         mTask.map.insert("data", resourceNode);
         mTask.State = 1;
      }
        mTask.map.insert("version", "1.1.1.1");
        mTask.map.insert("object", 2);
        mTask.map.insert("state",  0);
        mTask.map.insert("host_name", mTask.AuditHost);
        mTask.map.insert("host_ip",   mTask.SSHHost);
        mTask.map.insert("action",    mTask.AuditPath);

        mTask.State = 0;

        {
          spin_lock lock;
          listTask.push_back(mTask);
        }

        emit resultReady(mTask.State);
  }

  bool AuditProcess::findTask(const QString mUid, AuditTask& mTask){
    try{
     uint64_t i = 0;
     {
      spin_lock lock;
      for(auto&pTask:listTask){
          if(pTask.UID == mUid){
             mTask = std::move(pTask);
             listTask.erase(listTask.begin() + i);
             qWarning()<<"Task uid = " << mUid << " extract.";
             return true;
          }
         i++;
       }
     }
      return false;
    }
    catch(OSDSException & e) {
     qWarning()<<"Task error result = " << e.what();
     return false;
    }

  }
}



