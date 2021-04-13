/**
  @file
  @author Stefan Frings
*/

#ifndef STARTUP_H
#define STARTUP_H

#include <QtCore/QCoreApplication>

#include "qtservice.h"

namespace stefanfrings { class HttpListener; }

namespace osds
{

  class PipeListener;

  /** @brief The Startup class осуществляет запуск и остановку сервиса */
  class Startup : public QtService<QCoreApplication>
  {
  public:

    /** @brief Startup Конструктор
    * @param argc Количество параметров
    * @param argv Параметры */
    Startup(int argc, char *argv[]);

  protected:

    /** @brief Запускает сервис */
    void start();

    /** @brief Останавливат сервис
      * @warning Этот метод не вызывается если программа работате как открепленный сервис.
      * Ее можно убить либо сигналом kill -x или если произойдет вызов qFatal */
    void stop();

  private:
    stefanfrings::HttpListener * _Listener_po = nullptr;  ///< @brief Слушает связь по HTTP
    PipeListener * _PipeListener_po = nullptr;  ///< @brief Слушает разделяемую память
  };

}
#endif // STARTUP_H
