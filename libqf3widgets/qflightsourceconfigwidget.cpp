/*
Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center (DKFZ) & IWR, University of Heidelberg

    

    This file is part of QuickFit 3 (http://www.dkfz.de/Macromol/quickfit).

    This software is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License (LGPL) as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License (LGPL) for more details.

    You should have received a copy of the GNU Lesser General Public License (LGPL)
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "qflightsourceconfigwidget.h"

QFLightSourceConfigWidget::QFLightSourceConfigWidget(QWidget *parent) :
    QFrame(parent)
{
    useThread=true;
    m_thread=new QFLightSourceConfigWidgetThread(this);
    connect(m_thread, SIGNAL(linesChanged(QTime, QList<bool>,QList<double>,QList<double>,QStringList,QStringList,QList<bool>)), this, SLOT(linesChanged(QTime, QList<bool>,QList<double>,QList<double>,QStringList,QStringList,QList<bool>)));
    setFrameStyle(QFrame::Panel|QFrame::Raised);
    setLineWidth(1);
    stateUpdateInterval=237;
    dontAccessWidgets=false;

    timUpdate=new QTimer(this);

    m_log=NULL;
    m_pluginServices=NULL;
    locked=false;
    moving=false;


    createWidgets();
    createActions();
    updateStates();


    if (!useThread) {
        timUpdate->setSingleShot(true);
        timUpdate->setInterval(stateUpdateInterval);
        connect(timUpdate, SIGNAL(timeout()), this, SLOT(displayStates()));
        timUpdate->start(stateUpdateInterval);
    }
}

QFLightSourceConfigWidget::~QFLightSourceConfigWidget()
{
    locked=true;
    disconnect(timUpdate, SIGNAL(timeout()), this, SLOT(displayStates()));
    timUpdate->stop();
    m_thread->stopThread();
}

void QFLightSourceConfigWidget::init(QFPluginLogService *log, QFPluginServices *pluginServices) {
    m_log=log;
    m_pluginServices=pluginServices;

    if (m_pluginServices) {
        cmbLightSource->init(m_pluginServices->getExtensionManager());
    } else {
        //stages.clear();
        cmbLightSource->clear();
    }
    updateStates();
}

void QFLightSourceConfigWidget::setLog(QFPluginLogService *log) {
    m_log=log;
}

void QFLightSourceConfigWidget::loadSettings(QSettings &settings, QString prefix) {
    cmbLightSource->loadSettings(settings, prefix+"LightSource/");

    stateUpdateInterval=settings.value(prefix+"update_interval", stateUpdateInterval).toDouble();

}

void QFLightSourceConfigWidget::loadSettings(QFManyFilesSettings &settings, QString prefix)
{
    cmbLightSource->loadSettings(settings, prefix+"LightSource/");

    stateUpdateInterval=settings.value(prefix+"update_interval", stateUpdateInterval).toDouble();
}

void QFLightSourceConfigWidget::saveSettings(QSettings &settings, QString prefix) {
    cmbLightSource->storeSettings(settings, prefix+"LightSource/");
    settings.setValue(prefix+"update_interval", stateUpdateInterval);
}

void QFLightSourceConfigWidget::saveSettings(QFManyFilesSettings &settings, QString prefix)
{
    cmbLightSource->storeSettings(settings, prefix+"LightSource/");
    settings.setValue(prefix+"update_interval", stateUpdateInterval);
}

void QFLightSourceConfigWidget::connectLightSource() {
    bool b=actConnect->signalsBlocked();
    actConnect->blockSignals(true);
    actConnect->setChecked(true);
    disConnect();
    actConnect->blockSignals(b);
}

void QFLightSourceConfigWidget::disconnectLightSource() {
    bool b=actConnect->signalsBlocked();
    actConnect->blockSignals(true);
    actConnect->setChecked(false);
    disConnect();
    actConnect->blockSignals(b);
}


void QFLightSourceConfigWidget::lockLightSource() {
    locked=true;
    if (useThread)  {
        m_thread->stopThread();
    } else {
        disconnect(timUpdate, SIGNAL(timeout()), this, SLOT(displayStates()));
        timUpdate->stop();
    }
}

void QFLightSourceConfigWidget::unlockLightSource() {
    locked=false;
    if (useThread) {
        m_thread->start();
    } else {
        connect(timUpdate, SIGNAL(timeout()), this, SLOT(displayStates()));
        timUpdate->setSingleShot(true);
        timUpdate->setInterval(stateUpdateInterval);
        timUpdate->start(stateUpdateInterval);
    }
}

void QFLightSourceConfigWidget::createWidgets() {
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // create main layout
    ////////////////////////////////////////////////////////////////////////////////////////////////
    widgetLayout=new QGridLayout();
    setLayout(widgetLayout);
    widgetLayout->setContentsMargins(0,0,0,0);
    widgetLayout->setHorizontalSpacing(2);
    widgetLayout->setVerticalSpacing(2);



    ////////////////////////////////////////////////////////////////////////////////////////////////
    // create input widgets for camera devices
    ////////////////////////////////////////////////////////////////////////////////////////////////

    cmbLightSource=new QFLightSourceComboBox(this);
    cmbLightSource->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    widgetLayout->addWidget(cmbLightSource,0,0);
    btnConnect=new QToolButton(this);
    widgetLayout->addWidget(btnConnect,0,1);
    btnConfigure=new QToolButton(this);
    widgetLayout->addWidget(btnConfigure,0,2);
    btnExternalModulation=new QToolButton(this);
    widgetLayout->addWidget(btnExternalModulation,0,3);

    linesLayoutWidget=new QWidget(this);
    linesLayout=new QGridLayout();
    linesLayoutWidget->setLayout(linesLayout);
    linesLayout->setContentsMargins(0,0,0,0);
    linesLayout->setHorizontalSpacing(2);
    linesLayout->setVerticalSpacing(2);
    widgetLayout->addWidget(linesLayoutWidget, 1,0,1,3);
    linesLayout->addWidget(new QLabel(tr("<small><b>line</b></small>"), linesLayoutWidget), 0, 0);
    linesLayout->addWidget(new QLabel(tr("<small><b>set power</b></small>"), linesLayoutWidget), 0, 1);
    linesLayout->addWidget(new QLabel(tr("<small><b>measured power</b></small>"), linesLayoutWidget), 0, 2);
    /*QWidget* w=new QWidget(this);
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    widgetLayout->addWidget(w,widgetLayout->rowCount()-1,widgetLayout->columnCount());
    widgetLayout->setColumnStretch(widgetLayout->columnCount()-1,1);*/

    stretch=new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Expanding);
    widgetLayout->addItem(stretch, widgetLayout->rowCount()-1,widgetLayout->columnCount());


    cmbLightSource->setEnabled(false);


}

