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

#include "qfespimb040samplestagestepconfigdialog.h"
#include "ui_qfespimb040samplestagestepconfigdialog.h"

QFESPIMB040SampleStageStepConfigDialog::QFESPIMB040SampleStageStepConfigDialog(double x, double y, double z, double R, QWidget *parent) :
    QFDialog(parent),
    ui(new Ui::QFESPIMB040SampleStageStepConfigDialog)
{
    ui->setupUi(this);
    ui->spinX->setValue(x);
    ui->spinY->setValue(y);
    ui->spinZ->setValue(z);
    ui->spinR->setValue(R);
}

QFESPIMB040SampleStageStepConfigDialog::~QFESPIMB040SampleStageStepConfigDialog()
{
    delete ui;
}

double QFESPIMB040SampleStageStepConfigDialog::x() const {
    return ui->spinX->value();
}

double QFESPIMB040SampleStageStepConfigDialog::y() const {
    return ui->spinY->value();
}

double QFESPIMB040SampleStageStepConfigDialog::z() const {
    return ui->spinZ->value();
}

double QFESPIMB040SampleStageStepConfigDialog::R() const {
    return ui->spinR->value();
}
