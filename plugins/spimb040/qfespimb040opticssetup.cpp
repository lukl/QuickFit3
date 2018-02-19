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

#include "qfespimb040opticssetup.h"
#include "objectives.h"
#include "ui_qfespimb040opticssetup.h"
#include "objectives.h"
#include "filters.h"

#include "qfespimb040shortcutconfigdialog.h"
#include "qfespimb040lightpathsavedialog.h"

QFESPIMB040OpticsSetup::QFESPIMB040OpticsSetup(QWidget* pluginMainWidget, QWidget* parent,  QFPluginLogService* log, QFPluginServices* pluginServices) :
    QFESPIMB040OpticsSetupBase(parent),
    ui(new Ui::QFESPIMB040OpticsSetup)
{
    lightpathConfigModel=new QFTableModel(this);


    setting_lightpath=false;
    m_pluginServices=pluginServices;
    m_pluginMainWidget=pluginMainWidget;
    m_log=log;
    ui->setupUi(this);
    ui->camConfig1->init(0, m_pluginServices, m_pluginServices->getGlobalConfigFileDirectory(), this);
    ui->camConfig1->setLog(m_log);
    ui->camConfig2->init(1, m_pluginServices, m_pluginServices->getGlobalConfigFileDirectory(), this);
    ui->camConfig2->setLog(m_log);
    ui->stageSetup->init(m_log, m_pluginServices);
    ui->filtTransmission->setFilterINI(m_pluginServices->getGlobalConfigFileDirectory()+"/spimb040_filters.ini", m_pluginServices->getConfigFileDirectory()+"/spimb040_filters.ini");
    ui->filtLaser11->setFilterINI(m_pluginServices->getGlobalConfigFileDirectory()+"/spimb040_filters.ini", m_pluginServices->getConfigFileDirectory()+"/spimb040_filters.ini");
    ui->filtLaser12->setFilterINI(m_pluginServices->getGlobalConfigFileDirectory()+"/spimb040_filters.ini", m_pluginServices->getConfigFileDirectory()+"/spimb040_filters.ini");
    ui->filtLaser21->setFilterINI(m_pluginServices->getGlobalConfigFileDirectory()+"/spimb040_filters.ini", m_pluginServices->getConfigFileDirectory()+"/spimb040_filters.ini");
    ui->filtLaser22->setFilterINI(m_pluginServices->getGlobalConfigFileDirectory()+"/spimb040_filters.ini", m_pluginServices->getConfigFileDirectory()+"/spimb040_filters.ini");
    ui->filtDetection->setFilterINI(m_pluginServices->getGlobalConfigFileDirectory()+"/spimb040_filters.ini", m_pluginServices->getConfigFileDirectory()+"/spimb040_filters.ini");
    ui->filtSplitter->setFilterINI(m_pluginServices->getGlobalConfigFileDirectory()+"/spimb040_filters.ini", m_pluginServices->getConfigFileDirectory()+"/spimb040_filters.ini");
    ui->filtDetection11->setFilterINI(m_pluginServices->getGlobalConfigFileDirectory()+"/spimb040_filters.ini", m_pluginServices->getConfigFileDirectory()+"/spimb040_filters.ini");
    ui->filtDetection21->setFilterINI(m_pluginServices->getGlobalConfigFileDirectory()+"/spimb040_filters.ini", m_pluginServices->getConfigFileDirectory()+"/spimb040_filters.ini");
    ui->objDetection->setObjectivesINI(m_pluginServices->getGlobalConfigFileDirectory()+"/spimb040_objectives.ini", m_pluginServices->getConfigFileDirectory()+"/spimb040_objectives.ini");
    ui->objProjection->setObjectivesINI(m_pluginServices->getGlobalConfigFileDirectory()+"/spimb040_objectives.ini", m_pluginServices->getConfigFileDirectory()+"/spimb040_objectives.ini");
    ui->objTube1->setObjectivesINI(m_pluginServices->getGlobalConfigFileDirectory()+"/spimb040_objectives.ini", m_pluginServices->getConfigFileDirectory()+"/spimb040_objectives.ini");
    ui->objTube2->setObjectivesINI(m_pluginServices->getGlobalConfigFileDirectory()+"/spimb040_objectives.ini", m_pluginServices->getConfigFileDirectory()+"/spimb040_objectives.ini");
    ui->filtDualView1Long->setFilterINI(m_pluginServices->getGlobalConfigFileDirectory()+"/spimb040_filters.ini", m_pluginServices->getConfigFileDirectory()+"/spimb040_filters.ini");
    ui->filtDualView1Short->setFilterINI(m_pluginServices->getGlobalConfigFileDirectory()+"/spimb040_filters.ini", m_pluginServices->getConfigFileDirectory()+"/spimb040_filters.ini");
    ui->filtDualView1Splitter->setFilterINI(m_pluginServices->getGlobalConfigFileDirectory()+"/spimb040_filters.ini", m_pluginServices->getConfigFileDirectory()+"/spimb040_filters.ini");
    ui->filtDualView2Long->setFilterINI(m_pluginServices->getGlobalConfigFileDirectory()+"/spimb040_filters.ini", m_pluginServices->getConfigFileDirectory()+"/spimb040_filters.ini");
    ui->filtDualView2Short->setFilterINI(m_pluginServices->getGlobalConfigFileDirectory()+"/spimb040_filters.ini", m_pluginServices->getConfigFileDirectory()+"/spimb040_filters.ini");
    ui->filtDualView2Splitter->setFilterINI(m_pluginServices->getGlobalConfigFileDirectory()+"/spimb040_filters.ini", m_pluginServices->getConfigFileDirectory()+"/spimb040_filters.ini");


    ui->shutterMainIllumination->init(m_log, m_pluginServices);
    ui->tbMainShutter->setDefaultAction(ui->shutterMainIllumination->getStateAction());
    ui->shutterLaser1->init(m_log, m_pluginServices);
    ui->shutterLaser2->init(m_log, m_pluginServices);
    ui->shutterTransmission->init(m_log, m_pluginServices);
    ui->lsTransmission->init(m_log, m_pluginServices);
    ui->lsLaser1->init(m_log, m_pluginServices);
    ui->lsLaser2->init(m_log, m_pluginServices);
    ui->widMD1->init(m_log, m_pluginServices);
    ui->widMD2->init(m_log, m_pluginServices);
    ui->filtcDetection->init(m_pluginServices->getGlobalConfigFileDirectory()+"/spimb040_filters.ini", m_pluginServices->getConfigFileDirectory()+"/spimb040_filters.ini", m_log, m_pluginServices);
    ui->cmbLightpathConfig->init(ProgramOptions::getConfigValue("spimb040/optsetup_config_directory", m_pluginServices->getConfigFileDirectory()+"/plugins/ext_spimb040/").toString(), ProgramOptions::getConfigValue("spimb040/optsetup_config_directory_readonly", ProgramOptions::getInstance()->getGlobalConfigFileDirectory()).toString(), "lpc");
    ui->cmbLightpathConfig->setIcon(QIcon(":/spimb040/lightpath.png"));
    ui->btnLockFiltersEtc->setChecked(true);
    connect(ui->cmbLightpathConfig, SIGNAL(configsChanged(QFESPIMB040OpticsSetupItems)), this, SLOT(configsChanged(QFESPIMB040OpticsSetupItems)));
    connect(ui->cmbLightpathConfig, SIGNAL(currentConfigChanged(QString)), this, SLOT(userChangedLightpath(QString)));

    // create shortcuts
    connect(addShortCut("stage_x2", "translation stage: joystick speed x2"), SIGNAL(activated()), ui->stageSetup, SLOT(speedX2()));
    connect(addShortCut("stage_x10", "translation stage: joystick speed x10"), SIGNAL(activated()), ui->stageSetup, SLOT(speedX10()));
    connect(addShortCut("stage_d2", "translation stage: joystick speed /2"), SIGNAL(activated()), ui->stageSetup, SLOT(speedD2()));
    connect(addShortCut("stage_d10", "translation stage: joystick speed /10"), SIGNAL(activated()), ui->stageSetup, SLOT(speedD10()));
    connect(addShortCut("stage_stepx", "translation stage: step x"), SIGNAL(activated()), ui->stageSetup, SLOT(stepX()));
    connect(addShortCut("stage_stepmx", "translation stage: step -x"), SIGNAL(activated()), ui->stageSetup, SLOT(stepMinusX()));
    connect(addShortCut("stage_stepy", "translation stage: step y"), SIGNAL(activated()), ui->stageSetup, SLOT(stepY()));
    connect(addShortCut("stage_stepmy", "translation stage: step -y"), SIGNAL(activated()), ui->stageSetup, SLOT(stepMinusY()));
    connect(addShortCut("stage_stepz", "translation stage: step z"), SIGNAL(activated()), ui->stageSetup, SLOT(stepZ()));
    connect(addShortCut("stage_stepmz", "translation stage: step -z"), SIGNAL(activated()), ui->stageSetup, SLOT(stepMinusZ()));
    connect(addShortCut("stage_stepR", "translation stage: step R"), SIGNAL(activated()), ui->stageSetup, SLOT(stepR()));
    connect(addShortCut("stage_stepmR", "translation stage: step -R"), SIGNAL(activated()), ui->stageSetup, SLOT(stepMinusR()));
    connect(addShortCut("stage_joysticktoggle", "translation stage: toggle joystick"), SIGNAL(activated()), ui->stageSetup, SLOT(toggleJoystick()));
    connect(addShortCut("stage_joystick_on", "translation stage: joystick on"), SIGNAL(activated()), ui->stageSetup, SLOT(joystickOn()));
    connect(addShortCut("stage_joystick_off", "translation stage: joystick off"), SIGNAL(activated()), ui->stageSetup, SLOT(joystickOff()));

    connect(addShortCut("mainshutter_toggle", "main shutter: toggle"), SIGNAL(activated()), ui->shutterMainIllumination, SLOT(toggleShutter()));
    connect(addShortCut("mainshutter_on", "main shutter: on"), SIGNAL(activated()), ui->shutterMainIllumination, SLOT(shutterOn()));
    connect(addShortCut("mainshutter_off", "main shutter: off"), SIGNAL(activated()), ui->shutterMainIllumination, SLOT(shutterOff()));

    connect(addShortCut("shutter_laser1_toggle", "laser 1 shutter: toggle"), SIGNAL(activated()), ui->shutterLaser1, SLOT(toggleShutter()));
    connect(addShortCut("shutter_laser1_on", "laser 1 shutter: on"), SIGNAL(activated()), ui->shutterLaser1, SLOT(shutterOn()));
    connect(addShortCut("shutter_laser1_off", "laser 1 shutter: off"), SIGNAL(activated()), ui->shutterLaser1, SLOT(shutterOff()));

    connect(addShortCut("shutter_laser2_toggle", "laser 2 shutter: toggle"), SIGNAL(activated()), ui->shutterLaser2, SLOT(toggleShutter()));
    connect(addShortCut("shutter_laser2_on", "laser 2 shutter: on"), SIGNAL(activated()), ui->shutterLaser2, SLOT(shutterOn()));
    connect(addShortCut("shutter_laser2_off", "laser 2 shutter: off"), SIGNAL(activated()), ui->shutterLaser2, SLOT(shutterOff()));

    connect(addShortCut("shutter_transmission_toggle", "transmission illumination shutter: toggle"), SIGNAL(activated()), ui->shutterTransmission, SLOT(toggleShutter()));
    connect(addShortCut("shutter_transmission_on", "transmission illumination shutter: on"), SIGNAL(activated()), ui->shutterTransmission, SLOT(shutterOn()));
    connect(addShortCut("shutter_transmission_off", "transmission illumination shutter: off"), SIGNAL(activated()), ui->shutterTransmission, SLOT(shutterOff()));

    connect(addShortCut("filterchanger_detection_filter1", "detection filter changer: set filter #1"), SIGNAL(activated()), ui->filtcDetection, SLOT(setFilter0()));
    connect(addShortCut("filterchanger_detection_filter2", "detection filter changer: set filter #2"), SIGNAL(activated()), ui->filtcDetection, SLOT(setFilter1()));
    connect(addShortCut("filterchanger_detection_filter3", "detection filter changer: set filter #3"), SIGNAL(activated()), ui->filtcDetection, SLOT(setFilter2()));
    connect(addShortCut("filterchanger_detection_filter4", "detection filter changer: set filter #4"), SIGNAL(activated()), ui->filtcDetection, SLOT(setFilter3()));
    connect(addShortCut("filterchanger_detection_filter5", "detection filter changer: set filter #5"), SIGNAL(activated()), ui->filtcDetection, SLOT(setFilter4()));
    connect(addShortCut("filterchanger_detection_filter6", "detection filter changer: set filter #6"), SIGNAL(activated()), ui->filtcDetection, SLOT(setFilter5()));
    connect(addShortCut("filterchanger_detection_filter7", "detection filter changer: set filter #7"), SIGNAL(activated()), ui->filtcDetection, SLOT(setFilter6()));
    connect(addShortCut("filterchanger_detection_filter8", "detection filter changer: set filter #8"), SIGNAL(activated()), ui->filtcDetection, SLOT(setFilter7()));
    connect(addShortCut("filterchanger_detection_filter9", "detection filter changer: set filter #9"), SIGNAL(activated()), ui->filtcDetection, SLOT(setFilter8()));
    connect(addShortCut("filterchanger_detection_filter10", "detection filter changer: set filter #10"), SIGNAL(activated()), ui->filtcDetection, SLOT(setFilter9()));

    connect(addShortCut("cam1_acquire_single", "camera 1: acquire single frame"), SIGNAL(activated()), ui->camConfig1, SLOT(previewSingle()));
    connect(addShortCut("cam1_acquire_continuous_toggle", "camera 1: toggle preview acquisition"), SIGNAL(activated()), ui->camConfig1, SLOT(startStopPreview()));
    connect(addShortCut("cam1_acquire_continuous_stop", "camera 1: stop preview"), SIGNAL(activated()), ui->camConfig1, SLOT(stopPreview()));

    connect(addShortCut("cam2_acquire_single", "camera 2: acquire single frame"), SIGNAL(activated()), ui->camConfig2, SLOT(previewSingle()));
    connect(addShortCut("cam2_acquire_continuous_toggle", "camera 2: toggle preview acquisition"), SIGNAL(activated()), ui->camConfig2, SLOT(startStopPreview()));
    connect(addShortCut("cam2_acquire_continuous_stop", "camera 2: stop preview"), SIGNAL(activated()), ui->camConfig2, SLOT(stopPreview()));

}

