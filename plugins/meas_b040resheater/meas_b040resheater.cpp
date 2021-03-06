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

#include "meas_b040resheater.h"
#include<QtGlobal>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#else
#include <QtGui>
#endif


#include "qfplugin.h"
#include <iostream>

#define LOG_PREFIX "[ResHeaterArd]: "
#define CONNECTION_DELAY_MS 50

QFExtensionB040ResistorHeater::QFExtensionB040ResistorHeater(QObject* parent):
    QObject(parent)
{
    //serial=new QF3SimpleB040SerialProtocolHandler(&com, getName());
	logService=NULL;

}

QFExtensionB040ResistorHeater::~QFExtensionB040ResistorHeater() {
    devices.clear();
}


void QFExtensionB040ResistorHeater::deinit() {
	/* add code for cleanup here */
    QSettings inifile(services->getGlobalConfigFileDirectory()+"/meas_b040resheater.ini", QSettings::IniFormat);
    if (inifile.isWritable()) {
        inifile.setValue("device_count", getMeasurementDeviceCount());
        for (unsigned int i=0; i<getMeasurementDeviceCount(); i++) {
            int p=devices[i].port;
            if (ports.getCOMPort(p)) {
                ports.storeCOMPort(p, inifile, "device"+QString::number(i+1)+"/");
            }
        }
    }
}

void QFExtensionB040ResistorHeater::projectChanged(QFProject* /*oldProject*/, QFProject* /*project*/) {

}

void QFExtensionB040ResistorHeater::initExtension() {
    /* do initializations here but do not yet connect to the camera! */
    QString ini=services->getGlobalConfigFileDirectory()+QString("/meas_b040resheater.ini");
    if (!QFile::exists(ini)) ini=services->getConfigFileDirectory()+QString("/meas_b040resheater.ini");
    if (!QFile::exists(ini)) ini=services->getAssetsDirectory()+QString("/plugins/")+getID()+QString("/meas_b040resheater.ini");
    QFPluginServices::getInstance()->log_global_text(tr("%1loading INI-file %2\n").arg(QString(LOG_PREFIX)).arg(ini));

    QSettings inifile(ini, QSettings::IniFormat);
    int device_count=inifile.value("device_count", 0).toUInt();
    for (int i=0; i<device_count; i++) {
        DEVICEINFO s;
        s.port=ports.addCOMPort(inifile, "device"+QString::number(i+1)+"/", 115200);
        s.infoMessage="";
        s.lastAction=QTime::currentTime();
        s.serial=new QF3SimpleB040SerialProtocolHandler(ports.getCOMPort(s.port), getName());
        //s.serial->setAddToCommand("");
        s.label=inifile.value("device"+QString::number(i+1)+"/label", tr("B040 Temperature Controller #%1").arg(i+1)).toString();
        devices.append(s);
    }
}

void QFExtensionB040ResistorHeater::loadSettings(ProgramOptions* settingspo) {
	/* here you could read config information from the quickfit.ini file using settings object */
    if (!settingspo) return;
	if (settingspo->getQSettings()==NULL) return;
    //QSettings& settings=*(settingspo->getQSettings()); // the QSettings object for quickfit.ini

	// ALTERNATIVE: read/write Information to/from plugins/<ID>/<ID>.ini file
	// QSettings settings(services->getConfigFileDirectory()+"/plugins/"+getID()+"/"+getID()+".ini", QSettings::IniFormat);

}

void QFExtensionB040ResistorHeater::storeSettings(ProgramOptions* settingspo) {
	/* here you could write config information to the quickfit.ini file using settings object */
    if (!settingspo) return;
	if (settingspo->getQSettings()==NULL) return;
    //QSettings& settings=*(settingspo->getQSettings()); // the QSettings object for quickfit.ini

	// ALTERNATIVE: read/write Information to/from plugins/<ID>/<ID>.ini file
	// QSettings settings(services->getConfigFileDirectory()+"/plugins/"+getID()+"/"+getID()+".ini", QSettings::IniFormat);

}









void QFExtensionB040ResistorHeater::setLogging(QFPluginLogService* logService) {
    this->logService=logService;
}

unsigned int QFExtensionB040ResistorHeater::getMeasurementDeviceCount()
{
    return devices.size();

}

QString QFExtensionB040ResistorHeater::getMeasurementDeviceName(unsigned int measuremenDevice)
{
    if ((long long)measuremenDevice<devices.size()) {
        return devices[measuremenDevice].label;
    }
    return QString();
}

