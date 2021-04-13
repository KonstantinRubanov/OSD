#ifndef VALIDATION_H
#define VALIDATION_H

#include <QObject>

namespace osds
{
  /**
   * @brief The Validation class Состояние авторизации пользователя
   * @details Сохраняет внутри себя логин/пароль и позволяет выполнять команды в отдельном процессе
   * с возвратом QStringList для детального разбора
   */
  class Validation : public QObject
  {
    Q_OBJECT

    QString _Login_str = ""; ///< @brief Имя пользователя
    QString _Password_str = ""; ///< @brief Пароль
    QString _PasswordFileName_str;  ///< @brief Файл с паролем
    bool _IsValidParameters_b = false;  ///< @brief Состояние прохождения авторизации (true - пройдена)
    QString _LastError_str; ///< @brief Последнее сообщение об ошибке

  public:
    /**
     * @brief Validation
     * @param parent
     */
    explicit Validation(QObject *parent = nullptr);
    /**
     * @brief SetAutorization_b Устанавливает параметры авторизации, только при прохождении проверки
     * @param Login_str Имя пользователя
     * @param Password_str  Пароль
     * @return Состояние пройдена ли авторизация
     */
    bool SetAutorization_b(const QString & Login_str, const QString & Password_str);
    /**
     * @brief ALDProcess_lst Выполняет заданную команду от имени администратора ALD (ald-admin *********** -admin=_Login_str)
     * @param Command_str Команда для выполнения
     * @return Вывод команды в формате листа, если size == 0, то команда не прошла
     */
    QStringList ALDProcess_lst(const QString & Command_str);

    /**
     * @brief ExecProcess_lst Выполняет заданную команду
     * @param Command_str Команда для выполнения
     * @return Вывод команды в формате листа, если size == 0, то команда не прошла
     */
    QStringList ExecProcess_lst(const QString & Command_str);

    /**
     * @brief ALDProcess_lst Выполняет заданную команду от имени администратора ALD (ald-admin *********** -admin=_Login_str)
     * @param Command_str Команда для выполнения
     * @return Вывод команды в формате листа, если size == 0, то команда не прошла
     */
    QStringList ALDProcess_lst(const QString & Command_str, int & ExitCode_i);
    /**
     * @brief ALDExpectProcess_lst Выполнение команды ald-admin с вводом логина и пароля через expect
     * @param Command_str Команда для выполнения
     * @param ExitCode_i Код возврата QProcess
     * @return Вывод команды в формате листа, если size == 0, то команда не прошла
     */
    QStringList ALDExpectProcess_lst(const QString & Command_str, int & ExitCode_i);

    /**
     * @brief ExecProcess_lst Выполняет заданную команду
     * @param Command_str Команда для выполнения
     * @return Вывод команды в формате листа, если size == 0, то команда не прошла
     */
    QStringList ExecProcess_lst(const QString & Command_str, int & ExitCode_i);

    /**
     * @brief GetLastError_str Возвращает последнее сообщение об ошибке
     * @return Строка с ошибкой
     */
    QString GetLastError_str();

  private:
    /**
     * @brief GetCommandLine_str Возвращает команду для выполнения в QProcess с использованием login|pass
     * @param Login_str Имя пользователя
     * @param Password_str Пароль
     * @param Command_str Команда
     * @return Строку с командой
     */
    QString CommandExpect_str(const QString & Login_str, const QString & Password_str, const QString & Command_str);
    /**
     * @brief Command_str Возвращает команду для выполнения в QProcess с использованием _PasswordFileName_str
     * @param Command_str Команда
     * @return Строку с командой
     */
    QString CommandPassFile_str(const QString & Command_str);
  };
}
#endif // VALIDATION_H
