#ifndef ANTIVINSTALLPROCESS_H
#define ANTIVINSTALLPROCESS_H

#include <QObject>

namespace osds {

  class SSHWorker;

  /**
 * @brief The AntivInstallProcess class Обновление баз антивируса из URL
 */
  class AntivInstallProcess : public QObject {
    Q_OBJECT

    QString _HostName_str; ///< @brief Имя хоста
    QString _Distrib_str; ///< @brief Тип дистрибутива антивируса
    QString _DrWebPath_str; ///< @brief
    QString _KasperskyPath_str; ///< @brief
    QString _DrWebLicenseKey_str; ///< @brief
    QString _KasperskyLicenseKey_str; ///< @brief
    QString _KasperskyAutoinstall_str; ///< @brief
    QMap<QString, QStringList> * _Output_map = nullptr; ///< @brief Выходная информация (только проблемы)

  public:
    /**
   * @brief AntivInstallProcess Основной конструктор
   * @param AntivirusType_str Тип антивируса
   * @param UpdateUrl_str Путь к обновлению
   * @param parent родитель
   */
    AntivInstallProcess(const QString & Host_str, const QString & Distrib_str, const QString & DrWebPath_str,
                        const QString & KasperskyPath_str, const QString & DrWebLicenseKey_str,
                        const QString & KasperskyLicenseKey_str, const QString & KasperskyAutoinstall_str, QObject * parent = Q_NULLPTR);

  public slots:
    /** @brief создает и запускает процесс сканирования */
    void process();
    /** @brief останавливает процесс сканирования */
    void stop();

  private:
    /**
     * @brief InstallDrWeb_v Установка Dr.Web
     */
    void InstallDrWeb_v(SSHWorker & SSHWorker_ro);
    /**
     * @brief InstallDrWeb_v Установка Dr.Web
     */
    void InstallKaspersky_v(SSHWorker & SSHWorker_ro);

  signals:
    /** @brief сигнал о завершении  работы */
    void finished();
    /** @brief Output_sig Выдает состояние true - обновлено */
    void Output_sig(bool);
  };
}
#endif // ANTIVINSTALLPROCESS_H
