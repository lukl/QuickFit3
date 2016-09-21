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

#include "stage_pi2.h"
#include<QtGlobal>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#else
#include <QtGui>
#endif


#include "qfplugin.h"
#include <iostream>
#include "pimercury863calibrationdialog.h"

#define LOG_PREFIX "[PI863v2]: "
#define GLOBAL_CONFIGFILE "stage_pi863_2.ini"

QFExtensionLinearStagePI2::QFExtensionLinearStagePI2(QObject* parent):
    QObject(parent)
{
	logService=NULL;
}

QFExtensionLinearStagePI2::~QFExtensionLinearStagePI2() {
}

void QFExtensionLinearStagePI2::initExtension() {
    /* do initializations here but do not yet connect to the stage! */
    QString ini=services->getGlobalConfigFileDirectory()+QString("/stage_pi863_2.ini");
    if (!QFile::exists(ini)) ini=services->getConfigFileDirectory()+QString("/stage_pi863_2.ini");
    if (!QFile::exists(ini)) ini=services->getAssetsDirectory()+QString("plugins/")+getID()+QString("/stage_pi863_2.ini");
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
            d.serial=new QFExtensionLinearStagePI2ProtocolHandler(ports.getCOMPort(d.port), ports.getMutex(d.port), getName());

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
            d.label=inifile.value(axisname+"/label", tr("PI Mercury 863 (v2), axis %1").arg(axis)).toString();
            axes.append(d);
        }
    }

    actCalibrateJoysticks=new QFActionWithNoMenuRole(QIcon(":/stage_pi/pi_joystick.png"), tr("calibrate PI stage joysticks, v2"), this);
    connect(actCalibrateJoysticks, SIGNAL(triggered()), this, SLOT(calibrateJoysticks()));
    if (services) {
        QMenu* m=services->getMenu("extensions");
        if (m) {
            m->addAction(actCalibrateJoysticks);
        }
    }
}

