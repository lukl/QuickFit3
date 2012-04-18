#include "qflightsourceconfigwidget.h"

QFLightSourceConfigWidget::QFLightSourceConfigWidget(QWidget *parent) :
    QFrame(parent)
{
    setFrameStyle(QFrame::Panel|QFrame::Raised);
    setLineWidth(1);
    stateUpdateInterval=237;

    timUpdate=new QTimer(this);
    timUpdate->setSingleShot(true);
    timUpdate->setInterval(stateUpdateInterval);

    m_log=NULL;
    m_pluginServices=NULL;
    locked=false;
    moving=false;


    createWidgets();
    createActions();
    updateStates();


    connect(timUpdate, SIGNAL(timeout()), this, SLOT(displayStates()));
    timUpdate->start(stateUpdateInterval);
}

QFLightSourceConfigWidget::~QFLightSourceConfigWidget()
{
    locked=true;
    disconnect(timUpdate, SIGNAL(timeout()), this, SLOT(displayStates()));
    timUpdate->stop();
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

void QFLightSourceConfigWidget::saveSettings(QSettings &settings, QString prefix) {
    cmbLightSource->storeSettings(settings, prefix+"LightSource/");
    settings.setValue(prefix+"update_interval", stateUpdateInterval);
}

void QFLightSourceConfigWidget::connectLightSource() {
    actConnect->setChecked(true);
}

void QFLightSourceConfigWidget::disconnectLightSource() {
    actConnect->setChecked(false);
}


void QFLightSourceConfigWidget::lockLightSource() {
    locked=true;
    disconnect(timUpdate, SIGNAL(timeout()), this, SLOT(displayStates()));
    timUpdate->stop();
}

void QFLightSourceConfigWidget::unlockLightSource() {
    locked=false;
    connect(timUpdate, SIGNAL(timeout()), this, SLOT(displayStates()));
    timUpdate->setSingleShot(true);
    timUpdate->setInterval(stateUpdateInterval);
    timUpdate->start(stateUpdateInterval);
}

void QFLightSourceConfigWidget::createWidgets() {
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // create main layout
    ////////////////////////////////////////////////////////////////////////////////////////////////
    widgetLayout=new QGridLayout(this);
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

    linesLayoutWidget=new QWidget(this);
    linesLayout=new QGridLayout(this);
    linesLayoutWidget->setLayout(linesLayout);
    linesLayout->setContentsMargins(0,0,0,0);
    linesLayout->setHorizontalSpacing(2);
    linesLayout->setVerticalSpacing(2);
    widgetLayout->addWidget(linesLayoutWidget, 1,0,1,3);
    linesLayout->addWidget(new QLabel(tr("<small><b>line</b></small>")), 0, 0);
    linesLayout->addWidget(new QLabel(tr("<small><b>set power</b></small>")), 0, 1);
    linesLayout->addWidget(new QLabel(tr("<small><b>measured power</b></small>")), 0, 2);
    /*QWidget* w=new QWidget(this);
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    widgetLayout->addWidget(w,widgetLayout->rowCount()-1,widgetLayout->columnCount());
    widgetLayout->setColumnStretch(widgetLayout->columnCount()-1,1);*/

    stretch=new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Expanding);
    widgetLayout->addItem(stretch, widgetLayout->rowCount()-1,widgetLayout->columnCount());


    cmbLightSource->setEnabled(false);


}

void QFLightSourceConfigWidget::createActions() {
    actConnect=new QAction(QIcon(":/libqf3widgets/connect_lightsource.png"), tr("Connect to filter changer driver/hardware ..."), this);
    actConnect->setCheckable(true);
    connect(actConnect, SIGNAL(toggled(bool)), this, SLOT(disConnect()));
    btnConnect->setDefaultAction(actConnect);


    actConfigure=new QAction(QIcon(":/libqf3widgets/configure_lightsource.png"), tr("Configure filter changer ..."), this);
    connect(actConfigure, SIGNAL(triggered()), this, SLOT(configure()));
    btnConfigure->setDefaultAction(actConfigure);

}


