#include <QProcess>
#include <QFile>
#include <QDebug>

#include "validation.h"

namespace osds
{
  Validation::Validation(QObject *parent) : QObject(parent)
  {

  }

  bool Validation::SetAutorization_b(const QString & Login_str, const QString & Password_str)
  {
#if !defined(__NO_ASTRA_LINUX__)
    QString CommandAutorization_str = CommandExpect_str(Login_str, Password_str, "user-list");
    QProcess Process_o(this);
    Process_o.start(CommandAutorization_str);
    if(!Process_o.waitForStarted(1000) || !Process_o.waitForFinished(30000)) {
      _IsValidParameters_b = false;
      _PasswordFileName_str = "";
      _Login_str = "";
      return false;
    }
    QString Output_str = QString::fromUtf8(Process_o.readAllStandardOutput());
    if(Output_str.indexOf(QObject::trUtf8("ОШИБКА:")) != -1) {  // Если ошибка найдена
      _IsValidParameters_b = false;
      _PasswordFileName_str = "";
      _Login_str = "";
      return false;
    }
#endif  //__NO_ASTRA_LINUX__
    _IsValidParameters_b = true;
    _PasswordFileName_str = "/tmp/osdsfile.pas";
    _Login_str = Login_str;
    _Password_str = Password_str;
    QFile PassFile_o(_PasswordFileName_str);
    if(PassFile_o.open(QIODevice::WriteOnly | QIODevice::Text)) {
      PassFile_o.write(QString(_Login_str + ":" + Password_str).toStdString().c_str());
      PassFile_o.close();
      PassFile_o.setPermissions(QFile::ReadOwner);
    }
    return true;
  }

  QStringList Validation::ALDProcess_lst(const QString & Command_str)
  {
    int ExitCode_i = 0;
    return ALDProcess_lst(Command_str, ExitCode_i);
  }

  QStringList Validation::ExecProcess_lst(const QString & Command_str)
  {
    int ExitCode_i = 0;
    return ExecProcess_lst(Command_str, ExitCode_i);
  }


  QStringList Validation::ALDProcess_lst(const QString & Command_str, int & ExitCode_i)
  {
#if !defined(__NO_ASTRA_LINUX__)
    if(!_IsValidParameters_b) {  // Только после прохождении авторизации
      _LastError_str = "No valid login and password";
      qWarning()<<_LastError_str;
      return QStringList();
    }
    QProcess Process_o(this);
    Process_o.start(this->CommandPassFile_str(Command_str));
    if(!Process_o.waitForStarted(1000) || !Process_o.waitForFinished(-1)) {
      _LastError_str = "Process run timeout";
      qWarning()<<_LastError_str;
      return QStringList();
    }
    QString Output_str = QString::fromUtf8(Process_o.readAllStandardOutput());
    if(Output_str.indexOf(QObject::trUtf8("ОШИБКА:")) != -1) {  // Если ошибка найдена
      _LastError_str = "Error run command!";
      qWarning()<<_LastError_str;
      return QStringList();
    }
    ExitCode_i = Process_o.exitCode();
    QStringList ReturnList_lst = Output_str.split("\n");
    ReturnList_lst.removeAll(QString(""));
    return ReturnList_lst;
#else //__NO_ASTRA_LINUX__
    QProcess Process_o(this);
    Process_o.start(Command_str);
    if(!Process_o.waitForStarted(1000) || !Process_o.waitForFinished(-1)) {
      _LastError_str = "Process run timeout";
      qWarning()<<_LastError_str<<":::"<<Command_str;
      return QStringList();
    }
    QString Output_str = QString::fromUtf8(Process_o.readAllStandardOutput());
    ExitCode_i = Process_o.exitCode();
    QStringList ReturnList_lst = Output_str.split("\n");
    ReturnList_lst.removeAll(QString(""));
    return ReturnList_lst;
#endif  //__NO_ASTRA_LINUX__
  }

  QStringList Validation::ALDExpectProcess_lst(const QString & Command_str, int & ExitCode_i)
  {
#if !defined(__NO_ASTRA_LINUX__)
    if(!_IsValidParameters_b) {  // Только после прохождении авторизации
      _LastError_str = "No valid login and password";
      qWarning()<<_LastError_str;
      return QStringList();
    }
    QProcess Process_o(this);
    Process_o.start(this->CommandExpect_str(_Login_str, _Password_str, Command_str));
    if(!Process_o.waitForStarted(1000) || !Process_o.waitForFinished(-1)) {
      _LastError_str = "Process run timeout";
      qWarning()<<_LastError_str;
      return QStringList();
    }
    QString Output_str = QString::fromUtf8(Process_o.readAllStandardOutput());
    if(Output_str.indexOf(QObject::trUtf8("ОШИБКА:")) != -1) {  // Если ошибка найдена
      _LastError_str = "Error run command!";
      qWarning()<<_LastError_str;
      return QStringList();
    }
    ExitCode_i = Process_o.exitCode();
    return Output_str.split("\n");
#else //__NO_ASTRA_LINUX__
    qDebug()<<"sudo ald-admin " + Command_str + " --admin=" + _Login_str;
    ExitCode_i = 0;
    return QStringList();
#endif  //__NO_ASTRA_LINUX__
  }

  QStringList Validation::ExecProcess_lst(const QString & Command_str, int & ExitCode_i)
  {
    QProcess Process_o(this);
    Process_o.start(Command_str);
    if(!Process_o.waitForStarted(1000) || !Process_o.waitForFinished(-1)) {
      _LastError_str = "Process run timeout";
      qWarning()<<_LastError_str<<":::"<<Command_str;
      return QStringList();
    }
    ExitCode_i = Process_o.exitCode();
    QString Output_str = QString::fromUtf8(Process_o.readAllStandardOutput() + Process_o.readAllStandardError());
    return Output_str.split("\n");
  }


  QString Validation::GetLastError_str()
  {
    QString Temp_str = _LastError_str;
    _LastError_str = "";
    return Temp_str;
  }

  QString Validation::CommandExpect_str(const QString &Login_str, const QString & Password_str, const QString & Command_str)
  {
    return QString("expect -c \" spawn ald-admin ") + Command_str + " --admin=" + Login_str + "; expect \"ALD:\" {send -- \"" + Password_str + "\\r\"}; expect;\"";
  }

  QString Validation::CommandPassFile_str(const QString & Command_str)
  {
    qDebug()<< QString("ald-admin ") + Command_str + " --admin=" + _Login_str + " --force --pass-file=" + _PasswordFileName_str;
    return QString("ald-admin ") + Command_str + " --admin=" + _Login_str + " --force --pass-file=" + _PasswordFileName_str;
  }
}
