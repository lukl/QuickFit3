#include "ui_qfrdrtableaxissettingswidget.h"
#include "qfrdrtableaxissettingswidget.h"
#include "programoptions.h"
#include "qfrdrtable.h"
#include "qftools.h"
#include "qfdoubleedit.h"
#include "qfmathparserxfunctionlinegraph.h"
#include "qffitfunctionplottools.h"
#include "qffitfunctionmanager.h"
#include "qffitfunction.h"
#include "dlgcolorbarcoloring.h"


QFRDRTableAxisSettingsWidget::QFRDRTableAxisSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QFRDRTableAxisSettingsWidget)
{



    updating=true;
    ui->setupUi(this);

    //ui->scrollArea->setWidget();


    //ui->formLayout_3->removeWidget(ui->widSaveCoordSettings);
    //ui->tabWidget->setCornerWidget(ui->widSaveCoordSettings);


    ui->edtXMin->setCheckBounds(false, true);
    ui->edtXMax->setCheckBounds(true, false);
    connect(ui->edtXMin, SIGNAL(valueChanged(double)), ui->edtXMax, SLOT(setMinimum(double)));
    connect(ui->edtXMax, SIGNAL(valueChanged(double)), ui->edtXMin, SLOT(setMaximum(double)));

    updating=false;


    if (ProgramOptions::getInstance() && ProgramOptions::getInstance()->getQSettings()) readSettings(*(ProgramOptions::getInstance()->getQSettings()), "table/QFRDRTableAxisSettingsWidget/");
}

QFRDRTableAxisSettingsWidget::~QFRDRTableAxisSettingsWidget()
{
    updating=true;
    if (ProgramOptions::getInstance() && ProgramOptions::getInstance()->getQSettings()) writeSettings(*(ProgramOptions::getInstance()->getQSettings()), "table/QFRDRTableAxisSettingsWidget/");
    delete ui;
}

void QFRDRTableAxisSettingsWidget::loadPlotData(const QFRDRTable::AxisInfo& g)
{
    updating=true;

    setEnabled(true);

    ui->edtXLabel->setText(g.label);
    ui->chkLogX->setChecked(g.log);
    ui->chkX0Axis->setChecked(g.axis0);
    ui->edtXMin->setValue(g.min);
    ui->edtXMax->setValue(g.max);

    ui->spinXAxisDigits->setValue(g.digits);
    ui->spinXAxisMinTicks->setValue(g.minTicks);
    ui->spinXAxisMinorTicks->setValue(g.minorTicks);
    ui->cmbXLabelPosition->setPosition(g.labelPos);
    ui->cmbXLabel->setLabelType(g.labelType);
    ui->cmbXMode1->setDrawMode(g.labelMode1);
    ui->cmbXMode2->setDrawMode(g.labelMode2);


    ui->edtXTickSpacing->setValue(g.TickSpacing);
    ui->spinXILength->setValue(g.TickInsideLength);
    ui->spinXOLength->setValue(g.TickOutsideLength);
    ui->spinXMinILength->setValue(g.MinorTickInsideLength);
    ui->spinXMinOLength->setValue(g.MinorTickOutsideLength);

    ui->spinXTickWidth->setValue(g.AxisTickWidth);
    ui->spinXTickMinWidth->setValue(g.AxisMinorTickWidth);

    ui->chkXInverted->setChecked(g.AxisInverted);
    ui->chkXAutoTicks->setChecked(g.AutoTicks);

    updating=false;
    connectWidgets();
    doUpdateGraph();
}

