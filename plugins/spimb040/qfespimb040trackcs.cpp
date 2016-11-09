/*
    Copyright (c) 2016 Lukas Lau (<l.lau@dkfz.de>),
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

#include "qfespimb040trackcs.h"

#include<QtGlobal>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#else
#include <QtGui>
#endif


#include <iostream>




QFESPIMB040TrackCS::QFESPIMB040TrackCS(QWidget* parent):
    QWidget(parent)
{
    setWindowTitle(tr(" Track Cover Slip Movement: "));
    createWidgets();
    //setActive();

}


void QFESPIMB040TrackCS::createWidgets() {

////////////////////////////////////////////////////////////////////////////////////////////////
// create main layout
////////////////////////////////////////////////////////////////////////////////////////////////
QFormLayout* stagelayout=new QFormLayout();
stagelayout->setHorizontalSpacing(2);
setLayout(stagelayout);


QLabel* l;

////////////////////////////////////////////////////////////////////////////////////////////////
// create input widgets for camera devices
////////////////////////////////////////////////////////////////////////////////////////////////
l=new QLabel (tr("<b>Track Cover Slip:</b>"), this);
l->setStyleSheet("background-color: 	aliceblue;");
stagelayout->addRow(l);

btnConfigSteps=new QToolButton(this);
stagelayout->addRow(btnConfigSteps);

}


void QFESPIMB040TrackCS::setWidgetsInActive() {

    bool xyzstagesconnected=0;
    //stageSetup->



}



QFESPIMB040TrackCS::~QFESPIMB040TrackCS()
{
}



