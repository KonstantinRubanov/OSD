#include <QDir>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFileInfo>
#include <QFileInfoList>
#include <QTextStream>
#include <QLocale>
#include <QDebug>

#include "ControllerJournals.h"
#include "httpsessionstore.h"
#include "osdsapi.h"
#include "validation.h"
#include "constants.h"
#include <QUuid>

using namespace std;


using namespace stefanfrings;

namespace osds {

  QString ControllerJournals::_BackupPath_str = "/tmp";
  QString ControllerJournals::_TmpPath_str = "/tmp";

  QObject * CallbackCreateControllerJournals_po()
  {
    return new ControllerJournals;
  }

  ControllerJournals::ControllerJournals(QObject * parent) : ControllerAbstract (parent)
  {
    _ControllerName_str = "ControllerJournals";

    QLocale Locale_o(QLocale::English, QLocale::RussianFederation);
    for(int i = 1; i <= 12; ++i) {
      _MonthNamesByNumber_map[Locale_o.monthName(i, QLocale::ShortFormat)] = i;
    }
  }

  void ControllerJournals::service(HttpRequest& request, HttpResponse& response)
  { // Запрос времени с... по ... журналы
    if(request.getParameterMap().contains("get") && request.getParameter("get") == "data") {
      GetListLogs_v(response);
    } // Создание архива с журналом за выбранных период
    else if(request.getParameterMap().contains("set") && request.getParameter("set") == "archive") {
      if(request.getParameterMap().contains("start_date") && request.getParameterMap().contains("stop_date")) {
        SaveLogArchive_v(request.getParameter("start_date"), request.getParameter("stop_date"), response);
      }
    } // Вывод журнала на экран согласно фильтру
    else if(request.getParameterMap().contains("get") && request.getParameter("get") == "print") {
      if(request.getParameterMap().contains("start_date") && request.getParameterMap().contains("stop_date")) {
        PrintLogData_v(request.getParameter("start_date"), request.getParameter("stop_date"), response);
      }
    } // Удаление выбранного журнала
    else if(request.getParameterMap().contains("archive") && request.getParameter("archive") == "delete") {
      if(request.getParameterMap().contains("date_range")) {
        DeleteArchive_v(request.getParameter("date_range"), response);
      }
    } // Печать журнала
    else if(request.getParameterMap().contains("archive") && request.getParameter("archive") == "print") {
      if(request.getParameterMap().contains("date_range")) {
        PrintArchive_v(request.getParameter("date_range"), response);
      }
    }
  }

  void ControllerJournals::SetSettings_sv(QSettings & Settings_ro)
  {
    ControllerJournals::_BackupPath_str = Settings_ro.value("backupPathJournals").toString();
    ControllerJournals::_TmpPath_str = Settings_ro.value("tmpPath").toString();
  }

  void ControllerJournals::InitializeDataBase_sv()
  {

  }