void QFExtensionLinearStagePI2::deinit() {
	/* add code for cleanup here */
    QSettings inifile(services->getGlobalConfigFileDirectory()+"/stage_pi863_2.ini", QSettings::IniFormat);
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

void QFExtensionLinearStagePI2::projectChanged(QFProject* /*oldProject*/, QFProject* /*project*/) {
    /* usually stages do not have to react to a change of the project in QuickFit .. so you don't need to do anything here
	   But: possibly you could read config information from the project here
	 */
}


void QFExtensionLinearStagePI2::loadSettings(ProgramOptions* settingspo) {
	/* here you could read config information from the quickfit.ini file using settings object */
    if (!settingspo) return;
	if (settingspo->getQSettings()==NULL) return;
    //QSettings& settings=*(settingspo->getQSettings()); // the QSettings object for quickfit.ini

	// ALTERNATIVE: read/write Information to/from plugins/<ID>/<ID>.ini file
	// QSettings settings(services->getConfigFileDirectory()+"/plugins/"+getID()+"/"+getID()+".ini", QSettings::IniFormat);

}

void QFExtensionLinearStagePI2::storeSettings(ProgramOptions* settingspo) {
	/* here you could write config information to the quickfit.ini file using settings object */
    if (!settingspo) return;
	if (settingspo->getQSettings()==NULL) return;
    //QSettings& settings=*(settingspo->getQSettings()); // the QSettings object for quickfit.ini

	// ALTERNATIVE: read/write Information to/from plugins/<ID>/<ID>.ini file
	// QSettings settings(services->getConfigFileDirectory()+"/plugins/"+getID()+"/"+getID()+".ini", QSettings::IniFormat);

}

unsigned int QFExtensionLinearStagePI2::getAxisCount() {
	return axes.size();
}

void QFExtensionLinearStagePI2::showSettingsDialog(unsigned int /*axis*/, QWidget* parent) {


    QString ini1=services->getGlobalConfigFileDirectory()+QString("/")+GLOBAL_CONFIGFILE;
    QString ini2=services->getConfigFileDirectory()+QString("/")+GLOBAL_CONFIGFILE;
    QString ini3=services->getAssetsDirectory()+QString("plugins/")+getID()+QString("/")+GLOBAL_CONFIGFILE;
    QMessageBox::information(parent, getName(), tr("There is no configuration dialog for this device. Set all config in the appropriate ini file:\n  %1\n  or: %2\n  or: %3").arg(ini1).arg(ini2).arg(ini3));




//    bool globalIniWritable=QSettings(services->getGlobalConfigFileDirectory()+"/stage_pi863_2.ini", QSettings::IniFormat).isWritable();


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




void QFExtensionLinearStagePI2::calibrateJoysticks() {

    for (unsigned int axis=0; axis<getAxisCount(); axis++) {
        QMessageBox::StandardButton answer=QMessageBox::question(NULL, tr("PI Mercury C863 v2 joystick calibration"), tr("Do you want to calibrate a joystick on axis %1?").arg(axis), QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel, QMessageBox::Yes);
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



bool QFExtensionLinearStagePI2::isConnected(unsigned int axis) {
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        return axes[axis].serial->getCOM()->isConnectionOpen();
    }
    return false;
}

void QFExtensionLinearStagePI2::connectDevice(unsigned int axis) {
    log_text((tr(LOG_PREFIX "Connecting Mercury C-863 Motor Controller (v2) Axis %1 ...").arg(axis)));
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI2ProtocolHandler* serial=axes[axis].serial;
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
            if (!com->hasErrorOccured()) {
                int isRefSet=0;
                char block1[2];
                if(sscanf(serial->queryCommand("TS").c_str(), "S:%1s%1s %*x %*x %*x %*x %*x", &block1[0], &block1[1])) {
                    std::string binstr=twocharblockstrtobinstr(block1);
                    sscanf(binstr.c_str(),"%*4i%1i%*3i", &isRefSet);
                    if (isRefSet==1) {
                        log_text(tr(LOG_PREFIX "Reference Position is defined.(Undo by restarting Controller)\n"));
                    }
                    else {
                        log_text(tr(LOG_PREFIX "Reference Position not defined (Controller restarted?). Referencing, returning to inital position..."));
                        //if (axis==2) {serial->sendCommand("FE"); // Find Origin, z-dir: positive, negative else
                        //else  {serial->sendCommand("FE1");
                        serial->sendCommand("DH0");
                        int findingReference=1;
                        char block4[2];
                        if (axis==2) serial->sendCommand("FE0");
                        else serial->sendCommand("FE1");
                        while(findingReference) {
                            sscanf(serial->queryCommand("TS").c_str(), "S:%*x %*x %*x %1s%1s %*x %*x", &block4[0], &block4[1]);
                            std::string binstr=twocharblockstrtobinstr(block4);
                            sscanf(binstr.c_str(),"%*5i%1i%*2i", &findingReference);
                            QThread::msleep(axes[axis].ms);
                            }
                            double dist=getPosition(axis);
                            serial->sendCommand("DH0");
                            move(axis, -dist);
                            while("\x030"!=serial->queryCommandSingleChar("\x05c")) {QThread::msleep(axes[axis].ms);}
                            //serial->sendCommand("DH0");
                            log_text(tr("Done.\n"));
                    }
                }
                else {
                    log_error(tr(LOG_PREFIX "Invalid result string from TS command (Tell Status) in connectDevice [expected S:<6 blocks of 2 hex numbers>] from axis %1\n").arg(inttostr(axis).c_str()));
                    log_error(tr(LOG_PREFIX "Result of FRF? command was %1").arg(serial->queryCommand("TS").c_str()));
                }
            }
            axes[axis].velocity=axes[axis].initVelocity;
            axes[axis].joystickEnabled=false;

            if (com->hasErrorOccured()) {
                axes[axis].state=QFExtensionLinearStage::Error;
            } else {
                axes[axis].state=QFExtensionLinearStage::Ready;
            }
        } else {
            log_error(tr(LOG_PREFIX " Could not connect to Mercury C-863 Motor Controller (v2)!!!\n"));
            log_error(tr(LOG_PREFIX " reason: %1\n").arg(com->getLastError().c_str()));
            axes[axis].state=QFExtensionLinearStage::Disconnected;
        }
    }
}

void QFExtensionLinearStagePI2::disconnectDevice(unsigned int axis) {
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI2ProtocolHandler* serial=axes[axis].serial;
//        if (com->isConnectionOpen()) {
//            serial->selectAxis(axes[axis].ID);
//            serial->sendCommand("JF");
//            axes[axis].joystickEnabled=false;
//        }
        com->close();
        axes[axis].state=QFExtensionLinearStage::Disconnected;
    }
}

void QFExtensionLinearStagePI2::setLogging(QFPluginLogService* logService) {
	this->logService=logService;
}

void QFExtensionLinearStagePI2::setJoystickActive(unsigned int axis, bool enabled, double maxVelocity) {
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI2ProtocolHandler* serial=axes[axis].serial;

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

bool QFExtensionLinearStagePI2::isJoystickActive(unsigned int axis) {
	return axes[axis].joystickEnabled;
}


void QFExtensionLinearStagePI2::stop(unsigned int axis) {
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI2ProtocolHandler* serial=axes[axis].serial;
        if (com->isConnectionOpen()) {
            serial->selectAxis(axes[axis].ID);
            serial->sendCommand("AB");
        }
    }
}