void QFLightSourceConfigWidget::createActions() {
    actConnect=new QFActionWithNoMenuRole(QIcon(":/libqf3widgets/connect_lightsource.png"), tr("Connect to filter changer driver/hardware ..."), this);
    actConnect->setCheckable(true);
    connect(actConnect, SIGNAL(toggled(bool)), this, SLOT(disConnect()));
    btnConnect->setDefaultAction(actConnect);


    actConfigure=new QFActionWithNoMenuRole(QIcon(":/libqf3widgets/configure_lightsource.png"), tr("Configure filter changer ..."), this);
    connect(actConfigure, SIGNAL(triggered()), this, SLOT(configure()));
    btnConfigure->setDefaultAction(actConfigure);

    actExternalMod=new QFActionWithNoMenuRole(QIcon(":/libqf3widgets/configure_lightsource.png"), tr("Set external modulation ..."), this);
    connect(actExternalMod, SIGNAL(triggered()), this, SLOT(toggleexternalmod()));
    btnExternalModulation->setDefaultAction(actExternalMod);

}


QFExtensionLightSource *QFLightSourceConfigWidget::getLightSource() const {
    return cmbLightSource->currentExtensionLightSource();
}

QFExtension *QFLightSourceConfigWidget::getLightSourceExtension() const {
    return cmbLightSource->currentExtension();
}