void QFExtensionB040ResistorHeater::showMeasurementDeviceSettingsDialog(unsigned int /*measuremenDevice*/, QWidget *parent)
{
    QString ini1=services->getGlobalConfigFileDirectory()+QString("/meas_b040resheater.ini");
    QString ini2=services->getConfigFileDirectory()+QString("/meas_b040resheater.ini");
    QString ini3=services->getAssetsDirectory()+QString("/plugins/")+getID()+QString("/meas_b040resheater.ini");
    QMessageBox::information(parent, getName(), tr("There is no configuration dialog for this device. Set all config in the appropriate ini file:\n  %1\n  or: %2\n  or: %3").arg(ini1).arg(ini2).arg(ini3));

}

bool QFExtensionB040ResistorHeater::isMeasurementDeviceConnected(unsigned int measuremenDevice)
{
    if ( measuremenDevice>=getMeasurementDeviceCount()) return false;
    QFSerialConnection* com=ports.getCOMPort(devices[measuremenDevice].port);
    if (!com) return false;
    QMutex* mutex=ports.getMutex(devices[measuremenDevice].port);
    QMutexLocker locker(mutex);
    return com->isConnectionOpen();
}

void QFExtensionB040ResistorHeater::connectMeasurementDevice(unsigned int measuremenDevice)
{
    if ( measuremenDevice>=getMeasurementDeviceCount()) return;
    QFSerialConnection* com=ports.getCOMPort(devices[measuremenDevice].port);
    if (!com) return;
    QMutex* mutex=ports.getMutex(devices[measuremenDevice].port);
    QMutexLocker locker(mutex);
    com->open();
    if (com->isConnectionOpen()) {
        QTime t;
        t.start();
        // wait CONNECTION_DELAY_MS ms for connection!
        while (t.elapsed()<CONNECTION_DELAY_MS)  {
            QApplication::processEvents();
        }
        devices[measuremenDevice].infoMessage=devices[measuremenDevice].serial->queryCommand("?");
        //qDebug()<<"infoMessage '"<<infoMessage<<"'";
        if (!(devices[measuremenDevice].infoMessage.toLower().contains("resitance heater temperature control") && devices[measuremenDevice].infoMessage.toLower().contains("jan krieger"))) {
            com->close();
            log_error(tr("%1 Could not connect to B040 Temperature Controller [port=%1  baud=%2]!!!\n").arg(com->get_port().c_str()).arg(com->get_baudrate()));
            log_error(tr("%1 reason: received wrong ID string from B040 Temperature Controller: string was '%2'\n").arg(LOG_PREFIX).arg(devices[measuremenDevice].infoMessage));
        } else {
            log_text(tr("%1 Connected to B040 Temperature Controller [port=%2  baud=%3] \n%1 welcome message:'%4'\n").arg(LOG_PREFIX).arg(com->get_port().c_str()).arg(com->get_baudrate()).arg(devices[measuremenDevice].infoMessage));
        }
    } else {
        log_error(tr("%1 Could not connect to B040 Temperature Controller [port=%1  baud=%2]!!!\n").arg(com->get_port().c_str()).arg(com->get_baudrate()));
        log_error(tr("%1 reason: %2\n").arg(LOG_PREFIX).arg(devices[measuremenDevice].serial->getLastError()));
    }
}

void QFExtensionB040ResistorHeater::disconnectMeasurementDevice(unsigned int measuremenDevice)
{
    if ( measuremenDevice>=getMeasurementDeviceCount()) return;
    QFSerialConnection* com=ports.getCOMPort(devices[measuremenDevice].port);
    if (!com) return;
    QMutex* mutex=ports.getMutex(devices[measuremenDevice].port);
    QMutexLocker locker(mutex);
    com->close();

}








void QFExtensionB040ResistorHeater::setMeasurementDeviceLogging(QFPluginLogService *logService)
{
    this->logService=logService;
}


QVariant QFExtensionB040ResistorHeater::getMeasurementDeviceValue(unsigned int measuremenDevice, unsigned int value)
{
    if ( measuremenDevice>=getMeasurementDeviceCount()) return QVariant();
    QFSerialConnection* com=ports.getCOMPort(devices[measuremenDevice].port);
    QF3SimpleB040SerialProtocolHandler* serial=devices[measuremenDevice].serial;
    if (!com) return QVariant();
    QMutex* mutex=ports.getMutex(devices[measuremenDevice].port);
    QMutexLocker locker(mutex);
    if (!com->isConnectionOpen()) return QVariant();


    bool ok=false;
    if (value==0) {
        QString rs=serial->queryCommand("1");
        double r=rs.toDouble(&ok)/10.0;
        //qDebug()<<rs<<r<<ok;
        if (ok&&r<1000) return r;
    } else if (value==1) {
        double r=serial->queryCommand("2").toDouble(&ok)/10.0;
        if (ok&&r<1000) return r;
    } else if (value==2) {
        double r=serial->queryCommand("o").toDouble(&ok);
        if (ok) return r!=0;
    } else if (value==3) {
        double r=serial->queryCommand("t").toDouble(&ok)/10.0;
        if (ok&&r<1000) return r;
    } else if (value==4) {
        double r=serial->queryCommand("v").toDouble(&ok)/255.0*100.0;
        if (ok) return r;
    }

    return QVariant();
}

