#ifndef PIPEABSTRACT_H
#define PIPEABSTRACT_H

#include <QObject>
namespace osds {
  class PipeAbstract : public QObject
  {
  public:
    PipeAbstract(QObject * parent = Q_NULLPTR);
    virtual ~PipeAbstract();
    virtual void service(const QStringList & Parameters_lst) = 0;
  };
}
#endif // PIPEABSTRACT_H
