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

#include "stage_pi663.h"
#include<QtGlobal>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#else
#include <QtGui>
#endif


#include "qfplugin.h"
#include <iostream>
#include "pimercury663calibrationdialog.h"

#define LOG_PREFIX "[PI663v2]: "
#define GLOBAL_CONFIGFILE "stage_pi663.ini"

QFExtensionLinearStagePI663::QFExtensionLinearStagePI663(QObject* parent):
    QObject(parent)
{
	logService=NULL;
}

QFExtensionLinearStagePI663::~QFExtensionLinearStagePI663() {
}

void QFExtensionLinearStagePI663::initExtension() {
    /* do initializations here but do not yet connect to the stage! */
    QString ini=services->getGlobalConfigFileDirectory()+QString("/stage_pi663.ini");
    if (!QFile::exists(ini)) ini=services->getConfigFileDirectory()+QString("/stage_pi663.ini");
    if (!QFile::exists(ini)) ini=services->getAssetsDirectory()+QString("plugins/")+getID()+QString("/stage_pi663.ini");
    QFPluginServices::getInstance()->log_global_text(tr("%1loading INI-file %2\n").arg(QString(LOG_PREFIX)).arg(ini));
    QSettings inifile(ini, QSettings::IniFormat);


    AxisDescription defaultAD;

    int axisCount=inifile.value("axis_count", 0).toUInt();


    /* Counts axes - for old ini version compatibility
       There are max. 16 axes per controller */

    if (axisCount==0) {
        for (int i=0; i<16; i++) {
            if (inifile.contains(QString("axis%1").arg(i)+"/axis_id")) {axisCount++;};
        };
    };


    axes.clear();
    for (int axis=0; axis<axisCount; axis++) {
        QString axisname=QString("axis%1").arg(axis);
        if (inifile.contains(axisname+"/axis_id")) {
            AxisDescription d;
            QString s=inifile.value(axisname+"/axis_id", '0').toString();
            d.ID='0';
            if (s.size()>0) d.ID=s[0];

            d.port=ports.addCOMPort(inifile, axisname+"/", 38400);
            d.serial=new QFExtensionLinearStagePI663ProtocolHandler(ports.getCOMPort(d.port), ports.getMutex(d.port), getName());

            d.acceleration=inifile.value(axisname+"/acceleration", defaultAD.acceleration).toDouble();
            d.deceleration=inifile.value(axisname+"/acceleration", defaultAD.deceleration).toDouble();
            d.initVelocity=inifile.value(axisname+"/initvelocity", defaultAD.initVelocity).toDouble();
            d.maxVelocity=inifile.value(axisname+"/maxvelocity", defaultAD.maxVelocity).toDouble();
            d.maxCoord=inifile.value(axisname+"/maxcoord", defaultAD.maxCoord).toDouble();
            d.minCoord=inifile.value(axisname+"/mincoord", defaultAD.minCoord).toDouble();
            d.backlashCorr=inifile.value(axisname+"/backlashcorr", defaultAD.backlashCorr).toDouble();
            d.backlashCorr=inifile.value(axisname+"/ms", defaultAD.ms).toDouble();



            d.velocity=defaultAD.initVelocity;
            d.joystickEnabled=false;
            d.state=QFExtensionLinearStage::Disconnected;
            d.name=axisname;
            d.label=inifile.value(axisname+"/label", tr("PI Mercury 663, axis %1").arg(axis)).toString();
            axes.append(d);
        }
    }

    actCalibrateJoysticks=new QFActionWithNoMenuRole(QIcon(":/stage_pi/pi_joystick.png"), tr("Calibrate PI Mercury C-663 stage joysticks"), this);
    connect(actCalibrateJoysticks, SIGNAL(triggered()), this, SLOT(calibrateJoysticks()));
    if (services) {
        QMenu* m=services->getMenu("extensions");
        if (m) {
            m->addAction(actCalibrateJoysticks);
        }
    }
}

