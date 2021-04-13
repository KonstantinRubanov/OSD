#include <QFileSystemWatcher>
#include <QSettings>
#include <QFile>
#include <QThread>
#include <QDebug>

#include "piperequesthandler.h"
#include "FactoryPipe.h"
#include "PipeAbstract.h"

namespace osds {

  PipeRequestHandler::PipeRequestHandler(FactoryPipe * FactoryPipe_po, const QString & PipeRequest_str, QObject * parent ) : QObject(parent),
    _FactoryPipe_po(FactoryPipe_po)
  {
    _PipeRequest_lst = PipeRequest_str.split(";");
  }

  void PipeRequestHandler::process()
  {
    PipeAbstract * Pipe_po = _FactoryPipe_po->BuildController_po(_PipeRequest_lst.at(0));
    if(Pipe_po) {
      Pipe_po->service(_PipeRequest_lst);
    }

    emit finished();
  }

  void PipeRequestHandler::stop()
  {

  }

} // namespace osds


