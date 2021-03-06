/*
    Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>),
    German Cancer Research Center/University Heidelberg

    

    This file is part of QuickFit 3 (http://www.dkfz.de/Macromol/quickfit).

    This software is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "qfespimb040mainwindow2.h"
#include<QtGlobal>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include "spimb040.h"
#include <tiffio.h>
#include "libtiff_tools.h"



#define sqr(a) qfSqr(a)
#define PROCESS_EVENTS_TIMEOUT_MS 20
#define SPIMACQUISITION_ERROR(message, title) \
    log_error(QString("  - ")+(message)+QString("\n")); \
    QMessageBox::critical(this, title, (message));


QFESPIMB040MainWindow2::QFESPIMB040MainWindow2(const QString &optSetupFile, QFPluginServices* pluginServices, QWidget* parent, bool newOpticsSetup, QFESPIMB040 *plugin):
    QWidget(parent, Qt::Dialog|Qt::WindowMaximizeButtonHint|Qt::WindowCloseButtonHint|Qt::WindowSystemMenuHint)
{
    this->plugin=plugin;
    this->optSetupFile=optSetupFile;
    widImageStack=NULL;
    widScriptedAcquisition=NULL;
    widCamParamScan=NULL;
    widDeviceParamScan=NULL;
    widExperimentDescription=NULL;
    widAcquisition=NULL;
    widOverview=NULL;
    //widConfig=NULL;
    optSetup1=NULL;
    optSetup2=NULL;
    optSetup=NULL;
    m_pluginServices=pluginServices;
    // create widgets and actions
    createWidgets(pluginServices->getExtensionManager(), newOpticsSetup);
    setWindowTitle("B040 SPIM Control");
    setWindowIcon(QIcon(":/spimb040_logo.png"));
    connect(plugin, SIGNAL(styleChanged(QString,QString)), this, SLOT(styleChanged(QString,QString)));
    plugin->updateFromConfig();
}

QFESPIMB040MainWindow2::~QFESPIMB040MainWindow2()
{
}

void QFESPIMB040MainWindow2::loadSettings(ProgramOptions* settings) {
    setUpdatesEnabled(false);
    QString prefix="plugin_spim_b040/";
    if (!optSetupFile.isEmpty()) {
        prefix=prefix+QFileInfo(optSetupFile).fileName()+QString("/");
    }
    jkloadWidgetGeometry((*settings->getQSettings()), this, prefix+"");
    jkloadSplitter((*settings->getQSettings()), splitter, prefix+"");
    if (optSetup1) optSetup1->loadSettings((*settings->getQSettings()), prefix+"instrument/");
    if (optSetup2) optSetup2->loadSettings((settings->getQSettings()->fileName()), prefix+"instrument_new/");

    if (widExperimentDescription) widExperimentDescription->loadSettings((*settings->getQSettings()), prefix+"expdescription/");
    if (widAcquisitionDescription) widAcquisitionDescription->loadSettings((*settings->getQSettings()), prefix+"acqdescription/");
    if (widScriptedAcquisition) widScriptedAcquisition->loadSettings((*settings->getQSettings()), prefix+"acqscripted/");
    if (widImageStack) widImageStack->loadSettings((*settings->getQSettings()), prefix+"image_stack/");
    if (widAcquisition) widAcquisition->loadSettings((*settings->getQSettings()), prefix+"acquisition/");
    if (widOverview) widOverview->loadSettings((*settings->getQSettings()), prefix+"overviewacquisition/");
    if (widCamParamScan) widCamParamScan->loadSettings((*settings->getQSettings()), prefix+"camparamscan/");
    if (widDeviceParamScan) widDeviceParamScan->loadSettings((*settings->getQSettings()), prefix+"deviceparamscan/");
    //if (widConfig) widConfig->loadSettings((*settings->getQSettings()), prefix+"config/");
    setUpdatesEnabled(true);
    if (optSetup1) optSetup1->setUpdatesEnabled(true);
    if (optSetup2) optSetup2->setUpdatesEnabled(true);
    if (widExperimentDescription) widExperimentDescription->setUpdatesEnabled(true);
    if (widAcquisitionDescription) widAcquisitionDescription->setUpdatesEnabled(true);
    if (widScriptedAcquisition) widScriptedAcquisition->setUpdatesEnabled(true);
    if (widImageStack) widImageStack->setUpdatesEnabled(true);
    if (widAcquisition) widAcquisition->setUpdatesEnabled(true);
    if (widCamParamScan) widCamParamScan->setUpdatesEnabled(true);
    if (widDeviceParamScan) widDeviceParamScan->setUpdatesEnabled(true);
    //if (widConfig) widConfig->setUpdatesEnabled(true);
}

void QFESPIMB040MainWindow2::storeSettings(ProgramOptions* settings) {
    QString prefix="plugin_spim_b040/";
    if (!optSetupFile.isEmpty()) {
        prefix=prefix+QFileInfo(optSetupFile).fileName()+QString("/");
    }
    jksaveWidgetGeometry((*settings->getQSettings()), this, prefix+"");
    jksaveSplitter((*settings->getQSettings()), splitter, prefix+"");
    if (optSetup1) optSetup1->storeSettings((*settings->getQSettings()), prefix+"instrument/");
    if (optSetup2) optSetup2->storeSettings((settings->getQSettings()->fileName()), prefix+"instrument_new/");

    if (optSetup2) {
        //QString optSetupFile=ProgramOptions::getConfigValue(prefix+"config/optsetup_filename", m_pluginServices->getAssetsDirectory()+"plugins/spimb040/spim_at_b040.optSetup").toString();

        if (QFile::exists(optSetup2->getLastOptSetup()) && QFileInfo(optSetupFile)==QFileInfo(optSetup2->getLastOptSetup())) {
            QFileInfo fi(optSetupFile);
            QDir dir;
            dir=QDir(ProgramOptions::getConfigValue("spimb040/optsetup_config_directory_readonly", ProgramOptions::getInstance()->getGlobalConfigFileDirectory()).toString());
            QString optSetupGlobalConfigFile=dir.absoluteFilePath(fi.fileName()+".ini");
            dir=QDir(ProgramOptions::getConfigValue("spimb040/optsetup_config_directory", ProgramOptions::getInstance()->getConfigFileDirectory()).toString());
            QString optSetupUserConfigFile=dir.absoluteFilePath(fi.fileName()+".ini");

            //QString optSetupGlobalConfigFile=ProgramOptions::getConfigValue(prefix+"config/optsetup_config_filename_readonly", m_pluginServices->getGlobalConfigFileDirectory()+"/spim_at_b040.optSetup.ini").toString();
            //QString optSetupUserConfigFile=ProgramOptions::getConfigValue(prefix+"config/optsetup_config_filename", m_pluginServices->getConfigFileDirectory()+"plugins/spimb040/spim_at_b040.optSetup.ini").toString();
            QStringList optSetupFiles;
            QDir().mkpath(QFileInfo(optSetupUserConfigFile).absolutePath());
            QDir().mkpath(QFileInfo(optSetupGlobalConfigFile).absolutePath());
            optSetupFiles<<optSetupGlobalConfigFile;
            optSetupFiles<<optSetupUserConfigFile;
            optSetup2->storeSettings(optSetupFiles, "", true);
        }
    }


    if (widExperimentDescription) widExperimentDescription->storeSettings((*settings->getQSettings()), prefix+"expdescription/");
    if (widAcquisitionDescription) widAcquisitionDescription->storeSettings((*settings->getQSettings()), prefix+"acqdescription/");
    if (widScriptedAcquisition) widScriptedAcquisition->storeSettings((*settings->getQSettings()), prefix+"acqscripted/");
    if (widImageStack) widImageStack->storeSettings((*settings->getQSettings()), prefix+"image_stack/");
    if (widAcquisition) widAcquisition->storeSettings((*settings->getQSettings()), prefix+"acquisition/");
    if (widOverview) widOverview->storeSettings((*settings->getQSettings()), prefix+"overviewacquisition/");
    if (widCamParamScan) widCamParamScan->storeSettings((*settings->getQSettings()), prefix+"camparamscan/");
    if (widDeviceParamScan) widDeviceParamScan->storeSettings((*settings->getQSettings()), prefix+"deviceparamscan/");
    //if (widConfig) widConfig->storeSettings((*settings->getQSettings()), prefix+"config/");

}

void QFESPIMB040MainWindow2::setOptSetup(const QString &optSetupFile)
{
    this->optSetupFile=optSetupFile;
}

QFESPIMB040AcquisitionConfigWidget2 *QFESPIMB040MainWindow2::getWidAcquisition() const
{
    return widAcquisition;
}

void QFESPIMB040MainWindow2::closeEvent ( QCloseEvent * event ) {
    if (optSetup) optSetup->close();
    QFPluginServices::getInstance()->log_global_text("\n\n=========================================================\n");
    QFPluginServices::getInstance()->log_global_text("== CLOSING SPIM CONTROL PLUGIN!                        ==\n");
    QFPluginServices::getInstance()->log_global_text("=========================================================\n\n\n");
    QFPluginServices::getInstance()->setProjectMode(true);
    QWidget::closeEvent(event);
    //deleteLater();
}

void QFESPIMB040MainWindow2::showEvent( QShowEvent * /*event*/ )  {
    if (optSetup) optSetup->show();
}

