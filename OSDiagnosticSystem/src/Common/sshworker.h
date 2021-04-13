#ifndef SSHWORKER_H
#define SSHWORKER_H

#include <QObject>
#include <libssh/libssh.h>

namespace osds {

  class SSHWorker : public QObject
  {
    Q_OBJECT

    QString _LastError_str = "No error"; ///< @brief Последняя ошибка
    ssh_session _SSHSession_o; ///< @brief SSH сессия
    quint32 _BufferSize_u32 = 32000; ///< @brief Размер буфера для деления отправляемого файла
    QStringList _LastOutput_lst;  ///< @brief Вывод последней команды

  public:
    /**
     * @brief SSHWorker Создает объект для обработки SSH соединения
     * @param HostName_str Имя хоста для соединения (где запущен SSHD)
     * @param Login_str Имя пользователя для соединения
     * @param Password_str Пароль
     * @param parent Родительский объект */
    explicit SSHWorker(const QString & HostName_str, const QString & Login_str, const QString & Password_str, QObject *parent = nullptr);
    /**
     * @brief ~SSHWorker Деструктор */
    virtual ~SSHWorker();
    /**
     * @brief SendFileToHost_b Отправляет файл на сервер
     * @param FileName_str Имя файла с полным путем
     * @param PathName_str Папка на сервере, куда переслать файл
     * @return true - если все прошло успешно, false - не прошло (смотри GetLastError_str)
   */
    bool SendFileToHost_b(const QString & FileName_str, QString PathName_str);
    /**
     * @brief RecvFileFromHost_b Копирование файла с хоста
     * @param FileOnHost_str Полный путь до файла на хосте
     * @param FileName_str Полный путь с именем на локальной машине
     * @return true - если все прошло успешно, false - не прошло (смотри GetLastError_str)
     */
    bool RecvFileFromHost_b(const QString & FileOnHost_str, const QString & FileName_str);
    /**
     * @brief ExecCommandOnHost_b Выполнение команды на хосте
     * @param Command_str Команда
     * @return true - если все прошло успешно, false - не прошло (смотри GetLastError_str) */
    bool ExecCommandOnHost_b(const QString & Command_str);
    /**
     * @brief ExecCommandOnHost_b Выполнение команды на хосте
     * @param Command_str Команда
     * @param ExitStatus_ri Код завершения удаленного процесса
     * @return true - если все прошло успешно, false - не прошло (смотри GetLastError_str) */
    bool ExecCommandOnHost_b(const QString & Command_str, int & ExitStatus_ri);
    /**
     * @brief GetLastError_str Возвращает последнюю ошибку
     * @return Строка с ошибкой */
    QString GetLastError_str() const;
    /**
     * @brief GetLastOutput_lst Возвращает вывод последней команды
     * @return Набор строк вывода
     */
    QStringList GetLastOutput_lst() const { return _LastOutput_lst; }
  };

}
#endif // SSHWORKER_H
