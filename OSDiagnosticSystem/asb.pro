# This project demonstrates how to use QtWebAppLib by including the
# sources into this project.

TARGET = asb
TEMPLATE = app
QT = core network sql
CONFIG += console
VERSION = 1.0.0

build_no_astra {
    DEFINES += __NO_ASTRA_LINUX__
}

HEADERS +=  src/Common/startup.h \
            src/Common/databaseworker.h \
            src/Common/requesthandler.h \
            src/Common/osdsapi.h \
            src/Controllers/AntivirusRoutine/antivinstallprocess.h \
            src/Controllers/AntivirusRoutine/antivremoveprocess.h \
            src/Controllers/AntivirusRoutine/antivupdateprocess.h \
            src/Controllers/AntivirusRoutine/fullscanprocess.h \
            src/Controllers/ControllerAction.h \
            src/Controllers/ControllerBackup.h \
            src/Controllers/ControllerIntegrityControl.h \
            src/Controllers/ControllerLogin.h \
            src/Controllers/ControllerSettings.h \
            src/Controllers/ControllerUsers.h \
            src/Controllers/ControllerSecurity.h \
            src/Controllers/ControllerJournals.h \
            src/Controllers/ControllerAntivirus.h \
            src/Controllers/ControllerALD.h \
            src/Controllers/IntegrityControlRoutine/integritycontrolcreate.h \
            src/Controllers/IntegrityControlRoutine/integritycontroltest.h \
            src/Controllers/TestRoutine/testhostprocess.h \
            src/Controllers/BackupRoutine/backupcommon.h \
            src/Pipes/PipeUsers.h \
            src/Pipes/PipeSecurity.h \
            src/Pipes/PipeLogin.h \
            src/Pipes/PipeJournals.h \
            src/Pipes/PipeAntivirus.h \
            src/Pipes/PipeALD.h \
            src/Pipes/PipeAction.h \
            src/Common/osdsapi.h \
            src/Factory/FactoryAbstract.h \
            src/Factory/FactoryController.h \
            src/Factory/FactoryPipe.h \
            src/Common/piperequesthandler.h \
            src/Common/pipelistener.h \
            src/Pipes/PipeAbstract.h \
            src/Common/validation.h \
            src/Controllers/ControllerAbstract.h \
            src/Common/constants.h \
            src/Common/sshworker.h \
            src/Common/osdsexception.h \
            src/Common/dbqueriesstrings.h \
            src/Controllers/ProtectedFolders/controllerprotectedfolder.h \
            src/Controllers/ProtectedFolders/protectfolderwork.h \
            src/Controllers/ProtectedFolders/sequentialguid.h \
            src/Controllers/AuditProcessing/auditprocess.h \
            src/Controllers/controllerdomainpolicy.h \
            src/Controllers/HostTrace/HostTrace.h

SOURCES +=  src/main.cpp \
            src/Common/databaseworker.cpp \
            src/Common/startup.cpp \
            src/Common/requesthandler.cpp \
            src/Common/osdsapi.cpp \
            src/Controllers/AntivirusRoutine/antivinstallprocess.cpp \
            src/Controllers/AntivirusRoutine/antivremoveprocess.cpp \
            src/Controllers/AntivirusRoutine/antivupdateprocess.cpp \
            src/Controllers/AntivirusRoutine/fullscanprocess.cpp \
            src/Controllers/ControllerAction.cpp \
            src/Controllers/ControllerBackup.cpp \
            src/Controllers/ControllerIntegrityControl.cpp \
            src/Controllers/ControllerLogin.cpp \
            src/Controllers/ControllerSettings.cpp \
            src/Controllers/ControllerUsers.cpp \
            src/Controllers/ControllerSecurity.cpp \
            src/Controllers/ControllerJournals.cpp \
            src/Controllers/ControllerAntivirus.cpp \
            src/Controllers/ControllerALD.cpp \
            src/Controllers/IntegrityControlRoutine/integritycontrolcreate.cpp \
            src/Controllers/IntegrityControlRoutine/integritycontroltest.cpp \
            src/Controllers/TestRoutine/testhostprocess.cpp \
            src/Controllers/BackupRoutine/backupcommon.cpp \
            src/Pipes/PipeUsers.cpp \
            src/Pipes/PipeSecurity.cpp \
            src/Pipes/PipeLogin.cpp \
            src/Pipes/PipeJournals.cpp \
            src/Pipes/PipeAntivirus.cpp \
            src/Pipes/PipeALD.cpp \
            src/Pipes/PipeAction.cpp \
            src/Factory/FactoryAbstract.cpp \
            src/Factory/FactoryController.cpp \
            src/Factory/FactoryPipe.cpp \
            src/Common/piperequesthandler.cpp \
            src/Common/pipelistener.cpp \
            src/Pipes/PipeAbstract.cpp \
            src/Common/validation.cpp \
            src/Controllers/ControllerAbstract.cpp \
            src/Common/sshworker.cpp \
            src/Controllers/ProtectedFolders/controllerprotectedfolder.cpp \
            src/Controllers/ProtectedFolders/protectfolderwork.cpp \
            src/Controllers/ProtectedFolders/sequentialguid.cpp \
            src/Controllers/AuditProcessing/auditprocess.cpp \
            src/Controllers/controllerdomainpolicy.cpp \
            src/Controllers/HostTrace/HostTrace.cpp

OTHER_FILES += logs/* README.md

INCLUDEPATH +=  src \
                src/Controllers \
                src/Controllers/AntivirusRoutine \
                src/Controllers/BackupRoutine \
                src/Controllers/TestRoutine \
                src/Controllers/IntegrityControlRoutine \
                src/Controllers/ProtectedFolders \
                src/Controllers/AuditProcessing  \
                src/Controllers/HostTrace \
                src/Pipes \
                src/Factory \
                src/Common


unix {
  LIBS += -lcrypt -lssh
}

#---------------------------------------------------------------------------------------
# The following lines include the sources of the QtWebAppLib library
#---------------------------------------------------------------------------------------

include(libs/QtWebApp/qtservice/qtservice.pri)
include(libs/QtWebApp/httpserver/httpserver.pri)
include(libs/QtWebApp/logging/logging.pri)
include(libs/QtWebApp/templateengine/templateengine.pri)

DISTFILES += \
    etc/asb.ini \
    etc/docroot/index.html \
    docs/Diagramms/rootdiagramm.qmodel

RESOURCES += \
  res/extresource.qrc