QFESPIMB040OpticsSetup::~QFESPIMB040OpticsSetup()
{
    delete ui;
}

void QFESPIMB040OpticsSetup::lockStages() {
    ui->stageSetup->lockStages();
}

void QFESPIMB040OpticsSetup::unlockStages() {
    ui->stageSetup->unlockStages();
}

void QFESPIMB040OpticsSetup::closeEvent(QCloseEvent * event) {
    ui->camConfig1->close();
    ui->camConfig2->close();
    event->accept();
}

void QFESPIMB040OpticsSetup::showEvent( QShowEvent * /*event*/ ) {
    ui->camConfig1->show();
    ui->camConfig2->show();
}

QShortcut *QFESPIMB040OpticsSetup::addShortCut(const QString &id, const QString &label, const QKeySequence& sequence) {
    QFESPIMB040OpticsSetup::shortcutItem itm;
    itm.shortcut=new QShortcut(m_pluginMainWidget);
    itm.shortcut->setKey(sequence);
    itm.shortcut->setWhatsThis(label);
    itm.shortcut->setContext(Qt::ApplicationShortcut);
    itm.shortcut->setEnabled(!sequence.isEmpty());
    itm.id=id;
    itm.label=label;
    shortcuts.append(itm);
    return itm.shortcut;
}

void QFESPIMB040OpticsSetup::loadSettings(QSettings& settings, QString prefix) {
    bool updt=updatesEnabled() && isVisible();
    if (updt) setUpdatesEnabled(false);
    ui->camConfig1->loadSettings(settings, prefix+"cam_config1/");
    ui->camConfig2->loadSettings(settings, prefix+"cam_config2/");
    ui->widMD1->loadSettings(settings, prefix+"measure_device1/");
    ui->widMD2->loadSettings(settings, prefix+"measure_device2/");
    ui->stageSetup->loadSettings(settings, prefix+"stages/");
    ui->filtDetection11->loadSettings(settings, prefix+"filters/detection11");
    ui->filtDetection21->loadSettings(settings, prefix+"filters/detection21");
    ui->filtSplitter->loadSettings(settings, prefix+"filters/detection_splitter");
    ui->filtTransmission->loadSettings(settings, prefix+"filters/illumination_transmission");
    ui->filtLaser11->loadSettings(settings, prefix+"filters/laser11");
    ui->filtLaser12->loadSettings(settings, prefix+"filters/laser12");
    ui->filtLaser21->loadSettings(settings, prefix+"filters/laser21");
    ui->filtLaser22->loadSettings(settings, prefix+"filters/laser22");
    ui->filtDetection->loadSettings(settings, prefix+"filters/detection");

    ui->filtDualView1Long->loadSettings(settings, prefix+"filters/dv1_long");
    ui->filtDualView1Short->loadSettings(settings, prefix+"filters/dv1_short");
    ui->filtDualView1Splitter->loadSettings(settings, prefix+"filters/dv1_splitter");
    ui->filtDualView2Long->loadSettings(settings, prefix+"filters/dv2_long");
    ui->filtDualView2Short->loadSettings(settings, prefix+"filters/dv2_short");
    ui->filtDualView2Splitter->loadSettings(settings, prefix+"filters/dv2_splitter");
    ui->chkDualView1->setChecked(settings.value(prefix+"filters/dv1_enabled", false).toBool());
    ui->chkDualView2->setChecked(settings.value(prefix+"filters/dv2_enabled", false).toBool());
    ui->cmbDualViewOrientation->setCurrentIndex(settings.value(prefix+"filters/dv1_orientation", 0).toInt());
    ui->cmbDualView2Orientation->setCurrentIndex(settings.value(prefix+"filters/dv2_orientation", 0).toInt());

    ui->objDetection->loadSettings(settings, prefix+"objectives/detection");
    ui->objProjection->loadSettings(settings, prefix+"objectives/projection");
    ui->objTube1->loadSettings(settings, prefix+"objectives/tubelens1");
    ui->objTube2->loadSettings(settings, prefix+"objectives/tubelens2");
    ui->shutterMainIllumination->loadSettings(settings, prefix+"main_illumination_shutter");
    ui->shutterLaser1->loadSettings(settings, prefix+"laser1_shutter");
    ui->shutterLaser2->loadSettings(settings, prefix+"laser2_shutter");
    ui->shutterTransmission->loadSettings(settings, prefix+"transmission_shutter");
    ui->lsLaser1->loadSettings(settings, prefix+"lightsource_laser1");
    ui->lsLaser2->loadSettings(settings, prefix+"lightsource_laser2");
    ui->lsTransmission->loadSettings(settings, prefix+"lightsource_transmission");
    ui->filtcDetection->loadSettings(settings, prefix+"filterchanger_detection");
    ui->chkDetectionFilterWheel->setChecked(settings.value(prefix+"filterchanger_detection", false).toBool());

    for (int i=0; i<shortcuts.size(); i++) {
        QKeySequence seq(settings.value(prefix+"shortcut_"+shortcuts[i].id, shortcuts[i].shortcut->key().toString()).toString());
        shortcuts[i].shortcut->setKey(seq);
        shortcuts[i].shortcut->setEnabled(!seq.isEmpty());
    }
    if (updt) setUpdatesEnabled(true);
}

void QFESPIMB040OpticsSetup::storeSettings(QSettings& settings, QString prefix) {
    ui->camConfig1->storeSettings(settings, prefix+"cam_config1/");
    ui->camConfig2->storeSettings(settings, prefix+"cam_config2/");
    ui->widMD1->saveSettings(settings, prefix+"measure_device1/");
    ui->widMD2->saveSettings(settings, prefix+"measure_device2/");
    ui->stageSetup->saveSettings(settings, prefix+"stages/");
    ui->filtDetection11->saveSettings(settings, prefix+"filters/detection11");
    ui->filtDetection21->saveSettings(settings, prefix+"filters/detection21");
    ui->filtSplitter->saveSettings(settings, prefix+"filters/detection_splitter");
    ui->filtTransmission->saveSettings(settings, prefix+"filters/illumination_transmission");
    ui->filtLaser11->saveSettings(settings, prefix+"filters/laser11");
    ui->filtLaser12->saveSettings(settings, prefix+"filters/laser12");
    ui->filtLaser21->saveSettings(settings, prefix+"filters/laser21");
    ui->filtLaser22->saveSettings(settings, prefix+"filters/laser22");
    ui->filtDetection->saveSettings(settings, prefix+"filters/detection");
    ui->objDetection->saveSettings(settings, prefix+"objectives/detection");
    ui->objProjection->saveSettings(settings, prefix+"objectives/projection");
    ui->objTube1->saveSettings(settings, prefix+"objectives/tubelens1");
    ui->objTube2->saveSettings(settings, prefix+"objectives/tubelens2");
    ui->shutterMainIllumination->saveSettings(settings, prefix+"main_illumination_shutter");
    ui->shutterLaser1->saveSettings(settings, prefix+"laser1_shutter");
    ui->shutterLaser2->saveSettings(settings, prefix+"laser2_shutter");
    ui->filtcDetection->saveSettings(settings, prefix+"filterchanger_detection");
    ui->shutterTransmission->saveSettings(settings, prefix+"transmission_shutter");
    ui->lsLaser1->saveSettings(settings, prefix+"lightsource_laser1");
    ui->lsLaser2->saveSettings(settings, prefix+"lightsource_laser2");
    ui->lsTransmission->saveSettings(settings, prefix+"lightsource_transmission");
    ui->filtDualView1Long->saveSettings(settings, prefix+"filters/dv1_long");
    ui->filtDualView1Short->saveSettings(settings, prefix+"filters/dv1_short");
    ui->filtDualView1Splitter->saveSettings(settings, prefix+"filters/dv1_splitter");
    ui->filtDualView2Long->saveSettings(settings, prefix+"filters/dv2_long");
    ui->filtDualView2Short->saveSettings(settings, prefix+"filters/dv2_short");
    ui->filtDualView2Splitter->saveSettings(settings, prefix+"filters/dv2_splitter");
    settings.setValue(prefix+"filters/dv1_enabled", ui->chkDualView1->isChecked());
    settings.setValue(prefix+"filters/dv2_enabled", ui->chkDualView2->isChecked());
    settings.setValue(prefix+"filters/dv1_orientation", ui->cmbDualViewOrientation->currentIndex());
    settings.setValue(prefix+"filters/dv2_orientation", ui->cmbDualView2Orientation->currentIndex());

    settings.setValue(prefix+"filterchanger_detection", ui->chkDetectionFilterWheel->isChecked());

    for (int i=0; i<shortcuts.size(); i++) {
        settings.setValue(prefix+"shortcut_"+shortcuts[i].id, shortcuts[i].shortcut->key().toString());
    }
}

