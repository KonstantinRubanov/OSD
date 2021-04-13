#ifndef PIPELISTENER_H
#define PIPELISTENER_H


#include <QObject>

class QFileSystemWatcher;
class QSettings;
class QThread;

namespace osds {

  class FactoryPipe;

  class PipeListener : public QObject
  {
    Q_OBJECT
  public:
    /** PipeRequestHandler @brief Конструктор */
    PipeListener(QSettings * Settings_po, QObject * parent = nullptr);
    ~PipeListener();

  private:
    QFileSystemWatcher * _FileWatcher_po; ///< @brief Отслеживание изменения файла (pipe)
    QSettings * _Settings_po; ///< @brief Храним настройки
    FactoryPipe * _FactoryPipe_po;  ///< @brief Фабрика pipe-контроллеров

  private slots:
    /** @brief Отслеживание изменения в файле */
    void PipeWatch_slt(const QString &);
  };

} // namespace osds

#endif // PIPELISTENER_H
