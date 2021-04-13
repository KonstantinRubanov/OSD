#ifndef TESTHOSTPROCESS_H
#define TESTHOSTPROCESS_H

#include <QObject>
#include <QDateTime>

namespace osds {
  /**
 * @brief The TestHostProcess class Проверка всего хоста на вирусы
 */
  class TestHostProcess  : public QObject {
    Q_OBJECT

    QString _HostName_str; ///< @brief Имя хоста

  public:
    /**
   * @brief TestHostProcess Основной конструктор
   * @param AntivirusType_str Тип антивируса
   * @param parent родитель
   */
    TestHostProcess(const QString & Host_str, QObject * parent = Q_NULLPTR);

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
#endif // TESTHOSTPROCESS_H