  QPair<QString, QString> ControllerJournals::ParseAuthLogs_pair()
  {
    { //----- Удаляем старые файлы -----
      QDir TempDir_o(_TmpPath_str);
      TempDir_o.removeRecursively();
      if(!TempDir_o.mkdir(_TmpPath_str)) {
        qWarning("Directory not make: %s", _TmpPath_str.toStdString().c_str());
      }
    }
    { //----- Копируем логи в каталог -----
      QFileInfoList FilesInfo_lst = GetFileInfoList_lst(_AuthLogsPath, QStringList()<<"auth.log*");
      foreach(const QFileInfo & FileInfo_rc, FilesInfo_lst) { // Процесс копирования
        if(!QFile::copy(FileInfo_rc.absoluteFilePath(), QString(_TmpPath_str).append("/").append(FileInfo_rc.fileName()))){
          qWarning("Failure copy file: %s to path %s", FileInfo_rc.absoluteFilePath().toStdString().c_str(),
                   _TmpPath_str.toStdString().c_str());
        }
      }
    }
    { // Разархивируем файлы, при необходимости
      QFileInfoList FilesInfo_lst = GetFileInfoList_lst(_TmpPath_str, QStringList()<<"*.gz");
      foreach(const QFileInfo & FileInfo_rc, FilesInfo_lst) { // Процесс распвковки
        // распаковка с удалением исходника
        osdsapi->GetValidation_po()->ExecProcess_lst("gunzip " + FileInfo_rc.absoluteFilePath());
      }
    }
    // Вычленяем дату в числовом эквиваленте
    QFileInfoList FilesInfo_lst = GetFileInfoList_lst(_TmpPath_str);
    qint64 MinDate_i32 = 99999999, MaxDate_i32 = 0;
    foreach(const QFileInfo & FileInfo_rc, FilesInfo_lst) { // Процесс распвковки
      QFile File_o(FileInfo_rc.absoluteFilePath());
      if(!File_o.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Auth log file not openned! File = %s", FileInfo_rc.fileName().toStdString().c_str());
      }
      else {
        QTextStream Stream_o ( &File_o );
        while( !Stream_o.atEnd() ) {
          QString TempLine_str = Stream_o.readLine().left(6).replace("  ", " 0");
          QStringList ReadData_lst = TempLine_str.split(" ");
          if(ReadData_lst.size() == 2) {
            ReadData_lst[0] = QString::number(_MonthNamesByNumber_map[ReadData_lst[0]]).rightJustified(2, '0');
            qint64 Number_i32 = QString(QString::number(FileInfo_rc.created().date().year()) + ReadData_lst[0] +
                ReadData_lst[1]).toDouble();
            MinDate_i32 = qMin(MinDate_i32, Number_i32);
            MaxDate_i32 = qMax(MaxDate_i32, Number_i32);
          }
        }
        File_o.close(); // when your done.
      }
    }
    if (MinDate_i32 == 99999999 && MaxDate_i32 == 0) {
      return QPair<QString, QString>();
    }
    return QPair<QString, QString>(QString::number(MinDate_i32).insert(4, '-').insert(7,'-'),
                                   QString::number(MaxDate_i32).insert(4, '-').insert(7,'-'));
  }