QObject *QFLightSourceConfigWidget::getLightSourceExtensionObject() const
{
    return cmbLightSource->currentExtensionObject();
}

int QFLightSourceConfigWidget::getLightSourceID() const {
    return cmbLightSource->currentLightSourceID();
}

bool QFLightSourceConfigWidget::isLightSourceConnected() const {
    return actConnect->isChecked();
}

bool QFLightSourceConfigWidget::isLastActionComplete() const {
    QFExtensionLightSource* LightSource;
    int LightSourceID;
    LightSource=getLightSource();
    LightSourceID=getLightSourceID();
    if (LightSource && isLightSourceConnected()) {
        return LightSource->isLastLightSourceActionFinished(LightSourceID);
    }
    return true;
}

int QFLightSourceConfigWidget::getLineCount() const {
    QFExtensionLightSource* LightSource=getLightSource();
    int LightSourceID=getLightSourceID();
    if (LightSource && isLightSourceConnected()) {
        return LightSource->getLightSourceLineCount(LightSourceID);
    }
    return 0;
}

QString QFLightSourceConfigWidget::getLineDescription(int line) const {
    QFExtensionLightSource* LightSource=getLightSource();
    int LightSourceID=getLightSourceID();
    if (LightSource && isLightSourceConnected()) {
        return LightSource->getLightSourceLineDescription(LightSourceID, line);
    }
    return QString("");
}

double QFLightSourceConfigWidget::getSetPower(int line) const {
    QFExtensionLightSource* LightSource=getLightSource();
    int LightSourceID=getLightSourceID();
    if (LightSource && isLightSourceConnected()) {
        return LightSource->getLightSourceCurrentSetPower(LightSourceID, line);
    }
    return 0;
}

double QFLightSourceConfigWidget::getMeasuredPower(int line) const {
    QFExtensionLightSource* LightSource=getLightSource();
    int LightSourceID=getLightSourceID();
    if (LightSource && isLightSourceConnected()) {
        return LightSource->getLightSourceCurrentMeasuredPower(LightSourceID, line);
    }
    return 0;
}

bool QFLightSourceConfigWidget::isLineEnabled(int line) const {
    QFExtensionLightSource* LightSource=getLightSource();
    int LightSourceID=getLightSourceID();
    if (LightSource && isLightSourceConnected()) {
        return LightSource->getLightSourceLineEnabled(LightSourceID, line);
    }
    return 0;
}

QString QFLightSourceConfigWidget::getLineUnit(int line) const {
    QFExtensionLightSource* LightSource=getLightSource();
    int LightSourceID=getLightSourceID();
    if (LightSource && isLightSourceConnected()) {
        return LightSource->getLightSourceLinePowerUnit(LightSourceID, line);
    }
    return QString("");
}

void QFLightSourceConfigWidget::updateStates() {
    QFExtensionLightSource* LightSource;
    int LightSourceID;
    bool conn;


    LightSource=getLightSource();
    LightSourceID=getLightSourceID();
    //qDebug()<<"updateStates(    LightSource="<<LightSource<<", LightSourceID="<<LightSourceID<<")";
    conn=false;
    if (LightSource) {
        conn=LightSource->isLightSourceConnected(LightSourceID);
        if (conn) {
            if (!actConnect->isChecked()) actConnect->setChecked(true);
            QString txt=tr("Disconnect from light source driver/hardware ...");
            if (actConnect->text()!=txt) {
                actConnect->setText(txt);
                actConnect->setIcon(QIcon(":/libqf3widgets/disconnect_lightsource.png"));
            }
        } else {
            if (actConnect->isChecked()) actConnect->setChecked(false);

            QString txt=tr("Connect from light source driver/hardware ...");
            if (actConnect->text()!=txt) {
                actConnect->setText(txt);
                actConnect->setIcon(QIcon(":/libqf3widgets/connect_lightsource.png"));
            }
        }
    }
    bool en=(LightSource!=NULL) && (LightSourceID>=0);
    if (actConfigure->isEnabled()!=en) actConfigure->setEnabled(en);
    if (actExternalMod->isEnabled()!=en) actExternalMod->setEnabled(en);
    if (!actConnect->isEnabled()) actConnect->setEnabled(true);//(LightSource!=NULL) && (LightSourceID>=0));
    if (cmbLightSource->isEnabled()!=!conn) cmbLightSource->setEnabled(!conn);
}

