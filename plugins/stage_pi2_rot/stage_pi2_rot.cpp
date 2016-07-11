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

#include "stage_pi2_rot.h"
#include<QtGlobal>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#else
#include <QtGui>
#endif


#include "qfplugin.h"
#include <iostream>
#include "pimercury863calibrationdialog.h"

#define LOG_PREFIX "[PI863v2rot]: "
#define GLOBAL_CONFIGFILE "stage_pi863_2_rot.ini"

QFExtensionLinearStagePI2Rot::QFExtensionLinearStagePI2Rot(QObject* parent):
    QObject(parent)
{
	logService=NULL;
}

QFExtensionLinearStagePI2Rot::~QFExtensionLinearStagePI2Rot() {
}

void QFExtensionLinearStagePI2Rot::initExtension() {
    /* do initializations here but do not yet connect to the stage! */
    QString ini=services->getGlobalConfigFileDirectory()+QString("/stage_pi863_2_rot.ini");
    if (!QFile::exists(ini)) ini=services->getConfigFileDirectory()+QString("/stage_pi863_2_rot.ini");
    if (!QFile::exists(ini)) ini=services->getAssetsDirectory()+QString("plugins/")+getID()+QString("/stage_pi863_2_rot.ini");
    QFPluginServices::getInstance()->log_global_text(tr("%1loading INI-file %2\n").arg(QString(LOG_PREFIX)).arg(ini));
    QSettings inifile(ini, QSettings::IniFormat);


    AxisDescription defaultAD;

    int axisCount=inifile.value("axis_count", 0).toUInt();


    /* Counts axes - for old ini version compatibility
       There are max. 16 axes per controller */

    if (axisCount==0) {
        for (int i=0; i<16; i++) {
            if (inifile.contains(QString("axis%1").arg(i)+"/id")) {axisCount++;};
        };
    };


    axes.clear();
    for (int axis=0; axis<axisCount; axis++) {
        QString axisname=QString("axis%1").arg(axis);
        if (inifile.contains(axisname+"/id")) {
            AxisDescription d;
            QString s=inifile.value(axisname+"/id", '0').toString();
            d.ID='0';
            if (s.size()>0) d.ID=s[0];

            d.port=ports.addCOMPort(inifile, axisname+"/", 38400);
            d.serial=new QFExtensionLinearStagePI2RotProtHandler(ports.getCOMPort(d.port), ports.getMutex(d.port), getName());

            d.PTerm=inifile.value(axisname+"/pterm", defaultAD.PTerm).toUInt();
            d.iTerm=inifile.value(axisname+"/iterm", defaultAD.iTerm).toUInt();
            d.DTerm=inifile.value(axisname+"/dterm", defaultAD.DTerm).toUInt();
            d.iLimit=inifile.value(axisname+"/ilimit", defaultAD.iLimit).toUInt();
            d.acceleration=inifile.value(axisname+"/acceleration", defaultAD.acceleration).toDouble();
            d.initVelocity=inifile.value(axisname+"/initvelocity", defaultAD.initVelocity).toDouble();
            d.maxVelocity=inifile.value(axisname+"/maxvelocity", defaultAD.maxVelocity).toDouble();
            d.maxCoord=inifile.value(axisname+"/maxcoord", defaultAD.maxCoord).toDouble();
            d.minCoord=inifile.value(axisname+"/mincoord", defaultAD.minCoord).toDouble();
            d.backlashCorr=inifile.value(axisname+"/backlashcorr", defaultAD.backlashCorr).toDouble();



            d.velocity=defaultAD.initVelocity;
            d.joystickEnabled=false;
            d.state=QFExtensionLinearStage::Disconnected;
            d.name=axisname;
            d.label=inifile.value(axisname+"/label", tr("Rot PI Mercury 863, axis %1").arg(axis)).toString();
            axes.append(d);
        }
    }

    actCalibrateJoysticks=new QFActionWithNoMenuRole(QIcon(":/stage_pi/pi_joystick.png"), tr("calibrate PI stage joysticks, v2 Rot"), this);
    connect(actCalibrateJoysticks, SIGNAL(triggered()), this, SLOT(calibrateJoysticks()));
    if (services) {
        QMenu* m=services->getMenu("extensions");
        if (m) {
            m->addAction(actCalibrateJoysticks);
        }
    }
}