void QFRDRTableAxisSettingsWidget::storePlotData(QFRDRTable::AxisInfo &g)
{
    g.label=ui->edtXLabel->text();
    g.log=ui->chkLogX->isChecked();
    g.axis0=ui->chkX0Axis->isChecked();
    g.digits=ui->spinXAxisDigits->value();
    g.minTicks=ui->spinXAxisMinTicks->value();
    g.minorTicks=ui->spinXAxisMinorTicks->value();
    g.labelPos=ui->cmbXLabelPosition->getPosition();
    g.labelType=ui->cmbXLabel->getLabelType();
    g.labelMode1=ui->cmbXMode1->getDrawMode();
    g.labelMode2=ui->cmbXMode2->getDrawMode();
    g.min=ui->edtXMin->value();
    g.max=ui->edtXMax->value();

    g.TickSpacing=ui->edtXTickSpacing->value();
    g.TickInsideLength=ui->spinXILength->value();
    g.TickOutsideLength=ui->spinXOLength->value();
    g.MinorTickInsideLength=ui->spinXMinILength->value();
    g.MinorTickOutsideLength=ui->spinXMinOLength->value();
    g.AxisTickWidth=ui->spinXTickWidth->value();
    g.AxisMinorTickWidth=ui->spinXTickMinWidth->value();
    g.AxisInverted=ui->chkXInverted->isChecked();
    g.AutoTicks=ui->chkXAutoTicks->isChecked();

}

bool QFRDRTableAxisSettingsWidget::getLogX() const
{
    return ui->chkLogX->isChecked();
}


void QFRDRTableAxisSettingsWidget::readSettings(QSettings &settings, const QString &prefix)
{
}

void QFRDRTableAxisSettingsWidget::writeSettings(QSettings &settings, const QString &prefix)
{
}

void QFRDRTableAxisSettingsWidget::saveToSettings(QSettings &set, const QString &axis) const
{

    set.setValue(axis+"label",ui->edtXLabel->text());
    set.setValue(axis+"log",ui->chkLogX->isChecked());
    set.setValue(axis+"axis0",ui->chkX0Axis->isChecked());
    set.setValue(axis+"digits", ui->spinXAxisDigits->value());
    set.setValue(axis+"minTicks", ui->spinXAxisMinTicks->value());
    set.setValue(axis+"minorTicks", ui->spinXAxisMinorTicks->value());
    set.setValue(axis+"labelPos", JKQTPlabelPosition2String(ui->cmbXLabelPosition->getPosition()));
    set.setValue(axis+"labelType", JKQTPCAlabelType2String(ui->cmbXLabel->getLabelType()));
    set.setValue(axis+"labelMode1", JKQTPCAdrawMode2String(ui->cmbXMode1->getDrawMode()));
    set.setValue(axis+"labelMode2", JKQTPCAdrawMode2String(ui->cmbXMode2->getDrawMode()));


    set.setValue(axis+"inverted",ui->chkXInverted->isChecked());
    set.setValue(axis+"autoticks",ui->chkXAutoTicks->isChecked());

    set.setValue(axis+"tickspacing", ui->edtXTickSpacing->value());
    set.setValue(axis+"tickilength", ui->spinXILength->value());
    set.setValue(axis+"tickolength", ui->spinXOLength->value());
    set.setValue(axis+"minortickilength", ui->spinXMinILength->value());
    set.setValue(axis+"minortickolength", ui->spinXMinOLength->value());
    set.setValue(axis+"tickwidth", ui->spinXTickWidth->value());
    set.setValue(axis+"minortickwidth", ui->spinXTickMinWidth->value());
}