void QFLightSourceConfigWidget::setPowerChanged(double value) {
    QFExtensionLightSource* LightSource;
    int LightSourceID;
    LightSource=getLightSource();
    LightSourceID=getLightSourceID();
    if (LightSource) {
        setLinePower(getLineByWidget(sender()), value);
    }

}

void QFLightSourceConfigWidget::lineEnabledToggled(bool enabled) {
    QFExtensionLightSource* LightSource;
    int LightSourceID;
    LightSource=getLightSource();
    LightSourceID=getLightSourceID();
    if (LightSource) {
        setLineEnabled(getLineByWidget(sender()), enabled);
    }
}

void QFLightSourceConfigWidget::disConnect() {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool conn=actConnect->isChecked();
    QFExtensionLightSource* LightSource=getLightSource();
    int LightSourceID=getLightSourceID();
    bool gotConnection=false;

    if (LightSource) {
        //qDebug()<<"connecting "<<conn;
        if (conn) {
            LightSource->setLightSourceLogging(m_log);
            LightSource->lightSourceConnect(LightSourceID);
            if (LightSource->isLightSourceConnected(LightSourceID)) {
                m_log->log_text("connected to light source driver ...\n");
                gotConnection=true;
            } else {
                actConnect->setChecked(false);
                LightSource->lightSourceDisonnect(LightSourceID);
                m_log->log_error("error connecting to light source driver ...\n");
            }
        } else {
            LightSource->lightSourceDisonnect(LightSourceID);
            m_log->log_text("disconnected from light source driver ...\n");
        }
    } else {
        actConnect->setChecked(false);
    }
    updateStates();
    updateLSLinesWidgets();
    if (useThread) {
        if (gotConnection) m_thread->start();
        else m_thread->stopThread();
    }
    QApplication::restoreOverrideCursor();
}

void QFLightSourceConfigWidget::configure() {
    QFExtensionLightSource* LightSource=getLightSource();
    int LightSourceID=getLightSourceID();
    if (LightSource) LightSource->showLightSourceSettingsDialog(LightSourceID, this);
    updateLSLinesWidgets();
    updateStates();
}

void QFLightSourceConfigWidget::toggleexternalmod() {
    QFExtensionLightSource* LightSource=getLightSource();
    int LightSourceID=getLightSourceID();
    bool extmod=actExternalMod->isChecked();
    if (LightSource) LightSource->setExternalModulation(LightSourceID, this);
    updateLSLinesWidgets();
    updateStates();
}