void QFExtensionLinearStagePI2Rot::deinit() {
	/* add code for cleanup here */
    QSettings inifile(services->getGlobalConfigFileDirectory()+"/stage_pi863_2_rot.ini", QSettings::IniFormat);
    int axisCount=inifile.value("axis_count", 0).toUInt();


    inifile.setValue("axis_count", axes.size());
    for (int axis=0; axis<axes.size(); axis++) {
        QString axisname=QString("axis%1").arg(axis);
        inifile.setValue(axisname+"/id", axes[axis].ID);

        inifile.setValue(axisname+"/pterm", axes[axis].PTerm);
        inifile.setValue(axisname+"/iterm", axes[axis].iTerm);
        inifile.setValue(axisname+"/dterm", axes[axis].DTerm);
        inifile.setValue(axisname+"/ilimit", axes[axis].iLimit);
        inifile.setValue(axisname+"/acceleration", axes[axis].acceleration);
        inifile.setValue(axisname+"/initvelocity", axes[axis].initVelocity);
        inifile.setValue(axisname+"/maxvelocity", axes[axis].maxVelocity);
        inifile.setValue(axisname+"/label", axes[axis].label);
        inifile.setValue(axisname+"/maxcoord", axes[axis].maxCoord);
        inifile.setValue(axisname+"/mincoord", axes[axis].minCoord);
        inifile.setValue(axisname+"/backlashcorr", axes[axis].backlashCorr);
    }
}

void QFExtensionLinearStagePI2Rot::projectChanged(QFProject* /*oldProject*/, QFProject* /*project*/) {
    /* usually stages do not have to react to a change of the project in QuickFit .. so you don't need to do anything here
	   But: possibly you could read config information from the project here
	 */
}


void QFExtensionLinearStagePI2Rot::loadSettings(ProgramOptions* settingspo) {
	/* here you could read config information from the quickfit.ini file using settings object */
    if (!settingspo) return;
	if (settingspo->getQSettings()==NULL) return;
    //QSettings& settings=*(settingspo->getQSettings()); // the QSettings object for quickfit.ini

	// ALTERNATIVE: read/write Information to/from plugins/<ID>/<ID>.ini file
	// QSettings settings(services->getConfigFileDirectory()+"/plugins/"+getID()+"/"+getID()+".ini", QSettings::IniFormat);

}

void QFExtensionLinearStagePI2Rot::storeSettings(ProgramOptions* settingspo) {
	/* here you could write config information to the quickfit.ini file using settings object */
    if (!settingspo) return;
	if (settingspo->getQSettings()==NULL) return;
    //QSettings& settings=*(settingspo->getQSettings()); // the QSettings object for quickfit.ini

	// ALTERNATIVE: read/write Information to/from plugins/<ID>/<ID>.ini file
	// QSettings settings(services->getConfigFileDirectory()+"/plugins/"+getID()+"/"+getID()+".ini", QSettings::IniFormat);

}

unsigned int QFExtensionLinearStagePI2Rot::getAxisCount() {
	return axes.size();
}

