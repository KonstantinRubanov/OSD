#ifndef ANTIVUPDATEPROCESS_H
#define ANTIVUPDATEPROCESS_H

#include <QObject>

#include "constants.h"

namespace osds {
  /**
 * @brief The AntivUpdateProcess class Обновление баз антивируса из URL
 */
  class AntivUpdateProcess : public QObject {
    Q_OBJECT

    QString _HostName_str; ///< @brief Имя хоста
    QString _AntivirusType_str; ///< @brief Строка запуска сканирования
    QString _UpdateUrl_str; ///< @brief Путь к обновлению
    QMap<QString, QStringList> * _Output_map = nullptr; ///< @brief Выходная информация (только проблемы)

  public:
    /**
   * @brief AntivUpdateProcess Основной конструктор
   * @param AntivirusType_str Тип антивируса
   * @param UpdateUrl_str Путь к обновлению
   * @param parent родитель
   */
    AntivUpdateProcess(const QString & Host_str, const QString & AntivirusType_str, const QString UpdateUrl_str, QObject * parent = Q_NULLPTR);

  public slots:
    /** @brief создает и запускает процесс сканирования */
    void process();
    /** @brief останавливает процесс сканирования */
    void stop();

  signals:
    /** @brief сигнал о завершении  работы */
    void finished();
    /** @brief Output_sig Выдает состояние true - обновлено */
    void Output_sig(bool, AntivirusCodeErrors);
  };
}
#endif // ANTIVUPDATEPROCESS_H