void QFLightSourceConfigWidget::updateLSLinesWidgets() {
    dontAccessWidgets=true;
    bool updt=updatesEnabled(); bool widVisible=isVisible(); if (widVisible) setUpdatesEnabled(false);
    int oldLines=lineWidgets.size();
    int newLines=0;
    QFExtensionLightSource* lightSource=getLightSource();
    int id=getLightSourceID();
    if (lightSource && isLightSourceConnected()) {
        newLines=lightSource->getLightSourceLineCount(id);
    }

    if (newLines<oldLines) {
        for (int i=oldLines-1; i>=newLines; i--) {
            linesLayout->removeWidget(lineWidgets[i].labPower);
            linesLayout->removeWidget(lineWidgets[i].chkEnable);
            linesLayout->removeWidget(lineWidgets[i].spinPower);
            lineWidgets[i].labPower->deleteLater();
            lineWidgets[i].chkEnable->deleteLater();
            lineWidgets[i].spinPower->deleteLater();
            lineWidgets.removeAt(i);
        }
    } else if (newLines>oldLines) {
        for (int i=oldLines; i<newLines; i++) {
            LSLineWidgets w;
            w.chkEnable=new QCheckBox(this);
            w.chkEnable->setChecked(false);
            w.spinPower=new QDoubleSpinBox(this);
            w.spinPower->setEnabled(false);
            w.labPower=new QLabel(this);
            connect(w.chkEnable, SIGNAL(toggled(bool)), w.spinPower, SLOT(setEnabled(bool)));
            connect(w.chkEnable, SIGNAL(toggled(bool)), w.labPower, SLOT(setEnabled(bool)));
            int line=widgetLayout->rowCount();
            linesLayout->addWidget(w.chkEnable, line, 0);
            linesLayout->addWidget(w.spinPower, line, 1);
            linesLayout->addWidget(w.labPower, line, 2);
            lineWidgets.append(w);
            connect(w.chkEnable, SIGNAL(toggled(bool)), this, SLOT(lineEnabledToggled(bool)));
            //connect(w.spinPower, SIGNAL(valueChanged(double)), this, SLOT(setPowerChanged(double)));
            connect(w.spinPower, SIGNAL(editingFinished ()), this, SLOT(setPowerEditingFinished()));
        }
    }

    for (int i=0; i<newLines; i++) {
        lineWidgets[i].chkEnable->setText(lightSource->getLightSourceLineDescription(id, i));
        double min=0;
        double max=0;
        lightSource->getLightSourceLinePowerRange(id, i, min, max);
        disconnect(lineWidgets[i].chkEnable, SIGNAL(toggled(bool)), this, SLOT(lineEnabledToggled(bool)));
        //disconnect(lineWidgets[i].spinPower, SIGNAL(valueChanged(double)), this, SLOT(setPowerChanged(double)));
        disconnect(lineWidgets[i].spinPower, SIGNAL(editingFinished ()), this, SLOT(setPowerEditingFinished()));
        lineWidgets[i].spinPower->setRange(min, max);
        lineWidgets[i].spinPower->setValue(lightSource->getLightSourceCurrentSetPower(id, i));
        lineWidgets[i].spinPower->setSuffix(QString(" ")+lightSource->getLightSourceLinePowerUnit(id, i));
        lineWidgets[i].spinPower->setEnabled(lightSource->isLastLightSourceActionFinished(id));
        lineWidgets[i].chkEnable->setChecked(lightSource->getLightSourceLineEnabled(id, i));
        lineWidgets[i].labPower->setText(QString("%1 %2").arg(lightSource->getLightSourceCurrentMeasuredPower(id, i)).arg(lightSource->getLightSourceLinePowerUnit(id, i)));
        connect(lineWidgets[i].chkEnable, SIGNAL(toggled(bool)), this, SLOT(lineEnabledToggled(bool)));
        //connect(lineWidgets[i].spinPower, SIGNAL(valueChanged(double)), this, SLOT(setPowerChanged(double)));
        connect(lineWidgets[i].spinPower, SIGNAL(editingFinished ()), this, SLOT(setPowerEditingFinished()));
    }

    linesLayoutWidget->setVisible(newLines>0);
    dontAccessWidgets=false;
    if (widVisible) setUpdatesEnabled(updt);
}

int QFLightSourceConfigWidget::getLineByWidget(QObject *widget) {
    int result=-1;
    for (int i=0; i<lineWidgets.size(); i++){
        if (lineWidgets[i].chkEnable==widget) return i;
        if (lineWidgets[i].labPower==widget) return i;
        if (lineWidgets[i].spinPower==widget) return i;
    }
    return result;
}