void QFExtensionLinearStagePI2Rot::showSettingsDialog(unsigned int /*axis*/, QWidget* parent) {


    QString ini1=services->getGlobalConfigFileDirectory()+QString("/")+GLOBAL_CONFIGFILE;
    QString ini2=services->getConfigFileDirectory()+QString("/")+GLOBAL_CONFIGFILE;
    QString ini3=services->getAssetsDirectory()+QString("plugins/")+getID()+QString("/")+GLOBAL_CONFIGFILE;
    QMessageBox::information(parent, getName(), tr("There is no configuration dialog for this device. Set all config in the appropriate ini file:\n  %1\n  or: %2\n  or: %3").arg(ini1).arg(ini2).arg(ini3));




//    bool globalIniWritable=QSettings(services->getGlobalConfigFileDirectory()+"/stage_pi863_2_rot.ini", QSettings::IniFormat).isWritable();


//    /* open a dialog that configures the stage.

//       usually you should display a modal QDialog descendent which writes back config when the user clicks OK

//       alternatively you may also display a window which stays open and allows the suer to set settings also
//       during the measurement.

//       if the global ini file is not writable, there is no point in displaying a config dialog, as the data may not be stored
//    */


//    if (globalIniWritable) {
//        /////////////////////////////////////////////////////////////////////////////////
//        // if you want the settings dialog to be modal, you may uncomment the next lines
//        // and add implementations
//        /////////////////////////////////////////////////////////////////////////////////

//        QDialog* dlg=new QFDialog(parent);

//        QVBoxLayout* lay=new QVBoxLayout();
//        dlg->setLayout(lay);

//        QFormLayout* formlayout=new QFormLayout();


//        formlayout->addRow("", new QLabel(tr("<b>All settings marked with * will be<br>used when connecting the next time!</b>"), dlg));

//        QComboBox* cmbPort=new QComboBox(dlg);
//        std::vector<std::string> ports=JKSerialConnection::listPorts();
//        for (unsigned int i=0; i<ports.size(); i++) {
//            cmbPort->addItem(ports[i].c_str());
//        }
//        cmbPort->setEditable(false);
//        cmbPort->setCurrentIndex(cmbPort->findText(COMPort));
//        formlayout->addRow(tr("serial &port*:"), cmbPort);

//        QComboBox* cmbSpeed=new QComboBox(dlg);
//        cmbSpeed->setEditable(false);
//        cmbSpeed->addItem("9600");
//        cmbSpeed->addItem("19200");
//        cmbSpeed->addItem("38400");
//        cmbSpeed->setCurrentIndex(cmbSpeed->findText(QString::number(COMPortSpeed)));
//        formlayout->addRow(tr("serial port &baudrate*:"), cmbSpeed);


//        lay->addLayout(formlayout);

//        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, dlg);
//        lay->addWidget(buttonBox);

//        connect(buttonBox, SIGNAL(accepted()), dlg, SLOT(accept()));
//        connect(buttonBox, SIGNAL(rejected()), dlg, SLOT(reject()));

//        if ( dlg->exec()==QDialog::Accepted ) {
//             //  read back values entered into the widgets and store in settings
//             COMPort=cmbPort->currentText();
//             COMPortSpeed=cmbSpeed->itemText(cmbSpeed->currentIndex()).toInt();
//        }
//        delete dlg;

//    } else {
//        QMessageBox::information(parent, getName(), tr("Your user account is not allowd to write to the global config directory!"));
//    }
//	//QMessageBox::information(parent, getName(), tr("There is currently no configuration dialog!"));
}




void QFExtensionLinearStagePI2Rot::calibrateJoysticks() {

    for (unsigned int axis=0; axis<getAxisCount(); axis++) {
        QMessageBox::StandardButton answer=QMessageBox::question(NULL, tr("PI Mercury C863 joystick calibration"), tr("Do you want to calibrate a joystick on axis %1?").arg(axis), QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel, QMessageBox::Yes);
        if (answer==QMessageBox::Yes) {
            bool wasConnected=isConnected(axis);
            if (!wasConnected) connectDevice(axis);
            setJoystickActive(axis, false);
            PIMercury863CalibrationDialog* dlg=new PIMercury863CalibrationDialog(NULL, this, axis);
            dlg->exec();
            delete dlg;
            if (!wasConnected) disconnectDevice(axis);
        } else if (answer==QMessageBox::Cancel)  {
            break;
        }
    }
}



