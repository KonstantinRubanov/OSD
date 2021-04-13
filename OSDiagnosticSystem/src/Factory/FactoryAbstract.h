#ifndef FACTORYABSTRACT_H
#define FACTORYABSTRACT_H

#include <QObject>
#include <QMap>

namespace stefanfrings { class HttpRequestHandler; }

typedef QObject * (*CreateControllerMethod)();

class FactoryAbstract : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(FactoryAbstract)

public:
  FactoryAbstract(QObject * parent = Q_NULLPTR);

protected:
  void AddController_v(const QString & Name_str, CreateControllerMethod);

  QMap<QString, CreateControllerMethod> _CreateControllerMethod_map;  ///< @brief Хранилище фабричных методов
};

#endif // FACTORYABSTRACT_H