QWidget *QFESPIMB040OpticsSetup::takeLightpathWidget() const
{
    QLayoutItem* it= ui->layHTop->takeAt(ui->layHTop->indexOf(ui->frmLightpath));
    if (it) {
        return it->widget();
    }
    return NULL;
}

int QFESPIMB040OpticsSetup::getSpecialShutterID(QFESPIMB040OpticsSetupBase::Shutters shutter) const
{
    return (int) shutter;
}

int QFESPIMB040OpticsSetup::getSpecialStageID(QFESPIMB040OpticsSetupBase::specialStages shutter) const
{
    return (int) shutter;
}

int QFESPIMB040OpticsSetup::getSpecialBrightfieldID(QFESPIMB040OpticsSetupBase::specialBrightfieldSources shutter) const
{
    return (int) shutter;
}

int QFESPIMB040OpticsSetup::getSpecialFilterChangerID(QFESPIMB040OpticsSetupBase::specialFilterChangers shutter) const
{
    return (int) shutter;
}

void QFESPIMB040OpticsSetup::loadPluginGlobalSettings(QSettings &settings, QString prefix) {
    loadPluginGlobalSettings(settings, ui->lsLaser1->getLightSourceExtensionObject(), prefix);
    loadPluginGlobalSettings(settings, ui->lsLaser2->getLightSourceExtensionObject(), prefix);
    loadPluginGlobalSettings(settings, ui->lsTransmission->getLightSourceExtensionObject(), prefix);
    loadPluginGlobalSettings(settings, ui->shutterLaser1->getShutterExtensionObject(), prefix);
    loadPluginGlobalSettings(settings, ui->shutterLaser2->getShutterExtensionObject(), prefix);
    loadPluginGlobalSettings(settings, ui->shutterMainIllumination->getShutterExtensionObject(), prefix);
    loadPluginGlobalSettings(settings, ui->shutterTransmission->getShutterExtensionObject(), prefix);
    loadPluginGlobalSettings(settings, ui->filtcDetection->getFilterChangerExtensionObject(), prefix);
    loadPluginGlobalSettings(settings, ui->camConfig1->cameraComboBox()->currentCameraQObject(), prefix);
    loadPluginGlobalSettings(settings, ui->camConfig2->cameraComboBox()->currentCameraQObject(), prefix);
    loadPluginGlobalSettings(settings, ui->stageSetup->getXStageExtensionObject(), prefix);
    loadPluginGlobalSettings(settings, ui->stageSetup->getYStageExtensionObject(), prefix);
    loadPluginGlobalSettings(settings, ui->stageSetup->getZStageExtensionObject(), prefix);
    loadPluginGlobalSettings(settings, ui->stageSetup->getRStageExtensionObject(), prefix);
    loadPluginGlobalSettings(settings, ui->widMD1->getMeasurementDeviceExtensionObject(), prefix);
    loadPluginGlobalSettings(settings, ui->widMD2->getMeasurementDeviceExtensionObject(), prefix);
}

void QFESPIMB040OpticsSetup::storePluginGlobalSettings(QSettings &settings, QString prefix) const {
    storePluginGlobalSettings(settings, ui->lsLaser1->getLightSourceExtensionObject(), prefix);
    storePluginGlobalSettings(settings, ui->lsLaser2->getLightSourceExtensionObject(), prefix);
    storePluginGlobalSettings(settings, ui->lsTransmission->getLightSourceExtensionObject(), prefix);
    storePluginGlobalSettings(settings, ui->shutterLaser1->getShutterExtensionObject(), prefix);
    storePluginGlobalSettings(settings, ui->shutterLaser2->getShutterExtensionObject(), prefix);
    storePluginGlobalSettings(settings, ui->shutterMainIllumination->getShutterExtensionObject(), prefix);
    storePluginGlobalSettings(settings, ui->shutterTransmission->getShutterExtensionObject(), prefix);
    storePluginGlobalSettings(settings, ui->filtcDetection->getFilterChangerExtensionObject(), prefix);
    storePluginGlobalSettings(settings, ui->camConfig1->cameraComboBox()->currentCameraQObject(), prefix);
    storePluginGlobalSettings(settings, ui->camConfig2->cameraComboBox()->currentCameraQObject(), prefix);
    storePluginGlobalSettings(settings, ui->stageSetup->getXStageExtensionObject(), prefix);
    storePluginGlobalSettings(settings, ui->stageSetup->getYStageExtensionObject(), prefix);
    storePluginGlobalSettings(settings, ui->stageSetup->getZStageExtensionObject(), prefix);
    storePluginGlobalSettings(settings, ui->stageSetup->getRStageExtensionObject(), prefix);
    storePluginGlobalSettings(settings, ui->widMD1->getMeasurementDeviceExtensionObject(), prefix);
    storePluginGlobalSettings(settings, ui->widMD2->getMeasurementDeviceExtensionObject(), prefix);
}

void QFESPIMB040OpticsSetup::loadPluginGlobalSettings(QSettings &settings, QObject *extensionObject, QString prefix) {
    QFExtensionGlobalSettingsReadWrite* extensionRW=NULL;
    QFExtension* extension=NULL;

    extension=qobject_cast<QFExtension*>(extensionObject);
    extensionRW=qobject_cast<QFExtensionGlobalSettingsReadWrite*>(extensionObject);
    if (extensionRW) {
        extensionRW->readGlobalSettings(settings, prefix+extension->getID()+"/");
    }
}

void QFESPIMB040OpticsSetup::storePluginGlobalSettings(QSettings &settings, QObject *extensionObject, QString prefix) const {
    QFExtensionGlobalSettingsReadWrite* extensionRW=NULL;
    QFExtension* extension=NULL;

    extension=qobject_cast<QFExtension*>(extensionObject);
    extensionRW=qobject_cast<QFExtensionGlobalSettingsReadWrite*>(extensionObject);
    if (extensionRW) {
        extensionRW->writeGlobalSettings(settings, prefix+extension->getID()+"/");
    }
}

double QFESPIMB040OpticsSetup::getCameraMagnification(int setup_cam) const {
    if (setup_cam==0) {
        return ui->objDetection->objective().magnification*ui->objTube1->objective().magnification;
    } else if (setup_cam==1) {
        return ui->objDetection->objective().magnification*ui->objTube2->objective().magnification;
    }
    return 1;
}

