#include "sshworker.h"
#include "ControllerAntivirus.h"
#include "ControllerLogin.h"
#include "fullscanprocess.h"

namespace osds {

  FullScanProcess::FullScanProcess(const QString & Host_str, const QString & AntivirusType_str,  QObject * parent) : QObject(parent),
    _HostName_str(Host_str),
    _ProcessQuery_str(AntivirusType_str)
  {

  }

  void FullScanProcess::process()
  {
    SSHWorker SSHWorker_o(_HostName_str, ControllerLogin::GetUserName_str(), ControllerLogin::GetPassword_str());
   qWarning()<<"drweb request:" << _ProcessQuery_str;
    if(SSHWorker_o.ExecCommandOnHost_b(_ProcessQuery_str)) {
      QStringList Output_lst = SSHWorker_o.GetLastOutput_lst();
      QString res_str_answer_="";
      for(auto &p: Output_lst)
        res_str_answer_ += " " + p;
      qint32 Errors_i32 = 0;
      if(_ProcessQuery_str.indexOf("drweb") != -1) {
        qWarning()<<"drweb answer:" << res_str_answer_;
        //while read -r i; do    unknown=$(echo $i | grep -Po '(?<=No such file or directory: ).*'); if [ \"$unknown\" != \"\" ]; then echo \"Отсутствует файл или каталог: \"$unknown; fi;  done <<< \"$res\";
        QString drwAnalize ="res=\"" + res_str_answer_ + "\";  str=$(echo $res | grep -Po \"(\\d*)\");  while read -r i; do  case $var_scan in 0)  echo \"Проверено объектов: \"$i ;; 1) echo \"Ошибок проверки: \"$i ;; 2) echo \"Найдено угроз: \"$i ;; 3) echo \"Нейтрализовано угроз: \"$i ;; 4) echo \"Объем проверки (Кб): \"$i ;;  esac;  var_scan=$((var_scan+1));  done <<< $str;";
        QStringList Output_lst_drw;
        if(SSHWorker_o.ExecCommandOnHost_b(drwAnalize)) {
          Output_lst_drw = SSHWorker_o.GetLastOutput_lst();
        }
        qWarning()<<"drweb analize:" << Output_lst_drw;
        QStringList InfectOutput_lst;
        QStringList InfectOutput_lst_DrWeb;
        bool total = false;
        foreach(const QString & OutLine_str, Output_lst) {
          if(OutLine_str.contains("Scanned objects:")) {
            total = true;
          }
          if(!total) {
            InfectOutput_lst_DrWeb.push_back(OutLine_str);
          }
          if(OutLine_str.indexOf("ScanEngine is not available") != -1) {
            if(OutLine_str.indexOf("No such file or directory") != -1)
              InfectOutput_lst.push_back(OutLine_str);
            Errors_i32 = -2; break; // Это ошибка лицензии, можно выходить сразу
          } else if (OutLine_str.indexOf("Scanned objects:") != -1) {
            QRegExp rx("(\\d+)");
            QStringList list;
            int pos = 0;
            while ((pos = rx.indexIn(OutLine_str, pos)) != -1) {
              list << rx.cap(1);
              pos += rx.matchedLength();
            }
            Errors_i32 = list.size() >= 4 ? list[2].toInt() : 0;
          }
          InfectOutput_lst.push_back(OutLine_str);
        }

        InfectOutput_lst_DrWeb.push_back("--------------------------------");
        for (auto &OutLine_str_dr: Output_lst_drw) {
          InfectOutput_lst_DrWeb.push_back(OutLine_str_dr);
        }
        emit Output_sig(_HostName_str, InfectOutput_lst_DrWeb, Errors_i32);
      } else if (_ProcessQuery_str.indexOf("kesl") != -1) {
        QStringList Output_lst_kav;
        QString test_av_kav = "";
        int type_scan = -1;
        int kav_dngr = 0;
        if (_ProcessQuery_str.contains("fast=1")){
          type_scan = 0;
          test_av_kav = "test=\"" + res_str_answer_ + "\"; echo \"Тип проверки: \"$(echo $test | grep -Po \"(?<=TaskType=).*\" | cut -d ' ' -f1);  echo \"Дата  проверки: \"$(echo $test | grep -Po \"(?<=Date=).*\" | cut -d 'D' -f1);  echo \"Номер задания: \"$(echo $test | grep -Po \"(?<=EventId=).*\" | cut -d ' ' -f1); ";
          if(SSHWorker_o.ExecCommandOnHost_b(test_av_kav)) {
            Output_lst_kav = SSHWorker_o.GetLastOutput_lst();
          }
        }
        if (_ProcessQuery_str.contains("full=1")){
          type_scan = 1;
        }
        if (_ProcessQuery_str.contains("select=1")){
          type_scan = 2;
        }


        foreach(const QString & OutLine_str, Output_lst) {
          if(OutLine_str.contains("Ошибка задачи: Защита отключена")) {
            Errors_i32 = -1; break; // Это ошибка лицензии, можно выходить сразу
          } else if (OutLine_str.contains("Всего обнаружено объектов")) {
            QStringList Errors_lst = OutLine_str.split(": ");
            if(Errors_lst.size() == 2) {
              Errors_i32 = Errors_lst.at(1).toInt();
            }
          } else if (OutLine_str.contains("DetectNameQty")) {  // При полном сканировании считаем строки
            QStringList Errors_lst = OutLine_str.split("=");
            Errors_i32 = Errors_lst.size() >= 2 ? OutLine_str.split("=")[1].toInt() : 0;
          }

          if(type_scan == 1){
              if (OutLine_str.contains("DetectName=")){  // При полном сканировании считаем строки
                   QStringList Errors_lst = OutLine_str.split("=");
                   QString res_answer =  "Тип угрозы: " + QString(OutLine_str.split("=")[1]);
                   Output_lst_kav.append(res_answer);
                   kav_dngr++;
              }
              if (OutLine_str.contains("FileName")){  // При полном сканировании считаем строки
                   QStringList Errors_lst = OutLine_str.split("=");
                   QString res_answer =  "Зараженный файл: " + QString(OutLine_str.split("=")[1]);
                   Output_lst_kav.append(res_answer);
                   kav_dngr++;
              }
              if (OutLine_str.contains("DetectNameQty")){  // При полном сканировании считаем строки
                   QStringList Errors_lst = OutLine_str.split("=");
                   QString res_answer =  "Количество угроз: " + QString(OutLine_str.split("=")[1]);
                   Output_lst_kav.append(res_answer);
              }

          }
        }

        //FAST
        if(type_scan == 0 || type_scan == 1)
            Output_lst = Output_lst_kav;


        if(Errors_i32 == 0 || (type_scan==1 && kav_dngr==0)) {
          Output_lst.append("Угроз не обнаружено!");
        }
        emit Output_sig(_HostName_str, Output_lst, Errors_i32);
      }
      emit finished();
    }
  }

  void FullScanProcess::stop()
  {

  }
}
