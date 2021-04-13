#include "FactoryAbstract.h"

FactoryAbstract::FactoryAbstract(QObject * parent) : QObject (parent)
{

}

void FactoryAbstract::AddController_v(const QString & Name_str, CreateControllerMethod Method_o)
{
  _CreateControllerMethod_map[Name_str] = Method_o;
}