void QFRDRTableAxisSettingsWidget::loadFromSettings(QSettings &set, const QString &axis)
{
    updating=true;

    setEnabled(true);

    ui->edtXLabel->setText(set.value(axis+"label",ui->edtXLabel->text()).toString());
    ui->chkLogX->setChecked(set.value(axis+"log",ui->chkLogX->isChecked()).toBool());
    ui->chkX0Axis->setChecked(set.value(axis+"axis0",ui->chkX0Axis->isChecked()).toBool());
    ui->spinXAxisDigits->setValue(set.value(axis+"digits", ui->spinXAxisDigits->value()).toInt());
    ui->spinXAxisMinTicks->setValue(set.value(axis+"minTicks", ui->spinXAxisMinTicks->value()).toInt());
    ui->spinXAxisMinorTicks->setValue(set.value(axis+"minorTicks", ui->spinXAxisMinorTicks->value()).toInt());
    ui->cmbXLabelPosition->setPosition(String2JKQTPlabelPosition(set.value(axis+"labelPos", JKQTPlabelPosition2String(ui->cmbXLabelPosition->getPosition())).toString()));
    ui->cmbXLabel->setLabelType(String2JKQTPCAlabelType(set.value(axis+"labelType", JKQTPCAlabelType2String(ui->cmbXLabel->getLabelType())).toString()));
    ui->cmbXMode1->setDrawMode(String2JKQTPCAdrawMode(set.value(axis+"labelMode1", JKQTPCAdrawMode2String(ui->cmbXMode1->getDrawMode())).toString()));
    ui->cmbXMode2->setDrawMode(String2JKQTPCAdrawMode(set.value(axis+"labelMode2", JKQTPCAdrawMode2String(ui->cmbXMode2->getDrawMode())).toString()));

    ui->edtXTickSpacing->setValue(set.value(axis+"tickspacing", ui->edtXTickSpacing->value()).toDouble());
    ui->spinXILength->setValue(set.value(axis+"tickilength", ui->spinXILength->value()).toDouble());
    ui->spinXOLength->setValue(set.value(axis+"tickolength", ui->spinXOLength->value()).toDouble());
    ui->spinXMinILength->setValue(set.value(axis+"minortickilength", ui->spinXMinILength->value()).toDouble());
    ui->spinXMinOLength->setValue(set.value(axis+"minortickolength", ui->spinXMinOLength->value()).toDouble());
    ui->spinXTickWidth->setValue(set.value(axis+"tickwidth", ui->spinXTickWidth->value()).toDouble());
    ui->spinXTickMinWidth->setValue(set.value(axis+"minortickwidth", ui->spinXTickMinWidth->value()).toDouble());

    ui->chkXInverted->setChecked(set.value(axis+"inverted", ui->chkXInverted->isChecked()).toBool());
    ui->chkXAutoTicks->setChecked(set.value(axis+"autoticks", ui->chkXAutoTicks->isChecked()).toBool());

    updating=false;
    connectWidgets();
    doUpdateGraph();
}

QString QFRDRTableAxisSettingsWidget::getlabel() const
{
    return ui->edtXLabel->text();
}



void QFRDRTableAxisSettingsWidget::setXRange(double xmin, double xmax)
{
    updating=true;
    disconnectWidgets();
    ui->edtXMin->setValue(xmin);
    ui->edtXMax->setValue(xmax);
    updating=false;
    connectWidgets();
    plotDataChanged();
}

void QFRDRTableAxisSettingsWidget::setlabel(const QString &label)
{
    ui->edtXLabel->setText(label);
}



void QFRDRTableAxisSettingsWidget::on_btnAutoscaleX_clicked()
{
    emit autoscaleX();
}


void QFRDRTableAxisSettingsWidget::plotDataChanged() {
   if (!updating) emit dataChanged();
}

void QFRDRTableAxisSettingsWidget::doUpdateGraph() {
    if (!updating) emit updateGraph();
}