QMap<QString, QVariant> QFESPIMB040OpticsSetup::getSetup(int setup_cam) const {
    QMap<QString, QVariant> setup;

    FilterDescription filter=ui->filtDetection->filter();
    if (filter.isValid) {
        setup["filters/detection/name"]=filter.name;
        setup["filters/detection/type"]=filter.type;
        setup["filters/detection/manufacturer"]=filter.manufacturer;
    }

    if (ui->chkDetectionFilterWheel->isChecked()) {
        FilterDescription filter=ui->filtcDetection->getCurrentFilterDescription();
        if (filter.isValid) {
            setup["filters/detection_filterchanger/name"]=filter.name;
            setup["filters/detection_filterchanger/type"]=filter.type;
            setup["filters/detection_filterchanger/manufacturer"]=filter.manufacturer;
        }
    }

    if (setup_cam<0 || setup_cam==0) {
        filter=ui->filtDetection11->filter();
        if (filter.isValid) {
            setup["filters/detection_cam1/name"]=filter.name;
            setup["filters/detection_cam1/type"]=filter.type;
            setup["filters/detection_cam1/manufacturer"]=filter.manufacturer;
        }
        if (ui->chkDualView1->isChecked()) {
            if (ui->cmbDualViewOrientation->currentIndex()==0) setup["filters/dualview_cam1/orientation"]=QString("horicontal");
            else setup["filters/dualview_cam1/orientation"]=QString("vertical");
            filter=ui->filtDualView1Long->filter();
            if (filter.isValid) {
                setup["filters/dualview_cam1/detection_long/name"]=filter.name;
                setup["filters/dualview_cam1/detection_long/type"]=filter.type;
                setup["filters/dualview_cam1/detection_long/manufacturer"]=filter.manufacturer;
            }
            filter=ui->filtDualView1Short->filter();
            if (filter.isValid) {
                setup["filters/dualview_cam1/detection_short/name"]=filter.name;
                setup["filters/dualview_cam1/detection_short/type"]=filter.type;
                setup["filters/dualview_cam1/detection_short/manufacturer"]=filter.manufacturer;
            }
            filter=ui->filtDualView1Splitter->filter();
            if (filter.isValid) {
                setup["filters/dualview_cam1/detection_splitter/name"]=filter.name;
                setup["filters/dualview_cam1/detection_splitter/type"]=filter.type;
                setup["filters/dualview_cam1/detection_splitter/manufacturer"]=filter.manufacturer;
            }
        }
    }

    if (setup_cam<0 || setup_cam==1) {
        filter=ui->filtDetection21->filter();
        if (filter.isValid) {
            setup["filters/detection_cam2/name"]=filter.name;
            setup["filters/detection_cam2/type"]=filter.type;
            setup["filters/detection_cam2/manufacturer"]=filter.manufacturer;
        }
        if (ui->chkDualView2->isChecked()) {
            if (ui->cmbDualView2Orientation->currentIndex()==0) setup["filters/dualview_cam2/orientation"]=QString("horicontal");
            else setup["filters/dualview_cam2/orientation"]=QString("vertical");
            filter=ui->filtDualView2Long->filter();
            if (filter.isValid) {
                setup["filters/dualview_cam2/detection_long/name"]=filter.name;
                setup["filters/dualview_cam2/detection_long/type"]=filter.type;
                setup["filters/dualview_cam2/detection_long/manufacturer"]=filter.manufacturer;
            }
            filter=ui->filtDualView2Short->filter();
            if (filter.isValid) {
                setup["filters/dualview_cam2/detection_short/name"]=filter.name;
                setup["filters/dualview_cam2/detection_short/type"]=filter.type;
                setup["filters/dualview_cam2/detection_short/manufacturer"]=filter.manufacturer;
            }
            filter=ui->filtDualView2Splitter->filter();
            if (filter.isValid) {
                setup["filters/dualview_cam2/detection_splitter/name"]=filter.name;
                setup["filters/dualview_cam2/detection_splitter/type"]=filter.type;
                setup["filters/dualview_cam2/detection_splitter/manufacturer"]=filter.manufacturer;
            }
        }
    }

    filter=ui->filtSplitter->filter();
    if (filter.isValid) {
        setup["filters/detection_splitter/name"]=filter.name;
        setup["filters/detection_splitter/type"]=filter.type;
        setup["filters/detection_splitter/manufacturer"]=filter.manufacturer;
    }

    filter=ui->filtTransmission->filter();
    if (filter.isValid) {
        setup["filters/illumination_transmission/name"]=filter.name;
        setup["filters/illumination_transmission/type"]=filter.type;
        setup["filters/illumination_transmission/manufacturer"]=filter.manufacturer;
    }



    filter=ui->filtLaser11->filter();
    if (filter.isValid) {
        setup["filters/illumination_laser11/name"]=filter.name;
        setup["filters/illumination_laser11/type"]=filter.type;
        setup["filters/illumination_laser11/manufacturer"]=filter.manufacturer;
    }
    filter=ui->filtLaser12->filter();
    if (filter.isValid) {
        setup["filters/illumination_laser12/name"]=filter.name;
        setup["filters/illumination_laser12/type"]=filter.type;
        setup["filters/illumination_laser12/manufacturer"]=filter.manufacturer;
    }
    filter=ui->filtLaser21->filter();
    if (filter.isValid) {
        setup["filters/illumination_laser21/name"]=filter.name;
        setup["filters/illumination_laser21/type"]=filter.type;
        setup["filters/illumination_laser21/manufacturer"]=filter.manufacturer;
    }
    filter=ui->filtLaser22->filter();
    if (filter.isValid) {
        setup["filters/illumination_laser22/name"]=filter.name;
        setup["filters/illumination_laser22/type"]=filter.type;
        setup["filters/illumination_laser22/manufacturer"]=filter.manufacturer;
    }

    ObjectiveDescription objective=ui->objDetection->objective();
    setup["objectives/detection/name"]=objective.name;
    setup["objectives/detection/manufacturer"]=objective.manufacturer;
    setup["objectives/detection/NA"]=objective.NA;
    setup["objectives/detection/magnification"]=objective.magnification;

    objective=ui->objProjection->objective();
    setup["objectives/projection/name"]=objective.name;
    setup["objectives/projection/manufacturer"]=objective.manufacturer;
    setup["objectives/projection/NA"]=objective.NA;
    setup["objectives/projection/magnification"]=objective.magnification;

    if (setup_cam<0 || setup_cam==0) {
        objective=ui->objTube1->objective();
        setup["objectives/tube_lens1/name"]=objective.name;
        setup["objectives/tube_lens1/manufacturer"]=objective.manufacturer;
        setup["objectives/tube_lens1/NA"]=objective.NA;
        setup["objectives/tube_lens1/magnification"]=objective.magnification;
    }

    if (setup_cam<0 || setup_cam==1) {
        objective=ui->objTube2->objective();
        setup["objectives/tube_lens2/name"]=objective.name;
        setup["objectives/tube_lens2/manufacturer"]=objective.manufacturer;
        setup["objectives/tube_lens2/NA"]=objective.NA;
        setup["objectives/tube_lens2/magnification"]=objective.magnification;
    }

    setup["main_illumination/shutter"]=ui->shutterMainIllumination->getShutterState();
    setup["transmission_illumination/shutter"]=ui->shutterTransmission->getShutterState();
    if (ui->lsTransmission->getLightSource()) {
        setup["transmission_illumination/lightsource_description"]=ui->lsTransmission->getLightSource()->getLightSourceDescription(ui->lsTransmission->getLightSourceID());
        setup["transmission_illumination/short_name"]=ui->lsTransmission->getLightSource()->getLightSourceShortName(ui->lsTransmission->getLightSourceID());
        for (int i=0; i<ui->lsTransmission->getLineCount(); i++) {
            setup[QString("transmission_illumination/line%1/name").arg(i+1)]=ui->lsTransmission->getLineDescription(i);
            setup[QString("transmission_illumination/line%1/enabled").arg(i+1)]=ui->lsTransmission->isLineEnabled(i);
            setup[QString("transmission_illumination/line%1/set_power").arg(i+1)]=ui->lsTransmission->getSetPower(i);
            setup[QString("transmission_illumination/line%1/measured_power").arg(i+1)]=ui->lsTransmission->getMeasuredPower(i);
            setup[QString("transmission_illumination/line%1/unit").arg(i+1)]=ui->lsTransmission->getLineUnit(i);
        }
    }
    setup["laser1/shutter"]=ui->shutterLaser1->getShutterState();
    if (ui->lsLaser1->getLightSource()) {
        setup["laser1/lightsource_description"]=ui->lsLaser1->getLightSource()->getLightSourceDescription(ui->lsLaser1->getLightSourceID());
        setup["laser1/short_name"]=ui->lsLaser1->getLightSource()->getLightSourceShortName(ui->lsLaser1->getLightSourceID());
        for (int i=0; i<ui->lsLaser1->getLineCount(); i++) {
            setup[QString("laser1/line%1/name").arg(i+1)]=ui->lsLaser1->getLineDescription(i);
            setup[QString("laser1/line%1/enabled").arg(i+1)]=ui->lsLaser1->isLineEnabled(i);
            setup[QString("laser1/line%1/set_power").arg(i+1)]=ui->lsLaser1->getSetPower(i);
            setup[QString("laser1/line%1/measured_power").arg(i+1)]=ui->lsLaser1->getMeasuredPower(i);
            setup[QString("laser1/line%1/unit").arg(i+1)]=ui->lsLaser1->getLineUnit(i);
        }
    }
    setup["laser2/shutter"]=ui->shutterLaser2->getShutterState();
    if (ui->lsLaser2->getLightSource()) {
        setup["laser2/lightsource_description"]=ui->lsLaser2->getLightSource()->getLightSourceDescription(ui->lsLaser2->getLightSourceID());
        setup["laser2/short_name"]=ui->lsLaser2->getLightSource()->getLightSourceShortName(ui->lsLaser2->getLightSourceID());
        for (int i=0; i<ui->lsLaser2->getLineCount(); i++) {
            setup[QString("laser2/line%1/name").arg(i+1)]=ui->lsLaser2->getLineDescription(i);
            setup[QString("laser2/line%1/enabled").arg(i+1)]=ui->lsLaser2->isLineEnabled(i);
            setup[QString("laser2/line%1/set_power").arg(i+1)]=ui->lsLaser2->getSetPower(i);
            setup[QString("laser2/line%1/measured_power").arg(i+1)]=ui->lsLaser2->getMeasuredPower(i);
            setup[QString("laser2/line%1/unit").arg(i+1)]=ui->lsLaser2->getLineUnit(i);
        }
    }
    if (ui->chkDetectionFilterWheel->isChecked()) setup["filterchangers/detection/filter"]=ui->filtcDetection->getFilterChangerState();


    return setup;
}

int QFESPIMB040OpticsSetup::getCameraCount() const
{
    return 2;
}


void QFESPIMB040OpticsSetup::setLogging(QFPluginLogService* log) {
    m_log=log;
    ui->camConfig1->setLog(m_log);
    ui->camConfig2->setLog(m_log);
    ui->stageSetup->setLog(m_log);
    ui->shutterMainIllumination->setLog(m_log);
    ui->shutterLaser1->setLog(m_log);
    ui->shutterLaser2->setLog(m_log);
    ui->filtcDetection->setLog(m_log);
    ui->shutterTransmission->setLog(m_log);
    ui->lsLaser1->setLog(m_log);
    ui->lsLaser2->setLog(m_log);
    ui->lsTransmission->setLog(m_log);
}

bool QFESPIMB040OpticsSetup::lockCamera(int setup_cam, QFExtension** extension, QFExtensionCamera** ecamera, int* camera, QString* previewSettingsFilename) {
    switch(setup_cam) {
        case 0: return ui->camConfig1->lockCamera(extension, ecamera, camera,  previewSettingsFilename);
        case 1: return ui->camConfig2->lockCamera(extension, ecamera, camera,  previewSettingsFilename);
    }
    return false;
}

void QFESPIMB040OpticsSetup::releaseCamera(int setup_cam) {
    switch(setup_cam) {
        case 0: ui->camConfig1->releaseCamera(); break;
        case 1: ui->camConfig2->releaseCamera(); break;
    }
}

void QFESPIMB040OpticsSetup::overrideCameraPreview(int setup_cam, const QString &camera_settings, const QString &lightpath)
{
    if (!lightpath.isEmpty() && QFile::exists(lightpath)) {
        ui->cmbLightpathConfig->setEnabled(false);
        lockLightpath();
        loadLightpathConfig(lightpath, true);
        unlockLightpath();
    }
    if (setup_cam==0) ui->camConfig1->overridePreview(camera_settings);
    if (setup_cam==1) ui->camConfig2->overridePreview(camera_settings);
    loadLightpathConfig(lightpath, true);
}

void QFESPIMB040OpticsSetup::resetCameraPreview(int setup_cam)
{

    if (setup_cam==0) ui->camConfig1->resetPreview();
    if (setup_cam==1) ui->camConfig2->resetPreview();
    ensureLightpath();
    ui->cmbLightpathConfig->setEnabled(true);
}

void QFESPIMB040OpticsSetup::on_btnConnectDevices_clicked() {
    QProgressListDialog* dlg=new QProgressListDialog(this, Qt::Dialog|Qt::WindowStaysOnBottomHint);
    dlg->setWindowModality(Qt::WindowModal);
    dlg->setWindowIcon(windowIcon());
    dlg->setWindowTitle(tr("Connecting devices ..."));
    dlg->addItem(tr("shutter: main"));
    dlg->addItem(tr("shutter: laser 1"));
    dlg->addItem(tr("shutter: laser 2"));
    dlg->addItem(tr("shutter: transmission illumination"));
    dlg->addItem(tr("lightsource: laser 1"));
    dlg->addItem(tr("lightsource: laser 2"));
    dlg->addItem(tr("lightsource: transmission"));
    dlg->addItem(tr("stage"));
    dlg->addItem(tr("filterwheel: detection"));
    dlg->addItem(tr("measurement device 1"));
    dlg->addItem(tr("measurement device 2"));

    dlg->setHasCancelButton(true);
    dlg->show();
    dlg->start();

    if (!dlg->wasCanceled()) ui->shutterMainIllumination->connectShutter();
    dlg->nextItem((ui->shutterMainIllumination->isShutterConnected())?(QProgressListWidget::statusDone):(QProgressListWidget::statusFailed));
    QApplication::processEvents();
    if (!dlg->wasCanceled()) ui->shutterLaser1->connectShutter();
    dlg->nextItem((ui->shutterLaser1->isShutterConnected())?(QProgressListWidget::statusDone):(QProgressListWidget::statusFailed));
    QApplication::processEvents();
    if (!dlg->wasCanceled()) ui->shutterLaser2->connectShutter();
    dlg->nextItem((ui->shutterLaser2->isShutterConnected())?(QProgressListWidget::statusDone):(QProgressListWidget::statusFailed));
    QApplication::processEvents();
    if (!dlg->wasCanceled()) ui->shutterTransmission->connectShutter();
    dlg->nextItem((ui->shutterTransmission->isShutterConnected())?(QProgressListWidget::statusDone):(QProgressListWidget::statusFailed));
    QApplication::processEvents();
    if (!dlg->wasCanceled()) ui->lsLaser1->connectLightSource();
    dlg->nextItem((ui->lsLaser1->isLightSourceConnected())?(QProgressListWidget::statusDone):(QProgressListWidget::statusFailed));
    QApplication::processEvents();
    if (!dlg->wasCanceled()) ui->lsLaser2->connectLightSource();
    dlg->nextItem((ui->lsLaser2->isLightSourceConnected())?(QProgressListWidget::statusDone):(QProgressListWidget::statusFailed));
    QApplication::processEvents();
    if (!dlg->wasCanceled()) ui->lsTransmission->connectLightSource();
    dlg->nextItem((ui->lsTransmission->isLightSourceConnected())?(QProgressListWidget::statusDone):(QProgressListWidget::statusFailed));
    QApplication::processEvents();
    if (!dlg->wasCanceled()) ui->stageSetup->connectStages();
    dlg->nextItem((ui->stageSetup->isXStageConnected()||ui->stageSetup->isYStageConnected()||ui->stageSetup->isZStageConnected()||ui->stageSetup->isRStageConnected())?(QProgressListWidget::statusDone):(QProgressListWidget::statusFailed));
    QApplication::processEvents();
    if (!dlg->wasCanceled()) if (ui->chkDetectionFilterWheel->isChecked()) ui->filtcDetection->connectFilterChanger();
    dlg->nextItem((ui->filtcDetection->isFilterChangerConnected())?(QProgressListWidget::statusDone):(QProgressListWidget::statusFailed));
    QApplication::processEvents();
    if (!dlg->wasCanceled()) ui->widMD1->connectMeasurementDevice();
    dlg->nextItem((ui->widMD1->isMeasurementDeviceConnected())?(QProgressListWidget::statusDone):(QProgressListWidget::statusFailed));
    QApplication::processEvents();
    if (!dlg->wasCanceled()) ui->widMD2->connectMeasurementDevice();
    dlg->nextItem((ui->widMD2->isMeasurementDeviceConnected())?(QProgressListWidget::statusDone):(QProgressListWidget::statusFailed));

    dlg->close();
    delete dlg;
}

