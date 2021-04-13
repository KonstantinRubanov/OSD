#ifndef OSDSEXCEPTION_H
#define OSDSEXCEPTION_H

#include <exception>
#include <string>

namespace osds {
  /** @enuum EnExceptionCode
   *  Коды исключений проекта OSDS
   */
  enum EnExceptionCode {
    ExCNone_en, ///< @brief Без кода
    ExCSSHConnecting, ///< @brief Попытка соединения по SSH
    ExCSSHAuthorization_en,  ///< @brief Авторизация SSH не пройдена
    ExCSQLNotOpenDB_en,  ///< @brief Открыть БД не получилось
    ExCSQLQueryFailed   ///< @brief Выполнить запрос не удалось
  };

  class OSDSException : public std::exception
  {
    std::string _Text_str;  ///< @brief Текст исключения
    EnExceptionCode _Code_en; ///< @brief Код исключения
  public:
    /**
     * @brief OSDSException Основной конструктор
     * @param Code_en Код исключения
     * @param Error_pcc Сообщение к исключению
     */
    OSDSException(EnExceptionCode Code_en, const char * Error_pcc) { _Code_en = Code_en; _Text_str = Error_pcc; }
    virtual ~OSDSException() override {}

    const char* what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT override { return _Text_str.c_str(); }
    EnExceptionCode code() { return _Code_en; }
  };

} // namespace osds

#endif // OSDSEXCEPTION_H
