#include <QFile>
#include <QFileInfo>
#include <QException>
#include <libssh/sftp.h>
#include <fcntl.h>

#include "sshworker.h"
#include "osdsexception.h"

namespace osds {

  SSHWorker::SSHWorker(const QString & HostName_str, const QString & Login_str, const QString & Password_str, QObject *parent) : QObject(parent)
  {
    //----- Открытие сессии и задание настроек -----
    _SSHSession_o = ssh_new();
    if (_SSHSession_o == nullptr) {
      _LastError_str = "Failed to create ssh session";
      throw OSDSException(ExCSSHConnecting, _LastError_str.toUtf8());
    }
    ssh_options_set(_SSHSession_o, SSH_OPTIONS_HOST, HostName_str.toUtf8());
    //----- Присоединение к серверу -----
    int ReturnCode_i = ssh_connect(_SSHSession_o);
    if (ReturnCode_i != SSH_OK) {
      _LastError_str = "Error connecting to localhost:" + QString(ssh_get_error(_SSHSession_o));
      ssh_free(_SSHSession_o);
      throw OSDSException(ExCSSHConnecting, _LastError_str.toUtf8());
    }
    //----- Авторизация на сервере -----
    ReturnCode_i = ssh_userauth_password(_SSHSession_o, Login_str.toUtf8(), Password_str.toUtf8());
    if (ReturnCode_i != SSH_AUTH_SUCCESS) {
      _LastError_str = "Error authenticating with password: " + QString(ssh_get_error(_SSHSession_o));
      ssh_disconnect(_SSHSession_o);
      ssh_free(_SSHSession_o);
      throw OSDSException(ExCSSHAuthorization_en, _LastError_str.toUtf8());
    }
  }

  SSHWorker::~SSHWorker()
  {
    ssh_disconnect(_SSHSession_o);
    ssh_free(_SSHSession_o);
  }