bool QFExtensionLinearStagePI2Rot::isConnected(unsigned int axis) {
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        return axes[axis].serial->getCOM()->isConnectionOpen();
    }
    return false;
}

void QFExtensionLinearStagePI2Rot::connectDevice(unsigned int axis) {
    if (((int64_t)axis<axes.size())) {
        log_text((tr("Connecting Rotational Mercury C-863 Motor Controller Axis %1 ...").arg(axis)));
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI2RotProtHandler* serial=axes[axis].serial;
        com->open();
        if (com->isConnectionOpen()) {
            serial->selectAxis(axes[axis].ID);
            serial->selectAxis(axes[axis].ID);
            serial->queryCommand("VE");
            serial->sendCommand("DP"+inttostr(axes[axis].PTerm));
            serial->queryCommand("GP");
            serial->sendCommand("DI"+inttostr(axes[axis].iTerm));
            serial->sendCommand("DD"+inttostr(axes[axis].DTerm));
            serial->sendCommand("DL"+inttostr(axes[axis].iLimit));
            serial->sendCommand("SA"+inttostr(axes[axis].acceleration));
            serial->sendCommand("MN");
            serial->sendCommand("DH");
            log_text(tr("Finding origin..."));
            if (axis==0) {
                serial->sendCommand("FE1"); // Test Origin finding in one rot-dir
                double dist=getPosition(axis);
                serial->sendCommand("DH");
//                move(i, -(dist+1));
                move(axis, -dist);
            }
            log_text(tr("Done.\n"));
            axes[axis].velocity=axes[axis].initVelocity;
            axes[axis].joystickEnabled=false;

            if (com->hasErrorOccured()) {
                axes[axis].state=QFExtensionLinearStage::Error;
            } else {
                axes[axis].state=QFExtensionLinearStage::Ready;
            }
        } else {
            log_error(tr(LOG_PREFIX " Could not connect to Rotational Mercury C-863 Motor Controller !!!\n"));
            log_error(tr(LOG_PREFIX " reason: %1\n").arg(com->getLastError().c_str()));
            axes[axis].state=QFExtensionLinearStage::Disconnected;
        }
    }
}

void QFExtensionLinearStagePI2Rot::disconnectDevice(unsigned int axis) {
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI2RotProtHandler* serial=axes[axis].serial;
        com->close();
        axes[axis].state=QFExtensionLinearStage::Disconnected;
    }
}

void QFExtensionLinearStagePI2Rot::setLogging(QFPluginLogService* logService) {
	this->logService=logService;
}

void QFExtensionLinearStagePI2Rot::setJoystickActive(unsigned int axis, bool enabled, double maxVelocity) {
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI2RotProtHandler* serial=axes[axis].serial;

        if (enabled) {
            serial->selectAxis(axes[axis].ID);
            serial->sendCommand("JN"+inttostr((long)round(maxVelocity/axes[axis].velocityFactor)));
            axes[axis].joystickEnabled=true;
        } else {
            serial->selectAxis(axes[axis].ID);
            serial->sendCommand("JF");
            axes[axis].joystickEnabled=false;
        }
    }


}

bool QFExtensionLinearStagePI2Rot::isJoystickActive(unsigned int axis) {
	return axes[axis].joystickEnabled;
}


void QFExtensionLinearStagePI2Rot::stop(unsigned int axis) {
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI2RotProtHandler* serial=axes[axis].serial;
        if (com->isConnectionOpen()) {
            serial->selectAxis(axes[axis].ID);
            serial->sendCommand("AB");
        }
    }
}

