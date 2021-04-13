#ifndef BACKUPCOMMON_H
#define BACKUPCOMMON_H

#include <QMap>

class BackupCommon
{

public:
  /** @brief Ошибки при выполнении задач РК */
  enum EControllerBackupErrors {
    _CBENone_en,  ///< @brief Нет ошибок
    _CBECreateALDBackup_en, ///< @brief Ошибка при создании РК на сервере ALD
    _CBEReciveDataALDBackup_en, ///< @brief Ошибка при загрузке data файла РК ALD
    _CBEReciveKeyALDBackup_en,  ///< @brief Ошибка при загрузке key файла РК ALD
    _CBEDeleteALDBackupOnTemp_en, ///< @brief Ошибка при удалении файлов РК ALD
    _CBEChmodALDBackup_en, ///< @brief Ошибка при изменении прав доступа к файлам РК ALD
    _CBENoDataFile_en, ///< @brief Ошибка: нет файла данных РК ALD
    _CBENoKeyFile_en, ///< @brief Ошибка: нет файла РК ключей ALD
    _CBESendDataALDBackup_en, ///< @brief Ошибка при отправки data файла РК ALD
    _CBESendKeyALDBackup_en,  ///< @brief Ошибка при отправки key файла РК ALD
    _CBEALDBackupLoad_en,  ///< @brief Ошибка при восстановлении РК ALD
    _CBEProcessAlreadyRun ///< @brief Процесс отработки ALD-init уже запущен
  };

  explicit BackupCommon();

  /** @brief Возвращает текст ошибки */
  QString GetErrorText_str(EControllerBackupErrors);

private:
  QMap<EControllerBackupErrors, QString> _BacupErrorsDictionary_map;  ///< @brief Словарь ошибок при создании РК
};

#endif // BACKUPCOMMON_H