void QFESPIMB040OpticsSetup::on_btnDisconnectDevices_clicked() {
    QProgressListDialog* dlg=new QProgressListDialog(this, Qt::Dialog|Qt::WindowStaysOnBottomHint);
    dlg->setWindowModality(Qt::WindowModal);
    dlg->setWindowIcon(windowIcon());
    dlg->setWindowTitle(tr("Disconnecting devices ..."));
    dlg->addItem(tr("shutter: main"));
    dlg->addItem(tr("shutter: laser 1"));
    dlg->addItem(tr("shutter: laser 2"));
    dlg->addItem(tr("shutter: transmission illumination"));
    dlg->addItem(tr("lightsource: laser 1"));
    dlg->addItem(tr("lightsource: laser 2"));
    dlg->addItem(tr("lightsource: transmission"));
    dlg->addItem(tr("stage"));
    dlg->addItem(tr("filterwheel: detection"));
    dlg->addItem(tr("measurement device 1"));
    dlg->addItem(tr("measurement device 2"));
    dlg->setHasCancelButton(false);
    dlg->show();
    dlg->start();

    ui->shutterMainIllumination->disconnectShutter();
    dlg->nextItem();
    QApplication::processEvents();
    ui->shutterLaser1->disconnectShutter();
    dlg->nextItem();
    QApplication::processEvents();
    ui->shutterLaser2->disconnectShutter();
    dlg->nextItem();
    QApplication::processEvents();
    ui->shutterTransmission->disconnectShutter();
    dlg->nextItem();
    QApplication::processEvents();
    ui->lsLaser1->disconnectLightSource();
    dlg->nextItem();
    QApplication::processEvents();
    ui->lsLaser2->disconnectLightSource();
    dlg->nextItem();
    QApplication::processEvents();
    ui->lsTransmission->disconnectLightSource();
    dlg->nextItem();
    QApplication::processEvents();
    ui->stageSetup->disconnectStages();
    dlg->nextItem();
    QApplication::processEvents();
    if (ui->chkDetectionFilterWheel->isChecked()) ui->filtcDetection->disconnectFilterChanger();
    dlg->nextItem();
    QApplication::processEvents();
    ui->widMD1->disconnectMeasurementDevice();
    dlg->nextItem();
    QApplication::processEvents();
    ui->widMD2->disconnectMeasurementDevice();
    dlg->nextItem();
    QApplication::processEvents();
    dlg->close();
    delete dlg;
}
void QFESPIMB040OpticsSetup::on_btnConnectCameras_clicked() {
    QProgressListDialog* dlg=new QProgressListDialog(this, Qt::Dialog|Qt::WindowStaysOnBottomHint);
    dlg->setWindowModality(Qt::WindowModal);
    dlg->setWindowIcon(windowIcon());
    dlg->setWindowTitle(tr("Connecting cameras ..."));
    dlg->addItem(tr("camera 1"));
    dlg->addItem(tr("camera 2"));
    dlg->setHasCancelButton(true);
    dlg->show();
    dlg->start();

    if (!dlg->wasCanceled()) ui->camConfig1->connectCamera();
    dlg->nextItem((ui->camConfig1->isCameraConnected())?(QProgressListWidget::statusDone):(QProgressListWidget::statusFailed));
    QApplication::processEvents();
    if (!dlg->wasCanceled()) ui->camConfig2->connectCamera();
    dlg->nextItem((ui->camConfig2->isCameraConnected())?(QProgressListWidget::statusDone):(QProgressListWidget::statusFailed));
    QApplication::processEvents();

    dlg->close();
    delete dlg;

}


void QFESPIMB040OpticsSetup::on_btnDisconnectCameras_clicked() {
    QProgressListDialog* dlg=new QProgressListDialog(this, Qt::Dialog|Qt::WindowStaysOnBottomHint);
    dlg->setWindowModality(Qt::WindowModal);
    dlg->setWindowIcon(windowIcon());
    dlg->setWindowTitle(tr("Disconnecting cameras ..."));
    dlg->addItem(tr("camera 1"));
    dlg->addItem(tr("camera 2"));
    dlg->setHasCancelButton(true);
    dlg->show();
    dlg->start();

    if (!dlg->wasCanceled()) ui->camConfig1->disconnectCamera();
    dlg->nextItem();
    QApplication::processEvents();
    if (!dlg->wasCanceled()) ui->camConfig2->disconnectCamera();
    dlg->nextItem();
    QApplication::processEvents();

    dlg->close();
    delete dlg;

}

void QFESPIMB040OpticsSetup::on_chkDetectionFilterWheel_toggled(bool checked) {
    if (!checked) {
        ui->filtcDetection->disconnectFilterChanger();
    }
}

void QFESPIMB040OpticsSetup::configsChanged(QFESPIMB040OpticsSetupItems configs) {
    emit lightpathesChanged(configs);
}


void QFESPIMB040OpticsSetup::configShortcuts() {
    QFESPIMB040ShortcutConfigDialog* dlg=new QFESPIMB040ShortcutConfigDialog(this);

    for (int i=0; i<shortcuts.size(); i++) {
        dlg->addShortcut(shortcuts[i].label, shortcuts[i].shortcut->key());
    }

    if (dlg->exec()==QDialog::Accepted) {
        for (int i=0; i<shortcuts.size(); i++) {
            QKeySequence seq=dlg->getSequence(i);
            shortcuts[i].shortcut->setKey(seq);
            shortcuts[i].shortcut->setEnabled(!seq.isEmpty());
        }
    }
    delete dlg;
}


void QFESPIMB040OpticsSetup::updateMagnifications() {
    double m=ui->objDetection->objective().magnification*ui->objTube1->objective().magnification;
    ui->camConfig1->setMagnification(m);
    m=ui->objDetection->objective().magnification*ui->objTube2->objective().magnification;
    ui->camConfig2->setMagnification(m);
}




QFExtensionFilterChanger *QFESPIMB040OpticsSetup::getFilterChanger(int changer) const
{
    if (changer==FilterChangerDetection) return ui->filtcDetection->getFilterChanger();
    return NULL;
}

int QFESPIMB040OpticsSetup::getFilterChangerID(int changer) const
{
    if (changer==0) return ui->filtcDetection->getFilterChangerID();
    return -1;
}

QString QFESPIMB040OpticsSetup::getFilterChangerName(int changer) const
{
    if (changer==0) return tr("detection");
    return QString();
}

int QFESPIMB040OpticsSetup::getFilterChangerCount() const
{
    return 1;
}

QFExtensionLightSource *QFESPIMB040OpticsSetup::getLaser(int laser)
{
    if (laser==0) return ui->lsLaser1->getLightSource();
    if (laser==1) return ui->lsLaser2->getLightSource();
    return NULL;
}


QFExtensionLightSource *QFESPIMB040OpticsSetup::getBrightfieldLightSource(int source)
{
    if (source==0) return ui->lsTransmission->getLightSource();
    return NULL;
}

int QFESPIMB040OpticsSetup::getLaserID(int laser)
{
    if (laser==0) return ui->lsLaser1->getLightSourceID();
    if (laser==1) return ui->lsLaser2->getLightSourceID();
    return -1;
}


int QFESPIMB040OpticsSetup::getBrightfieldLightSourceID(int source)
{
    if (source==0) return ui->lsTransmission->getLightSourceID();
    return -1;
}

int QFESPIMB040OpticsSetup::getLaserCount() const
{
    return 2;
}

bool QFESPIMB040OpticsSetup::isStageConnected(QFExtensionLinearStage* stage, int id, bool& found) {
    found=false;
    if (!stage || id<0) return false;
    found=true;
    if (stage==getStage(QFESPIMB040OpticsSetupBase::StageX) && id==getStageAxis(QFESPIMB040OpticsSetupBase::StageX)) return isStageConnected(QFESPIMB040OpticsSetupBase::StageX);
    if (stage==getStage(QFESPIMB040OpticsSetupBase::StageY) && id==getStageAxis(QFESPIMB040OpticsSetupBase::StageY)) return isStageConnected(QFESPIMB040OpticsSetupBase::StageY);
    if (stage==getStage(QFESPIMB040OpticsSetupBase::StageZ) && id==getStageAxis(QFESPIMB040OpticsSetupBase::StageZ)) return isStageConnected(QFESPIMB040OpticsSetupBase::StageZ);
    if (stage==getStage(QFESPIMB040OpticsSetupBase::StageR) && id==getStageAxis(QFESPIMB040OpticsSetupBase::StageR)) return isStageConnected(QFESPIMB040OpticsSetupBase::StageR);
    found=false;
    return false;
}

int QFESPIMB040OpticsSetup::getStageAxis(int stage)
{
    if (stage==QFESPIMB040OpticsSetupBase::StageX) return ui->stageSetup->getXStageAxis();
    if (stage==QFESPIMB040OpticsSetupBase::StageY) return ui->stageSetup->getYStageAxis();
    if (stage==QFESPIMB040OpticsSetupBase::StageZ) return ui->stageSetup->getZStageAxis();
    if (stage==QFESPIMB040OpticsSetupBase::StageR) return ui->stageSetup->getRStageAxis();
    return -1;

}

QString QFESPIMB040OpticsSetup::getStageName(int stage) const
{
    if (stage==QFESPIMB040OpticsSetupBase::StageX) return "x";
    if (stage==QFESPIMB040OpticsSetupBase::StageY) return "y";
    if (stage==QFESPIMB040OpticsSetupBase::StageZ) return "z";
    if (stage==QFESPIMB040OpticsSetupBase::StageR) return "r";
    return "";

}

