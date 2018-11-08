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

#include "qfespimb040acquisitiondescription.h"
#include "ui_qfespimb040acquisitiondescription.h"
#include <QTimer>
#include <QDir>
#include "programoptions.h"
#include "qfcompleterfromfile.h"
#include "qfstyledbutton.h"

QFESPIMB040AcquisitionDescription::QFESPIMB040AcquisitionDescription(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QFESPIMB040AcquisitionDescription)
{
    ui->setupUi(this);
    this->expDescription=NULL;
    this->opticsSetup=NULL;

    QDir().mkpath(ProgramOptions::getInstance()->getConfigFileDirectory()+"/plugins/ext_spimb040/completers/");
    QFCompleterFromFile* c1=new QFCompleterFromFile(this);
    c1->setFilename(ProgramOptions::getInstance()->getConfigFileDirectory()+"/plugins/ext_spimb040/completers/acqdescription_prefix.txt");
    QFCompleterFromFile* c2=new QFCompleterFromFile(this);
    c2->setFilename(ProgramOptions::getInstance()->getConfigFileDirectory()+"/plugins/ext_spimb040/completers/acqdescription_comment.txt");
    QFCompleterFromFile* c3=new QFCompleterFromFile(this);
    c3->setFilename(ProgramOptions::getInstance()->getConfigFileDirectory()+"/plugins/ext_spimb040/completers/acqdescription_currentsample.txt");
    ui->edtPrefix->setCompleter(c1);
    ui->edtComment->setCompleter(c2);
    ui->edtCurrentSampleName->setCompleter(c3);
    ui->edtPrefix->addButton(new QFStyledButton(QFStyledButton::SelectFromCompleter, ui->edtPrefix, ui->edtPrefix));
    ui->edtComment->addButton(new QFStyledButton(QFStyledButton::SelectFromCompleter, ui->edtComment, ui->edtComment));
    ui->edtCurrentSampleName->addButton(new QFStyledButton(QFStyledButton::SelectFromCompleter, ui->edtCurrentSampleName, ui->edtCurrentSampleName));

    updateTime();

}

QFESPIMB040AcquisitionDescription::~QFESPIMB040AcquisitionDescription()
{
    delete ui;
}

void QFESPIMB040AcquisitionDescription::updateReplaces()
{
    setGlobalReplaces(opticsSetup, expDescription, this);
}



void QFESPIMB040AcquisitionDescription::loadSettings(QSettings& settings, QString prefix) {
    ui->edtPrefix->setText(settings.value(prefix+"prefix", QDir::homePath()+"spim_").toString());
    ui->edtComment->setText(settings.value(prefix+"comment", "").toString());
    ui->edtCurrentSampleName->setText(settings.value(prefix+"sample", "").toString());
    ui->spinCell->setValue(settings.value(prefix+"cell", 1).toInt());
    ui->spinPlate->setValue(settings.value(prefix+"plate", 1).toInt());
    ui->spinWell->setValue(settings.value(prefix+"well", 1).toInt());
}

void QFESPIMB040AcquisitionDescription::storeSettings(QSettings& settings, QString prefix) const {
    settings.setValue(prefix+"prefix", ui->edtPrefix->text());
    settings.setValue(prefix+"comment", ui->edtComment->text());
    settings.setValue(prefix+"sample", ui->edtCurrentSampleName->text());
    settings.setValue(prefix+"cell", ui->spinCell->value());
    settings.setValue(prefix+"plate", ui->spinPlate->value());
    settings.setValue(prefix+"well", ui->spinWell->value());

}

QMap<QString, QVariant> QFESPIMB040AcquisitionDescription::getDescription() const {
    QMap<QString, QVariant> description;
    description["prefix"]=ui->edtPrefix->text();
    description["comment"]=ui->edtComment->text();
    description["samplename"]=ui->edtCurrentSampleName->text();
    description["cell"]=ui->spinCell->value();
    description["plate"]=ui->spinPlate->value();
    description["well"]=ui->spinWell->value();
    description["date"]=ui->edtDateTime->dateTime();
    return description;
}

int QFESPIMB040AcquisitionDescription::getCell() const
{
    return ui->spinCell->value();
}

int QFESPIMB040AcquisitionDescription::getWell() const
{
    return ui->spinWell->value();
}

int QFESPIMB040AcquisitionDescription::getPlate() const
{
    return ui->spinPlate->value();
}

QString QFESPIMB040AcquisitionDescription::getComment() const
{
    return ui->edtComment->text();
}

QString QFESPIMB040AcquisitionDescription::getSample() const
{
    return ui->edtCurrentSampleName->text();
}

QString QFESPIMB040AcquisitionDescription::getPrefix() const
{
    return ui->edtPrefix->text();
}

void QFESPIMB040AcquisitionDescription::setCurrentSample(const QString &name)
{
    ui->edtCurrentSampleName->setText(name);
}

void QFESPIMB040AcquisitionDescription::setPrefix(const QString &name)
{
    ui->edtPrefix->setText(name);
}

void QFESPIMB040AcquisitionDescription::setPlate(int p)
{
    ui->spinPlate->setValue(p);
}

void QFESPIMB040AcquisitionDescription::setWell(int p)
{
    ui->spinWell->setValue(p);
}

void QFESPIMB040AcquisitionDescription::setCell(int p)
{
    ui->spinCell->setValue(p);
}

void QFESPIMB040AcquisitionDescription::setComment(const QString &name)
{
    ui->edtComment->setText(name);
}

void QFESPIMB040AcquisitionDescription::setOtherSettingWidgets(QFESPIMB040OpticsSetupBase *setup, QFESPIMB040ExperimentDescription *exp)
{
    this->opticsSetup=setup;
    this->expDescription=exp;
    updateReplaces();
    bindLineEdit(ui->edtPrefix);

}

void QFESPIMB040AcquisitionDescription::nextCell()
{
    ui->edtComment->setText("");
    ui->spinCell->setValue(ui->spinCell->value()+1);
}

void QFESPIMB040AcquisitionDescription::firstCell()
{
    ui->edtComment->setText("");
    ui->spinCell->setValue(1);
}

void QFESPIMB040AcquisitionDescription::on_btnClearAll_clicked() {
    ui->edtComment->setText("");
    ui->spinCell->setValue(1);
    ui->spinPlate->setValue(1);
    ui->spinWell->setValue(1);
}

void QFESPIMB040AcquisitionDescription::on_btnNextCell_clicked()
{
    nextCell();
}

void QFESPIMB040AcquisitionDescription::on_btnFirstCell_clicked()
{
    firstCell();
}

void QFESPIMB040AcquisitionDescription::updateTime() {
    ui->edtDateTime->setDateTime(QDateTime::currentDateTime());
    QTimer::singleShot(1013, this, SLOT(updateTime()));
}

void QFESPIMB040AcquisitionDescription::spinChanged() {
    ui->edtComment->setText("");
}