void QFESPIMB040MainWindow2::displayHelp() {
    m_pluginServices->displayHelpWindow(m_pluginServices->getExtensionManager()->getPluginHelp(plugin->getID()));
}

void QFESPIMB040MainWindow2::createWidgets(QFExtensionManager* /*extManager*/, bool newOpticsSetup) {
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // create main tab and help button as corner widget of the QTabWidget
    ////////////////////////////////////////////////////////////////////////////////////////////////
    QVBoxLayout* mainl=new QVBoxLayout();
    mainl->setContentsMargins(0,0,0,0);
    setLayout(mainl);
    splitter=new QVisibleHandleSplitter(Qt::Vertical, this);
    mainl->addWidget(splitter);
    tabMain=new QFEnhancedTabWidget(this);
    QVBoxLayout* topl=new QVBoxLayout();
    QWidget* topW=new QWidget(this);
    topW->setLayout(topl);
    topl->addWidget(tabMain,1);
    widAcquisitionDescription=new QFESPIMB040AcquisitionDescription(this);
    topl->addWidget(widAcquisitionDescription);
    splitter->addWidget(topW);
    btnHelp=new QPushButton(QIcon(":/lib/help.png"), tr("Help"), this);
    btnHelp->setToolTip(tr("display the online help window"));
    connect(btnHelp, SIGNAL(clicked()), this,SLOT(displayHelp()));
    tabMain->setCornerWidget(btnHelp, Qt::TopRightCorner);
    tabMain->setCurrentIndex(0);


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // create log pane
    ////////////////////////////////////////////////////////////////////////////////////////////////
    QVBoxLayout* loglayout=new QVBoxLayout();
    QWidget* w=new QWidget(this);
    w->setLayout(loglayout);
    splitter->addWidget(w);
    loglayout->setContentsMargins(0,0,0,0);
    logMain=new QtLogFile(this);
    logMain->set_log_date_time(true);
    loglayout->addWidget(new QLabel(tr("<b>Log Messages:</b>")));
    loglayout->addWidget(logMain);
    logMain->set_log_file_append(true);
    logMain->open_logfile(ProgramOptions::getInstance()->getConfigFileDirectory()+"/b040spim_main.log", false);


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // optics setup tab
    ////////////////////////////////////////////////////////////////////////////////////////////////
    if (newOpticsSetup) {
        ////////////////////////////////////////////////////////////////////////////////////////////////
        // optics setup tab
        ////////////////////////////////////////////////////////////////////////////////////////////////
        optSetup2=new QFESPIMB040OpticsSetup2(this, this, this, m_pluginServices);
        optSetup2->setAcquisitionTools(this);
        //QString optSetupFile=ProgramOptions::getConfigValue("plugin_spim_b040/config/optsetup_filename", m_pluginServices->getAssetsDirectory()+"plugins/spimb040/spim_at_b040.optSetup").toString();
        QFileInfo fi(optSetupFile);
        QDir dir;
        dir=QDir(ProgramOptions::getConfigValue("spimb040/optsetup_config_directory_readonly", ProgramOptions::getInstance()->getGlobalConfigFileDirectory()).toString());
        QString optSetupGlobalConfigFile=dir.absoluteFilePath(fi.fileName()+".ini");
        dir=QDir(ProgramOptions::getConfigValue("spimb040/optsetup_config_directory", ProgramOptions::getInstance()->getConfigFileDirectory()).toString());
        QString optSetupUserConfigFile=dir.absoluteFilePath(fi.fileName()+".ini");
        //QString optSetupUserConfigFile=ProgramOptions::getConfigValue("plugin_spim_b040/config/optsetup_config_filename", m_pluginServices->getConfigFileDirectory()+"plugins/spimb040/spim_at_b040.optSetup.ini").toString();
        optSetup2->loadOptSetup(optSetupFile);
        //optSetup2->loadSettings(ProgramOptions::getConfigValue("spimb040/optsetup_config_filename", m_pluginServices->getAssetsDirectory()+"plugins/spimb040/spim_at_b040.optSetup.ini").toString());
        QStringList optSetupFiles;
        QDir().mkpath(QFileInfo(optSetupUserConfigFile).absolutePath());
        QDir().mkpath(QFileInfo(optSetupGlobalConfigFile).absolutePath());
        optSetupFiles<<optSetupGlobalConfigFile;
        optSetupFiles<<optSetupUserConfigFile;
        optSetup2->loadSettings(optSetupFiles, "", true);

        tabMain->addTab(optSetup2, tr("TESTING: Instrument Setup 2"));
        optSetup=optSetup2;
    } else {
        optSetup1=new QFESPIMB040OpticsSetup(this, this, this, m_pluginServices);
        optSetup1->setAcquisitionTools(this);
        tabMain->addTab(optSetup1, tr("Instrument Setup"));
        topl->insertWidget(0, optSetup1->takeLightpathWidget());
        optSetup=optSetup1;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // create tab for experiment description input
    ////////////////////////////////////////////////////////////////////////////////////////////////
    widExperimentDescription=new QFESPIMB040ExperimentDescription(this);
    tabMain->addTab(widExperimentDescription, tr("Experiment Description"));



    widAcquisitionDescription->setOtherSettingWidgets(optSetup, widExperimentDescription);


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // create tab which groups different acquisition modes
    ////////////////////////////////////////////////////////////////////////////////////////////////
    //tabAcquisition=new QFEnhancedTabWidget(this);
    //tabMain->addTab(tabAcquisition, tr("Acquisition"));

        //------------------------------------------------------------------------------------------
        // create tab for image series acquisition
        //------------------------------------------------------------------------------------------
        widAcquisition=new QFESPIMB040AcquisitionConfigWidget2(this, this, this, m_pluginServices, optSetup, widAcquisitionDescription, widExperimentDescription, m_pluginServices->getConfigFileDirectory());
        //tabAcquisition->addTab(widAcquisition, tr("Image Series Acquisition"));
        tabMain->addTab(widAcquisition, tr("Acquisition: Image Series"));
        //connect(widAcquisition, SIGNAL(doAcquisition()), this, SLOT(doAcquisition()));
        connect(optSetup, SIGNAL(lightpathesChanged(QFESPIMB040OpticsSetupItems)), widAcquisition, SLOT(lightpathesChanged(QFESPIMB040OpticsSetupItems)));

        //------------------------------------------------------------------------------------------
        // create tab for image stack acquisition
        //------------------------------------------------------------------------------------------
        widImageStack=new QFESPIMB040ImageStackConfigWidget2(this, this, this, m_pluginServices, optSetup, widAcquisitionDescription, widExperimentDescription, m_pluginServices->getConfigFileDirectory());
        //tabAcquisition->addTab(widImageStack, tr("Image S&tack"));
        tabMain->addTab(widImageStack, tr("Acquisition: Image S&tack"));
        connect(widImageStack, SIGNAL(doStack()), this, SLOT(doImageStack()));
        connect(optSetup, SIGNAL(lightpathesChanged(QFESPIMB040OpticsSetupItems)), widImageStack, SLOT(lightpathesChanged(QFESPIMB040OpticsSetupItems)));

        //------------------------------------------------------------------------------------------
        // create tab for overview image series acquisition
        //------------------------------------------------------------------------------------------
        widOverview=new QFESPIMB040OverviewAcquisitionConfigWidget(this, this, this, m_pluginServices, optSetup, widAcquisitionDescription, widExperimentDescription, m_pluginServices->getConfigFileDirectory());
        tabMain->addTab(widOverview, tr("Acquisition: Overview Images"));
        connect(optSetup, SIGNAL(lightpathesChanged(QFESPIMB040OpticsSetupItems)), widOverview, SLOT(lightpathesChanged(QFESPIMB040OpticsSetupItems)));

        //------------------------------------------------------------------------------------------
        // create tab for cam parameter image series acquisition
        //------------------------------------------------------------------------------------------
        widCamParamScan=new QFESPIMB040CamParamStackConfigWidget2(this, this, this, m_pluginServices, optSetup, widAcquisitionDescription, widExperimentDescription, m_pluginServices->getConfigFileDirectory());
        tabMain->addTab(widCamParamScan, tr("Acquisition: Camera Parameter Series"));
        connect(widCamParamScan, SIGNAL(doStack()), this, SLOT(doCamParamStack()));
        connect(optSetup, SIGNAL(lightpathesChanged(QFESPIMB040OpticsSetupItems)), widCamParamScan, SLOT(lightpathesChanged(QFESPIMB040OpticsSetupItems)));


        //------------------------------------------------------------------------------------------
        // create tab for device parameter image series acquisition
        //------------------------------------------------------------------------------------------
        widDeviceParamScan=new QFESPIMB040DeviceParamStackConfigWidget(this, this, this, m_pluginServices, optSetup, widAcquisitionDescription, widExperimentDescription, m_pluginServices->getConfigFileDirectory());
        tabMain->addTab(widDeviceParamScan, tr("Acquisition: Device Parameter Series"));
        //connect(widDeviceParamScan, SIGNAL(doStack()), this, SLOT(doDeviceParamStack()));
        connect(optSetup, SIGNAL(lightpathesChanged(QFESPIMB040OpticsSetupItems)), widDeviceParamScan, SLOT(lightpathesChanged(QFESPIMB040OpticsSetupItems)));


        //------------------------------------------------------------------------------------------
        // create tab for scripted acquisition
        //------------------------------------------------------------------------------------------
        widScriptedAcquisition=new QFESPIMB040ScriptedAcquisition(this, this, this, this, m_pluginServices, optSetup, widAcquisitionDescription, widExperimentDescription, m_pluginServices->getConfigFileDirectory());
        tabMain->addTab(widScriptedAcquisition, tr("Scripted Acquisition"));
        //connect(optSetup, SIGNAL(lightpathesChanged(QFESPIMB040OpticsSetupItems)), widDeviceParamScan, SLOT(lightpathesChanged(QFESPIMB040OpticsSetupItems)));


        //------------------------------------------------------------------------------------------
        // create configuration tab
        //------------------------------------------------------------------------------------------
        /*widConfig=new QFESPIMB040ConfigTabWidget(this);
        tabMain->addTab(widConfig, tr("&Configuration"));
        connect(widConfig, SIGNAL(styleChanged(QString,QString)), this, SLOT(styleChanged(QString,QString)));*/



        optSetup->emitLighpathesChanged();
}

#define IMAGESTACK_ERROR(message) \
    log_error(QString("  - ")+(message)+QString("\n")); \
    QMessageBox::critical(this, tr("B040SPIM: Image Stack Acquisition Error"), (message));


void QFESPIMB040MainWindow2::doImageStack() {
    widImageStack->performStack();

}


#define CAMPARAMSTACK_ERROR(message) \
    log_error(QString("  - ")+(message)+QString("\n")); \
    QMessageBox::critical(this, tr("B040SPIM: Parameter Stack Acquisition Error"), (message));


void QFESPIMB040MainWindow2::doCamParamStack() {
    widCamParamScan->performStack();

}

void QFESPIMB040MainWindow2::doDeviceParameterStack()
{
    widDeviceParamScan->performStack();
}








bool QFESPIMB040MainWindow2::savePreview(QFExtension* /*extension*/, QFExtensionCamera* ecamera, int camera, const QString& previewSettingsFilename, const QString& filename, QString* filename32, QMap<QString, QVariant>* acquisitionDescription, const QString& acquisitionDescriptionPrefix, bool mainShutterOpenOnlyForAcquisition, int frames, bool getMeasurements) {
    //////////////////////////////////////////////////////////////////////////////////////
    // INIT variables
    //////////////////////////////////////////////////////////////////////////////////////
    bool ok=true;
    QString TIFFFIlename=filename;
    QString TIFFFIlename32=filename.left(filename.size()-(QFileInfo(filename).suffix().size()+1))+".32."+QFileInfo(filename).suffix();
    TIFF* tiff=NULL;

    if (filename32) *filename32="";


    bool oldShutterState=false;
    if (mainShutterOpenOnlyForAcquisition && optSetup->isMainIlluminationShutterAvailable()) oldShutterState=optSetup->getMainIlluminationShutter();
    //////////////////////////////////////////////////////////////////////////////////////
    // Close Main shutter
    //////////////////////////////////////////////////////////////////////////////////////
    if (mainShutterOpenOnlyForAcquisition && oldShutterState && optSetup->isMainIlluminationShutterAvailable()) {
        optSetup->setMainIlluminationShutter(false, true);
    }


    //////////////////////////////////////////////////////////////////////////////////////
    // PREPARE CAMERA
    //////////////////////////////////////////////////////////////////////////////////////
    int width=0, height=0;
    uint32_t* buffer=NULL;
    if (ok) {
        if (!previewSettingsFilename.isEmpty())  {
            QSettings settings(previewSettingsFilename, QSettings::IniFormat);
            ecamera->useCameraSettings(camera, settings);
        }
        width=ecamera->getCameraImageWidth(camera);
        height=ecamera->getCameraImageHeight(camera);
        buffer=(uint32_t*)qfCalloc(width*height, sizeof(uint32_t));
        if (!buffer) {
            ok=false;
        }
    }


    //////////////////////////////////////////////////////////////////////////////////////
    // Open Main shutter
    //////////////////////////////////////////////////////////////////////////////////////
    if (mainShutterOpenOnlyForAcquisition && optSetup->isMainIlluminationShutterAvailable()) {
        optSetup->setMainIlluminationShutter(true, true);
    }

    //////////////////////////////////////////////////////////////////////////////////////
    // OPEN OUTPUT TIFF FILES
    //////////////////////////////////////////////////////////////////////////////////////
    if (ok) {
        QMap<QString, QVariant> acqD;
        if (frames<=1) {

            QTime time=QTime::currentTime();
            if (ecamera->acquireOnCamera(camera, buffer, NULL, &acqD)) {
                //////////////////////////////////////////////////////////////////////////////////////
                // Close Main shutter
                //////////////////////////////////////////////////////////////////////////////////////
                if (mainShutterOpenOnlyForAcquisition && optSetup->isMainIlluminationShutterAvailable()) {
                    optSetup->setMainIlluminationShutter(false, true);
                }
                if (acquisitionDescription) {
                    QMapIterator<QString, QVariant> it(acqD);
                    while (it.hasNext()) {
                        it.next();
                        (*acquisitionDescription)[acquisitionDescriptionPrefix+it.key()]=it.value();
                    }
                    (*acquisitionDescription)[acquisitionDescriptionPrefix+"/dualview_mode"]=optSetup->dualViewMode(ecamera, camera);
                    (*acquisitionDescription)[acquisitionDescriptionPrefix+"/image_width"]=width;
                    (*acquisitionDescription)[acquisitionDescriptionPrefix+"/image_height"]=height;
                    (*acquisitionDescription)[acquisitionDescriptionPrefix+"/exposure_time"]=ecamera->getCameraExposureTime(camera);
                    optSetup->saveLightpathConfig((*acquisitionDescription), optSetup->getCurrentLightpath(), acquisitionDescriptionPrefix+"/lightpath/", QList<bool>(), true);
                    (*acquisitionDescription)[acquisitionDescriptionPrefix+"/timestamp"]=time;
                    getAdditionalCameraSettings(ecamera, camera, acquisitionDescriptionPrefix, (*acquisitionDescription), getMeasurements);

                }
                QDir().mkpath(QFileInfo(TIFFFIlename.toLatin1().data()).absolutePath());
                tiff=TIFFOpen(TIFFFIlename.toLatin1().data(), "w");
                if (!tiff) {
                    ok=false;
                } else {
                    TIFFTWriteUint16from32(tiff, buffer, width, height, false);
                    TIFFClose(tiff);
                }
                bool is32bit=false;
                for (int i=0; i<width*height; i++) {
                    if ((buffer[i]&0xFFFF0000) != 0) {
                        is32bit=true;
                        break;
                    }
                }
                if (ok && is32bit) {
                    tiff=TIFFOpen(TIFFFIlename32.toLatin1().data(), "w");
                    if (!tiff) {
                        ok=false;
                    } else {
                        if (filename32) {
                            *filename32=TIFFFIlename32;
                        }
                        TIFFTWriteUint32(tiff, buffer, width, height);
                        TIFFClose(tiff);
                    }
                }
            } else {
                ok=false;
            }
        } else if (frames>=2) {

            QDir().mkpath(QFileInfo(TIFFFIlename.toLatin1().data()).absolutePath());
            tiff=TIFFOpen(TIFFFIlename.toLatin1().data(), "w");
            if (tiff) {
                ok=true;
                bool is32bit=false;
                for (int f=0; f<frames; f++) {
                    QTime time=QTime::currentTime();
                    if (ecamera->acquireOnCamera(camera, buffer, NULL, &acqD)) {

                        if (acquisitionDescription) {
                            if (f==0) {
                                QMapIterator<QString, QVariant> it(acqD);
                                while (it.hasNext()) {
                                    it.next();
                                    (*acquisitionDescription)[acquisitionDescriptionPrefix+it.key()]=it.value();
                                }
                                (*acquisitionDescription)[acquisitionDescriptionPrefix+"/dualview_mode"]=optSetup->dualViewMode(ecamera, camera);
                                (*acquisitionDescription)[acquisitionDescriptionPrefix+"/image_width"]=width;
                                (*acquisitionDescription)[acquisitionDescriptionPrefix+"/image_height"]=height;
                                (*acquisitionDescription)[acquisitionDescriptionPrefix+"/exposure_time"]=ecamera->getCameraExposureTime(camera);
                                optSetup->saveLightpathConfig((*acquisitionDescription), optSetup->getCurrentLightpath(), acquisitionDescriptionPrefix+"/lightpath/", QList<bool>(), true);
                                getAdditionalCameraSettings(ecamera, camera, acquisitionDescriptionPrefix, (*acquisitionDescription), getMeasurements);
                            }
                            (*acquisitionDescription)[acquisitionDescriptionPrefix+QString("/timestamp%1").arg(f+1)]=time;

                        }
                        if (f==0) {
                            for (int i=0; i<width*height; i++) {
                                if ((buffer[i]&0xFFFF0000) != 0) {
                                    is32bit=true;
                                    break;
                                }
                            }
                        }
                        if (!tiff) {
                            ok=false;
                        } else {
                            if (ok && is32bit) {
                                TIFFTWriteUint32(tiff, buffer, width, height);
                            } else if (ok) {
                                TIFFTWriteUint16from32(tiff, buffer, width, height, false);
                            }
                            TIFFWriteDirectory(tiff);
                        }
                    } else {
                        ok=false;
                    }
                    if (!ok) break;
                }
                //////////////////////////////////////////////////////////////////////////////////////
                // Close Main shutter
                //////////////////////////////////////////////////////////////////////////////////////
                if (mainShutterOpenOnlyForAcquisition && optSetup->isMainIlluminationShutterAvailable()) {
                    optSetup->setMainIlluminationShutter(false, true);
                }
                TIFFClose(tiff);
            } else {
                ok=false;
            }
        }

        if (buffer) qfFree(buffer);
    }

    //////////////////////////////////////////////////////////////////////////////////////
    // Reset Main shutter
    //////////////////////////////////////////////////////////////////////////////////////
    if (mainShutterOpenOnlyForAcquisition && optSetup->isMainIlluminationShutterAvailable()) {
        optSetup->setMainIlluminationShutter(oldShutterState, true);
    }


    return ok;
}






#define ACQUISITION_ERROR(message) \
    log_error(QString("  - ")+(message)+QString("\n")); \
    QMessageBox::critical(this, tr("B040SPIM: Image Stack Acquisition Error"), (message));

void QFESPIMB040MainWindow2::doAcquisition() {
    widAcquisition->performAcquisition();


}


QString QFESPIMB040MainWindow2::saveAcquisitionDescription(int use_cam, QFExtension* /*extension*/, QFExtensionCamera* ecamera, int camera, const QString& filenamePrefix, const QMap<QString, QVariant>& acquisitionDescription, const QList<QFExtensionCamera::CameraAcquititonFileDescription>& moreFiles, QDateTime startDateTime, bool getAcquisitionSettings) {
    QString iniFilename=filenamePrefix+".configuration.ini";
    QSettings settings(iniFilename, QSettings::IniFormat);

    double magnification=optSetup->getCameraMagnification(use_cam);

    QList<QFExtensionCamera::CameraAcquititonFileDescription> files;
    QMap<QString, QVariant> parameters;
    if (getAcquisitionSettings) ecamera->getCameraAcquisitionDescription(camera, &files, &parameters);

    // WRITE ACQUISITION SETTINGS

    {
        QMapIterator <QString, QVariant> it1(acquisitionDescription);
        while (it1.hasNext()) {
            it1.next();
            if (!it1.value().toString().isEmpty()) {
                if (it1.value().type()==QVariant::List) {
                    settings.setValue("acquisition/"+it1.key(), jkVariantListToString(it1.value().toList(), "; "));
                } else {
                    settings.setValue("acquisition/"+it1.key(), it1.value().toString());
                }

            }
        }
    }
    {
        QMapIterator <QString, QVariant> it(parameters);
        while (it.hasNext()) {
            it.next();
            if (!it.value().toString().isEmpty()) {
                if (it.value().type()==QVariant::List) {
                    settings.setValue("acquisition/"+it.key(), jkVariantListToString(it.value().toList(), "; "));
                } else {
                    settings.setValue("acquisition/"+it.key(), it.value().toString());
                }

            }
        }
    }
    settings.setValue("acquisition/pixel_width", ecamera->getCameraPixelWidth(camera)/magnification);
    settings.setValue("acquisition/pixel_height", ecamera->getCameraPixelHeight(camera)/magnification);
    settings.setValue("acquisition/camera_pixel_width", ecamera->getCameraPixelWidth(camera));
    settings.setValue("acquisition/camera_pixel_height", ecamera->getCameraPixelHeight(camera));
    settings.setValue("acquisition/camera_model", ecamera->getCameraName(camera));
    settings.setValue("acquisition/sensor_model", ecamera->getCameraSensorName(camera));
    settings.setValue("acquisition/magnification", magnification);


    // OPTICS SETUP
    QMap<QString, QVariant> setup=optSetup->getSetup(use_cam);
    {
        QMapIterator <QString, QVariant> it(setup);
        while (it.hasNext()) {
            it.next();
            if (!it.value().toString().isEmpty()) {
                if (it.value().type()==QVariant::List) {
                    settings.setValue("setup/"+it.key(), jkVariantListToString(it.value().toList(), "; "));
                } else {
                    settings.setValue("setup/"+it.key(), it.value().toString());
                }

            }
        }
    }

    // Experiment Description
    QMap<QString, QVariant> experiment=widExperimentDescription->getDescription();
    experiment=experiment.unite(widAcquisitionDescription->getDescription());
    experiment["start_time"]= startDateTime;
    {
        QMapIterator <QString, QVariant> it(experiment);
        while (it.hasNext()) {
            it.next();
            if (!it.value().toString().isEmpty()) {
                if (it.value().type()==QVariant::List) {
                    settings.setValue("experiment/"+it.key(), jkVariantListToString(it.value().toList(), "; "));
                } else {
                    settings.setValue("experiment/"+it.key(), it.value().toString());
                }

            }
        }
    }

    // WRITE FILES LIST
    settings.setValue("files/count", files.size()+moreFiles.size());
    for (int i=0; i<moreFiles.size(); i++) {
        settings.setValue("files/name"+QString::number(i), QFileInfo(settings.fileName()).absoluteDir().relativeFilePath(moreFiles[i].name));
        settings.setValue("files/type"+QString::number(i), moreFiles[i].type);
        settings.setValue("files/description"+QString::number(i), moreFiles[i].description);
    }
    for (int i=0; i<files.size(); i++) {
        settings.setValue("files/name"+QString::number(moreFiles.size()+i), QFileInfo(settings.fileName()).absoluteDir().relativeFilePath(files[i].name));
        settings.setValue("files/type"+QString::number(moreFiles.size()+i), files[i].type);
        settings.setValue("files/description"+QString::number(moreFiles.size()+i), files[i].description);
    }
    settings.sync();
    return iniFilename;
}


QString QFESPIMB040MainWindow2::savePreviewDescription(int use_cam, QFExtension* /*extension*/, QFExtensionCamera* ecamera, int camera, const QString& filenamePrefix, const QMap<QString, QVariant>& acquisitionDescription, const QList<QFExtensionCamera::CameraAcquititonFileDescription>& files, QDateTime startDateTime, const QString& lightpathPrefix, const QString& prefix) {
    QString iniFilename=filenamePrefix+".configuration.ini";
    QSettings settings(iniFilename, QSettings::IniFormat);

    settings.setValue("software/QuickFit/version", qfInfoVersion());
    settings.setValue("software/QuickFit/SVNVersion", qfInfoGITVersion());
    settings.setValue("software/QuickFit/CompileDate", qfInfoCompileDate());
    settings.setValue(QString("software/ControlPlugin/ID"), plugin->getID());
    int min=1, maj=0;
    plugin->getVersion(min, maj);
    settings.setValue(QString("software/ControlPlugin/Version"), QString("%1.%2").arg(min).arg(maj));
    plugin->getQFLibVersion(min, maj);
    settings.setValue(QString("software/ControlPlugin/QFLibVersion"), QString("%1.%2").arg(min).arg(maj));

    settings.beginGroup(prefix);

    double magnification=optSetup->getCameraMagnification(use_cam);

    QMap<QString, QVariant> parameters;

    // WRITE ACQUISITION SETTINGS

    QMapIterator <QString, QVariant> it1(acquisitionDescription);
    while (it1.hasNext()) {
        it1.next();
        if (!it1.value().toString().isEmpty()) {
            if (it1.value().type()==QVariant::List) {
                settings.setValue("acquisition/"+it1.key(), jkVariantListToString(it1.value().toList(), "; "));
            } else {
                settings.setValue("acquisition/"+it1.key(), it1.value().toString());
            }

        }
    }
    settings.setValue("acquisition/pixel_width", ecamera->getCameraPixelWidth(camera)/magnification);
    settings.setValue("acquisition/pixel_height", ecamera->getCameraPixelHeight(camera)/magnification);
    settings.setValue("acquisition/camera_pixel_width", ecamera->getCameraPixelWidth(camera));
    settings.setValue("acquisition/camera_pixel_height", ecamera->getCameraPixelHeight(camera));
    settings.setValue("acquisition/camera_model", ecamera->getCameraName(camera));
    settings.setValue("acquisition/sensor_model", ecamera->getCameraSensorName(camera));
    settings.setValue("acquisition/exposure", ecamera->getCameraExposureTime(camera));
    settings.setValue("acquisition/image_width", ecamera->getCameraImageWidth(camera));
    settings.setValue("acquisition/image_height", ecamera->getCameraImageHeight(camera));
    settings.setValue("acquisition/magnification", magnification);
    // OPTICS SETUP
    QMap<QString, QVariant> setup=optSetup->getSetup(use_cam);
    {
        QMapIterator <QString, QVariant> it(setup);
        while (it.hasNext()) {
            it.next();
            if (!it.value().toString().isEmpty()) {
                if (it.value().type()==QVariant::List) {
                    settings.setValue(lightpathPrefix+"setup/"+it.key(), jkVariantListToString(it.value().toList(), "; "));
                } else {
                    settings.setValue(lightpathPrefix+"setup/"+it.key(), it.value().toString());
                }

            }
        }
    }

    // Experiment Description
    QMap<QString, QVariant> experiment=widExperimentDescription->getDescription();
    experiment=experiment.unite(widAcquisitionDescription->getDescription());
    experiment["start_time"]= startDateTime;
    {
        QMapIterator <QString, QVariant> it(experiment);
        while (it.hasNext()) {
            it.next();
            if (!it.value().toString().isEmpty()) {
                if (it.value().type()==QVariant::List) {
                    settings.setValue("experiment/"+it.key(), jkVariantListToString(it.value().toList(), "; "));
                } else {
                    settings.setValue("experiment/"+it.key(), it.value().toString());
                }

            }
        }
    }
    // WRITE FILES LIST
    settings.setValue("files/count", files.size());
    for (int i=0; i<files.size(); i++) {
        settings.setValue("files/name"+QString::number(i), QFileInfo(settings.fileName()).absoluteDir().relativeFilePath(files[i].name));
        settings.setValue("files/type"+QString::number(i), files[i].type);
        settings.setValue("files/description"+QString::number(i), files[i].description);
    }

    settings.sync();
    return iniFilename;
}


QString QFESPIMB040MainWindow2::saveMeasuredData(const QString& filenamePrefix, const QList<QFESPIMB040OpticsSetupBase::measuredValues>& data) {
    QString csvFilename=filenamePrefix+".measured.dat";

    QFile file(csvFilename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out.setCodec(QTextCodec::codecForName("ISO-8859-1"));
        out.setLocale(QLocale::c());
        QStringList columns;
        columns<<"time [s]";
        for (int i=0; i<data.size(); i++) {
            QMapIterator <QString, QVariant> it1(data[i].data);
            while (it1.hasNext()) {
                it1.next();
                if (!columns.contains(it1.key())) columns<<it1.key();
            }
        }

        out<<"#! ";
        for (int c=0; c<columns.size(); c++) {
            if (c>0) out<<",\t";
            out<<columns[c];
        }
        out<<"\n";
        for (int i=0; i<data.size(); i++) {
            out<<double(data[0].time.msecsTo(data[i].time))/1000.0;
            //out<<",\t\""<<data[0].time.toString("yyyy.MM.dd-hh:mm:ss.zzz")<<"\"";
            for (int j=1; j<columns.size(); j++) {
                QVariant v=data[i].data.value(columns[j], QVariant());
                out<<",\t";
                if (   v.type()==QVariant::Bool
                       || v.type()==QVariant::Char
                       || v.type()==QVariant::Int
                       || v.type()==QVariant::LongLong ) {
                    out<<v.toLongLong();

                } else if ( v.type()==QVariant::UInt || v.type()==QVariant::ULongLong ) {
                    out<<v.toULongLong();
                } else if ( v.type()==QVariant::Double ) {
                    out<<v.toDouble();
                } else if (v.canConvert(QVariant::String)) {
                    out<<"\""<<v.toString()<<"\"";
                }
            }
            out<<"\n";
        }
        file.close();

    } else {
        csvFilename="";
    }


    return csvFilename;
}

void QFESPIMB040MainWindow2::log_text(QString message) {
    logMain->log_text(message);
    //optSetup->log_text(message);
};

void QFESPIMB040MainWindow2::log_warning(QString message) {
    logMain->log_warning(message);
    //optSetup->log_warning(message);
};

void QFESPIMB040MainWindow2::log_error(QString message) {
    logMain->log_error(message);
    //optSetup->log_error(message);
}

void QFESPIMB040MainWindow2::setStyleAndStylesheet(QWidget* widget, const QString &style, const QString &stylesheet) {
    widget->setStyle(QStyleFactory::create(style));
    widget->setPalette(widget->style()->standardPalette());

    QFile f(stylesheet);
    f.open(QFile::ReadOnly);
    QTextStream s(&f);
    QString qss=s.readAll();
    f.close();
    widget->setStyleSheet(qss);

}

void QFESPIMB040MainWindow2::styleChanged(const QString &style, const QString &stylesheet)
{
    setStyleAndStylesheet(this, style, stylesheet);
}

bool QFESPIMB040MainWindow2::setMainIlluminationShutter(bool on_off, bool blocking) {
    /*if (optSetup->isMainIlluminationShutterAvailable()) {
        optSetup->setMainIlluminationShutter(on_off);
        if (blocking) {
            QTime t;
            t.start();
            while (optSetup->getMainIlluminationShutter()!=on_off && t.elapsed()<10000) {
                QApplication::processEvents();
            }

            if (t.elapsed()>=10000) {
                log_error("main shutter timed out after 10s!\n");
                return false;
            }
        }
        return true;
    }
    log_error("main shutter not available!\n");
    return false;*/
    return optSetup->setMainIlluminationShutter(on_off, blocking);
}

void QFESPIMB040MainWindow2::savePreviewMovie(int camera, int frames, const QString &fileName)
{
   QMap<QString, QVariant> cam_params;
   savePreviewMovie( camera,  frames,  fileName, cam_params);
}

void QFESPIMB040MainWindow2::savePreviewMovie(int camera, int frames, const QString &fileName, const QMap<QString, QVariant> &cam_params)
{
    //if (m_stopresume) m_stopresume->stop();
    //saveJKImage(rawImage, tr("Save Raw Image ..."));
    QFExtension* extension;
    QFExtensionCamera* camExt;
    int camID;
    QString previewSettingsFilename;

    if (optSetup->lockCamera(camera, &extension, &camExt, &camID, &previewSettingsFilename) && (QDir().mkpath(QFileInfo(fileName).absolutePath()))) {

        JKImage<uint32_t> rawImage;
        int w=camExt->getCameraImageWidth(camID);
        int h=camExt->getCameraImageHeight(camID);
        QFESPIMB040AcquisitionTools* acqTools;
        if (optSetup==optSetup1) acqTools=optSetup1->getAcquisitionTools();
        if (optSetup==optSetup2) acqTools=optSetup2->getAcquisitionTools();
        rawImage.resize(w, h);



        QProgressDialog progress(tr("Acquiring image series ..."), tr("&Cancel"), 0, frames, this);
        QString tmpName1=QDir::temp().absoluteFilePath("qf3spimb040_preview_camtmpsettings.ini");
        QFTemporaryFile file1;
        if (file1.open()) {
             tmpName1=file1.fileName();
        }
        if (QFile::exists(tmpName1)) QFile::remove(tmpName1);

        QFile::copy(previewSettingsFilename, tmpName1);

        QSettings settingPrev(tmpName1, QSettings::IniFormat);
        QMapIterator<QString, QVariant> it(cam_params);
        while (it.hasNext()) {
            it.next();
            settingPrev.setValue(it.key(), it.value());
        }
        //QSettings settingPrev(previewSettingsFilename, QSettings::IniFormat);
        camExt->useCameraSettings(camID, settingPrev);

        QFileInfo fi(fileName);
        QString fn=fi.absolutePath()+"/"+fi.completeBaseName();
        TIFF* tiff1=TIFFOpen(QString(fn+".tif").toLatin1().data(), "w");
        TIFF* tiff2=TIFFOpen(QString(fn+"_uint32.tif").toLatin1().data(), "w");
        TIFF* tiff3=TIFFOpen(QString(fn+"_float.tif").toLatin1().data(), "w");
        uint64_t timestamp=0;
        QMap<QString, QVariant> camConfig;
        QList<double> times;
        QElapsedTimer timer;
        timer.start();
        QList<QFESPIMB040OpticsSetup::measuredValues> measvec;
        QList<QList<QVariant> > measdat;
        QStringList measdat_cols;
        if (tiff1&&tiff2&&tiff3) {
            for (int i=0; i<frames; i++) {
                if (camExt->acquireOnCamera(camID, rawImage.data(), &timestamp, &camConfig)) {
                    times.append(double(timer.elapsed())/1000.0);
                    QFESPIMB040OpticsSetup::measuredValues meas;
                    if (optSetup) {
                        meas=optSetup->getMeasuredValues();
                    }
                    measvec.append(meas);
                    TIFFTWriteUint16from32(tiff1, rawImage.data(), w, h, false);
                    TIFFWriteDirectory(tiff1);
                    TIFFTWriteUint32(tiff2, rawImage.data(), w, h);
                    TIFFWriteDirectory(tiff2);
                    TIFFTWriteFloatfrom32(tiff3, rawImage.data(), w, h);
                    TIFFWriteDirectory(tiff3);
                }
                if (progress.wasCanceled()) break;
                progress.setValue(i);
                QApplication::processEvents();
            }
        }
        /*QFile f(QString(fn+"_measurements.dat"));
        if (f.open(QFile::WriteOnly|QFile::Text)&&times.size()>0) {
            QTextStream str(&f);
            str<<"timeindex [s], measurement_time [s]";
            QMapIterator<QString, QVariant> it(measvec.first());
            while (it.hasNext()) {
                it.next();
                str<<", "<<it.key();
            }
            str<<"\n";

            for (int i=0; i<times.size(); i++) {
                str<<CDoubleToQString(times[i])<<", "<<double(measvec.first().time.msecsTo(measvec[i].time))/1000.0;
                QMapIterator<QString, QVariant> it(measvec[i]);
                while (it.hasNext()) {
                    it.next();
                    str<<", "<< getQVariantData(it.value());

                }
                str<<"\n";
            }
            f.close();
        }*/


        if (times.size()>0) {
            measdat_cols<<"timeindex [s]"<<"measurement_time [s]";
            QMapIterator<QString, QVariant> it(measvec.first().data);
            while (it.hasNext()) {
                it.next();
                measdat_cols<<it.key();
            }


            for (int i=0; i<times.size(); i++) {
                QList<QVariant> d;
                d<<times[i]<<double(measvec.first().time.msecsTo(measvec[i].time))/1000.0;
                QMapIterator<QString, QVariant> it(measvec[i].data);
                while (it.hasNext()) {
                    it.next();
                    d<<it.value();
                }
                measdat<<d;
            }
            QFDataExportHandler::save(dataRotate(measdat), 0, QString(fn+"_measurements.dat"), measdat_cols);
        }

        QSettings setting(QString(fn+".configuration.ini"), QSettings::IniFormat);
        //storeCameraConfig(setting);
        storeCameaConfig(setting, camExt, camID);
        QString acquisitionDescriptionPrefix="acquisition/";
        setting.setValue(acquisitionDescriptionPrefix+"exposure", camExt->getCameraExposureTime(camID));
        setting.setValue(acquisitionDescriptionPrefix+"image_width", rawImage.width());
        setting.setValue(acquisitionDescriptionPrefix+"image_height", rawImage.height());
        setting.setValue(acquisitionDescriptionPrefix+"image_frames", frames);

        if (camConfig.size()>0) {
            QMapIterator<QString, QVariant> it(camConfig);
            while (it.hasNext()) {
                it.next();
                setting.setValue(acquisitionDescriptionPrefix+it.key(), it.value());
            }
        }

        widExperimentDescription->storeSettings(setting, "experiment");

        setting.setValue("files/count", 4);
        setting.setValue("files/name0", QFileInfo(fn+".configuration.ini").absoluteDir().relativeFilePath(fn+".tif"));
        setting.setValue("files/type0", "TIFF16");
        setting.setValue("files/description0", "image stack");
        setting.setValue("files/name1", QFileInfo(fn+".configuration.ini").absoluteDir().relativeFilePath(fn+"_uint32.tif"));
        setting.setValue("files/type1", "TIFF32");
        setting.setValue("files/description1", "image stack");
        setting.setValue("files/name2", QFileInfo(fn+".configuration.ini").absoluteDir().relativeFilePath(fn+"_float.tif"));
        setting.setValue("files/type2", "TIFF_FLOAT");
        setting.setValue("files/description2", "image stack");
        setting.setValue("files/name3", QFileInfo(fn+".configuration.ini").absoluteDir().relativeFilePath(fn+"_measurements.dat"));
        setting.setValue("files/type3", "CSV");
        setting.setValue("files/description3", "measureable properties of setup");

        optSetup->releaseCamera(camera);
    }
}


void QFESPIMB040MainWindow2::storeCameaConfig(QSettings& setting, QFExtensionCamera* cam, int camID, bool getMeasured) {
    setting.setValue("acquisition/type", "preview");
    setting.setValue("acquisition/start_time", QDateTime::currentDateTime().toString(Qt::ISODate));
    setting.setValue("acquisition/pixel_width", cam->getCameraPixelWidth(camID)/optSetup->getCameraMagnification(camID));
    setting.setValue("acquisition/pixel_height", cam->getCameraPixelHeight(camID)/optSetup->getCameraMagnification(camID));
    if (optSetup) setting.setValue("acquisition/magnification", optSetup->getCameraMagnification(camID));
    if (cam) setting.setValue("acquisition/camera_model", cam->getCameraSensorName(camID));
    if (cam) setting.setValue("acquisition/sensor_model", cam->getCameraName(camID));


    QMap<QString, QVariant> acqD;
    QFESPIMB040OpticsSetup::measuredValues meas;
    QString acquisitionDescriptionPrefix="setup/";
    if (optSetup) {
        acqD=optSetup->getSetup(camID);
        if (getMeasured) meas=optSetup->getMeasuredValues();
    }
    if (acqD.size()>0) {
        QMapIterator<QString, QVariant> it(acqD);
        while (it.hasNext()) {
            it.next();
            setting.setValue(acquisitionDescriptionPrefix+it.key(), it.value());
        }
    }

    if (meas.data.size()>0){
        QMapIterator<QString, QVariant> it(meas.data);
        acquisitionDescriptionPrefix="acquisition/measured_values/";
        setting.setValue(acquisitionDescriptionPrefix+"time", meas.time.toString(Qt::ISODate));
        while (it.hasNext()) {
            it.next();
            setting.setValue(acquisitionDescriptionPrefix+it.key(), it.value());
        }
    }
}

bool QFESPIMB040MainWindow2::prepareCamera(int num, int camera, QFExtensionCamera *ecamera, const QString& acquisitionSettingsFilename, int &width, int &height, uint32_t **buffer, const QString& acquisitionTitle) {
    width=0;
    height=0;
    *buffer=NULL;
    QSettings settings(acquisitionSettingsFilename, QSettings::IniFormat);
    ecamera->useCameraSettings(camera, settings);
    width=ecamera->getCameraImageWidth(camera);
    height=ecamera->getCameraImageHeight(camera);
    *buffer=(uint32_t*)qfCalloc(width*height, sizeof(uint32_t));
    if (!(*buffer)) {
        SPIMACQUISITION_ERROR(tr("could not allocate image buffer for camera 1!\n"), acquisitionTitle);
        return false;
    }
    log_text(tr("  - prepared camera %1!\n").arg(num));
    return true;
}




bool QFESPIMB040MainWindow2::connectStageForAcquisition(QFExtensionLinearStage* stage, int stageAxis, double& stageInitialPos, const QString& acquisitionTitle, int stageNum) {
    bool ok=true;
    stageInitialPos=0;
    if (!stage) {

        SPIMACQUISITION_ERROR(tr("no stage %1 selected").arg(stage->getStageName(stageAxis)), acquisitionTitle);
        optSetup->unlockStages();
        optSetup->unlockLightpath();
        return false;
    }
    if (ok && (!stage->isConnected(stageAxis))) {
        log_text(tr("  - connecting to stage %3: '%1', axis %2!\n").arg(stage->getStageName(stageAxis)).arg(stageAxis).arg(stageNum));
        stage->connectDevice(stageAxis);
    }
    if (ok && (!stage->isConnected(stageAxis)))  {
        ok=false;
        SPIMACQUISITION_ERROR(tr("error connecting to stage %3: '%1', axis %2").arg(stage->getStageName(stageAxis)).arg(stageAxis).arg(stageNum), acquisitionTitle);
    }
    if (ok) {
        stage->setJoystickActive(stageAxis, false);
        stageInitialPos=stage->getPosition(stageAxis);
    }
    return ok;
}

bool QFESPIMB040MainWindow2::acquireImageWithLightpath(const QString& lightpathFilename, const QString& lightpathName, QFExtension* extension1, QFExtensionCamera* ecamera1, int camera1, const QString& previewSettingsFilename1, const QString& outputFilename, const QString& imageID, const QString& imageDescription, QList<QFExtensionCamera::CameraAcquititonFileDescription>& moreFiles1, QMap<QString, QVariant>& acquisitionDescription1, bool mainShutterOpenOnlyForAcquisition, int frames, bool measureDuringAcquisitions) {


    bool oldShutterState=false;
    if (mainShutterOpenOnlyForAcquisition && optSetup->isMainIlluminationShutterAvailable()) oldShutterState=optSetup->getMainIlluminationShutter();
    //////////////////////////////////////////////////////////////////////////////////////
    // Close Main shutter
    //////////////////////////////////////////////////////////////////////////////////////
    if (mainShutterOpenOnlyForAcquisition && oldShutterState && optSetup->isMainIlluminationShutterAvailable()) {
        optSetup->setMainIlluminationShutter(false, true);
    }

    if (!lightpathFilename.isEmpty()) {
        if (!QFile::exists(lightpathFilename)) {
            log_error(tr("  - acquisition lighpath configuration '%1' does not exist!\n").arg(lightpathName));
            //optSetup->unlockLightpath();
            return false;

        } else {
            log_text(tr("  - setting acquisition lightpath settings '%1' ...\n").arg(lightpathName));
            optSetup->loadLightpathConfig(lightpathFilename, true);
            log_text(tr("  - setting acquisition lightpath settings '%1' ... DONE\n").arg(lightpathName));
        }
    }


    QDateTime time=QDateTime::currentDateTime();
    QString filename32="";
    bool ok=savePreview(extension1, ecamera1, camera1, previewSettingsFilename1, outputFilename, &filename32, &acquisitionDescription1, imageID+"/", mainShutterOpenOnlyForAcquisition, frames, measureDuringAcquisitions);
    if (ok) {
        log_text(tr("  - acquired %1!\n").arg(imageDescription));
        /*acquisitionDescription1[imageID+"/image_width"]=ecamera1->getCameraImageWidth(camera1);
        acquisitionDescription1[imageID+"/image_height"]=ecamera1->getCameraImageHeight(camera1);
        acquisitionDescription1[imageID+"/exposure_time"]=ecamera1->getCameraExposureTime(camera1);
        optSetup->saveLightpathConfig(acquisitionDescription1, lightpathName, imageID+"/lightpath/", QList<bool>(), true);
        acquisitionDescription1[imageID+"/dualview_mode"]=optSetup->dualViewMode(ecamera1, camera1);*/
        acquisitionDescription1[imageID+"/timestamp"]=time;
        //getAdditionalCameraSettings(ecamera1, camera1, imageID, acquisitionDescription1, measureDuringAcquisitions);

        QFExtensionCamera::CameraAcquititonFileDescription d;
        d.description=imageDescription;
        d.name=outputFilename;
        d.type="TIFF16";
        moreFiles1.append(d);
        if (!filename32.isEmpty()) {
            d.description=imageDescription;
            d.name=filename32;
            d.type="TIFF32";
            moreFiles1.append(d);
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////
    // Reset Main shutter
    //////////////////////////////////////////////////////////////////////////////////////
    if (mainShutterOpenOnlyForAcquisition  && optSetup->isMainIlluminationShutterAvailable()) {
        optSetup->setMainIlluminationShutter(oldShutterState, true);
    }

    return ok;
}

bool QFESPIMB040MainWindow2::acquireSeries(const QString& lightpathName, const QString& imageID, const QString& imageDescription, bool useCam1, QFExtension* extension1, QFExtensionCamera* ecamera1, int camera1, const QString& acquisitionPrefix1, const QString& acquisitionSettingsFilename1, QMap<QString, QVariant>& acquisitionDescription1, QList<QFExtensionCamera::CameraAcquititonFileDescription>& moreFiles1, bool useCam2, QFExtension* extension2, QFExtensionCamera* ecamera2, int camera2, const QString& acquisitionPrefix2, const QString& acquisitionSettingsFilename2, QMap<QString, QVariant>& acquisitionDescription2, QList<QFExtensionCamera::CameraAcquititonFileDescription>& moreFiles2, const QMap<QFExtensionCamera::CameraSetting, QVariant>& camsettings1, const QMap<QFExtensionCamera::CameraSetting, QVariant>& camsettings2, const QMap<QString, QVariant> &camsettingsstr1, const QMap<QString, QVariant> &camsettingsstr2, QList<QFESPIMB040OpticsSetupBase::measuredValues>* measured, QProgressListDialog* progress, bool* userCanceled, bool  measureDuringAcquisitions )
{
    QString tmpName1=QDir::temp().absoluteFilePath("qf3spimb040_cam1tmpsettings.ini");

    QFTemporaryFile file1;
    if (file1.open()) {
         tmpName1=file1.fileName();
    }
    if (QFile::exists(tmpName1)) QFile::remove(tmpName1);

    if (useCam1) QFile::copy(acquisitionSettingsFilename1, tmpName1);

    QSettings settings1(tmpName1, QSettings::IniFormat);

    //if (useCam1 && frames1>0 && ecamera1->isCameraSettingChangable(QFExtensionCamera::CamSetNumberFrames)) ecamera1->changeCameraSetting(settings1, QFExtensionCamera::CamSetNumberFrames, frames1);
    if (useCam1) {
        QMapIterator<QFExtensionCamera::CameraSetting, QVariant> it(camsettings1);
        while (it.hasNext()) {
            it.next();
            if (it.value().isValid() && ecamera1->isCameraSettingChangable(it.key())) {
                ecamera1->changeCameraSetting(settings1, it.key(), it.value());
            }
        }
        QMapIterator<QString, QVariant> it2(camsettingsstr1);
        while (it2.hasNext()) {
            it2.next();
            if (it2.value().isValid() && ecamera1->isCameraSettingChangable(it2.key())) {
                ecamera1->changeCameraSetting(settings1, it2.key(), it2.value());
            }
        }
    }







    QString tmpName2=QDir::temp().absoluteFilePath("qf3spimb040_cam2tmpsettings.ini");

    QFTemporaryFile file2;
    if (file2.open()) {
         tmpName2=file2.fileName();
    }
    if (QFile::exists(tmpName2)) QFile::remove(tmpName2);

    if (useCam2) QFile::copy(acquisitionSettingsFilename2, tmpName2);

    QSettings settings2(tmpName2, QSettings::IniFormat);

    //if (useCam2 && frames2>0 && ecamera2->isCameraSettingChangable(QFExtensionCamera::CamSetNumberFrames)) ecamera2->changeCameraSetting(settings2, QFExtensionCamera::CamSetNumberFrames, frames2);
    if (useCam2) {
        QMapIterator<QFExtensionCamera::CameraSetting, QVariant> it(camsettings2);
        while (it.hasNext()) {
            it.next();
            if (it.value().isValid() && ecamera2->isCameraSettingChangable(it.key())) {
                ecamera2->changeCameraSetting(settings2, it.key(), it.value());
            }
        }
        QMapIterator<QString, QVariant> it2(camsettingsstr2);
        while (it2.hasNext()) {
            it2.next();
            if (it2.value().isValid() && ecamera2->isCameraSettingChangable(it2.key())) {
                ecamera2->changeCameraSetting(settings2, it2.key(), it2.value());
            }
        }
    }




    bool result=acquireSeries(lightpathName, imageID, imageDescription, useCam1, extension1, ecamera1, camera1, acquisitionPrefix1, settings1, acquisitionDescription1, moreFiles1, useCam2, extension2, ecamera2, camera2, acquisitionPrefix2, settings2, acquisitionDescription2, moreFiles2, measured, progress, userCanceled, measureDuringAcquisitions);


    if (QFile::exists(tmpName1)) QFile::remove(tmpName1);
    if (QFile::exists(tmpName2)) QFile::remove(tmpName2);

    return result;

}

bool QFESPIMB040MainWindow2::acquireSeries(const QString &lightpathName, const QString &imageID, const QString &imageDescription, bool useCam1, QFExtension */*extension1*/, QFExtensionCamera *ecamera1, int camera1, const QString &acquisitionPrefix1, QSettings &acquisitionSettings1, QMap<QString, QVariant> &acquisitionDescription1, QList<QFExtensionCamera::CameraAcquititonFileDescription> &moreFiles1, bool useCam2, QFExtension */*extension2*/, QFExtensionCamera *ecamera2, int camera2, const QString &acquisitionPrefix2, QSettings &acquisitionSettings2, QMap<QString, QVariant> &acquisitionDescription2, QList<QFExtensionCamera::CameraAcquititonFileDescription> &moreFiles2, QList<QFESPIMB040OpticsSetupBase::measuredValues> *measured, QProgressListDialog *progress, bool *userCanceled, bool measureDuringAcquisitions)
{

    bool ok=true;
    QDateTime time;

     //////////////////////////////////////////////////////////////////////////////////////
     // prepare cameras  (set camera settings)
     //////////////////////////////////////////////////////////////////////////////////////
     if (ok && useCam1) {
         if (progress) progress->setLabelText(tr("preparing camera 1 for %1 ...").arg(imageDescription));
         QApplication::processEvents();


         ok=ecamera1->prepareCameraAcquisition(camera1, acquisitionSettings1, acquisitionPrefix1);

         if (ok) {
             log_text(tr("  - prepared camer 1 for %1!\n").arg(imageDescription));
         } else {
             log_error(tr("  - error preparing camera 1 for %1!\n").arg(imageDescription));
             ok=false;
         }

     }
     if (ok && useCam2) {
         if (progress) progress->setLabelText(tr("preparing camera 2 for %1 ...").arg(imageDescription));
         QApplication::processEvents();

         ok=ecamera2->prepareCameraAcquisition(camera2, acquisitionSettings2, acquisitionPrefix2);

         if (ok) {
             log_text(tr("  - prepared camer 2 for %1!\n").arg(imageDescription));
         } else {
             log_error(tr("  - error preparing camera 2 for %1!\n").arg(imageDescription));
             ok=false;
         }

     }

     if (measured) measured->append(optSetup->getMeasuredValues());

     //////////////////////////////////////////////////////////////////////////////////////
     // start background acquisition and wait until finished
     //////////////////////////////////////////////////////////////////////////////////////
     if (ok) {
         if (progress) progress->setLabelText(tr("acquiring %1 images ...").arg(imageDescription));
         log_text(tr("  - acquiring %1 frames!\n").arg(imageDescription));
         time=QDateTime::currentDateTime();
         if (useCam1) {
             ok=ecamera1->startCameraAcquisition(camera1);
             if (!ok) {
                 log_error(tr("  - error starting acquisition on camera 1 for %1!\n").arg(imageDescription));
                 ok=false;
             }
         }
         if (ok && useCam2) {
             ok=ecamera2->startCameraAcquisition(camera2);
             if (!ok) {
                 log_error(tr("  - error starting acquisition on camera 2 for %1!\n").arg(imageDescription));
                 ok=false;
             }
         }
         bool running=ok;

         QTime time1;
         time1.start();
         QTime timA;
         timA.start();
         while (running) {
             int prog1=99, prog2=99;
             if (useCam1) prog1=ecamera1->getCameraAcquisitionProgress(camera1);
             if (useCam2) prog2=ecamera2->getCameraAcquisitionProgress(camera2);
             progress->setValue(qMin(prog1,prog2));

             QString estimation;
             if (qMin(prog1,prog2)>0) {
                 // so lange lauuft die Aufnahme schon in Sekunden
                 double duration=double(timA.elapsed())/1000.0;
                 // zeit pro 1% Fortschritt
                 double eta=duration/double(qMin(prog1,prog2));
                 double etc=100.0*eta-duration;
                 uint mini=floor(etc/60.0);
                 uint secs=round(etc-double(mini)*60.0);
                 estimation=tr("\nest. remaining duration (min:secs): %1:%2 ").arg(mini, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
                 if (progress) progress->setLabelText(tr("acquiring images ...\n")+estimation);
             }
             if (measureDuringAcquisitions && measured && time1.elapsed()>200) {
                 measured->append(optSetup->getMeasuredValues());
                 time1.start();
             }
             QApplication::processEvents();

             if (progress && progress->wasCanceled()) {
                 if (userCanceled) *userCanceled=true;
                 running=false;
                 if (useCam1) ecamera1->cancelCameraAcquisition(camera1);
                 if (useCam2) ecamera2->cancelCameraAcquisition(camera2);
                 log_warning(tr("  - acquisition canceled by user!\n"));
             } else {
                 bool run1=false;
                 bool run2=false;
                 if (useCam1) run1=ecamera1->isCameraAcquisitionRunning(camera1);
                 if (useCam2) run2=ecamera2->isCameraAcquisitionRunning(camera2);
                 running=run1||run2;
             }

             QElapsedTimer te;
             te.start();
             while (te.elapsed()>100) {
                 QApplication::processEvents();
             }
         }
     }
     if (progress) progress->setValue(100);

     //////////////////////////////////////////////////////////////////////////////////////
     // retrieve acquisition description
     //////////////////////////////////////////////////////////////////////////////////////
     if (ok && useCam1) {
         QMap<QString, QVariant> acquisitionDescription;
         ecamera1->getCameraAcquisitionDescription(camera1, &moreFiles1, &acquisitionDescription);
         QMapIterator<QString, QVariant> it(acquisitionDescription);
         while(it.hasNext()) {
             it.next();
             acquisitionDescription1[imageID+"/"+it.key()]=it.value();
         }

         log_text(tr("  - acquired %1 image from camera 1!\n").arg(imageDescription));
         acquisitionDescription1[imageID+"/timestamp"]=time;
         optSetup->saveLightpathConfig(acquisitionDescription1, lightpathName, imageID+"/lightpath/", QList<bool>(), true);
         acquisitionDescription1[imageID+"/dualview_mode"]=optSetup->dualViewMode(0);
         getAdditionalCameraSettings(ecamera1, camera1, imageID, acquisitionDescription1);
     }
     if (ok && useCam2) {
         QMap<QString, QVariant> acquisitionDescription;
         ecamera2->getCameraAcquisitionDescription(camera2, &moreFiles2, &acquisitionDescription);
         QMapIterator<QString, QVariant> it(acquisitionDescription);
         while(it.hasNext()) {
             it.next();
             acquisitionDescription2[imageID+"/"+it.key()]=it.value();
         }
         log_text(tr("  - acquired %1 image from camera 2!\n").arg(imageDescription));
         acquisitionDescription2[imageID+"/timestamp"]=time;
         optSetup->saveLightpathConfig(acquisitionDescription2, lightpathName, imageID+"/lightpath/", QList<bool>(), true);
         acquisitionDescription2[imageID+"/dualview_mode"]=optSetup->dualViewMode(1);
         getAdditionalCameraSettings(ecamera2, camera2, imageID, acquisitionDescription2);
     }

     if (measured) measured->append(optSetup->getMeasuredValues());


     return ok;
}

void QFESPIMB040MainWindow2::getAdditionalCameraSettings(QFExtensionCamera *ecamera, int camera, const QString &prefix, QMap<QString, QVariant> &acquisitionDescription, bool getMeasurements) {
    acquisitionDescription[prefix+"/camera_pixel_width"]=ecamera->getCameraPixelWidth(camera);
    acquisitionDescription[prefix+"/camera_pixel_height"]=ecamera->getCameraPixelHeight(camera);
    acquisitionDescription[prefix+"/camera_model"]=ecamera->getCameraName(camera);
    acquisitionDescription[prefix+"/sensor_model"]=ecamera->getCameraSensorName(camera);
    acquisitionDescription[prefix+"/image_width"]=ecamera->getCameraImageWidth(camera);
    acquisitionDescription[prefix+"/image_height"]=ecamera->getCameraImageHeight(camera);
    acquisitionDescription[prefix+"/dualview_mode"]=optSetup->dualViewMode(ecamera, camera);
    optSetup->saveLightpathConfig(acquisitionDescription, "", prefix+"/lightpath/", QList<bool>(), true);
    {
        QMap<QString, QVariant> setup=optSetup->getSetup(optSetup->camNumFromExtension(ecamera, camera));
        QMapIterator <QString, QVariant> it(setup);
        while (it.hasNext()) {
            it.next();
            if (!it.value().toString().isEmpty()) {
                if (it.value().type()==QVariant::List) {
                    acquisitionDescription[prefix+"/setup/"+it.key()]=jkVariantListToString(it.value().toList(), "; ");
                } else {
                    acquisitionDescription[prefix+"/setup/"+it.key()]=it.value().toString();
                }

            }
        }
    }
    if (getMeasurements) {
        QMap<QString, QVariant> setup=optSetup->getMeasuredValues().data;
        QMapIterator <QString, QVariant> it(setup);
        while (it.hasNext()) {
            it.next();
            if (!it.value().toString().isEmpty()) {
                if (it.value().type()==QVariant::List) {
                    acquisitionDescription[prefix+"/measured/"+it.key()]=jkVariantListToString(it.value().toList(), "; ");
                } else {
                    acquisitionDescription[prefix+"/measured/"+it.key()]=it.value().toString();
                }

            }
        }
    }

    //acquisitionDescription[prefix+"/setting"]=ecamera->getCameraSetting();

}