void QFExtensionLinearStagePI663::deinit() {
	/* add code for cleanup here */
    QSettings inifile(services->getGlobalConfigFileDirectory()+"/stage_pi663.ini", QSettings::IniFormat);
    int axisCount=inifile.value("axis_count", 0).toUInt();


    inifile.setValue("axis_count", axes.size());
    for (int axis=0; axis<axes.size(); axis++) {
        QString axisname=QString("axis%1").arg(axis);
        inifile.setValue(axisname+"/id", axes[axis].ID);

        inifile.setValue(axisname+"/acceleration", axes[axis].acceleration);
        inifile.setValue(axisname+"/initvelocity", axes[axis].initVelocity);
        inifile.setValue(axisname+"/maxvelocity", axes[axis].maxVelocity);
        inifile.setValue(axisname+"/label", axes[axis].label);
        inifile.setValue(axisname+"/maxcoord", axes[axis].maxCoord);
        inifile.setValue(axisname+"/mincoord", axes[axis].minCoord);
        inifile.setValue(axisname+"/backlashcorr", axes[axis].backlashCorr);
        inifile.setValue(axisname+"/ms", axes[axis].ms);
    }
}

void QFExtensionLinearStagePI663::projectChanged(QFProject* /*oldProject*/, QFProject* /*project*/) {
    /* usually stages do not have to react to a change of the project in QuickFit .. so you don't need to do anything here
	   But: possibly you could read config information from the project here
	 */
}


void QFExtensionLinearStagePI663::loadSettings(ProgramOptions* settingspo) {
	/* here you could read config information from the quickfit.ini file using settings object */
    if (!settingspo) return;
	if (settingspo->getQSettings()==NULL) return;
    //QSettings& settings=*(settingspo->getQSettings()); // the QSettings object for quickfit.ini

	// ALTERNATIVE: read/write Information to/from plugins/<ID>/<ID>.ini file
	// QSettings settings(services->getConfigFileDirectory()+"/plugins/"+getID()+"/"+getID()+".ini", QSettings::IniFormat);

}

void QFExtensionLinearStagePI663::storeSettings(ProgramOptions* settingspo) {
	/* here you could write config information to the quickfit.ini file using settings object */
    if (!settingspo) return;
	if (settingspo->getQSettings()==NULL) return;
    //QSettings& settings=*(settingspo->getQSettings()); // the QSettings object for quickfit.ini

	// ALTERNATIVE: read/write Information to/from plugins/<ID>/<ID>.ini file
	// QSettings settings(services->getConfigFileDirectory()+"/plugins/"+getID()+"/"+getID()+".ini", QSettings::IniFormat);

}

unsigned int QFExtensionLinearStagePI663::getAxisCount() {
	return axes.size();
}

void QFExtensionLinearStagePI663::showSettingsDialog(unsigned int /*axis*/, QWidget* parent) {


    QString ini1=services->getGlobalConfigFileDirectory()+QString("/")+GLOBAL_CONFIGFILE;
    QString ini2=services->getConfigFileDirectory()+QString("/")+GLOBAL_CONFIGFILE;
    QString ini3=services->getAssetsDirectory()+QString("plugins/")+getID()+QString("/")+GLOBAL_CONFIGFILE;
    QMessageBox::information(parent, getName(), tr("There is no configuration dialog for this device. Set all config in the appropriate ini file:\n  %1\n  or: %2\n  or: %3").arg(ini1).arg(ini2).arg(ini3));




//    bool globalIniWritable=QSettings(services->getGlobalConfigFileDirectory()+"/stage_pi663.ini", QSettings::IniFormat).isWritable();


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




void QFExtensionLinearStagePI663::calibrateJoysticks() {

    for (unsigned int axis=0; axis<getAxisCount(); axis++) {
        QMessageBox::StandardButton answer=QMessageBox::question(NULL, tr("PI Mercury C663 joystick calibration"), tr("Do you want to calibrate a joystick on axis %1?").arg(axis), QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel, QMessageBox::Yes);
        if (answer==QMessageBox::Yes) {
            bool wasConnected=isConnected(axis);
            if (!wasConnected) connectDevice(axis);
            setJoystickActive(axis, false);
            PIMercury663CalibrationDialog* dlg=new PIMercury663CalibrationDialog(NULL, this, axis);
            dlg->exec();
            delete dlg;
            if (!wasConnected) disconnectDevice(axis);
        } else if (answer==QMessageBox::Cancel)  {
            break;
        }
    }
}



bool QFExtensionLinearStagePI663::isConnected(unsigned int axis) {
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        return axes[axis].serial->getCOM()->isConnectionOpen();
    }
    return false;
}