QFExtensionLightSource *QFLightSourceConfigWidget::getLightSource() const {
    return cmbLightSource->currentExtensionLightSource();
}

QFExtension *QFLightSourceConfigWidget::getLightSourceExtension() const {
    return cmbLightSource->currentExtension();
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
    if (LightSource) {
        //qDebug()<<"set LightSource state opened="<<opened;
        return LightSource->isLastLightSourceActionFinished(LightSourceID);
    }
    return true;
}

void QFLightSourceConfigWidget::updateStates() {
    QFExtensionLightSource* LightSource;
    int LightSourceID;
    bool conn;


    LightSource=getLightSource();
    LightSourceID=getLightSourceID();
    conn=false;
    if (LightSource) {
        conn=LightSource->isLightSourceConnected(LightSourceID);
        if (conn) {
            actConnect->setChecked(true);
            actConnect->setIcon(QIcon(":/libqf3widgets/disconnect_lightsource.png"));
            actConnect->setText(tr("Disconnect from light source driver/hardware ..."));
        } else {
            actConnect->setChecked(false);
            actConnect->setIcon(QIcon(":/libqf3widgets/connect_lightsource.png"));
            actConnect->setText(tr("Connect from light source driver/hardware ..."));
        }
    }
    actConfigure->setEnabled(LightSource!=NULL && LightSourceID>=0);
    actConnect->setEnabled(LightSource!=NULL && LightSourceID>=0);
    cmbLightSource->setEnabled(!conn);
}

void QFLightSourceConfigWidget::setPowerChanged(double value) {
    QFExtensionLightSource* LightSource;
    int LightSourceID;
    LightSource=getLightSource();
    LightSourceID=getLightSourceID();
    if (LightSource) {
        LightSource->setLightSourcePower(LightSourceID, getLineByWidget(sender()), value);
    }

}

void QFLightSourceConfigWidget::lineEnabledToggled(bool enabled) {
    QFExtensionLightSource* LightSource;
    int LightSourceID;
    LightSource=getLightSource();
    LightSourceID=getLightSourceID();
    if (LightSource) {
        LightSource->setLightSourceLineEnabled(LightSourceID, getLineByWidget(sender()), enabled);
    }
}

void QFLightSourceConfigWidget::disConnect() {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool conn=actConnect->isChecked();
    QFExtensionLightSource* LightSource=getLightSource();
    int LightSourceID=getLightSourceID();

    if (LightSource) {
        //qDebug()<<"connecting "<<conn;
        if (conn) {
            LightSource->setLightSourceLogging(m_log);
            LightSource->lightSourceConnect(LightSourceID);
            if (LightSource->isLightSourceConnected(LightSourceID)) {
                m_log->log_text("connected to light source driver ...\n");
            } else {
                actConnect->setChecked(false);
                LightSource->lightSourceDisonnect(LightSourceID);
                m_log->log_error("error connecting to light source driver ...\n");
            }
        } else {
            LightSource->lightSourceConnect(LightSourceID);
            m_log->log_text("disconnected from light source driver ...\n");
        }
    } else {
        actConnect->setChecked(false);
    }
    updateLSLinesWidgets();
    updateStates();
    QApplication::restoreOverrideCursor();
}

void QFLightSourceConfigWidget::configure() {
    QFExtensionLightSource* LightSource=getLightSource();
    int LightSourceID=getLightSourceID();
    if (LightSource) LightSource->showLightSourceSettingsDialog(LightSourceID, this);
    updateLSLinesWidgets();
    updateStates();
}

