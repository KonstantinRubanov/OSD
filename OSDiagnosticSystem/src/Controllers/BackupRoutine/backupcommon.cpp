#include "backupcommon.h"

BackupCommon::BackupCommon()
{
  _BacupErrorsDictionary_map[_CBENone_en] = "Нет ошибок";
  _BacupErrorsDictionary_map[_CBECreateALDBackup_en] = "Ошибка создания РК ALD";
  _BacupErrorsDictionary_map[_CBEReciveDataALDBackup_en] = "Ошика копирования РК ALD";
  _BacupErrorsDictionary_map[_CBEReciveKeyALDBackup_en] = "Ошибка копирования РК ключей ALD";
  _BacupErrorsDictionary_map[_CBEDeleteALDBackupOnTemp_en] = "Ошибка удаления временных файлов с сервера ALD";
  _BacupErrorsDictionary_map[_CBEChmodALDBackup_en] = "Ошибка изменения прав РК ALD";
  _BacupErrorsDictionary_map[_CBENoDataFile_en] = "Нет файла данных РК ALD";
  _BacupErrorsDictionary_map[_CBENoKeyFile_en] = "Нет файла РК ключей ALD";
  _BacupErrorsDictionary_map[_CBESendDataALDBackup_en] = "Ошика отправки РК ALD";
  _BacupErrorsDictionary_map[_CBESendKeyALDBackup_en] = "Ошибка отправки РК ключей ALD";
  _BacupErrorsDictionary_map[_CBEALDBackupLoad_en] = "Ошибка при восстановлении ALD из РК";
  _BacupErrorsDictionary_map[_CBEProcessAlreadyRun] = "ald-init еще выполняется на сервере";
}

QString BackupCommon::GetErrorText_str(BackupCommon::EControllerBackupErrors BackupError_en)
{
  QString ErrorText_str("Ошибка неизвестна");
  if(_BacupErrorsDictionary_map.contains(BackupError_en)) {
    ErrorText_str = _BacupErrorsDictionary_map[BackupError_en];
  }
  return ErrorText_str;
}