double QFExtensionLinearStagePI2::getSpeed(unsigned int axis) {
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI2ProtocolHandler* serial=axes[axis].serial;
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

double QFExtensionLinearStagePI2::getPosition(unsigned int axis) {
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI2ProtocolHandler* serial=axes[axis].serial;
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

void QFExtensionLinearStagePI2::move(unsigned int axis, double newPosition) {
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI2ProtocolHandler* serial=axes[axis].serial;
        serial->selectAxis(axes[axis].ID);
        if (com->isConnectionOpen() && (axes[axis].state==QFExtensionLinearStage::Ready) && (!axes[axis].joystickEnabled)) {
            long xx=(long)round(newPosition/axes[axis].lengthFactor);
            if ( (axes[axis].maxCoord !=0  && newPosition>axes[axis].maxCoord) || (axes[axis].minCoord !=0 && newPosition<axes[axis].minCoord) ) {
                log_warning(tr(LOG_PREFIX " error on axis %1: Move attempt with position exceeding limits").arg(axis));
            }
            else {
                if (!com->hasErrorOccured()) {
                    double currentPosition=getPosition(axis);
                    if(currentPosition<newPosition) {
                        serial->sendCommand("MA"+inttostr((long)round(xx+(axes[axis].backlashCorr/axes[axis].lengthFactor)))); // Always approach from same side, default 50 microns correction
                        axes[axis].state=QFExtensionLinearStage::Moving;
                        while("\x030"!=serial->queryCommandSingleChar("\x05c")) {QThread::msleep(axes[axis].ms);}
                        serial->sendCommand("MA"+inttostr(xx));
                    }
                    else if (currentPosition>newPosition) {
                        serial->sendCommand("SV"+inttostr((long)round(axes[axis].velocity/axes[axis].velocityFactor))+",MA"+inttostr(xx));
                        axes[axis].state=QFExtensionLinearStage::Moving;
                        while("\x030"!=serial->queryCommandSingleChar("\x05c")) {QThread::msleep(axes[axis].ms);}
                    }
                }
                //axes[axis].state=QFExtensionLinearStage::Moving;
            }
        }
    }
}


QFExtensionLinearStage::AxisState QFExtensionLinearStagePI2::getAxisState(unsigned int axis)  {
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI2ProtocolHandler* serial=axes[axis].serial;


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

QString QFExtensionLinearStagePI2::getStageName(unsigned int axis) const
{
    QString n=tr("PI Mercury 863 v2, axis %1").arg(axis);
    if ( (int64_t)axis<axes.size()) n=axes[axis].label;
    return n;
}

QFExtensionLinearStage::StageInfo QFExtensionLinearStagePI2::getStageInfo(unsigned int axis) const
{
    QFExtensionLinearStage::StageInfo info;
    info.maxSpeed=axes[axis].maxVelocity;
    return info;
}


void QFExtensionLinearStagePI2::log_text(QString message) {
	if (logService) logService->log_text(message);
	else if (services) services->log_text(message);
}

void QFExtensionLinearStagePI2::log_warning(QString message) {
	if (logService) logService->log_warning(message);
	else if (services) services->log_warning(message);
}

void QFExtensionLinearStagePI2::log_error(QString message) {
	if (logService) logService->log_error(message);
	else if (services) services->log_error(message);
}


std::string QFExtensionLinearStagePI2::twocharblockstrtobinstr(std::string twocharblock) {
    char charone=0, chartwo=0;
    sscanf(twocharblock.c_str(), "%c%c", &charone, &chartwo);
    std::string binary="";
    switch(charone) {
        case '0':
            binary+="0000";
            break;
        case '1':
            binary+="0001";
            break;
        case '2':
            binary+="0010";
            break;
        case '3':
            binary+="0011";
            break;
        case '4':
            binary+="0100";
            break;
        case '5':
            binary+="0101";
            break;
        case '6':
            binary+="0110";
            break;
        case '7':
            binary+="0111";
            break;
        case '8':
            binary+="1000";
            break;
        case '9':
            binary+="1001";
            break;
        case 'A':
            binary+="1010";
            break;
        case 'B':
            binary+="1011";
            break;
        case 'C':
            binary+="1100";
            break;
        case 'D':
            binary+="1101";
            break;
        case 'E':
            binary+="1110";
            break;
        case 'F':
            binary+="1111";
            break;
    }
    switch(chartwo) {
        case '0':
            binary+="0000";
            break;
        case '1':
            binary+="0001";
            break;
        case '2':
            binary+="0010";
            break;
        case '3':
            binary+="0011";
            break;
        case '4':
            binary+="0100";
            break;
        case '5':
            binary+="0101";
            break;
        case '6':
            binary+="0110";
            break;
        case '7':
            binary+="0111";
            break;
        case '8':
            binary+="1000";
            break;
        case '9':
            binary+="1001";
            break;
        case 'A':
            binary+="1010";
            break;
        case 'B':
            binary+="1011";
            break;
        case 'C':
            binary+="1100";
            break;
        case 'D':
            binary+="1101";
            break;
        case 'E':
            binary+="1110";
            break;
        case 'F':
            binary+="1111";
            break;
    }
    return(binary);
}


Q_EXPORT_PLUGIN2(stage_pi863_2, QFExtensionLinearStagePI2)