void QFLightSourceConfigWidget::displayStates() {
    if (useThread) return;
    if (locked) return;

    bool updt=updatesEnabled(); bool widVisible=isVisible(); if (widVisible) setUpdatesEnabled(false);

    QFExtensionLightSource* LightSource;
    int LightSourceID;
    LightSource=getLightSource();
    LightSourceID=getLightSourceID();
    if (LightSource) {
        bool enabled=!moving && LightSource->isLastLightSourceActionFinished(LightSourceID);
        for (int i=0; i<lineWidgets.size(); i++) {
            disconnect(lineWidgets[i].chkEnable, SIGNAL(toggled(bool)), this, SLOT(lineEnabledToggled(bool)));
            //disconnect(lineWidgets[i].spinPower, SIGNAL(valueChanged(double)), this, SLOT(setPowerChanged(double)));
            disconnect(lineWidgets[i].spinPower, SIGNAL(editingFinished ()), this, SLOT(setPowerEditingFinished()));
            if (enabled) {
                lineWidgets[i].chkEnable->setChecked(LightSource->getLightSourceLineEnabled(LightSourceID, i));
            }

            double setP=LightSource->getLightSourceCurrentSetPower(LightSourceID, i);
            if (enabled && (lineWidgets[i].spinPower->value()!=setP) && !lineWidgets[i].spinPower->hasFocus()) lineWidgets[i].spinPower->setValue(setP);

            lineWidgets[i].labPower->setText(QString("%1 %2").arg(LightSource->getLightSourceCurrentMeasuredPower(LightSourceID, i)).arg(LightSource->getLightSourceLinePowerUnit(LightSourceID, i)));
            lineWidgets[i].chkEnable->setText(LightSource->getLightSourceLineDescription(LightSourceID, i));

            lineWidgets[i].spinPower->setEnabled(enabled);
            lineWidgets[i].chkEnable->setEnabled(enabled);
            lineWidgets[i].labPower->setEnabled(enabled);

            connect(lineWidgets[i].chkEnable, SIGNAL(toggled(bool)), this, SLOT(lineEnabledToggled(bool)));
            //connect(lineWidgets[i].spinPower, SIGNAL(valueChanged(double)), this, SLOT(setPowerChanged(double)));
            connect(lineWidgets[i].spinPower, SIGNAL(editingFinished ()), this, SLOT(setPowerEditingFinished()));
        }
    }

    updateStates();
    if (widVisible) setUpdatesEnabled(updt);

    //QTimer::singleShot(stateUpdateInterval, this, SLOT(displayLightSourceStates()));
    if (!locked) {
        if (!useThread)  {
            timUpdate->setSingleShot(true);
            timUpdate->setInterval(stateUpdateInterval);
            timUpdate->start(stateUpdateInterval);
        }

    }
}

void QFLightSourceConfigWidget::setReadOnly(bool readonly) {
    cmbLightSource->setReadOnly(readonly);
}

void QFLightSourceConfigWidget::setLineEnabled(int line, bool enabled) {
    QFExtensionLightSource* LightSource=getLightSource();
    int LightSourceID=getLightSourceID();
    if (LightSource) {
        LightSource->setLightSourceLineEnabled(LightSourceID, line, enabled);
        if (!locked) {
            moving=true;
            QTime started=QTime::currentTime();
            while (!LightSource->isLastLightSourceActionFinished(LightSourceID) && (started.elapsed()<20000)) {
                //qDebug()<<started.elapsed();
                QApplication::processEvents();
            }
            moving=false;
        }
    }
}

void QFLightSourceConfigWidget::setLinePower(int line, double power) {
    QFExtensionLightSource* LightSource=getLightSource();
    int LightSourceID=getLightSourceID();
    if (LightSource) {
        LightSource->setLightSourcePower(LightSourceID, line, power);
        if (!locked) {
            moving=true;
            QTime started=QTime::currentTime();
            while (!LightSource->isLastLightSourceActionFinished(LightSourceID) && (started.elapsed()<20000)) {
                //qDebug()<<started.elapsed();
                QApplication::processEvents();
            }
            moving=false;
        }
    }
}