void QFExtensionLinearStagePI663::connectDevice(unsigned int axis) {
    log_text((tr(LOG_PREFIX "Connecting Mercury C-663 Motor Controller Axis %1 ...\n").arg(axis)));
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI663ProtocolHandler* serial=axes[axis].serial;
        com->open();
        if (com->isConnectionOpen()) {

            std::string error=serial->queryCommand(std::string("ERR?")); // Clear Errors
            if (error!="0") log_text(tr(LOG_PREFIX "Last Occured Error: %1 (see Manual p. 162 for details)\n").arg(error.c_str()));

            log_text(tr(LOG_PREFIX "Mercury C-663 Motor Controller (use with caution)\n"));
            log_text(tr(LOG_PREFIX "Controller Version: %1\n").arg(serial->queryCommand("VER?").c_str()));
            log_text(tr(LOG_PREFIX "Device Identification: %1\n").arg(serial->queryCommand("*IDN?").c_str()));
            log_warning(tr(LOG_PREFIX "WARNING: GCS Controller, Daisy Chain not implemented, so only one controller via USB supported\n"));

            serial->sendCommand("ACC "+ axes[axis].ID.toStdString() + " " + inttostr(axes[axis].acceleration/axes[axis].accelerationFactor));
            serial->sendCommand("DEC "+ axes[axis].ID.toStdString() + " " + inttostr(axes[axis].deceleration/axes[axis].accelerationFactor));

            error=serial->queryCommand(std::string("ERR?"));
            if (error!="0") log_text(tr(LOG_PREFIX "Last Error: %1 (see Manual p. 162 for details)\n").arg(error.c_str()));

            serial->sendCommand("JAX 1 " + axes[axis].ID.toStdString() + " 1"); // Attribute joystick on controller to control of corresponding controller axis. See C-663 Manual for details.

            // Tell controller state
            log_text(tr(LOG_PREFIX "Controller setup. ACCELERATION: %1, DECELERATION: %2, REFERENCE MODE: %3, AXES IDS: %4, MOTOR STATES: %5\n").arg(serial->queryCommand("ACC?").c_str()).arg(serial->queryCommand("DEC?").c_str()).arg(serial->queryCommand("RON?").c_str()).arg(serial->queryCommand("SAI?").c_str()).arg(serial->queryCommand("SVO?").c_str()));

            serial->sendCommand("SVO " + axes[axis].ID.toStdString() + " 1"); // Motor ON

            error=serial->queryCommand(std::string("ERR?"));
            if (error!="0") log_text(tr(LOG_PREFIX "Last Error: %1 (see Manual p. 162 for details)\n").arg(error.c_str()));

            if (!com->hasErrorOccured()) {
                if (axes[axis].doRefMove==true) {
                    int isRefSet;
                    std::string frf_response;
                    if(sscanf(serial->queryCommand("FRF? "+axes[axis].ID.toStdString()).c_str(), "%*i=%i", &isRefSet)) {
                        if(isRefSet==true) {
                            log_text(tr(LOG_PREFIX "Reference Position is defined.(Undo by restarting Controller)\n"));
                        }
                        else {
                            log_text(tr(LOG_PREFIX "Reference Position not defined, Referencing activated. Referencing, returning to inital position...\n"));
                            std::string ref_pos_string=serial->queryCommand(std::string("SPA? " + axes[axis].ID.toStdString() + " 0x16")); // Position at Reference Switch
                            float reference_switch_pos=8;
                            sscanf(ref_pos_string.c_str(), "%*1s %*i=%f", &reference_switch_pos);
                            serial->sendCommand("RON "+ axes[axis].ID.toStdString() +" 0"); // Deactivate referencing requirement for axis
                            serial->sendCommand("POS "+axes[axis].ID.toStdString()+" 0");
                            serial->sendCommand("FED "+ axes[axis].ID.toStdString() +" 3 0"); // Find reference switch position (1 negative limit, 2 pos limit, 3 reference switch)
                            while("0\x00a"!=serial->queryCommandSingleChar("\x005")) QThread::msleep(axes[axis].ms); // (\x00a is the same character as \n or [LF])
                            double RelativeInitialPosition=-getPosition(axis);
                            serial->sendCommand("POS "+axes[axis].ID.toStdString()+" "+ floattostr(reference_switch_pos,6,true));
                            serial->sendCommand("MVR "+axes[axis].ID.toStdString()+" "+floattostr((double)RelativeInitialPosition/axes[axis].lengthFactor,6,true));
                            while("0\x00a"!=serial->queryCommandSingleChar("\x005")) QThread::msleep(axes[axis].ms);
                            log_text(tr("Done.\n"));
                        }
                    }
                    else {
                        log_error(tr(LOG_PREFIX "Invalid result string from FRF? command (Getting Reference Result) [expected <axis>=<bool>] from axis %1\n").arg(inttostr(axis).c_str()));
                        log_error( tr(LOG_PREFIX "Result of FRF? command was %1").arg(frf_response.c_str()) );
                    }
                }
            }

            axes[axis].velocity=axes[axis].initVelocity;
            serial->sendCommand("VEL " + axes[axis].ID.toStdString() + " " + floattostr(((double)axes[axis].velocity/axes[axis].velocityFactor), 6));
            axes[axis].joystickEnabled=false;

            if (com->hasErrorOccured()) {
                axes[axis].state=QFExtensionLinearStage::Error;
            } else {
                axes[axis].state=QFExtensionLinearStage::Ready;
            }
        } else {
            log_error(tr(LOG_PREFIX " Could not connect to Mercury C-663 Motor Controller!!!\n"));
            log_error(tr(LOG_PREFIX " reason: %1\n").arg(com->getLastError().c_str()));
            axes[axis].state=QFExtensionLinearStage::Disconnected;
        }
    }
}

