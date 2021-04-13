#ifndef INTEGRITYCONTROLCREATE_H
#define INTEGRITYCONTROLCREATE_H

#include <QObject>
#include <QString>

namespace osds {
  /**
 * @brief The IntegrityControlCreate class Проверка всего хоста на вирусы
 */
  class IntegrityControlCreate  : public QObject {
    Q_OBJECT

    QString _HostName_str; ///< @brief Имя хоста
    QStringList _AddedDirs_str; ///< @brief Добавляемые каталоги

  public:
    /**
   * @brief IntegrityControlCreate Основной конструктор
   * @param parent родитель
   */
    IntegrityControlCreate(const QString & Host_str, const QStringList & Dirs_lst, QObject * parent = Q_NULLPTR);

  public slots:
    /** @brief создает и запускает процесс сканирования */
    void process();
    /** @brief останавливает процесс сканирования */
    void stop();

  signals:
    /** @brief сигнал о завершении  работы */
    void finished();
    /** @brief Output_sig Выдает состояние */
    void Output_sig(QString, quint32, QString);
  };
}
#endif // INTEGRITYCONTROLCREATE_H