void QFLightSourceConfigWidget::linesChanged(QTime /*time*/, QList<bool> lineenabled, QList<double> setValues, QList<double> measuredValues, QStringList powerUnits, QStringList lineNames, QList<bool> widgetsEnabled) {
    if (dontAccessWidgets) return;
    bool updt=updatesEnabled(); bool widVisible=isVisible(); if (widVisible) setUpdatesEnabled(false);
    //qDebug()<<"linesChanged("<< lineenabled<< setValues <<measuredValues<<")";
    for (int i=0; i<lineWidgets.size(); i++) {
        if (i<lineenabled.size()) {
            disconnect(lineWidgets[i].chkEnable, SIGNAL(toggled(bool)), this, SLOT(lineEnabledToggled(bool)));
            //disconnect(lineWidgets[i].spinPower, SIGNAL(valueChanged(double)), this, SLOT(setPowerChanged(double)));
            disconnect(lineWidgets[i].spinPower, SIGNAL(editingFinished ()), this, SLOT(setPowerEditingFinished()));

            bool enabled=widgetsEnabled.value(i, false);

            bool en=lineenabled.value(i, false);
            if (en!=lineWidgets[i].chkEnable->isEnabled()) lineWidgets[i].chkEnable->setChecked(en);

            double setP=setValues.value(i, lineWidgets[i].spinPower->value());
            if (enabled && (lineWidgets[i].spinPower->value()!=setP) && !lineWidgets[i].spinPower->hasFocus()) lineWidgets[i].spinPower->setValue(setP);

            QString txt=QString("%1 %2").arg(measuredValues.value(i, 0.0)).arg(powerUnits.value(i, ""));
            if (lineWidgets[i].labPower->text()!=txt) lineWidgets[i].labPower->setText(txt);
            txt=lineNames.value(i, tr("line #%1").arg(i+1));
            if (lineWidgets[i].chkEnable->text()!=txt) lineWidgets[i].chkEnable->setText(txt);

            if (lineWidgets[i].spinPower->isEnabled()!=enabled) lineWidgets[i].spinPower->setEnabled(enabled);
            if (lineWidgets[i].chkEnable->isEnabled()!=enabled) lineWidgets[i].chkEnable->setEnabled(enabled);
            if (lineWidgets[i].labPower->isEnabled()!=enabled) lineWidgets[i].labPower->setEnabled(enabled);

            connect(lineWidgets[i].chkEnable, SIGNAL(toggled(bool)), this, SLOT(lineEnabledToggled(bool)));
            //connect(lineWidgets[i].spinPower, SIGNAL(valueChanged(double)), this, SLOT(setPowerChanged(double)));
            connect(lineWidgets[i].spinPower, SIGNAL(editingFinished ()), this, SLOT(setPowerEditingFinished()));
        }
    }
    updateStates();
    if (widVisible) setUpdatesEnabled(updt);
}

void QFLightSourceConfigWidget::setPowerEditingFinished() {
    QDoubleSpinBox* spin=qobject_cast<QDoubleSpinBox*>(sender());
    if (spin) {
        setPowerChanged(spin->value());
    }

}

QString QFLightSourceConfigWidget::getLightsoureConfigForFilename() const
{
    if (isLightSourceConnected()) {
        QString l="";
        for (int i=0; i<getLineCount(); i++) {
            if (isLineEnabled(i)) {
                if (!l.isEmpty()) l+="_";
                l+=getLineDescription(i)+"_"+doubleToQString(getSetPower(i))+getLineUnit(i);
            }
        }
        if (!l.isEmpty()) {
            return cleanStringForFilename(l,64,false,true);
        }
    }
    return "";

}