double QFExtensionLinearStagePI2Rot::getSpeed(unsigned int axis) {
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI2RotProtHandler* serial=axes[axis].serial;
        if (com->isConnectionOpen()) {
            serial->selectAxis(axes[axis].ID);
            std::string r=serial->queryCommand("TV")+"\n";
            double v=0;
            if (!com->hasErrorOccured()) {
                if (sscanf(r.c_str(), "V:%lf", &v)) {
                    return v*axes[axis].velocityFactor;
                } else {
                    log_error(tr(LOG_PREFIX " invalid result string from TV command [expected V:<number>] from axis %1. String was '%2'.\n").arg(axis).arg(toprintablestr(r).c_str()));
                }
            }
        }
    }

    return 0;
}

double QFExtensionLinearStagePI2Rot::getPosition(unsigned int axis) {
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI2RotProtHandler* serial=axes[axis].serial;
        if (com->isConnectionOpen()) {
            serial->selectAxis(axes[axis].ID);
            std::string r=serial->queryCommand("TP")+"\n";
            double v=0;
            if (!com->hasErrorOccured()) {
                if (sscanf(r.c_str(), "P:%lf", &v)) {
                    return v*axes[axis].lengthFactor;
                } else {
                    log_error(tr(LOG_PREFIX " invalid result string from TP command [expected P:<number>] from axis %1. String was '%2'.\n").arg(axis).arg(toprintablestr(r).c_str()));
                }
            }
        }    }


    return 0;
}

void QFExtensionLinearStagePI2Rot::move(unsigned int axis, double newPosition) {
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI2RotProtHandler* serial=axes[axis].serial;
        serial->selectAxis(axes[axis].ID);
        if (com->isConnectionOpen() && (axes[axis].state==QFExtensionLinearStage::Ready) && (!axes[axis].joystickEnabled)) {
            long xx=(long)round(newPosition/axes[axis].lengthFactor);
            if ( (axes[axis].maxCoord !=0  && newPosition>axes[axis].maxCoord) || (axes[axis].minCoord !=0 && newPosition<axes[axis].minCoord) ) {
                axes[axis].state=QFExtensionLinearStage::Error;
                log_error(tr(LOG_PREFIX " error on axis %1: Move attempt with position exceeding limits").arg(axis));
                axes[axis].state=QFExtensionLinearStage::Ready;
            }
            else {

                if (!com->hasErrorOccured()) {
                    serial->sendCommand("SV"+inttostr((long)round(axes[axis].velocity/axes[axis].velocityFactor))+",MA"+inttostr(xx+(axes[axis].backlashCorr/axes[axis].lengthFactor))+",MA"+inttostr(xx)); // Always approach from same side, default 1 deg correction
                }
                axes[axis].state=QFExtensionLinearStage::Moving;
            }
        }
    }
}

