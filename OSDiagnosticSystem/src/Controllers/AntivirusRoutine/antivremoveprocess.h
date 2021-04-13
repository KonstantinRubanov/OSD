#ifndef ANTIVREMOVEPROCESS_H
#define ANTIVREMOVEPROCESS_H

#include <QObject>

namespace osds {

  class SSHWorker;

  /**
 * @brief The AntivRemoveProcess class Обновление баз антивируса из URL
 */
  class AntivRemoveProcess : public QObject {
    Q_OBJECT

    QString _HostName_str; ///< @brief Имя хоста
    QString _Distrib_str; ///< @brief Тип дистрибутива антивируса
    QMap<QString, QStringList> * _Output_map = nullptr; ///< @brief Выходная информация (только проблемы)

  public:
    /**
   * @brief AntivRemoveProcess Основной конструктор
   * @param AntivirusType_str Тип антивируса
   * @param parent родитель
   */
    AntivRemoveProcess(const QString & Host_str, const QString & Distrib_str, QObject * parent = Q_NULLPTR);

  public slots:
    /** @brief создает и запускает процесс сканирования */
    void process();
    /** @brief останавливает процесс сканирования */
    void stop();

  private:
    /**
     * @brief InstallDrWeb_v Установка Dr.Web
     */
    void RemoveDrWeb_v(SSHWorker & SSHWorker_ro);
    /**
     * @brief InstallDrWeb_v Установка Dr.Web
     */
    void RemoveKaspersky_v(SSHWorker & SSHWorker_ro);

  signals:
    /** @brief сигнал о завершении  работы */
    void finished();
    /** @brief Output_sig Выдает состояние true - обновлено */
    void Output_sig(bool);
  };
}
#endif // ANTIVREMOVEPROCESS_H