int QFESPIMB040OpticsSetup::getStageCount() const
{
    return 4;
}


bool QFESPIMB040OpticsSetup::isStageConnected(int stage) const
{
    if (stage==QFESPIMB040OpticsSetupBase::StageX) return ui->stageSetup->isXStageConnected();
    if (stage==QFESPIMB040OpticsSetupBase::StageY) return ui->stageSetup->isYStageConnected();
    if (stage==QFESPIMB040OpticsSetupBase::StageZ) return ui->stageSetup->isZStageConnected();
    if (stage==QFESPIMB040OpticsSetupBase::StageR) return ui->stageSetup->isRStageConnected();
    return false;
}


QFExtension *QFESPIMB040OpticsSetup::getStageExtension(int stage)
{
    if (stage==QFESPIMB040OpticsSetupBase::StageX) return ui->stageSetup->getXStageExtension();
    if (stage==QFESPIMB040OpticsSetupBase::StageY) return ui->stageSetup->getYStageExtension();
    if (stage==QFESPIMB040OpticsSetupBase::StageZ) return ui->stageSetup->getZStageExtension();
    if (stage==QFESPIMB040OpticsSetupBase::StageR) return ui->stageSetup->getRStageExtension();
    return NULL;
}

QFExtensionLinearStage *QFESPIMB040OpticsSetup::getStage(int stage)
{
    if (stage==QFESPIMB040OpticsSetupBase::StageX) return ui->stageSetup->getXStage();
    if (stage==QFESPIMB040OpticsSetupBase::StageY) return ui->stageSetup->getYStage();
    if (stage==QFESPIMB040OpticsSetupBase::StageZ) return ui->stageSetup->getZStage();
    if (stage==QFESPIMB040OpticsSetupBase::StageR) return ui->stageSetup->getRStage();
    return NULL;
}


QFCameraComboBox* QFESPIMB040OpticsSetup::cameraComboBox(int camera) const {
    if (camera==0) return ui->camConfig1->cameraComboBox();
    if (camera==1) return ui->camConfig2->cameraComboBox();

    return NULL;
}

QFESPIMB040SimpleCameraConfig *QFESPIMB040OpticsSetup::cameraConfig(int camera) const
{
    if (camera==0) return ui->camConfig1;
    if (camera==1) return ui->camConfig2;

    return NULL;
}

QFCameraConfigComboBoxStartResume* QFESPIMB040OpticsSetup::getStopRelease(int camera) const {
    if (camera==0) return ui->camConfig1;
    if (camera==1) return ui->camConfig2;

    return NULL;
}

bool QFESPIMB040OpticsSetup::setMainIlluminationShutter(bool opened, bool blocking) {
    if (!isMainIlluminationShutterAvailable()) return false;

    ui->shutterMainIllumination->setShutter(opened);
    //if (opened==false) ui->shutterTransmission->setShutter(false);

    if (blocking) {
        QTime t;
        t.start();
        while (getMainIlluminationShutter()!=opened && t.elapsed()<10000) {
            QApplication::processEvents();
        }

        if (t.elapsed()>=10000) {
            m_log->log_error("main shutter timed out after 10s!\n");
            return false;
        }
        return true;
    }
    return true;
}

bool QFESPIMB040OpticsSetup::setAlex(bool AlexOnOff, bool blocking) {
    if (!isMainIlluminationShutterAvailable()) return false;

    ui->shutterMainIllumination->setAlex(AlexOnOff);
    return true;
}


void QFESPIMB040OpticsSetup::setSpecialShutter(int shutter, bool opened, bool blocking)
{
    if (shutter==QFESPIMB040OpticsSetup::ShutterMain) {
        setMainIlluminationShutter(opened, blocking);
    } else if (shutter==QFESPIMB040OpticsSetup::ShutterLaser1) {
        ui->shutterLaser1->setShutter(opened);
        if (!opened) ui->shutterLaser1->setShutter(false);

        if (blocking) {
            QTime t;
            t.start();
            while (ui->shutterLaser1->getShutterState()!=opened && t.elapsed()<10000) {
                QApplication::processEvents();
            }

            if (t.elapsed()>=10000) {
                m_log->log_error("laser 1 shutter timed out after 10s!\n");
            }
        }
    } else if (shutter==QFESPIMB040OpticsSetup::ShutterLaser2) {
        ui->shutterLaser2->setShutter(opened);
        if (!opened) ui->shutterLaser2->setShutter(false);

        if (blocking) {
            QTime t;
            t.start();
            while (ui->shutterLaser2->getShutterState()!=opened && t.elapsed()<10000) {
                QApplication::processEvents();
            }

            if (t.elapsed()>=10000) {
                m_log->log_error("laser 2 shutter timed out after 10s!\n");
            }
        }
    } else if (shutter==QFESPIMB040OpticsSetup::ShutterTransmission) {
        ui->shutterTransmission->setShutter(opened);
        if (!opened) ui->shutterTransmission->setShutter(false);

        if (blocking) {
            QTime t;
            t.start();
            while (ui->shutterTransmission->getShutterState()!=opened && t.elapsed()<10000) {
                QApplication::processEvents();
            }

            if (t.elapsed()>=10000) {
                m_log->log_error("transmission shutter timed out after 10s!\n");
            }
        }
    }
}

void QFESPIMB040OpticsSetup::setShutter(int shutter, bool opened, bool blocking)
{
    setSpecialShutter(shutter, opened, blocking);
}

int QFESPIMB040OpticsSetup::getShutterCount() const
{
    return 4;
}



bool QFESPIMB040OpticsSetup::getMainIlluminationShutter() {
    return ui->shutterMainIllumination->getShutterState();
}

QString QFESPIMB040OpticsSetup::getCurrentLightpathFilename() const {
    return ui->cmbLightpathConfig->currentConfigFilename();
}

QString QFESPIMB040OpticsSetup::getCurrentLightpath() const {
    return ui->cmbLightpathConfig->currentText();
}


void QFESPIMB040OpticsSetup::saveLightpathConfig(const QString &filename, const QString& name, QList<bool> saveProp, bool saveMeasured) {
    { // this block ensures that set is destroyed (and the file written) before updateItems() is called
        if (QFile::exists(filename)) QFile::remove(filename);
        QSettings set(filename, QSettings::IniFormat);
        set.clear();
        set.setValue("name", name);


        QMap<QString, QVariant> data;
        saveLightpathConfig(data, name, QString(""), saveProp, saveMeasured);
        QMap<QString, QVariant>::iterator i=data.begin();
        for (i = data.begin(); i != data.end(); ++i) {
            set.setValue(i.key(), i.value());
        }

    }
    ui->cmbLightpathConfig->updateItems(name);
    ui->cmbLightpathConfig->setEnabled(true);
}


void QFESPIMB040OpticsSetup::deleteCurrentLightpatConfig() {
    QString fn=ui->cmbLightpathConfig->currentConfigFilename();
    QString name=ui->cmbLightpathConfig->currentConfig();
    if (QFile::exists(fn)) {
        if (QMessageBox::question(this, tr("B040 SPIM"), tr("Do you really want to delete lightpath '%1'?\nfile: '%2'").arg(name).arg(fn),
                                  QMessageBox::Yes|QMessageBox::No, QMessageBox::No)==QMessageBox::Yes)
        {
            QFile::remove(fn);
            ui->cmbLightpathConfig->updateItems();
        }
    }
    ui->cmbLightpathConfig->setEnabled(true);
}

void QFESPIMB040OpticsSetup::emitLighpathesChanged() {
    ui->cmbLightpathConfig->updateItems(ui->cmbLightpathConfig->currentText());
    ui->cmbLightpathConfig->setEnabled(true);
}

bool QFESPIMB040OpticsSetup::isMainIlluminationShutterAvailable()  {
    return ui->shutterMainIllumination->isShutterConnected();
}


void QFESPIMB040OpticsSetup::userChangedLightpath(QString filename) {
    lockLightpath();
    ui->cmbLightpathConfig->setEnabled(false);
    loadLightpathConfig(filename, true);
    ui->cmbLightpathConfig->setEnabled(true);
    unlockLightpath();
    ui->cmbLightpathConfig->setEnabled(true);
}

void QFESPIMB040OpticsSetup::unlockLighpathCombobox()
{
    ui->cmbLightpathConfig->setEnabled(true);
}

void QFESPIMB040OpticsSetup::lockLighpathCombobox()
{
    ui->cmbLightpathConfig->setEnabled(false);
}

void QFESPIMB040OpticsSetup::ensureLightpath() {
   //qDebug()<<"ensureLightpath()";
    lockLightpath();
   //qDebug()<<"ensureLightpath(): locked";
    ui->cmbLightpathConfig->setEnabled(false);
   //qDebug()<<"ensureLightpath(): disabled";
    loadLightpathConfig(getCurrentLightpathFilename(), true);
   //qDebug()<<"ensureLightpath(): load";
    ui->cmbLightpathConfig->setEnabled(true);
   //qDebug()<<"ensureLightpath(): enabled";
    unlockLightpath();
    //qDebug()<<"ensureLightpath(): unlocked";
    ui->cmbLightpathConfig->setEnabled(true);
}




























void QFESPIMB040OpticsSetup::saveLightpathConfig(QMap<QString, QVariant> &data, const QString &name, const QString& prefix,  QList<bool> saveProp, bool saveMeasured) {
    data[prefix+"name"]=name;

    //qDebug()<<saveProp;

    // SAVE RELEVANT WIDGETS HERE
    if (saveProp.value(0, true) && ui->lsTransmission->getLightSource() && ui->lsTransmission->isLightSourceConnected()) {
        for (int i=0; i<ui->lsTransmission->getLineCount(); i++) {
            data[prefix+QString("transmission/line%1/enabled").arg(i+1)]=ui->lsTransmission->isLineEnabled(i);
            data[prefix+QString("transmission/line%1/power").arg(i+1)]=ui->lsTransmission->getSetPower(i);
            if (saveMeasured) {
                data[prefix+QString("transmission/line%1/measured_power").arg(i+1)]=ui->lsTransmission->getMeasuredPower(i);
            }
        }
    }
    if (saveProp.value(1, true) && ui->shutterTransmission->getShutter() && ui->shutterTransmission->getShutter()->isShutterConnected(ui->shutterTransmission->getShutterID())) {
        data[prefix+"transmission/shutter/state"]=ui->shutterTransmission->getShutterState();
    }
    if (saveProp.value(2, true) && ui->lsLaser1->getLightSource() && ui->lsLaser1->isLightSourceConnected()) {
        for (int i=0; i<ui->lsLaser1->getLineCount(); i++) {
            data[prefix+QString("laser1/line%1/enabled").arg(i+1)]=ui->lsLaser1->isLineEnabled(i);
            data[prefix+QString("laser1/line%1/power").arg(i+1)]=ui->lsLaser1->getSetPower(i);
            if (saveMeasured) {
                data[prefix+QString("laser1/line%1/measured_power").arg(i+1)]=ui->lsLaser1->getMeasuredPower(i);
            }
        }
    }
    if (saveProp.value(3, true) && ui->shutterLaser1->getShutter() && ui->shutterLaser1->getShutter()->isShutterConnected(ui->shutterLaser1->getShutterID())) {
        data[prefix+"laser1/shutter/state"]=ui->shutterLaser1->getShutterState();
    }
    if (saveProp.value(4, true) && ui->lsLaser2->getLightSource() && ui->lsLaser2->isLightSourceConnected()) {
        for (int i=0; i<ui->lsLaser2->getLineCount(); i++) {
            data[prefix+QString("laser2/line%1/enabled").arg(i+1)]=ui->lsLaser2->isLineEnabled(i);
            data[prefix+QString("laser2/line%1/power").arg(i+1)]=ui->lsLaser2->getSetPower(i);
            if (saveMeasured) {
                data[prefix+QString("laser2/line%1/measured_power").arg(i+1)]=ui->lsLaser2->getMeasuredPower(i);
            }
        }
    }
    if (saveProp.value(5, true) && ui->shutterLaser2->getShutter() && ui->shutterLaser2->getShutter()->isShutterConnected(ui->shutterLaser2->getShutterID())) {
        data[prefix+"laser2/shutter/state"]=ui->shutterLaser2->getShutterState();
    }
    if (saveProp.value(6, true) && ui->chkDetectionFilterWheel->isChecked() && ui->filtcDetection->getFilterChanger() && ui->filtcDetection->isFilterChangerConnected()) {
        data[prefix+"detection/filterchanger/filter"]=ui->filtcDetection->getFilterChangerState();
        if (saveMeasured) {
            data[prefix+"detection/filterchanger/filter_name"]=ui->filtcDetection->getCurrentFilter();
            data[prefix+"detection/filterchanger/filter_manufacturer"]=ui->filtcDetection->getCurrentFilterDescription().manufacturer;
            data[prefix+"detection/filterchanger/filter_type"]=ui->filtcDetection->getCurrentFilterDescription().type;
        }
    }

    if (saveMeasured) {
        QFESPIMB040OpticsSetup::measuredValues m=getMeasuredValues();
        QMapIterator<QString, QVariant> it(m.data);
        while (it.hasNext()) {
            it.next();
            if (!data.contains(prefix+it.key())) {
                data[prefix+it.key()]=it.value();
            }
        }

    }
}