void QFExtensionLinearStagePI663::disconnectDevice(unsigned int axis) {
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI663ProtocolHandler* serial=axes[axis].serial;
        if (com->isConnectionOpen()) {
            std::string error=serial->queryCommand(std::string("ERR?"));
            if (error!="0") log_text(tr(LOG_PREFIX "Last Error: %1 (see Manual p. 162 for details)\n").arg(error.c_str()));
            serial->sendCommand("SVO " + axes[axis].ID.toStdString() + " 0"); // Motor OFF
            serial->sendCommand("JON 1 0"); // Switch off joystick
            axes[axis].joystickEnabled=false;
        }
        com->close();
        axes[axis].state=QFExtensionLinearStage::Disconnected;
    }
}

void QFExtensionLinearStagePI663::setLogging(QFPluginLogService* logService) {
	this->logService=logService;
}

void QFExtensionLinearStagePI663::setJoystickActive(unsigned int axis, bool enabled, double setVelocity) {
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI663ProtocolHandler* serial=axes[axis].serial;

        if (enabled) {
            //serial->sendCommand("VEL " + axes[axis].ID.toStdString() + " " + floattostr(((double)setVelocity/axes[axis].velocityFactor), 6));
            //serial->sendCommand("JON 1 1"); // Switch ON joystick
            //axes[axis].joystickEnabled=true;
        } else {
            while("\x030\x00a"!=serial->queryCommandSingleChar("\x005")) {QThread::msleep(axes[axis].ms);}
            //serial->sendCommand("JON 1 0"); // Switch OFF joystick
            axes[axis].joystickEnabled=false;
        }
    }


}

bool QFExtensionLinearStagePI663::isJoystickActive(unsigned int axis) {
	return axes[axis].joystickEnabled;
}


void QFExtensionLinearStagePI663::stop(unsigned int axis) { // Stop abruptly, error code 10 emitted
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI663ProtocolHandler* serial=axes[axis].serial;
        if (com->isConnectionOpen()) {
            serial->sendCommandSingleChar("\x024");
        }
    }
}