  void ControllerJournals::GetListLogs_v(HttpResponse & response)
  {
    // Создадим папки для дальнейшей работы
    osdsapi->GetValidation_po()->ExecProcess_lst(QString("mkdir -p ") + _BackupPath_str);
    osdsapi->GetValidation_po()->ExecProcess_lst(QString("rm -rf ") + osdsapi->GetDocroot_str() + "/backup");
    osdsapi->GetValidation_po()->ExecProcess_lst(QString("ln -s ") + _BackupPath_str + " " + osdsapi->GetDocroot_str() + "/backup");
    int list_cnt = 0;
    QFileInfoList FilesInfo_lst = GetFileInfoList_lst(_BackupPath_str, QStringList()<<"*.*");     //получаем список файлов директории
    QJsonObject MainObject_o;
    SetJsonPackageHead_v(MainObject_o, "list_logs", "done");
    QJsonArray DataArray_o = MainObject_o["data"].toArray();
    foreach(const QFileInfo & FileInfo_rc, FilesInfo_lst) {
      if(FileInfo_rc.baseName().left(4) == "auth") {  // Только для архивов лога
        QString BaseName_str = FileInfo_rc.fileName().left(26);
        QStringList NameData_lst = BaseName_str.split("_");
        if(NameData_lst.size() >= 3) {  // Должно быть 3 элемента (auth, start_date, stop_date))
          QJsonObject ArrayObject_o;
          ArrayObject_o["type"] = "archive";
          ArrayObject_o["start_date"] = NameData_lst[1];
          ArrayObject_o["stop_date"] = NameData_lst[2];
          ArrayObject_o["create_date_time"] = FileInfo_rc.created().toString("yyyy-MM-dd hh:mm:ss");
          ArrayObject_o["link"] = "backup/" + FileInfo_rc.fileName();
          ArrayObject_o["num"]  = QString::number(list_cnt);
          DataArray_o.append(ArrayObject_o);
          list_cnt++;
        }
      }
    }
    QPair<QString, QString> LogDate_pair = ParseAuthLogs_pair();
    if(!LogDate_pair.first.isEmpty() && !LogDate_pair.second.isEmpty()) {
      QJsonObject ArrayObject_o;
      ArrayObject_o["type"] = "source";
      ArrayObject_o["start_date"] = LogDate_pair.first;
      ArrayObject_o["stop_date"]  = QDateTime::currentDateTime().toString("yyyy-MM-dd"); // LogDate_pair.second;
      ArrayObject_o["create_date_time"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
      DataArray_o.append(ArrayObject_o);
    }

    MainObject_o["data"] = DataArray_o;
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerJournals::SaveLogArchive_v(const QString & StartDate_str, const QString & StopDate_str,
                                            stefanfrings::HttpResponse& response)
  {
    qint64 StartNumber_i32 = 0;
    qint64 StopNumber_i32 =  0;
    QString State_str = "done";
    QJsonObject MainObject_o;
    if(StartDate_str.indexOf('.') > 0 && StopDate_str.indexOf('.') > 0) {
      StartNumber_i32 = QString(StartDate_str).remove(QChar('.'), Qt::CaseInsensitive).toDouble();
      StopNumber_i32 = QString(StopDate_str).remove(QChar('.'), Qt::CaseInsensitive).toDouble();
    }
    else if(StartDate_str.indexOf('-') > 0 && StopDate_str.indexOf('-') > 0) {
      StartNumber_i32 = QString(StartDate_str).remove(QChar('-'), Qt::CaseInsensitive).toDouble();
      StopNumber_i32 = QString(StopDate_str).remove(QChar('-'), Qt::CaseInsensitive).toDouble();
    }
    else {
      qWarning("Start date %s and end date %s is failure!", StartDate_str.toStdString().c_str(), StopDate_str.toStdString().c_str());
      SetJsonPackageHead_v(MainObject_o, "make_archive", "error");
      response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
      return;
    }
    ParseAuthLogs_pair();
    QFileInfoList FilesInfo_lst = GetFileInfoList_lst(_TmpPath_str);
    QFile TempFile_o("/tmp/auth.log");
    if(!TempFile_o.open(QIODevice::WriteOnly | QIODevice::Text)) {
      qWarning("Failure to open temp auth.log file");
      SetJsonPackageHead_v(MainObject_o, "make_archive", "error");
      response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
      return;
    }
    QTextStream WriteStream_o(&TempFile_o);
    for(int i = FilesInfo_lst.size() - 1; i >= 0; --i) { // Процесс распаковки
      const QFileInfo & FileInfo_rc = FilesInfo_lst[i];
      QFile File_o(FileInfo_rc.absoluteFilePath());
      if(!File_o.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Auth log file not openned! File = %s", FileInfo_rc.fileName().toStdString().c_str());
        SetJsonPackageHead_v(MainObject_o, "make_archive", "error");
        response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
        return;
      }
      else {
        QTextStream Stream_o ( &File_o );
        while( !Stream_o.atEnd() ) {
          QString TempLine_str =  Stream_o.readLine().replace("  ", " 0");
          QStringList ReadData_lst = TempLine_str.left(6).split(" ");
          if(ReadData_lst.size() == 2) {
            ReadData_lst[0] = QString::number(_MonthNamesByNumber_map[ReadData_lst[0]]).rightJustified(2, '0');
            qint64 Number_i32 = QString(QString::number(FileInfo_rc.created().date().year()) + ReadData_lst[0] +
                ReadData_lst[1]).toDouble();
            if(Number_i32 >= StartNumber_i32 && Number_i32 <= StopNumber_i32) {
              WriteStream_o << TempLine_str << '\n';
            }
          }
        }
        File_o.close(); // when your done.
      }
    }
    TempFile_o.close(); // Закончили заполнять файл
    QString NewFileName_str = "/tmp/auth_"  + StartDate_str + "_" + StopDate_str + "_" + QUuid::createUuid().toString() + ".log";
    if(QFile(NewFileName_str).exists()) { // Если такой уже есть
      QFile(NewFileName_str).remove();
      QFile(NewFileName_str + ".gz").remove();
      qDebug()<<"JOURNALS: Remove old files";
    }
    TempFile_o.rename(NewFileName_str);
    // Упаковка файла в архив
    osdsapi->GetValidation_po()->ExecProcess_lst(QString("gzip ").append(QFileInfo(TempFile_o).absoluteFilePath()));
    QFile GZFile_o(QFileInfo(TempFile_o).absoluteFilePath() + ".gz");
    NewFileName_str = _BackupPath_str + "/" + QFileInfo(GZFile_o).fileName();
    if(QFile(NewFileName_str).exists()) {
      QFile(NewFileName_str).remove();
      qDebug()<<"JOURNALS: Remove old backup files";
    }
    if(!GZFile_o.rename(NewFileName_str)) {
      qWarning()<<"Rename file crashed"<<GZFile_o.errorString();
      State_str = "error";
    }

    SetJsonPackageHead_v(MainObject_o, "make_archive", State_str);
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerJournals::PrintLogData_v(const QString & StartDate_str, const QString & StopDate_str,
                                          stefanfrings::HttpResponse& response)
  {
    QPair<QString, QString> LogDate_pair = ParseAuthLogs_pair();
    qint64 StartDate_i32 = QString(StartDate_str).remove(QChar('-'), Qt::CaseInsensitive).toDouble();
    qint64 StopDate_i32 = QString(StopDate_str).remove(QChar('-'), Qt::CaseInsensitive).toDouble();
    qint64 StartLog_i32 = LogDate_pair.first.remove(QChar('-'), Qt::CaseInsensitive).toDouble();
    qint64 StopLog_i32 = LogDate_pair.second.remove(QChar('-'), Qt::CaseInsensitive).toDouble();
    //----- Если только в логах - читаем и формируем json со всеми данными -----
    if(!LogDate_pair.first.isEmpty() && !LogDate_pair.first.isEmpty() && StartDate_i32 >= StartLog_i32 &&
       StopDate_i32 <= StopLog_i32) {
      QJsonObject MainObject_o;
      SetJsonPackageHead_v(MainObject_o, "print_log", "done");
      QJsonArray DataArray_o = MainObject_o["data"].toArray();
      QFileInfoList FilesInfo_lst = GetFileInfoList_lst(_TmpPath_str);
      for(int i = FilesInfo_lst.size() - 1; i >= 0; --i) {
        QFileInfo & FileInfo_ro = FilesInfo_lst[i];
        QFile File_o(FilesInfo_lst[i].absoluteFilePath());
        if(File_o.open(QIODevice::ReadOnly | QIODevice::Text)) {
          QTextStream TextStream_o(&File_o);
          while( !TextStream_o.atEnd() ) {
            QString ReadLine_str = TextStream_o.readLine().replace("  ", " 0");
            QStringList ReadData_lst = ReadLine_str.left(6).split(" ");
            if(ReadData_lst.size() == 2) {
              ReadData_lst[0] = QString::number(_MonthNamesByNumber_map[ReadData_lst[0]]).rightJustified(2, '0');
              qint64 LineDate_i32 = QString(QString::number(FileInfo_ro.created().date().year()) + ReadData_lst[0] +
                  ReadData_lst[1]).toDouble();
              if(LineDate_i32 >= StartDate_i32 && LineDate_i32 <= StopDate_i32) { // Запишем в Json
                QJsonObject ArrayObject_o;
                ArrayObject_o["string"] = ReadLine_str;
                DataArray_o.append(ArrayObject_o);
              }
            }
          }
          File_o.close();
        }
      }
      MainObject_o["data"] = DataArray_o;
      response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
      return;
    }
    else {  // Теперь ищем внутри архивных файлах
      QFileInfoList FilesInfo_lst = GetFileInfoList_lst(_BackupPath_str);
      foreach(const QFileInfo & QFileInfo_rc, FilesInfo_lst) {
        QStringList Dates_lst = QFileInfo_rc.fileName().left(26).split("_");
        if(Dates_lst.size() == 4 && Dates_lst[0].left(4) == "auth") { // Первичная проверка
          QFile TempFile_o(QFileInfo_rc.absoluteFilePath());
          qint64 ArchStartDate_i32 = Dates_lst[1].remove(QChar('-'), Qt::CaseInsensitive).toDouble();
          qint64 ArchStopDate_i32 = Dates_lst[2].remove(QChar('-'), Qt::CaseInsensitive).toDouble();
          if(StartDate_i32 >= ArchStartDate_i32 && StopDate_i32 <= ArchStopDate_i32) {
            QDir TempDir_o(_TmpPath_str);
            TempDir_o.removeRecursively(); // Удаляем старые файлы
            if(!TempDir_o.mkdir(_TmpPath_str)) {
              qWarning("Directory not make: %s", _TmpPath_str.toStdString().c_str());
            }
            QString CopyFileName_str = _TmpPath_str + "/" + QFileInfo(TempFile_o).fileName();
            TempFile_o.copy(CopyFileName_str);
            osdsapi->GetValidation_po()->ExecProcess_lst("gunzip " + CopyFileName_str);
            CopyFileName_str = CopyFileName_str.left(CopyFileName_str.size() - 3);
            QJsonObject MainObject_o;
            SetJsonPackageHead_v(MainObject_o, "print_log", "done");
            QFile File_o(CopyFileName_str);
            if(!File_o.open(QIODevice::ReadOnly | QIODevice::Text)) {
              qWarning("Auth log file not openned! File = %s", File_o.fileName().toStdString().c_str());
            }
            QJsonArray DataArray_o = MainObject_o["data"].toArray();
            QTextStream Stream_o(&File_o);
            while( !Stream_o.atEnd() ) {
              QString ReadLine_str = Stream_o.readLine().replace("  ", " 0");
              QStringList ReadData_lst = ReadLine_str.left(6).split(" ");
              if(ReadData_lst.size() == 2) {
                ReadData_lst[0] = QString::number(_MonthNamesByNumber_map[ReadData_lst[0]]).rightJustified(2, '0');
                qint64 Number_i32 = QString(QString::number(QFileInfo_rc.created().date().year()) + ReadData_lst[0] +
                    ReadData_lst[1]).toDouble();
                if(Number_i32 >= StartDate_i32 && Number_i32 <= StopDate_i32 &&
                   ReadLine_str.indexOf(" aldd: GSSAPI client step ") == -1 &&
                   ReadLine_str.indexOf(" ald-admin: GSSAPI client step ") == -1) {
                  QJsonObject ArrayObject_o;
                  ArrayObject_o["string"] = ReadLine_str;
                  DataArray_o.append(ArrayObject_o);
                }
              }
            }
            MainObject_o["data"] = DataArray_o;
            response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
            break;
          }
        }
      }
    }
  }

  void ControllerJournals::DeleteArchive_v(const QString & ArchiveRange_str, stefanfrings::HttpResponse& response)
  {
    QFile RemovedFile_o(_BackupPath_str + "/" + ArchiveRange_str);
    bool IsRemove_b = false;
    if(RemovedFile_o.exists()) {
      if(RemovedFile_o.remove()) {
        IsRemove_b = true;
      }
    }
    QJsonObject MainObject_o;
    SetJsonPackageHead_v(MainObject_o, "delete_archive", IsRemove_b ? "done" : "error");
    response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
  }

  void ControllerJournals::PrintArchive_v(const QString & ArchiveRange_str, stefanfrings::HttpResponse& response)
  {
    if(!QFile::exists(_BackupPath_str + "/" + ArchiveRange_str)) {
      QJsonObject MainObject_o;
      SetJsonPackageHead_v(MainObject_o, "print_log", "error");
      response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
    }
    else {
      QStringList Range_lst = ArchiveRange_str.split("_");
      if(Range_lst.size() == 4) {
        PrintLogData_v(Range_lst[1], Range_lst[2], response);
      }
      else {
        QJsonObject MainObject_o;
        SetJsonPackageHead_v(MainObject_o, "print_log", "error");
        response.write(QJsonDocument(MainObject_o).toJson(QJsonDocument::Indented));
      }
    }
  }

  QFileInfoList ControllerJournals::GetFileInfoList_lst(const QString & Path, QStringList Filters_lst)
  {
    QDir TempDir_o(Path);
    TempDir_o.setFilter(QDir::Files | QDir::NoSymLinks);
    if(Filters_lst.size() > 0) {
      auto entryList = TempDir_o.entryInfoList(Filters_lst, QDir::NoFilter, QDir::Time);
      std::sort(
        entryList.begin(),
        entryList.end(),
             [&](const QFileInfo &file1, const QFileInfo &file2)
             {
               return file1.created().toMSecsSinceEpoch() < file2.created().toMSecsSinceEpoch();

             });

         //qWarning()<<"list backups:";
        // foreach (auto &p, entryList) {
        //  qWarning()<<p.created();
        //};
      return entryList;
   }
   return TempDir_o.entryInfoList();
  }
}