void QFESPIMB040OpticsSetup::saveCurrentLightpatConfig() {
    bool dlgOK=true;
    QString name=ui->cmbLightpathConfig->currentText();

    QStringList lightpathlist;
    lightpathlist<<tr("transmission illumintaion: source");
    lightpathlist<<tr("transmission illumintaion: shutter");
    lightpathlist<<tr("laser 1: source");
    lightpathlist<<tr("laser 1: shutter");
    lightpathlist<<tr("laser 2: source");
    lightpathlist<<tr("laser 2: shutter");
    lightpathlist<<tr("detection: filter changer");

    lightpathConfigModel->setReadonly(false);
    lightpathConfigModel->resize(lightpathlist.size(), 2);
    lightpathConfigModel->setColumnTitle(1, tr("item"));
    for (int i=0; i<lightpathlist.size(); i++) {
        lightpathConfigModel->setCellCheckedRole(i, 0, Qt::Checked);
        lightpathConfigModel->setCell(i, 1, lightpathlist[i]);
    }

    lightpathConfigModel->setCellCheckedRole(0, 0, (ui->lsTransmission->isLightSourceConnected())?Qt::Checked:Qt::Unchecked);
    lightpathConfigModel->setCellCheckedRole(1, 0, (ui->shutterTransmission->isShutterConnected())?Qt::Checked:Qt::Unchecked);
    lightpathConfigModel->setCellCheckedRole(2, 0, Qt::Unchecked);
    lightpathConfigModel->setCellCheckedRole(3, 0, (ui->shutterLaser1->isShutterConnected())?Qt::Checked:Qt::Unchecked);
    lightpathConfigModel->setCellCheckedRole(4, 0, Qt::Unchecked);
    lightpathConfigModel->setCellCheckedRole(5, 0, (ui->shutterLaser2->isShutterConnected())?Qt::Checked:Qt::Unchecked);
    lightpathConfigModel->setCellCheckedRole(6, 0, (ui->chkDetectionFilterWheel->isChecked()&&ui->filtcDetection->isFilterChangerConnected())?Qt::Checked:Qt::Unchecked);
    lightpathConfigModel->setReadonlyButStillCheckable(true);


    while (true) {
        QStringList l=ui->cmbLightpathConfig->getConfigs();
        int idx=-1;
        if (!name.isEmpty() && !l.contains(name)) {
            l.append(name);
        }
        idx=l.indexOf(name);
        /*name=QInputDialog::getItem(this, tr("B040 SPIM"),
                                      tr("Enter a name for the lightpath:"), l,idx, true, &dlgOK);*/
        QFESPIMB040LightPathSaveDialog* dlg=new QFESPIMB040LightPathSaveDialog(name, l, lightpathConfigModel, this);
        if (dlg->exec()==QDialog::Accepted) {
            name=dlg->getName();
            if (!name.isEmpty()) {
                QString filename=ProgramOptions::getConfigValue("spimb040/optsetup_config_directory", m_pluginServices->getConfigFileDirectory()+"/plugins/ext_spimb040/").toString()+cleanStringForFilename(name)+".lpc";
                if (QFile::exists(filename)) {
                    QMessageBox::StandardButton res=QMessageBox::question(this, tr("B040 SPIM"),
                                                                          tr("A lightpath config with the name '%1' already exists.\n  filename: '%2'\nOverwrite?").arg(name).arg(filename),
                                                                          QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel, QMessageBox::Yes);
                    if (res==QMessageBox::Yes) {
                        //qDebug()<<dlg->getCheckedItems();
                        QFile::remove(filename);
                        //qDebug()<<dlg->getCheckedItems();
                        saveLightpathConfig(filename, name, dlg->getCheckedItems());
                        ui->cmbLightpathConfig->setCurrentConfig(name);
                        return;
                    } else if (res==QMessageBox::Cancel) {
                        return;
                    }
                } else {
                    saveLightpathConfig(filename, name);
                    ui->cmbLightpathConfig->setCurrentConfig(name);
                    return;
                }
            }
            delete dlg;
        } else {
            delete dlg;
            return;
        }

    }
    ui->cmbLightpathConfig->setEnabled(true);
}

bool QFESPIMB040OpticsSetup::lightpathLoaded(const QString &filename) {
    if (setting_lightpath) {
        bool ok=true;
        QSettings set(filename, QSettings::IniFormat);
        if (ok && set.contains("laser1/shutter/state")) ok=ok&&ui->shutterLaser1->isShutterDone()&&(ui->shutterLaser1->getShutterState()==set.value("laser1/shutter/state").toBool());
        if (ok && set.contains("laser2/shutter/state")) ok=ok&&ui->shutterLaser2->isShutterDone()&&(ui->shutterLaser2->getShutterState()==set.value("laser2/shutter/state").toBool());
        if (ok && set.contains("transmission/shutter/state")) ok=ok&&ui->shutterTransmission->isShutterDone()&&(ui->shutterTransmission->getShutterState()==set.value("transmission/shutter/state").toBool());
        if (ui->lsTransmission->isLightSourceConnected()) {
            for (int i=0; i<ui->lsTransmission->getLineCount(); i++) {
                if (ok && set.contains(QString("transmission/line%1/enabled").arg(i+1))) ok=ok&&ui->lsTransmission->isLastActionComplete()&&(ui->lsTransmission->isLineEnabled(i)==set.value(QString("transmission/line%1/enabled").arg(i+1)).toBool());
                if (ok && set.contains(QString("transmission/line%1/power").arg(i+1))) ok=ok&&ui->lsTransmission->isLastActionComplete();
            }
        }
        if (ui->lsLaser1->isLightSourceConnected()) {
            for (int i=0; i<ui->lsLaser1->getLineCount(); i++) {
                if (ok && set.contains(QString("laser1/line%1/enabled").arg(i+1))) ok=ok&&ui->lsLaser1->isLastActionComplete()&&(ui->lsLaser1->isLineEnabled(i)==set.value(QString("laser1/line%1/enabled").arg(i+1)).toBool());
                if (ok && set.contains(QString("laser1/line%1/power").arg(i+1))) ok=ok&&ui->lsLaser1->isLastActionComplete();
            }
        }
        if (ui->lsLaser2->isLightSourceConnected()) {
            for (int i=0; i<ui->lsLaser2->getLineCount(); i++) {
                if (ok && set.contains(QString("laser2/line%1/enabled").arg(i+1))) ok=ok&&ui->lsLaser2->isLastActionComplete()&&(ui->lsLaser2->isLineEnabled(i)==set.value(QString("laser2/line%1/enabled").arg(i+1)).toBool());
                if (ok && set.contains(QString("laser2/line%1/power").arg(i+1))) ok=ok&&ui->lsLaser2->isLastActionComplete();
            }
        }
        if (ok && set.contains("detection/filterchanger/filter") && ui->chkDetectionFilterWheel->isChecked())
            ok=ok&&ui->filtcDetection->isLastActionComplete()&&(ui->filtcDetection->getFilterChangerState()==set.value("detection/filterchanger/filter").toInt());
        return ok;
    }
    return true;
}


void QFESPIMB040OpticsSetup::loadLightpathConfig(const QString &filename, bool waiting, QString*name) {
    if (!QFile::exists(filename)) return;
    //qDebug()<<"loadLightpathConfig("<<filename<<")";
    QSettings set(filename, QSettings::IniFormat);

    if (name) *name=set.value("name", *name).toString();

    // LOAD RELEVANT WIDGETS HERE
    if (set.contains("laser1/shutter/state")) ui->shutterLaser1->setShutter(set.value("laser1/shutter/state").toBool());
    if (set.contains("laser2/shutter/state")) ui->shutterLaser2->setShutter(set.value("laser2/shutter/state").toBool());
    if (set.contains("transmission/shutter/state")) ui->shutterTransmission->setShutter(set.value("transmission/shutter/state").toBool());
    if (set.contains("detection/filterchanger/filter") && ui->chkDetectionFilterWheel->isChecked()) ui->filtcDetection->setFilterChanger(set.value("detection/filterchanger/filter").toInt());
    if (ui->lsTransmission->isLightSourceConnected()) {
        for (int i=0; i<ui->lsTransmission->getLineCount(); i++) {
            if (set.contains(QString("transmission/line%1/enabled").arg(i+1))) ui->lsTransmission->setLineEnabled(i, set.value(QString("transmission/line%1/enabled").arg(i+1)).toBool());
            if (set.contains(QString("transmission/line%1/power").arg(i+1))) ui->lsTransmission->setLinePower(i, set.value(QString("transmission/line%1/power").arg(i+1)).toDouble());
        }
    }
    if (ui->lsLaser1->isLightSourceConnected()) {
        for (int i=0; i<ui->lsLaser1->getLineCount(); i++) {
            if (set.contains(QString("laser1/line%1/enabled").arg(i+1))) ui->lsLaser1->setLineEnabled(i, set.value(QString("laser1/line%1/enabled").arg(i+1)).toBool());
            if (set.contains(QString("laser1/line%1/power").arg(i+1))) ui->lsLaser1->setLinePower(i, set.value(QString("laser1/line%1/power").arg(i+1)).toDouble());
        }
    }
    if (ui->lsLaser2->isLightSourceConnected()) {
        for (int i=0; i<ui->lsLaser2->getLineCount(); i++) {
            if (set.contains(QString("laser2/line%1/enabled").arg(i+1))) ui->lsLaser2->setLineEnabled(i, set.value(QString("laser2/line%1/enabled").arg(i+1)).toBool());
            if (set.contains(QString("laser2/line%1/power").arg(i+1))) ui->lsLaser2->setLinePower(i, set.value(QString("laser2/line%1/power").arg(i+1)).toDouble());
        }
    }
    if (waiting) {
        setting_lightpath=true;
        QTime t1;
        t1.start();
        while (!lightpathLoaded(filename)) {
            if (t1.elapsed()>20) {
                QApplication::processEvents();
                t1.start();
            }
        }
        setting_lightpath=false;
    } else {
        setting_lightpath=true;
    }

}


