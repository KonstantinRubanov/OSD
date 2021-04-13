#include <QDateTime>
#include <QStringRef>

#include "sshworker.h"
#include "osdsexception.h"
#include "ControllerAntivirus.h"
#include "ControllerLogin.h"
#include "integritycontroltest.h"

namespace osds {

  IntegrityControlTest::IntegrityControlTest(const QString & Host_str, const QStringList & Dirs_lst, bool IsAudit_b, QObject * parent) : QObject(parent),
    _HostName_str(Host_str),
    _AddedDirs_str(Dirs_lst),
    _IsAudit_b(IsAudit_b),
    _TempFilename_str("/tmp/" + Host_str + "_afick_test.log")
  {
    _ExceptionDict_map[_ICETempFile_en] = tr("Невозможно считать файл с КЦ");
    _ExceptionDict_map[_ICEReciveTempFile_en] = tr("Невозможно принять файл с КЦ");
  }

  void IntegrityControlTest::process()
  {
    try {
      SSHWorker SSHWorker_o(_HostName_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
      quint32 ErrorsCount_u32 = 0;
      QString Journal_str;
      bool IsBreak_b = false, IsStartChanges_b = false;
      QMap<QString, QPair<QString, QString> > Changes_map; // Список измененных данных

      QMap<QString, QPair<QString, QString> > Kernlog_map; // Список измененных данных

      QString TestControlMain=" str=\"\"; var_scan=0; cnl_file=\"" + _TempFilename_str + "_analize.log\"; sudo afick -k > \"$cnl_file\";  if [ -e  \"$cnl_file\" ]; then  str_res=$(sudo  cat \"$cnl_file\" | grep \"# Hash database\"); if [ \"$str_res\" != \"\" ]; then  str=$(echo $str_res | grep -Po \"(\\d*)\");   while read -r i; do  if [ \"$var_scan\" == 0 ]; then  echo \"Проверено объектов: \"$i; fi; if [ \"$var_scan\" == 1 ]; then  echo \"Всего модифицировано  объектов: \"$i; fi;   if [ \"$var_scan\" == 2 ]; then  echo \"Новых  объектов: \"$i; fi;  if [ \"$var_scan\" == 3 ]; then  echo \"Удаленных  объектов: \"$i; fi;  if [ \"$var_scan\" == 4 ]; then  echo \"Измененных  объектов: \"$i; fi;  var_scan=$((var_scan+1));  done <<< $str; fi;  if [ $var_scan -eq 0 ]; then echo \"Данные контроля целостости отсутствуют.\";  else echo \"----------список измененных объектов-----------\"; fld=\"\"; while read -r i; do  fld=$(echo \"$i\" | awk '{print $1}'); case $fld in  deleted) echo \"удален объект -> \"$i ;; changed) echo \"изменен объект -> \"$i ;; new) echo \"создан объект -> \"$i ;; *) echo \" изменение объекта  -> \"$i ;;  esac;   done <<<  $(grep -ws \"detailed changes\" $cnl_file -A 1000000 |  grep \" : \/\");   fi;  else echo \"Данные контроля целостости отсутствуют.\"; fi;";
      //QString TestControlMain="str=\"\"; var_scan=0; cnl_file=\"" + _TempFilename_str + "_analize.log\"; sudo afick -k > \"$cnl_file\";  if [ -e  \"$cnl_file\" ]; then  str_res=$(sudo  cat \"$cnl_file\" | grep \"# Hash database\"); if [ \"$str_res\" != \"\" ]; then  str=$(echo $str_res | grep -Po \"(\\d*)\");   while read -r i; do  if [ \"$var_scan\" == 0 ]; then  echo \"Проверено объектов: \"$i; fi; if [ \"$var_scan\" == 1 ]; then  echo \"Всего модифицировано  объектов: \"$i; fi;   if [ \"$var_scan\" == 2 ]; then  echo \"Новых  объектов: \"$i; fi;  if [ \"$var_scan\" == 3 ]; then  echo \"Удаленных  объектов: \"$i; fi;  if [ \"$var_scan\" == 4 ]; then  echo \"Измененных  объектов: \"$i; fi;  var_scan=$((var_scan+1));  done <<< $str; fi;  if [ $var_scan -eq 0 ]; then echo \"Данные контроля целостости отсутствуют.\"; fi;  else echo \"Данные контроля целостости отсутствуют.\"; fi;";
      QString TestControlFiles="str=\"\"; var_scan=0; cnl_file=\""  + _TempFilename_str + "_analize.log\"; sudo afick -k > \"$cnl_file\";  if [ -e \"$cnl_file\" ]; then  res=$(grep -ws \"detailed changes\" $cnl_file -A 100000 );  echo \"$res\" | grep \" : \/\";  else echo \"Данные контроля целостости отсутствуют.\"; fi;";
      QString Journal_str_analize = "";
      QString Journal_str_analize_files = "";
      QStringList SSHWorkerObjlst;
      QStringList SSHWorkerObjlstFiles;

      if(SSHWorker_o.ExecCommandOnHost_b(TestControlMain)) {
        SSHWorkerObjlst = SSHWorker_o.GetLastOutput_lst();
        SSHWorkerObjlst.removeAll(QString(""));
        for(auto &p_list:  SSHWorkerObjlst)
          Journal_str_analize += p_list + "<br>";
      }

      if(SSHWorker_o.ExecCommandOnHost_b(TestControlFiles)) {
        SSHWorkerObjlstFiles = SSHWorker_o.GetLastOutput_lst();
        SSHWorkerObjlstFiles.removeAll(QString(""));
        for(auto &p_list:  SSHWorkerObjlstFiles)
          Journal_str_analize_files += p_list + "<br>";
      }

      if(SSHWorker_o.ExecCommandOnHost_b("sudo afick -k > " + _TempFilename_str + ".tmp")) {
        if (!SSHWorker_o.RecvFileFromHost_b(_TempFilename_str + ".tmp", _TempFilename_str)) { // Только если скачали
          throw(_ICEReciveTempFile_en);
        }
        QFile TempFile_o(_TempFilename_str);
        if(!TempFile_o.open(QIODevice::ReadOnly | QIODevice::Text)) {
          throw(_ICETempFile_en);
        }
        QStringList Output_lst = QString::fromUtf8(TempFile_o.readAll()).split("\n");
        TempFile_o.close();
        for(int i = 0; i < Output_lst.size(); ++i) {
          const QString & OutLine_str = Output_lst[i];
          Journal_str += OutLine_str + "<br>";
          if(OutLine_str.indexOf("# Hash database : ") != -1 && OutLine_str.indexOf("files scanned, ") != -1) {
            IsStartChanges_b = false;
            QStringList lst = OutLine_str.split(", ");
            if(lst.size() >= 2) {
              lst = lst[1].split(" ");
              if(lst.size() >= 1) {
                ErrorsCount_u32 = lst[0].toUInt();
              }
            }
          } else if (OutLine_str.indexOf("# detailed changes") != -1) { // Для выборки изменений
            IsStartChanges_b = true;
          } else if(IsStartChanges_b) { // Считываем данные
            if(OutLine_str.isEmpty()) {
              IsStartChanges_b = false;
              continue;
            }
            QStringList TempList_lst = OutLine_str.split(" : ");
            if(TempList_lst.size() >= 2) {
              QString FileName_str = TempList_lst[1];
              TempList_lst = Output_lst[i + 1].split(" : ");
              if(TempList_lst.size() >= 2) {
                QString FileDate_str = TempList_lst[1].left(24);
                foreach(QString Name_str, _AddedDirs_str) {
                  if(Name_str[Name_str.size() - 1] == '/' || Name_str[Name_str.size() - 1] == '\\' ) {
                    Name_str = Name_str.left(Name_str.size() - 1);
                  }
                  if(FileName_str.simplified().indexOf(Name_str.simplified()) != -1) {
                    Changes_map[FileName_str] = QPair<QString, QString>(FileDate_str, "");
                  }
                }
              }
            }
            i++;
          }
        }

        //анализ и перевод в читаемый вид
        Journal_str = Journal_str_analize;

      } else {  // Если не прошла команда
        ErrorsCount_u32 = 1;
        Journal_str = SSHWorker_o.GetLastError_str();
        IsBreak_b = true;
      }
      if(_IsAudit_b && !IsBreak_b && ErrorsCount_u32 > 0) { // Если есть ошибки КЦ


         QString Log_str_analize = "";
         QStringList Log_SSHWorkerObjlstFiles;

         qWarning()<<"ControllerIntegrityControl->Changes_map";
         QStringList Keys_lst_kern = Changes_map.keys();
          foreach(const QString & Key_str, Keys_lst_kern) {
            QPair<QString, QString> & Data_pair = Changes_map[Key_str];
            //QString ruleFields="var_scan=0; res=$(sudo  kernlog | grep \"" + Key_str + "\" | sort -t: -u -k1,1 | awk '{ out=\"\"; for(i=10;i<=NF;i++){out=out\" \"$i}; err=$out; print $2\" \"$4\" \"$3\" \"$6\" \"$5\"|\"$7\"|\"$8\"|\"out\"\"; count++; if (count==1000) exit }'); while read -r i; do  var_scan=0;  parse_fld=$(echo \"$i\" | cut -d'|' -f3);  parse_data=$(echo \"$i\" | cut -d'|' -f1);  parse_source=$(echo \"$i\" | cut -d'|' -f2); parse_obj=$(echo \"$i\" | cut -d'|' -f4 | grep -Po \"(?>\\\").*\\\"\"; );  if [ \"$(echo \"$i\" | grep unlink)\" != \"\" ]; then echo $parse_data\" источник: \"$parse_source\" объект: \"$parse_obj\" тип действия: удален \"; var_scan=$((var_scan+1));  fi;  if [ \"$(echo \"$i\" | grep setacl)\" != \"\" ]; then echo $parse_data\" источник: \"$parse_source\" объект: \"$parse_obj\" тип действия: изменение ACL\"; var_scan=$((var_scan+1)); fi;  if [ \"$(echo \"$i\" |  grep chmod)\" != \"\" ]; then echo $parse_data\" источник: \"$parse_source\" объект: \"$parse_obj\" тип действия: изменение дискретных атрибутов\"; var_scan=$((var_scan+1));  fi;  if [ \"$(echo \"$i\" | sudo grep chown)\" != \"\" ]; then echo $parse_data\" источник: \"$parse_source\" объект: \"$parse_obj\" тип действия: смена владельца\"; var_scan=$((var_scan+1));  fi; if [ \"$(echo \"$i\" | grep -Po  \"(?>.*create.*delete)\")\" != \"\" ]; then echo $parse_data\" источник: \"$parse_source\" объект: \"$parse_obj\" тип действия: модифицирован\"; var_scan=$((var_scan+1));  fi; if [ \"$(echo \"$i\" | grep  modify)\" != \"\" ]; then echo $parse_data\" источник: \"$parse_source\" объект: \"$parse_obj\" тип действия: модифицирован\"; var_scan=$((var_scan+1)); fi; if [ \"$(echo \"$i\" | grep -Po  \"(?>.*create.*)\" |  grep -v  \"(?>.*delete.*)\" )\" != \"\" ]; then echo $parse_data\" источник: \"$parse_source\" объект: \"$parse_obj\" тип действия: создан\";  var_scan=$((var_scan+1)); fi;  done <<< \"$res\";";
            //QString ruleFields="var_scan=0; res=$(sudo  kernlog | grep \"" + Key_str + "\" | awk '{ out=\"\"; for(i=10;i<=NF;i++){out=out\" \"$i}; err=$out; print $2\" \"$4\" \"$3\" \"$6\" \"$5\"|\"$7\"|\"$8\"|\"out\"\"; count++; if (count==1000) exit }' ); while read -r i; do  var_scan=0;  parse_fld=$(echo \"$i\" | cut -d'|' -f3);  parse_data=$(echo \"$i\" | cut -d'|' -f1);  parse_source=$(echo \"$i\" | cut -d'|' -f2); parse_obj=$(echo \"$i\" | cut -d'|' -f4 | grep -Po \"(?>\\\").*\\\"\"; );  if [ \"$(echo \"$i\" | grep unlink)\" != \"\" ]; then echo $parse_data\" источник: \"$parse_source\" объект: \"$parse_obj\" тип действия: удален \"; var_scan=$((var_scan+1));  fi;  if [ \"$(echo \"$i\" | grep setacl)\" != \"\" ]; then echo $parse_data\" источник: \"$parse_source\" объект: \"$parse_obj\" тип действия: изменение ACL\"; var_scan=$((var_scan+1)); fi;  if [ \"$(echo \"$i\" |  grep chmod)\" != \"\" ]; then echo $parse_data\" источник: \"$parse_source\" объект: \"$parse_obj\" тип действия: изменение дискретных атрибутов\"; var_scan=$((var_scan+1));  fi;  if [ \"$(echo \"$i\" | sudo grep chown)\" != \"\" ]; then echo $parse_data\" источник: \"$parse_source\" объект: \"$parse_obj\" тип действия: смена владельца\"; var_scan=$((var_scan+1));  fi; if [ \"$(echo \"$i\" | grep -Po  \"(?>.*create.*delete)\")\" != \"\" ]; then echo $parse_data\" источник: \"$parse_source\" объект: \"$parse_obj\" тип действия: модифицирован\"; var_scan=$((var_scan+1));  fi; if [ \"$(echo \"$i\" | grep  modify)\" != \"\" ]; then echo $parse_data\" источник: \"$parse_source\" объект: \"$parse_obj\" тип действия: модифицирован\"; var_scan=$((var_scan+1)); fi; if [ \"$(echo \"$i\" | grep -Po  \"(?>.*create.*)\" |  grep -v  \"(?>.*delete.*)\" )\" != \"\" ]; then echo $parse_data\" источник: \"$parse_source\" объект: \"$parse_obj\" тип действия: создан\";  var_scan=$((var_scan+1)); fi;  done <<< \"$res\";";
            QString ruleFields="var_scan=0; res=$(sudo  kernlog | grep \"" + Key_str + "\"  | awk '{ out=\"\"; for(i=10;i<=NF;i++){out=out\" \"$i}; err=$out; print $2\" \"$4\" \"$3\" \"$6\" \"$5\"|\"$7\"|\"$8\"|\"out\"\"; count++; if (count==1000) exit }'); while read -r i; do  var_scan=0;  parse_source=$(echo \"$i\" | cut -d'|' -f2);  echo $parse_data\" источник: \"$parse_source\" данные: \"$i\" \";  var_scan=$((var_scan+1));   done <<< \"$res\";";
            qWarning()<<"ControllerIntegrityControl->Rule:"<<ruleFields;
            if(SSHWorker_o.ExecCommandOnHost_b(ruleFields)) {
                Log_str_analize="";
                Log_SSHWorkerObjlstFiles = SSHWorker_o.GetLastOutput_lst();
                Log_SSHWorkerObjlstFiles.removeAll(QString(""));
                if(Log_SSHWorkerObjlstFiles.size()){
                  //Log_str_analize += "Информация об изменении: "  + Key_str +  "<br>";
                  int cnt = 0;
                  for(auto &p_list:  Log_SSHWorkerObjlstFiles){                    
                     //Log_str_analize += p_list + "<br>";
                     Log_str_analize += "<tr><td>"  + Key_str +  "</td><td>" + p_list + "</td></tr>";
                     //break;
                  }
                  Kernlog_map[Key_str] = QPair<QString, QString>(" ", Log_str_analize);
                }

              }
            }

        /*
        qWarning()<<"ControllerIntegrityControl->Changes_map";
        QStringList Keys_lst = Changes_map.keys();
        foreach(const QString & Key_str, Keys_lst) {
          QPair<QString, QString> & Data_pair = Changes_map[Key_str];
          if(SSHWorker_o.ExecCommandOnHost_b("sudo kernlog | grep '" + Key_str + "' > " +  _TempFilename_str + ".tmp")) {
            if (!SSHWorker_o.RecvFileFromHost_b(_TempFilename_str + ".tmp", _TempFilename_str)) { // Только если скачали
              throw(_ICEReciveTempFile_en);
            }
            QFile TempFile_o(_TempFilename_str);
            if(!TempFile_o.open(QIODevice::ReadOnly | QIODevice::Text)) {
              throw(_ICETempFile_en);
            }
            Data_pair.first.replace("  ", " 0");
            QLocale Locale_o(QLocale::English, QLocale::Russia);
            QDateTime StartTime_o = Locale_o.toDateTime(Data_pair.first, "ddd MMM dd hh:mm:ss yyyy");
            QStringList Output_lst = QString::fromUtf8(TempFile_o.readAll()).split("\n");
            TempFile_o.close();
            foreach(const QString & Text_str, Output_lst) {
              if(Text_str.size() > 30) {
                QStringList SplitData_lst = Text_str.split("'");
                if(SplitData_lst.size() >= 3) {
                  SplitData_lst[1].replace("  ", " 0");
                  QDateTime DataTime_o = Locale_o.toDateTime(SplitData_lst[1], "ddd MMM dd hh:mm:ss yyyy");
                  if(StartTime_o.toSecsSinceEpoch() < DataTime_o.toSecsSinceEpoch()) {
                    Data_pair.second += Text_str.simplified() + "<br>";
                    //Data_pair.second += "<br>";
                  }
                  qint32 StartArrow_i32 = Text_str.indexOf('<') + 1;
                  qint32 StopArrow_i32 = Text_str.indexOf('>');
                  if(StartArrow_i32 != -1 && StopArrow_i32 != -1) {
                    QStringRef SubString_str(&Text_str, StartArrow_i32, StopArrow_i32 - StartArrow_i32);
                    QStringList SubString_lst = SubString_str.toString().split(",");
                    if(SubString_lst.size() == 5) {

                    }
                  }
                }
              }
            }
          }
        }*/
      }
      //emit Output_sig(_HostName_str, ErrorsCount_u32, Journal_str, Changes_map);
      //Changes_map.clear();
      //Changes_map["список измененных объектов"] = QPair<QString, QString>(" ", Journal_str_analize_files);

      emit Output_sig(_HostName_str, ErrorsCount_u32, Journal_str, Kernlog_map);
      emit finished();
    } catch (OSDSException & e) {
      qWarning()<<"IntegrityControlTest::"<< e.what();
      emit Output_sig(_HostName_str, 1, "Error_SSH_Connect", QMap<QString, QPair<QString, QString> >());
      emit finished();
    } catch (EIntegrityControlException Exception_en) {
      qWarning()<<"IntegrityControlTest::process->enum "<<Exception_en;
      QString Exception_str = "Неизвестная ошибка";
      if(_ExceptionDict_map.contains(Exception_en)) {
        Exception_str = _ExceptionDict_map[Exception_en];
      }
      emit Output_sig(_HostName_str, 1, Exception_str, QMap<QString, QPair<QString, QString> >());
      emit finished();
    }

  }

  void IntegrityControlTest::stop()
  {

  }
}
