#ifndef INTEGRITYCONTROLTEST_H
#define INTEGRITYCONTROLTEST_H

#include <QObject>
#include <QMap>
#include <QPair>

namespace osds {
  /**
   * @brief The IntegrityControlTest class Проведение контроля целостности для данного хоста
   */
  class IntegrityControlTest  : public QObject {
    Q_OBJECT
    /** @enum EIntegrityControlException @brief Набор исключений*/
    enum EIntegrityControlException {
      _ICENone_en,  ///< @brief Нет исключений
      _ICETempFile_en, ///< @brief Ошибка при открытии временного файла
      _ICEReciveTempFile_en, ///< @brief Ошибка при загрузке временного файла
    };

    QString _HostName_str; ///< @brief Имя хоста
    QStringList _AddedDirs_str; ///< @brief Добавляемые каталоги
    bool _IsAudit_b; ///< @brief Стоит ли проводить проверку аудита
    const QString _TempFilename_str;  ///< @brief Временный файл
    QMap<EIntegrityControlException, QString> _ExceptionDict_map; ///< @brief Словарь с исключениями

  public:
    /**
   * @brief IntegrityControlTest Основной конструктор
   * @param AntivirusType_str Тип антивируса
   * @param parent родитель
   */
    IntegrityControlTest(const QString & Host_str, const QStringList & Dirs_lst, bool IsAudit_b = false, QObject * parent = Q_NULLPTR);

  public slots:
    /** @brief создает и запускает процесс сканирования */
    void process();
    /** @brief останавливает процесс сканирования */
    void stop();

  signals:
    /** @brief сигнал о завершении  работы */
    void finished();
    /** @brief Output_sig Выдает состояние */
    void Output_sig(QString, quint32, QString, QMap<QString, QPair<QString, QString> >);
  };
}

#endif // INTEGRITYCONTROLTEST_H