QFExtensionLinearStage::AxisState QFExtensionLinearStagePI2Rot::getAxisState(unsigned int axis)  {
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI2RotProtHandler* serial=axes[axis].serial;


        std::string failedaxes="";
        if (com->isConnectionOpen()) {
            serial->selectAxis(axes[axis].ID);
            std::string r=serial->queryCommand("TS")+"\n";
            int B1=0, B2=0, B3=0, B4=0, B5=0, B6=0;
            if (!com->hasErrorOccured()) {
                if (sscanf(r.c_str(), "S:%x %x %x %x %x %x", &B1, &B2, &B3, &B4, &B5, &B6)) {
                    //std::cout<<"\n\n"<<std::string(1, (char)('x'+i-1))<<": "<<r<<"\n\n";
                    if (B6==1) {
                        log_error(tr(LOG_PREFIX " error on axis %1: RS-232 timeout\n").arg(axis));
                        axes[axis].state=QFExtensionLinearStage::Error;
                    } else if (B6==2) {
                        log_error(tr(LOG_PREFIX " error on axis %1: RS-232 overflow\n").arg(axis));
                        axes[axis].state=QFExtensionLinearStage::Error;
                    } else if (B6==3) {
                        log_error(tr(LOG_PREFIX " error on axis %1: Macro storage full\n").arg(axis));
                        axes[axis].state=QFExtensionLinearStage::Error;
                    } else if (B6==4) {
                        log_error(tr(LOG_PREFIX " error on axis %1: Macro out of range\n").arg(axis));
                        axes[axis].state=QFExtensionLinearStage::Error;
                    } else if (B6==5) {
                        log_error(tr(LOG_PREFIX " error on axis %1: Macro wrong com\n").arg(axis));
                        axes[axis].state=QFExtensionLinearStage::Error;
                    } else if (B6==6) {
                        log_error(tr(LOG_PREFIX " error on axis %1: wrong command\n").arg(axis));
                        axes[axis].state=QFExtensionLinearStage::Error;
                    } else if (B6==7) {
                        log_error(tr(LOG_PREFIX " error on axis %1: hard stop\n").arg(axis));
                        axes[axis].state=QFExtensionLinearStage::Error;
                    } else if (B6==8) {
                        log_error(tr(LOG_PREFIX " error on axis %1: not defined\n").arg(axis));
                        axes[axis].state=QFExtensionLinearStage::Error;
                    } else if (B6==9) {
                        log_error(tr(LOG_PREFIX " error on axis %1: Position following error\n").arg(axis));
                        axes[axis].state=QFExtensionLinearStage::Error;
                    } else if (B6==10) {
                        log_error(tr(LOG_PREFIX " error on axis %1: Move attempt while servo off\n").arg(axis));
                        axes[axis].state=QFExtensionLinearStage::Error;
                    } else if (B6==11) {
                        log_error(tr(LOG_PREFIX " error on axis %1: Move attempt while joystick on\n").arg(axis));
                        axes[axis].state=QFExtensionLinearStage::Error;
                    }

                    if (axes[axis].state!=QFExtensionLinearStage::Error) {
                        // read current velocity
                        axes[axis].state=QFExtensionLinearStage::Ready;
                        locker.unlock();
                        double v=getSpeed(axis);
                        if (axes[axis].state!=QFExtensionLinearStage::Error) {
                            if (fabs(v)>0) axes[axis].state=QFExtensionLinearStage::Moving;
                        }
                    }
                } else {
                    log_error(tr(LOG_PREFIX " invalid result string from TS command [expected S:<6 blocks of 2 hex numbers>] in getAxisState() [axis %1]. String was '%2'.\n").arg(axis).arg(toprintablestr(r).c_str()));
                    axes[axis].state=QFExtensionLinearStage::Error;
                }


            } else {
                log_error(tr(LOG_PREFIX " communication error\n").arg(axis));
                axes[axis].state=QFExtensionLinearStage::Error;
            }
        } else {
            axes[axis].state=QFExtensionLinearStage::Disconnected;
        }



        return axes[axis].state;
    }
    return QFExtensionLinearStage::Disconnected;
}

QString QFExtensionLinearStagePI2Rot::getStageName(unsigned int axis) const
{
    QString n=tr("Rotational PI Mercury 863, axis %1").arg(axis);
    if ( (int64_t)axis<axes.size()) n=axes[axis].label;
    return n;
}

QFExtensionLinearStage::StageInfo QFExtensionLinearStagePI2Rot::getStageInfo(unsigned int axis) const
{
    QFExtensionLinearStage::StageInfo info;
    info.maxSpeed=axes[axis].maxVelocity;
    return info;
}


void QFExtensionLinearStagePI2Rot::log_text(QString message) {
	if (logService) logService->log_text(message);
	else if (services) services->log_text(message);
}

void QFExtensionLinearStagePI2Rot::log_warning(QString message) {
	if (logService) logService->log_warning(message);
	else if (services) services->log_warning(message);
}

void QFExtensionLinearStagePI2Rot::log_error(QString message) {
	if (logService) logService->log_error(message);
	else if (services) services->log_error(message);
}


Q_EXPORT_PLUGIN2(stage_pi863_2_rot, QFExtensionLinearStagePI2Rot)