void QFESPIMB040OpticsSetup::lockLightpath() {
    lockLighpathCombobox();
   //qDebug()<<"locking lighpath 1";
    ui->shutterLaser1->lockShutters();
   //qDebug()<<"locking lighpath 2";
    ui->shutterLaser2->lockShutters();
   //qDebug()<<"locking lighpath 3";
    ui->shutterTransmission->lockShutters();
   //qDebug()<<"locking lighpath 4";
    ui->lsLaser1->lockLightSource();
   //qDebug()<<"locking lighpath 5";
    ui->lsLaser2->lockLightSource();
   //qDebug()<<"locking lighpath 6";
    ui->lsTransmission->lockLightSource();
   //qDebug()<<"locking lighpath 7";
    ui->filtcDetection->lockFilterChangers();
   //qDebug()<<"locking lighpath done";
}

void QFESPIMB040OpticsSetup::unlockLightpath() {
   //qDebug()<<"unlocking lightpath";
    ui->shutterLaser1->unlockShutters();
    ui->shutterLaser2->unlockShutters();
    ui->shutterTransmission->unlockShutters();
    ui->lsLaser1->unlockLightSource();
    ui->lsLaser2->unlockLightSource();
    ui->lsTransmission->unlockLightSource();
    ui->filtcDetection->unlockFilterChangers();
    unlockLighpathCombobox();
   //qDebug()<<"unlocking lightpath done";
}

void saveMeasurementDevice1(QMap<QString, QVariant>& setup, const QString& prefix, const QString& name, QFExtensionMeasurementAndControlDevice* device, int id) {
    if (!device) return;
    for (unsigned int i=0; i<device->getMeasurementDeviceValueCount(id); i++) {
        setup[prefix+QString("%2/%1").arg(device->getMeasurementDeviceValueShortName(id, i)).arg(name)]=device->getMeasurementDeviceValue(id, i);
    }
}


QFESPIMB040OpticsSetup::measuredValues QFESPIMB040OpticsSetup::getMeasuredValues() {
    QFESPIMB040OpticsSetup::measuredValues m;
    for (int i=0; i<ui->lsTransmission->getLineCount(); i++) {
        m.data[QString("transmission/line%1/set_power").arg(i+1)]=ui->lsTransmission->getSetPower(i);
        m.data[QString("transmission/line%1/measured_power").arg(i+1)]=ui->lsTransmission->getMeasuredPower(i);
    }

    for (int i=0; i<ui->lsLaser1->getLineCount(); i++) {
        m.data[QString("laser1/line%1/set_power").arg(i+1)]=ui->lsLaser1->getSetPower(i);
        m.data[QString("laser1/line%1/measured_power").arg(i+1)]=ui->lsLaser1->getMeasuredPower(i);
    }

    for (int i=0; i<ui->lsLaser2->getLineCount(); i++) {
        m.data[QString("laser2/line%1/set_power").arg(i+1)]=ui->lsLaser2->getSetPower(i);
        m.data[QString("laser2/line%1/measured_power").arg(i+1)]=ui->lsLaser2->getMeasuredPower(i);
    }

    if (ui->stageSetup->getXStage()) {
        m.data["stagex/position"]=ui->stageSetup->getXStage()->getPosition(ui->stageSetup->getXStageAxis());
        m.data["stagex/velocity"]=ui->stageSetup->getXStage()->getSpeed(ui->stageSetup->getXStageAxis());
    }
    if (ui->stageSetup->getYStage()) {
        m.data["stagey/position"]=ui->stageSetup->getYStage()->getPosition(ui->stageSetup->getYStageAxis());
        m.data["stagey/velocity"]=ui->stageSetup->getYStage()->getSpeed(ui->stageSetup->getYStageAxis());
    }
    if (ui->stageSetup->getZStage()) {
        m.data["stagez/position"]=ui->stageSetup->getZStage()->getPosition(ui->stageSetup->getZStageAxis());
        m.data["stagez/velocity"]=ui->stageSetup->getZStage()->getSpeed(ui->stageSetup->getZStageAxis());
    }
    if (ui->stageSetup->getRStage()) {
        m.data["stageR/position"]=ui->stageSetup->getRStage()->getPosition(ui->stageSetup->getRStageAxis());
        m.data["stageR/velocity"]=ui->stageSetup->getRStage()->getSpeed(ui->stageSetup->getRStageAxis());
    }

    ui->camConfig1->storeMeasurements(m.data, "camera1/");
    ui->camConfig2->storeMeasurements(m.data, "camera2/");

    saveMeasurementDevice1(m.data, "measurement_device1", "", ui->widMD1->getMeasurementDevice(), ui->widMD1->getMeasurementDeviceID());
    saveMeasurementDevice1(m.data, "measurement_device2", "", ui->widMD2->getMeasurementDevice(), ui->widMD2->getMeasurementDeviceID());

    //qDebug()<<"measuredValues(): "<<m.data;

    return m;
}

QString QFESPIMB040OpticsSetup::dualViewMode(int camera) const {
    QString dv="none";
    if (camera==0 &&  ui->chkDualView1->isChecked()) {
        if (ui->cmbDualViewOrientation->currentIndex()==0) dv="horicontal";
        else if (ui->cmbDualViewOrientation->currentIndex()==1) dv="vertical";
        else dv="unknown";
    } else if (camera==1 && ui->chkDualView2->isChecked()) {
        if (ui->cmbDualView2Orientation->currentIndex()==0) dv="horicontal";
        else if (ui->cmbDualView2Orientation->currentIndex()==1) dv="vertical";
        else dv="unknown";
    }
    return dv;
}

QString QFESPIMB040OpticsSetup::dualViewMode(QFExtensionCamera *ecam, int camera) const
{
    return dualViewMode(camNumFromExtension(ecam, camera));
}

int QFESPIMB040OpticsSetup::camNumFromExtension(QFExtensionCamera *ecam, int camera) const
{
    if (ui->camConfig1->cameraComboBox()->currentExtensionCamera()==ecam && ui->camConfig1->cameraComboBox()->currentCameraID()==camera) return 0;
    if (ui->camConfig2->cameraComboBox()->currentExtensionCamera()==ecam && ui->camConfig2->cameraComboBox()->currentCameraID()==camera) return 1;
    return -1;
}

QString QFESPIMB040OpticsSetup::getAxisNameForStage(QFExtensionLinearStage *stage, int axis)
{
    if (ui->stageSetup->getXStage()==stage && ui->stageSetup->getXStageAxis()==axis) return "x";
    if (ui->stageSetup->getYStage()==stage && ui->stageSetup->getYStageAxis()==axis) return "y";
    if (ui->stageSetup->getZStage()==stage && ui->stageSetup->getZStageAxis()==axis) return "z";
    if (ui->stageSetup->getRStage()==stage && ui->stageSetup->getRStageAxis()==axis) return "r";
    return "";
}

QString QFESPIMB040OpticsSetup::getLaserConfig()
{
    QString r="";
    if (ui->lsLaser1->isLightSourceConnected() && ui->shutterLaser1->isShutterConnected() && ui->shutterLaser1->getShutterState())   {
        QString l=ui->lsLaser1->getLightsoureConfigForFilename();
        if (!l.isEmpty()) {
            if (!r.isEmpty()) r+="_";
            r+="laser1_"+cleanStringForFilename(l);
        }
    }
    if (ui->lsLaser2->isLightSourceConnected() && ui->shutterLaser2->isShutterConnected() && ui->shutterLaser2->getShutterState())   {
        QString l=ui->lsLaser2->getLightsoureConfigForFilename();
        if (!l.isEmpty()) {
            if (!r.isEmpty()) r+="_";
            r+="laser2_"+cleanStringForFilename(l);
        }
    }
    return r;
}

void QFESPIMB040OpticsSetup::on_btnLoadSetup_clicked() {
    QDir().mkpath(ProgramOptions::getInstance()->getConfigFileDirectory()+"/plugins/ext_spimb040/optics_setup/");
    QString dir=ProgramOptions::getInstance()->getQSettings()->value("QFESPIMB040ExperimentDescription/lastopticssetupdir", ProgramOptions::getInstance()->getConfigFileDirectory()+"/plugins/ext_spimb040/optics_setup/").toString();
    QString filename=qfGetOpenFileName(this, tr("open optics setup ..."), dir, tr("optics setup configuration (*.osi)"))    ;
    if (!filename.isEmpty()) {
        QSettings set(filename, QSettings::IniFormat);
        loadSettings(set, "optics_setup/");
        loadPluginGlobalSettings(set, "plugin_global/");
    }
    ProgramOptions::getInstance()->getQSettings()->setValue("QFESPIMB040ExperimentDescription/lastopticssetupdir", dir);

}

void QFESPIMB040OpticsSetup::on_btnSaveSetup_clicked()  {
    QDir().mkpath(ProgramOptions::getInstance()->getConfigFileDirectory()+"/plugins/ext_spimb040/optics_setup/");
    QString dir=ProgramOptions::getInstance()->getQSettings()->value("QFESPIMB040ExperimentDescription/lastopticssetupdir", ProgramOptions::getInstance()->getConfigFileDirectory()+"/plugins/ext_spimb040/optics_setup/").toString();
    QString filename=qfGetSaveFileName(this, tr("save optics setup ..."), dir, tr("optics setup configuration (*.osi)"))    ;
    if (!filename.isEmpty()) {
        bool ok=true;
        if (QFile::exists(filename)) {
            ok=false;
            if (QMessageBox::question(this, tr("save optics setup ..."), tr("The file\n  '%1'\nalready exists. Overwrite?").arg(filename), QMessageBox::Yes|QMessageBox::No, QMessageBox::No)==QMessageBox::Yes) {
                ok=true;
            }
        }
        if (ok) {
            QSettings set(filename, QSettings::IniFormat);
            storeSettings(set, "optics_setup/");
            storePluginGlobalSettings(set, "plugin_global/");
        }
    }
    ProgramOptions::getInstance()->getQSettings()->setValue("QFESPIMB040ExperimentDescription/lastopticssetupdir", dir);

}


int QFESPIMB040OpticsSetup::getBrightfieldLightSourceCount() const
{
    return 1;
}

QString QFESPIMB040OpticsSetup::getLaserName(int laser) const
{
    return tr("Laser %1").arg(laser);
}

QString QFESPIMB040OpticsSetup::getBrightfieldLightSourceName(int lightsource) const
{
    if (lightsource==QFESPIMB040OpticsSetupBase::BrightfieldTransmission) return tr("transmission illumination");
    return QString();
}