double QFExtensionLinearStagePI663::getSpeed(unsigned int axis) {
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI663ProtocolHandler* serial=axes[axis].serial;
        if (com->isConnectionOpen()) {
            if (!com->hasErrorOccured()) {
                int isMoving=0;
                std::string reply=serial->queryCommandSingleChar("\x005");
                if (sscanf(reply.c_str(), "%i\n", &isMoving)) {
                    if (isMoving!=0) {
                        std::string reply=serial->queryCommand(std::string("VEL? ") + axes[axis].ID.toStdString());
                        replace_reply_punctuation(&reply);
                        double vel=0;
                        if(sscanf(reply.c_str(), "%*i=%lf", &vel)) {
                            return vel*axes[axis].velocityFactor;
                        }
                        else {
                           log_error(tr(LOG_PREFIX " invalid result string from VEL? after ASCII #5 command [expected <axis>=<speed>] from axis %1. String was '%2'.\n").arg(axis).arg(toprintablestr(reply).c_str()));
                        }
                    }
                    else return (double)isMoving;
                } else {
                    log_error(tr(LOG_PREFIX " invalid result string from ASCII #5 command (Request Motion Status) [expected <bool>] from axis %1. String was '%2'.\n").arg(axis).arg(toprintablestr(reply).c_str()));
                }
            }
        }
    }

    return 0;
}

double QFExtensionLinearStagePI663::getPosition(unsigned int axis) {
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI663ProtocolHandler* serial=axes[axis].serial;
        if (com->isConnectionOpen()) {
            std::string reply=serial->queryCommand("POS? "+ axes[axis].ID.toStdString());
            replace_reply_punctuation(&reply);
            double pos=0;
            if (!com->hasErrorOccured()) {
                if (sscanf(reply.c_str(), "%*i=%lf", &pos)) {
                    return pos*axes[axis].lengthFactor;
                } else {
                    log_error(tr(LOG_PREFIX " invalid result string from POS? command [expected <axis id>=<number>] from axis %1. String was '%2'.\n").arg(axis).arg(toprintablestr(reply).c_str()));
                }
            }
        }    }


    return 0;
}

void QFExtensionLinearStagePI663::move(unsigned int axis, double newPosition) {
    if (((int64_t)axis<axes.size())) {
        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI663ProtocolHandler* serial=axes[axis].serial;
        if (com->isConnectionOpen() && (axes[axis].state==QFExtensionLinearStage::Ready) && (!axes[axis].joystickEnabled)) {
            double xx=newPosition/axes[axis].lengthFactor;
            if ( (axes[axis].maxCoord !=0  && newPosition>axes[axis].maxCoord) || (axes[axis].minCoord !=0 && newPosition<axes[axis].minCoord) ) {
                log_warning(tr(LOG_PREFIX " error on axis %1: Move attempt with position exceeding limits").arg(axis));
            }
            else {
                if (!com->hasErrorOccured()) {
                    double currentPosition=getPosition(axis);
                    serial->sendCommand("VEL " + axes[axis].ID.toStdString() + " " + floattostr((axes[axis].velocity/axes[axis].velocityFactor), 6));
                    axes[axis].state=QFExtensionLinearStage::Moving;
                    if(currentPosition<newPosition) {

                        serial->sendCommand("MOV " + axes[axis].ID.toStdString() + " "  + floattostr(xx+(axes[axis].backlashCorr/axes[axis].lengthFactor),6)); // Always approach from same side to evade backlash
                        while("\x030\x00a"!=serial->queryCommandSingleChar("\x005")) {QThread::msleep(axes[axis].ms);}
                        serial->sendCommand("MOV " + axes[axis].ID.toStdString() + " " + floattostr(xx,6));

                    }
                    else if (currentPosition>newPosition) {

                        serial->sendCommand("MOV " + axes[axis].ID.toStdString() + " " + floattostr(xx,6));

                    }
                    else if (currentPosition==newPosition) log_text(tr(LOG_PREFIX "Stage already in Position.\n"));
                    //while("\x030\x00a"!=serial->queryCommandSingleChar("\x005")) {QThread::msleep(axes[axis].ms);}
                    //axes[axis].state=QFExtensionLinearStage::Ready;
                }
            }
        }
    }
}


