#include <QFileSystemWatcher>
#include <QSettings>
#include <QFile>
#include <QThread>
#include <QDebug>

#include "pipelistener.h"
#include "piperequesthandler.h"
#include "FactoryPipe.h"

namespace osds {

  PipeListener::PipeListener(QSettings * Settings_po, QObject * parent) : QObject (parent),
    _FileWatcher_po(new QFileSystemWatcher(this)),
    _Settings_po(Settings_po),
    _FactoryPipe_po(new FactoryPipe(this))
  {
    QString FileName_str = _Settings_po->value("pfilename").toString();
    QFile WatchFile_o(FileName_str);  // Потрогаем файл
    WatchFile_o.open(QIODevice::WriteOnly);
    WatchFile_o.close();
    _FileWatcher_po->addPath(FileName_str);
    connect(_FileWatcher_po, SIGNAL(fileChanged(const QString &)), this, SLOT(PipeWatch_slt(const QString &)));
  }

  PipeListener::~PipeListener()
  {
    delete _FileWatcher_po;
    delete _Settings_po;
  }

  void PipeListener::PipeWatch_slt(const QString & Path_str)
  {
    QFile WatchFile_o(Path_str);
    if(WatchFile_o.open(QIODevice::ReadOnly | QIODevice::Text)) {
      PipeRequestHandler * PipeRequestHandler_po = new PipeRequestHandler(_FactoryPipe_po, WatchFile_o.readAll());
      QThread * PipeThread_po = new QThread;
      PipeRequestHandler_po->moveToThread(PipeThread_po);
      connect(PipeThread_po, SIGNAL(started()), PipeRequestHandler_po, SLOT(process()));
      connect(PipeRequestHandler_po, SIGNAL(finished()), PipeThread_po, SLOT(quit()));
      connect(PipeRequestHandler_po, SIGNAL(finished()), PipeRequestHandler_po, SLOT(deleteLater()));
      connect(PipeThread_po, SIGNAL(finished()), PipeThread_po, SLOT(deleteLater()));
      PipeThread_po->start();
    }
    WatchFile_o.close();
  }
} // namespace osds


