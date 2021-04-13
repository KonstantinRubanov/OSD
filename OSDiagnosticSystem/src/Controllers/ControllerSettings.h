#ifndef CONTROLLERSETTINGS_H
#define CONTROLLERSETTINGS_H

#include <QObject>
#include <QMutex>

#include "ControllerAbstract.h"

namespace osds {

  /** @brief CallbackCreateControllerSettings_po Создает объект класса ControllerSettings
 *  @return указатель на ControllerSettings (stefanfrings::HttpRequestHandler) */
  QObject * CallbackCreateControllerSettings_po();

  class ControllerSettings : public ControllerAbstract
  {
    Q_OBJECT
    Q_DISABLE_COPY(ControllerSettings)

    static QString SettingsFileName_sstr; ///< @brief Имя файла настроек

  public:
    ControllerSettings(QObject * parent = Q_NULLPTR);
    /**
     * @brief InitializeDataBase_sv Инициализация БД
     */
    static void InitializeDataBase_sv();

    void service(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response) override;
    /**
     * @brief SetSettingsFileName_sv Установка имени файла настроек
     * @param FileName_str Строка с именем
     */
    static void SetSettingsFileName_sv(const QString & FileName_str);

  private:
    /** @brief Формирует список настроек ПО */
    void GetSettingsList_v(stefanfrings::HttpResponse& response);
    /** @brief Устанавливает корректные настройки взятые от клиента */
    void SetSettingsList_v(const QString & SettingsList_str, stefanfrings::HttpResponse& response);
  };
}

#endif // CONTROLLERSETTINGS_H