QFExtensionLinearStage::AxisState QFExtensionLinearStagePI663::getAxisState(unsigned int axis)  {
    if (((int64_t)axis<axes.size())) {

        QMutexLocker locker(axes[axis].serial->getMutex());
        QFSerialConnection* com=axes[axis].serial->getCOM();
        QFExtensionLinearStagePI663ProtocolHandler* serial=axes[axis].serial;


        std::string failedaxes="";
        if (com->isConnectionOpen()) {

            std::string reply=serial->queryCommandSingleChar(std::string("\x004"));
            std::string binstat=hextobin(reply.substr(2,4));

            int BZERO=0, B1=0, B2=0, B3=0, B4=0, B5=0, B6=0, B7=0, B8=0, B9=0, B10=0, B11=0, B12=0, B13=0, B14=0, B15=0;

            if (!com->hasErrorOccured()) {
                if (sscanf(binstat.c_str(), "%1x %1x %1x %1x %1x %1x %1x %1x %1x %1x %1x %1x %1x %1x %1x %1x", &B15, &B14, &B13, &B12, &B11, &B10, &B9, &B8, &B7, &B6, &B5, &B4, &B3, &B2, &B1, &BZERO)) {
                    //std::cout<<"\n\n"<<std::string(1, (char)('x'+i-1))<<": "<<r<<"\n\n";
                    if (B8!=0) {
                        log_error(tr(LOG_PREFIX " error on axis %1: Error flag (Bit #8) is set\n").arg(axis));
                        std::string error=serial->queryCommand(std::string("ERR?"));
                        log_error(tr(LOG_PREFIX " error on axis %1: Error code: %2\n").arg(axis).arg(error.c_str()));
                        if (error==inttostr(0)) {
                            log_error(tr(LOG_PREFIX " error on axis %1: Controller says: No error. Therefore Software Error\n").arg(axis));
                            axes[axis].state=QFExtensionLinearStage::Error;
                        } else if (error==inttostr(1)) {
                            log_error(tr(LOG_PREFIX " error on axis %1: Parameter syntax error\n").arg(axis));
                            axes[axis].state=QFExtensionLinearStage::Error;
                        } else if (error==inttostr(2)) {
                            log_error(tr(LOG_PREFIX " error on axis %1: Unknown command\n").arg(axis));
                            axes[axis].state=QFExtensionLinearStage::Error;
                        } else if (error==inttostr(3)) {
                            log_error(tr(LOG_PREFIX " error on axis %1: Command length out of limits or command buffer overrun\n").arg(axis));
                            axes[axis].state=QFExtensionLinearStage::Error;
                        } else if (error==inttostr(4)) {
                            log_error(tr(LOG_PREFIX " error on axis %1: Error while scanning\n").arg(axis));
                            axes[axis].state=QFExtensionLinearStage::Error;
                        } else if (error==inttostr(5)) {
                            log_error(tr(LOG_PREFIX " error on axis %1: Unallowable move attempted on unreferenced axis, or move attempted with servo off\n").arg(axis));
                            axes[axis].state=QFExtensionLinearStage::Error;
                        } else if (error==inttostr(6)) {
                            log_error(tr(LOG_PREFIX " error on axis %1: Parameter for SGA not valid\n").arg(axis));
                            axes[axis].state=QFExtensionLinearStage::Error;
                        } else if (error==inttostr(7)) {
                            log_error(tr(LOG_PREFIX " error on axis %1: Position out of limits\n").arg(axis));
                            axes[axis].state=QFExtensionLinearStage::Error;
                        } else if (error==inttostr(8)) {
                            log_error(tr(LOG_PREFIX " error on axis %1: Velocity out of limits\n").arg(axis));
                            axes[axis].state=QFExtensionLinearStage::Error;
                        } else if (error==inttostr(9)) {
                            log_error(tr(LOG_PREFIX " error on axis %1: Attempt to set pivot point while U,V and W not all 0\n").arg(axis));
                            axes[axis].state=QFExtensionLinearStage::Error;
                        } else if (error==inttostr(10)) {
                            log_error(tr(LOG_PREFIX " error on axis %1: Controller was stopped by command\n").arg(axis));
                            axes[axis].state=QFExtensionLinearStage::Error;
                        } else if (error==inttostr(11)) {
                            log_error(tr(LOG_PREFIX " error on axis %1: Parameter for SST or for one of the embedded scan algorithms out of range\n").arg(axis));
                            axes[axis].state=QFExtensionLinearStage::Error;
                        } else if (error==inttostr(15)) {
                            log_error(tr(LOG_PREFIX " error on axis %1: Invalid axis identifier\n").arg(axis));
                            axes[axis].state=QFExtensionLinearStage::Error;
                        } else if (error==inttostr(17)) {
                            log_error(tr(LOG_PREFIX " error on axis %1: Parameter out of range\n").arg(axis));
                            axes[axis].state=QFExtensionLinearStage::Error;
                        } else if (error==inttostr(23)) {
                            log_error(tr(LOG_PREFIX " error on axis %1: Illegal Axis\n").arg(axis));
                            axes[axis].state=QFExtensionLinearStage::Error;
                        } else if (error==inttostr(24)) {
                            log_error(tr(LOG_PREFIX " error on axis %1: Incorrect number of parameters\n").arg(axis));
                            axes[axis].state=QFExtensionLinearStage::Error;
                        } else if (error==inttostr(25)) {
                            log_error(tr(LOG_PREFIX " error on axis %1: Invalid floating point number\n").arg(axis));
                            axes[axis].state=QFExtensionLinearStage::Error;
                        } else if (error==inttostr(26)) {
                            log_error(tr(LOG_PREFIX " error on axis %1: Parameter missing\n").arg(axis));
                            axes[axis].state=QFExtensionLinearStage::Error;
                        } else if (error==inttostr(45)) {
                            log_error(tr(LOG_PREFIX " error on axis %1: Referencing failed\n").arg(axis));
                            axes[axis].state=QFExtensionLinearStage::Error;
                        } else {
                            log_error(tr(LOG_PREFIX " error on axis %1: Error Code unknown. See C-663 User Manual for further codes\n").arg(axis));
                            axes[axis].state=QFExtensionLinearStage::Error;
                        }
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
                    log_error(tr(LOG_PREFIX " invalid result string from Status Register Request [\x004] command [expected hex response] in getAxisState() [axis %1]. String was '%2'.\n").arg(axis).arg(toprintablestr(reply).c_str()));
                    axes[axis].state=QFExtensionLinearStage::Error;
                }


            } else {
                log_error(tr(LOG_PREFIX "communication error\n").arg(axis));
                axes[axis].state=QFExtensionLinearStage::Error;
            }
        } else {
            axes[axis].state=QFExtensionLinearStage::Disconnected;
        }



        return axes[axis].state;
    }
    return QFExtensionLinearStage::Disconnected;
}

QString QFExtensionLinearStagePI663::getStageName(unsigned int axis) const
{
    QString n=tr("PI Mercury 663, axis %1").arg(axis);
    if ( (int64_t)axis<axes.size()) n=axes[axis].label;
    return n;
}

QFExtensionLinearStage::StageInfo QFExtensionLinearStagePI663::getStageInfo(unsigned int axis) const
{
    QFExtensionLinearStage::StageInfo info;
    info.maxSpeed=axes[axis].maxVelocity;
    return info;
}

void QFExtensionLinearStagePI663::setRefMoveActive(unsigned int axis, bool enabled) {
    axes[axis].doRefMove=enabled;
}

void QFExtensionLinearStagePI663::setSoftLimit(unsigned int axis, double limit) {
    log_text("getSoftLimit not implemented in stage_pi663");
}

double QFExtensionLinearStagePI663::getSoftLimit(unsigned int axis) {
    log_text("getSoftLimit not implemented in stage_pi663");
    return(0);
}

void QFExtensionLinearStagePI663::log_text(QString message) {
	if (logService) logService->log_text(message);
	else if (services) services->log_text(message);
}

void QFExtensionLinearStagePI663::log_warning(QString message) {
	if (logService) logService->log_warning(message);
	else if (services) services->log_warning(message);
}

void QFExtensionLinearStagePI663::log_error(QString message) {
	if (logService) logService->log_error(message);
	else if (services) services->log_error(message);
}


void QFExtensionLinearStagePI663::replace_reply_punctuation (std::string *s) {
    QChar npunct=QLocale::system().decimalPoint();
    if(npunct.toLatin1()!='.') {
        std::replace((*s).begin(),(*s).end(), '.', npunct.toLatin1());
    }
}


Q_EXPORT_PLUGIN2(stage_pi663, QFExtensionLinearStagePI663)

