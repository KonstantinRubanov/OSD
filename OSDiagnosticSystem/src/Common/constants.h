#ifndef CONSTANTS_H
#define CONSTANTS_H

#define OSDS_VERSION "1.0.0"

#include <QString>

/** @brief Коды ошибок антивируса */
enum AntivirusCodeErrors {
  ACENone_en, ///< @brief Нет ошибок
  ACENoLicense_en, ///< @brief Нет рабочей лицензии
  ACENoUpdates_en,  ///< @brief Нет доступных обновлений
  ACEEngineFailed_en,  ///< @brief Проблема с движком антивируса
  ACESetUpdateSettings, ///< @brief Ошибка при установки новых настроек обновления
  ACESetUpdateProcess ///< @brief Ошибка при процессе обновления антивируса
};

/** @brief The EnHostRunState enum Состояние процесса */
enum EnHostRunState {
  _HRSStop_en,  ///< @brief Не запущен / остановлен
  _HRSProcess_en,  ///< @brief В процессе выполнения
  _HRSError_en ///< @brief Ошбка выполнения
};

/** @brief The ThreadHostState class Класс с полным состоянием потока */
class ThreadHostState {
public:
  EnHostRunState _RunState_en = _HRSStop_en; ///< @brief Состояние процесса тестирования
  quint32 _ErrorsCount_u32 = 0; ///< @brief Результат прохождения теста (true - пройден успешно, false - не пройден)
  QString _Journal_str = ""; ///< @brief Журнал прохождения теста
  QString _CurrentDate_str = "";
  ThreadHostState() {}
  ThreadHostState(EnHostRunState RunState_en, quint32 ErrorsCount_u32 = false, const QString & Journal_str = QString(), const QString & Date_str = "") :
    _RunState_en(RunState_en),
    _ErrorsCount_u32(ErrorsCount_u32),
    _Journal_str(Journal_str),
    _CurrentDate_str(Date_str){}
};
#endif // CONSTANTS_H
