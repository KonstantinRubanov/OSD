#ifndef PIPEREQUESTHANDLER_H
#define PIPEREQUESTHANDLER_H


#include <QObject>


namespace osds {

  class FactoryPipe;

  class PipeRequestHandler : public QObject
  {
    Q_OBJECT

  public:
    PipeRequestHandler(FactoryPipe * FactoryPipe_po, const QString & PipeRequest_str, QObject * parent = Q_NULLPTR);

  private:
    FactoryPipe * _FactoryPipe_po;  ///< @brief Фабрика pipe-контроллеров
    QStringList _PipeRequest_lst;

  public slots:
    /** @brief создает и запускает pipe-контроллер */
    void process();
    /** @brief останавливает pipe-контроллер */
    void stop();

  signals:
    /** @brief сигнал о завершении  работы */
    void finished();
  };

} // namespace osds

#endif // PIPEREQUESTHANDLER_H