void QFRDRTableAxisSettingsWidget::connectWidgets()
{
    //qDebug()<<"connectWidgets";

    connect(ui->edtXLabel, SIGNAL(editingFinished()), this, SLOT(plotDataChanged()));
    connect(ui->chkLogX, SIGNAL(toggled(bool)), this, SLOT(plotDataChanged()));
    connect(ui->chkX0Axis, SIGNAL(toggled(bool)), this, SLOT(plotDataChanged()));
    connect(ui->edtXMin, SIGNAL(valueChanged(double)), this, SLOT(plotDataChanged()));
    connect(ui->edtXMax, SIGNAL(valueChanged(double)), this, SLOT(plotDataChanged()));

    connect(ui->spinXAxisDigits, SIGNAL(valueChanged(int)), this, SLOT(plotDataChanged()));
    connect(ui->spinXAxisMinTicks, SIGNAL(valueChanged(int)), this, SLOT(plotDataChanged()));
    connect(ui->spinXAxisMinorTicks, SIGNAL(valueChanged(int)), this, SLOT(plotDataChanged()));
    connect(ui->cmbXLabelPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(plotDataChanged()));
    connect(ui->cmbXLabel, SIGNAL(currentIndexChanged(int)), this, SLOT(plotDataChanged()));
    connect(ui->cmbXMode1, SIGNAL(currentIndexChanged(int)), this, SLOT(plotDataChanged()));
    connect(ui->cmbXMode2, SIGNAL(currentIndexChanged(int)), this, SLOT(plotDataChanged()));


    connect(ui->chkXAutoTicks, SIGNAL(toggled(bool)), this, SLOT(plotDataChanged()));
    connect(ui->chkXInverted, SIGNAL(toggled(bool)), this, SLOT(plotDataChanged()));
    connect(ui->edtXTickSpacing, SIGNAL(valueChanged(double)), this, SLOT(plotDataChanged()));
    connect(ui->spinXILength, SIGNAL(valueChanged(double)), this, SLOT(plotDataChanged()));
    connect(ui->spinXOLength, SIGNAL(valueChanged(double)), this, SLOT(plotDataChanged()));
    connect(ui->spinXMinILength, SIGNAL(valueChanged(double)), this, SLOT(plotDataChanged()));
    connect(ui->spinXMinOLength, SIGNAL(valueChanged(double)), this, SLOT(plotDataChanged()));
    connect(ui->spinXTickWidth, SIGNAL(valueChanged(double)), this, SLOT(plotDataChanged()));
    connect(ui->spinXTickMinWidth, SIGNAL(valueChanged(double)), this, SLOT(plotDataChanged()));


}

void QFRDRTableAxisSettingsWidget::disconnectWidgets()
{
    disconnect(ui->edtXLabel, SIGNAL(editingFinished()), this, SLOT(plotDataChanged()));
    disconnect(ui->chkLogX, SIGNAL(toggled(bool)), this, SLOT(plotDataChanged()));
    disconnect(ui->chkX0Axis, SIGNAL(toggled(bool)), this, SLOT(plotDataChanged()));
    disconnect(ui->edtXMin, SIGNAL(valueChanged(double)), this, SLOT(plotDataChanged()));
    disconnect(ui->edtXMax, SIGNAL(valueChanged(double)), this, SLOT(plotDataChanged()));
    disconnect(ui->spinXAxisDigits, SIGNAL(valueChanged(int)), this, SLOT(plotDataChanged()));
    disconnect(ui->spinXAxisMinTicks, SIGNAL(valueChanged(int)), this, SLOT(plotDataChanged()));
    disconnect(ui->spinXAxisMinorTicks, SIGNAL(valueChanged(int)), this, SLOT(plotDataChanged()));
    disconnect(ui->cmbXLabelPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(plotDataChanged()));
    disconnect(ui->cmbXLabel, SIGNAL(currentIndexChanged(int)), this, SLOT(plotDataChanged()));
    disconnect(ui->cmbXMode1, SIGNAL(currentIndexChanged(int)), this, SLOT(plotDataChanged()));
    disconnect(ui->cmbXMode2, SIGNAL(currentIndexChanged(int)), this, SLOT(plotDataChanged()));

    disconnect(ui->chkXAutoTicks, SIGNAL(toggled(bool)), this, SLOT(plotDataChanged()));
    disconnect(ui->chkXInverted, SIGNAL(toggled(bool)), this, SLOT(plotDataChanged()));
    disconnect(ui->edtXTickSpacing, SIGNAL(valueChanged(double)), this, SLOT(plotDataChanged()));
    disconnect(ui->spinXILength, SIGNAL(valueChanged(double)), this, SLOT(plotDataChanged()));
    disconnect(ui->spinXOLength, SIGNAL(valueChanged(double)), this, SLOT(plotDataChanged()));
    disconnect(ui->spinXMinILength, SIGNAL(valueChanged(double)), this, SLOT(plotDataChanged()));
    disconnect(ui->spinXMinOLength, SIGNAL(valueChanged(double)), this, SLOT(plotDataChanged()));
    disconnect(ui->spinXTickWidth, SIGNAL(valueChanged(double)), this, SLOT(plotDataChanged()));
    disconnect(ui->spinXTickMinWidth, SIGNAL(valueChanged(double)), this, SLOT(plotDataChanged()));

}
