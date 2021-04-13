#include <QSqlError>
#include <QSqlQuery>
#include <QSettings>
#include <QDebug>
#include <QDir>

#include "databaseworker.h"
#include "osdsexception.h"

namespace osds {

  QString DatabaseWorker::_DataBaseName_str = "/tmp/osds.sqlite";
  QMutex DatabaseWorker::_SQLMutex_so;

  DatabaseWorker::DatabaseWorker(QObject *parent) : QObject(parent)
  {

  }

  void DatabaseWorker::SetSettings_sv(QSettings & Settings_ro)
  {
    DatabaseWorker::_DataBaseName_str = Settings_ro.value("databasename").toString();
    if (QDir::isRelativePath(DatabaseWorker::_DataBaseName_str))
    {
        QString PATH_var=QString(qgetenv("LD_LIBRARY_PATH"));
        if(PATH_var.isEmpty()) return;
        int nameIdx = PATH_var.indexOf("/");
        if(nameIdx){
          QStringRef refPath = PATH_var.midRef(nameIdx);
          QDir TestDir(refPath.toString());
          DatabaseWorker::_DataBaseName_str = TestDir.cleanPath(TestDir.absoluteFilePath(DatabaseWorker::_DataBaseName_str));
          qWarning()<<"Database PATH "<<DatabaseWorker::_DataBaseName_str;
        }
    }
  }

  QSqlQuery DatabaseWorker::QueryExec_lst(QString Query_str, bool IsExceptionQuery_b)
  {
    int ExitCode_i = 0;
    return QueryExecWithCode_lst(Query_str, ExitCode_i, IsExceptionQuery_b);
  }

  QSqlQuery DatabaseWorker::QueryExecWithCode_lst(QString Query_str, int & ExitCode_ri, bool IsExceptionQuery_b)
  {
    ExitCode_ri = 0;
    _SQLMutex_so.lock();
    QSqlDatabase QSqlDatabase_o;
    if(QSqlDatabase::contains(QSqlDatabase::defaultConnection)) {
      QSqlDatabase_o = QSqlDatabase::database();
    }
    else {
      QSqlDatabase_o = QSqlDatabase::addDatabase("QSQLITE");
    }
    QSqlDatabase_o.setDatabaseName(_DataBaseName_str);
    if(!QSqlDatabase_o.open()) {
      qWarning()<<"DATABASE ERROR: "<<QSqlDatabase_o.lastError().text();
      _SQLMutex_so.unlock();
      throw OSDSException(ExCSQLNotOpenDB_en, QString("Not openned ").append(_DataBaseName_str).toUtf8());
    }
    QSqlQuery QSqlQuery_o;
    if(!QSqlQuery_o.exec(Query_str)) {
      qWarning()<<"DatabaseWorker::QUERY: "<<Query_str<<"Error: "<<QSqlDatabase_o.lastError().text();
      ExitCode_ri = 1;
      if(IsExceptionQuery_b) {
        _SQLMutex_so.unlock();
        throw OSDSException(ExCSQLQueryFailed, QString("Not exec query: ").append(Query_str).toUtf8());
      }
    }
    QSqlDatabase_o.close();
    _SQLMutex_so.unlock();
    return QSqlQuery_o;
  }

  bool DatabaseWorker::QueryExec_lstExt(QString Query_str, bool IsExceptionQuery_b,  QSqlQuery& QSqlQuery_o)
  {
    _SQLMutex_so.lock();
    bool state = true;
    QSqlDatabase QSqlDatabase_o;
    if(QSqlDatabase::contains(QSqlDatabase::defaultConnection)) {
      QSqlDatabase_o = QSqlDatabase::database();
    }
    else {
      QSqlDatabase_o = QSqlDatabase::addDatabase("QSQLITE");
    }
    QSqlDatabase_o.setDatabaseName(_DataBaseName_str);
    if(!QSqlDatabase_o.open()) {
      qWarning()<<"DATABASE ERROR: "<<QSqlDatabase_o.lastError().text();
      _SQLMutex_so.unlock();
      state = false;
      throw OSDSException(ExCSQLNotOpenDB_en, QString("Not openned ").append(_DataBaseName_str).toUtf8());
    }
    if(!QSqlQuery_o.exec(Query_str)) {
      qWarning()<<"DatabaseWorker::QUERY: "<<Query_str<<"Error: "<<QSqlDatabase_o.lastError().text();
      if(IsExceptionQuery_b) {
        _SQLMutex_so.unlock();
        state = false;
        throw OSDSException(ExCSQLQueryFailed, QString("Not exec query: ").append(Query_str).toUtf8());
      }
    }
    else
      while(QSqlQuery_o.next());
    QSqlDatabase_o.close();
    _SQLMutex_so.unlock();
    return state;
  }

  QSqlQuery DatabaseWorker::QueryExec_lst(QString Query_str, QList <QPair<QString,QString>> BindList_list, bool IsExceptionQuery_b, QSql::ParamType paramType)
  {
    _SQLMutex_so.lock();
    QSqlDatabase QSqlDatabase_o;
    if(QSqlDatabase::contains(QSqlDatabase::defaultConnection)) {
      QSqlDatabase_o = QSqlDatabase::database();
    }
    else {
      QSqlDatabase_o = QSqlDatabase::addDatabase("QSQLITE");
    }
    QSqlDatabase_o.setDatabaseName(_DataBaseName_str);
    if(!QSqlDatabase_o.open()) {
      qWarning()<<"DATABASE ERROR: "<<QSqlDatabase_o.lastError().text();
      _SQLMutex_so.unlock();
      throw OSDSException(ExCSQLNotOpenDB_en, QString("Not openned ").append(_DataBaseName_str).toUtf8());
    }
    QSqlQuery QSqlQuery_o(QSqlDatabase_o);
    if (!QSqlQuery_o.prepare(Query_str) ){
      _SQLMutex_so.unlock();
      qWarning() << "SQLQUERY: PREPARE NOT SUCCESS: " << Query_str ;
      return QSqlQuery_o;
    }
    if (!BindList_list.empty()){
      for (auto i : BindList_list){
        QSqlQuery_o.bindValue(i.first, i.second, paramType);
      }
    }
    if(!QSqlQuery_o.exec()) {
      qWarning()<<"DatabaseWorker::QUERY: "<<Query_str<<"Error: "<<QSqlDatabase_o.lastError().text();
      if(IsExceptionQuery_b) {
        _SQLMutex_so.unlock();
        throw OSDSException(ExCSQLQueryFailed, QString("Not exec query: ").append(Query_str).toUtf8());
      }
    }
    QSqlDatabase_o.close();
    _SQLMutex_so.unlock();
    return QSqlQuery_o;
  }

  void DatabaseWorker::WriteAdministratorAction_v(const QString & Controller_str, const QString & Text_str)
  {

  }
}