void QFLightSourceConfigWidget::updateLSLinesWidgets() {
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
            linesLayout->addWidget(w.chkEnable, widgetLayout->rowCount(), 0);
            linesLayout->addWidget(w.spinPower, widgetLayout->rowCount()-1, 1);
            linesLayout->addWidget(w.labPower, widgetLayout->rowCount()-1, 2);
            lineWidgets.append(w);
            connect(w.chkEnable, SIGNAL(toggled(bool)), this, SLOT(lineEnabledToggled(bool)));
            connect(w.spinPower, SIGNAL(valueCHanged(double)), this, SLOT(setPowerChanged(double)));
        }
    }

    for (int i=0; i<newLines; i++) {
        lineWidgets[i].chkEnable->setText(lightSource->getLightSourceLineDescription(id, i));
        double min=0;
        double max=0;
        lightSource->getLightSourceLinePowerRange(id, i, min, max);
        disconnect(lineWidgets[i].chkEnable, SIGNAL(toggled(bool)), this, SLOT(lineEnabledToggled(bool)));
        disconnect(lineWidgets[i].spinPower, SIGNAL(valueCHanged(double)), this, SLOT(setPowerChanged(double)));
        lineWidgets[i].spinPower->setRange(min, max);
        lineWidgets[i].spinPower->setValue(lightSource->getLightSourceCurrentSetPower(id, i));
        lineWidgets[i].spinPower->setSuffix(QString(" ")+lightSource->getLightSourceLinePowerUnit(id, i));
        lineWidgets[i].spinPower->setEnabled(lightSource->isLastLightSourceActionFinished(id));
        lineWidgets[i].chkEnable->setChecked(lightSource->getLightSourceLineEnabled(id, i));
        lineWidgets[i].labPower->setText(QString("%1 %2").arg(lightSource->getLightSourceCurrentMeasuredPower(id, i)).arg(lightSource->getLightSourceLinePowerUnit(id, i)));
        connect(lineWidgets[i].chkEnable, SIGNAL(toggled(bool)), this, SLOT(lineEnabledToggled(bool)));
        connect(lineWidgets[i].spinPower, SIGNAL(valueCHanged(double)), this, SLOT(setPowerChanged(double)));
    }

    linesLayoutWidget->setVisible(newLines>0);

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
    if (locked) return;
    QFExtensionLightSource* LightSource;
    int LightSourceID;
    LightSource=getLightSource();
    LightSourceID=getLightSourceID();
    if (LightSource) {
        for (int i=0; i<lineWidgets.size(); i++) {
            disconnect(lineWidgets[i].chkEnable, SIGNAL(toggled(bool)), this, SLOT(lineEnabledToggled(bool)));
            disconnect(lineWidgets[i].spinPower, SIGNAL(valueCHanged(double)), this, SLOT(setPowerChanged(double)));
            lineWidgets[i].chkEnable->setChecked(LightSource->getLightSourceLineEnabled(LightSourceID, i));
            lineWidgets[i].labPower->setText(QString("%1 %2").arg(LightSource->getLightSourceCurrentMeasuredPower(LightSourceID, i)).arg(LightSource->getLightSourceLinePowerUnit(LightSourceID, i)));
            lineWidgets[i].spinPower->setEnabled(LightSource->isLastLightSourceActionFinished(LightSourceID));
            double setP=LightSource->getLightSourceCurrentSetPower(LightSourceID, i);
            if (lineWidgets[i].spinPower->value()!=setP) lineWidgets[i].spinPower->setValue(setP);
            connect(lineWidgets[i].chkEnable, SIGNAL(toggled(bool)), this, SLOT(lineEnabledToggled(bool)));
            connect(lineWidgets[i].spinPower, SIGNAL(valueCHanged(double)), this, SLOT(setPowerChanged(double)));
        }
    }

    updateStates();

    //QTimer::singleShot(stateUpdateInterval, this, SLOT(displayLightSourceStates()));
    if (!locked) {
        timUpdate->setSingleShot(true);
        timUpdate->setInterval(stateUpdateInterval);
        timUpdate->start(stateUpdateInterval);

    }
}

void QFLightSourceConfigWidget::setReadOnly(bool readonly) {
    cmbLightSource->setReadOnly(readonly);
}
