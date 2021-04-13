#ifndef FULLSCANPROCESS_H
#define FULLSCANPROCESS_H

#include <QObject>

namespace osds {
  /**
 * @brief The FullScanProcess class Проверка всего хоста на вирусы
 */
  class FullScanProcess  : public QObject {
    Q_OBJECT

    QString _ProcessString_str; ///< @brief Строка с командой запуска
    QString _HostName_str; ///< @brief Имя хоста
    QString _ProcessQuery_str; ///< @brief Строка запуска сканирования
    QMap<QString, QStringList> * _Output_map = nullptr; ///< @brief Выходная информация (только проблемы)

  public:
    /**
   * @brief FullScanProcess Основной конструктор
   * @param AntivirusType_str Тип антивируса
   * @param parent родитель
   */
    FullScanProcess(const QString & Host_str, const QString & AntivirusType_str, QObject * parent = Q_NULLPTR);

  public slots:
    /** @brief создает и запускает процесс сканирования */
    void process();
    /** @brief останавливает процесс сканирования */
    void stop();

  signals:
    /** @brief сигнал о завершении  работы */
    void finished();
    /** @brief Output_sig Выдает состояние */
    void Output_sig(QString, QStringList, qint32);
  };
}
#endif // FULLSCANPROCESS_H
