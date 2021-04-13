/**
  @file
  @author Stefan Frings
*/
#include <QLockFile>
#include <QDir>
#include <QDebug>

#include "startup.h"

/** @brief Точка входа */
int main(int argc, char *argv[])
{
  system("echo \"Version N 1.00\" > /tmp/osds_version.txt");
  QLockFile LockFile_o(QDir::temp().absoluteFilePath("osdssingle.lock"));
  /* Пытаемся закрыть Lock File, если попытка безуспешна в течение 100 миллисекунд,
   * значит уже существует Lock File созданный другим процессом.
   * Следовательно, выбрасываем предупреждение и закрываем программу
   */
  if(!LockFile_o.tryLock(100)) {
    qCritical()<<"Program OSDS already is start!!!";
    return 1;
  }
  Q_INIT_RESOURCE(extresource);
  // Запускается либо как программа, либо как сервис
  osds::Startup startup(argc, argv);
  return startup.exec();
}