void QFExtensionB040ResistorHeater::setMeasurementDeviceValue(unsigned int measuremenDevice, unsigned int value, const QVariant &data)
{
    if ( measuremenDevice>=getMeasurementDeviceCount()) return ;
    QFSerialConnection* com=ports.getCOMPort(devices[measuremenDevice].port);
    QF3SimpleB040SerialProtocolHandler* serial=devices[measuremenDevice].serial;
    if (!com) return ;
    QMutex* mutex=ports.getMutex(devices[measuremenDevice].port);
    QMutexLocker locker(mutex);
    if (!com->isConnectionOpen()) return ;

    if (value==0) {
        // can not be set
    } else if (value==1) {
        // can not be set
    } else if (value==2) {
        if (data.toBool()) serial->sendCommand("O1");
        else  serial->sendCommand("O0");
    } else if (value==3) {
        serial->sendCommand(QString("T")+QString::number(round(data.toDouble()*10.0)));
    } else if (value==4) {
        // can not be set
    }
    return ;
}

unsigned int QFExtensionB040ResistorHeater::getMeasurementDeviceValueCount(unsigned int /*measuremenDevice*/)
{
    return 5;
}

QString QFExtensionB040ResistorHeater::getMeasurementDeviceValueName(unsigned int /*measuremenDevice*/, unsigned int value)
{
    switch (value) {
        case 0: return tr("temperature 0");
        case 1: return tr("temperature 1");
        case 2: return tr("heating on/off");
        case 3: return tr("set temperature");
        case 4: return tr("output value");
        default: return "";
    }
}

QString QFExtensionB040ResistorHeater::getMeasurementDeviceValueShortName(unsigned int /*measuremenDevice*/, unsigned int value)
{
    switch (value) {
        case 0: return tr("TEMP0");
        case 1: return tr("TEMP1");
        case 2: return tr("HEATING_STATE");
        case 3: return tr("SET_TEMP");
        case 4: return tr("OUTPUT");
        default: return "";
    }
}

bool QFExtensionB040ResistorHeater::isMeasurementDeviceValueEditable(unsigned int /*measuremenDevice*/, unsigned int value)
{
    return (value==2 || value==3) ;
}

QVariant::Type QFExtensionB040ResistorHeater::getMeasurementDeviceEditableValueType(unsigned int /*measuremenDevice*/, unsigned int value)
{
    switch (value) {
        case 0: return QVariant::Double;
        case 1: return QVariant::Double;
        case 2: return QVariant::Bool;
        case 3: return QVariant::Double;
        case 4: return QVariant::Double;
        default: return QVariant::Invalid;
    }

    return QVariant::Invalid;

}

QFExtensionMeasurementAndControlDevice::WidgetTypes QFExtensionB040ResistorHeater::getMeasurementDeviceValueWidget(unsigned int /*measuremenDevice*/, unsigned int value, QStringList */*comboboxEntries*/)
{
    switch (value) {
        case 0: return QFExtensionMeasurementAndControlDevice::mdDoubleEdit;
        case 1: return QFExtensionMeasurementAndControlDevice::mdDoubleEdit;
        case 2: return QFExtensionMeasurementAndControlDevice::mdCheckBox;
        case 3: return QFExtensionMeasurementAndControlDevice::mdDoubleEdit;
        case 4: return QFExtensionMeasurementAndControlDevice::mdDoubleEdit;
        default: return QFExtensionMeasurementAndControlDevice::mdDefault;
    }

    return QFExtensionMeasurementAndControlDevice::mdDefault;

}

void QFExtensionB040ResistorHeater::getMeasurementDeviceEditableValueRange(unsigned int /*measuremenDevice*/, unsigned int value, double &minimum, double &maximum)
{
    switch (value) {
        case 0: minimum=-100; maximum=200; break;
        case 1: minimum=-100; maximum=200; break;
        case 2: minimum=0; maximum=1; break;
        case 3: minimum=0; maximum=100; break;
        case 4: minimum=0; maximum=100; break;
    }

    return;

}

void QFExtensionB040ResistorHeater::log_text(QString message) {
	if (logService) logService->log_text(message);
	else if (services) services->log_text(message);
}

void QFExtensionB040ResistorHeater::log_warning(QString message) {
	if (logService) logService->log_warning(message);
	else if (services) services->log_warning(message);
}

void QFExtensionB040ResistorHeater::log_error(QString message) {
	if (logService) logService->log_error(message);
	else if (services) services->log_error(message);
}



Q_EXPORT_PLUGIN2(meas_b040resheater, QFExtensionB040ResistorHeater)