  bool SSHWorker::SendFileToHost_b(const QString & FileName_str, QString PathName_str)
  {
    sftp_session SFTPSession_o = sftp_new(_SSHSession_o);
    if (SFTPSession_o == nullptr) {
      _LastError_str = "Error allocating SFTP session: " + QString(ssh_get_error(_SSHSession_o));
      return false;
    }
    int ReturnCode_i = sftp_init(SFTPSession_o);
    if (ReturnCode_i != SSH_OK) {
      _LastError_str = "Error initializing SFTP session: " + QString(sftp_get_error(SFTPSession_o));
      sftp_free(SFTPSession_o);
      return false;
    }
    QFile SendFile_o(FileName_str);
    if(PathName_str[PathName_str.size() - 1] != '/') {  // Если нет /, то добавим его
      PathName_str.append('/');
    }
    sftp_file SFTPFile_o = sftp_open(SFTPSession_o, QString(PathName_str + QFileInfo(SendFile_o).fileName()).toUtf8(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (SFTPFile_o == nullptr) {
      _LastError_str = "Can't open file for writing: " + QString(ssh_get_error(_SSHSession_o));
      return false;
    }
    if(SendFile_o.open(QIODevice::ReadOnly)) {
      qint64 FileSize_i64 = SendFile_o.size(), Ssize_i64 = 0;
      char * Buffer_pc = new char [_BufferSize_u32];  // Канонично, будет работать на любой системе
      while(FileSize_i64 - Ssize_i64 > 0) {
        qint64 ReadLength_i64 = SendFile_o.read(Buffer_pc, _BufferSize_u32);
        Ssize_i64 += ReadLength_i64;
        if(sftp_write(SFTPFile_o, Buffer_pc, static_cast<size_t>(ReadLength_i64)) != static_cast<ssize_t>(ReadLength_i64)) {
          _LastError_str = "Can't write data to file: " + QString(ssh_get_error(_SSHSession_o));
          sftp_close(SFTPFile_o);
          return false;
        }
      }
      delete [] Buffer_pc;
    }
    SendFile_o.close();
    ReturnCode_i = sftp_close(SFTPFile_o);
    if (ReturnCode_i != SSH_OK) {
      _LastError_str = "Can't close the written file: " + QString(ssh_get_error(_SSHSession_o));
      return false;
    }
    sftp_free(SFTPSession_o);
    return true;
  }

  bool SSHWorker::RecvFileFromHost_b(const QString & FileOnHost_str, const QString & FileName_str)
  {
    sftp_session SFTPSession_o = sftp_new(_SSHSession_o);
    if (SFTPSession_o == nullptr) {
      _LastError_str = "Error allocating SFTP session: " + QString(ssh_get_error(_SSHSession_o));
      return false;
    }
    int ReturnCode_i = sftp_init(SFTPSession_o);
    if (ReturnCode_i != SSH_OK) {
      _LastError_str = "Error initializing SFTP session: " + QString(sftp_get_error(SFTPSession_o));
      sftp_free(SFTPSession_o);
      return false;
    }
    sftp_file file = sftp_open(SFTPSession_o, FileOnHost_str.toUtf8(), O_RDONLY, 0);
    if (file == nullptr) {
      _LastError_str = QString("Can't open file for reading: ") + ssh_get_error(SFTPSession_o);
      sftp_free(SFTPSession_o);
      return false;
    }
    QFile ReciveFile_o(FileName_str);
    if(!ReciveFile_o.open(QIODevice::WriteOnly)) {
      _LastError_str = QString("Can't open file for writing: ");
      sftp_free(SFTPSession_o);
      return false;
    }
//    int fd = open(FileName_str.toUtf8(), O_CREAT);
//    if (fd < 0) {
//      _LastError_str = QString("Can't open file for writing: ") + strerror(errno);
//      sftp_free(SFTPSession_o);
//      return false;
//    }
    char buffer[16384];
    int nbytes, nwritten;
    for (;;) {
      nbytes = static_cast<int>(sftp_read(file, buffer, sizeof(buffer)));
      if (nbytes == 0) {
        break; // EOF
      } else if (nbytes < 0) {
        _LastError_str = QString("Error while reading file: ") + ssh_get_error(SFTPSession_o);
        ReciveFile_o.close();
        sftp_close(file);
        sftp_free(SFTPSession_o);
        return false;
      }
      nwritten = static_cast<int>(ReciveFile_o.write(buffer, static_cast<qint64>(nbytes)));
      if (nwritten != nbytes) {
        _LastError_str = QString("Error writing: ") + strerror(errno);
        sftp_close(file);
        sftp_free(SFTPSession_o);
        return false;
      }
    }
    if (sftp_close(file) != SSH_OK) {
      _LastError_str = QString("Can't close the read file: ") + ssh_get_error(SFTPSession_o);
      ReciveFile_o.close();
      sftp_free(SFTPSession_o);
      return false;
    }
    ReciveFile_o.close();
    sftp_free(SFTPSession_o);
    return true;
  }

  bool SSHWorker::ExecCommandOnHost_b(const QString & Command_str)
  {
    int ExitStatus_i = 0;
    return ExecCommandOnHost_b(Command_str, ExitStatus_i);
  }

  bool SSHWorker::ExecCommandOnHost_b(const QString & Command_str, int & ExitStatus_ri)
  {
    char Buffer_ac[256];
    ssh_channel Channel_o = ssh_channel_new(_SSHSession_o);
    if (Channel_o == nullptr) {
      _LastError_str = "Don't create SSH channel " + QString(ssh_get_error(_SSHSession_o));
      return false;
    }
    int ReturnCode_i = ssh_channel_open_session(Channel_o);
    if (ReturnCode_i != SSH_OK) {
      ssh_channel_free(Channel_o);
      _LastError_str = "Don't open channel session " + QString(ssh_get_error(_SSHSession_o));
      return false;
    }
    ReturnCode_i = ssh_channel_request_exec(Channel_o, Command_str.toUtf8());
    if (ReturnCode_i != SSH_OK) {
      ssh_channel_close(Channel_o);
      ssh_channel_free(Channel_o);
      _LastError_str = "Don't exec command " + QString(ssh_get_error(_SSHSession_o));
      return false;
    }
    ExitStatus_ri = ssh_channel_get_exit_status(Channel_o);
    int LengthRead_i = ssh_channel_read(Channel_o, Buffer_ac, sizeof(Buffer_ac), 0);
    _LastOutput_lst.clear();
    QByteArray Output_str;
    while (LengthRead_i > 0) {  // Чтение вывода
      Output_str.append(Buffer_ac, LengthRead_i);
      LengthRead_i = ssh_channel_read(Channel_o, Buffer_ac, sizeof(Buffer_ac), 0);
    }
    _LastOutput_lst = QString::fromUtf8(Output_str).split("\n");

    ssh_channel_send_eof(Channel_o);
    ssh_channel_close(Channel_o);
    ssh_channel_free(Channel_o);
    return true;
  }

  QString SSHWorker::GetLastError_str() const
  {
    return _LastError_str;
  }

}
