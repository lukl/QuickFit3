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

#include "qfautooutputtimer.h"
#include <QDebug>
#include <QtGlobal>

QFAutoOutputTimer::QFAutoOutputTimer(const QString& message) :
    QElapsedTimer()
{
    this->message=message;
    start();
}

QFAutoOutputTimer::~QFAutoOutputTimer()
{
    #if QT_VERSION >= 0x040800
        qDebug()<<"TIMERMESSAGE: "<<message<<"    DUR: "<<double(nsecsElapsed())/1.0e6<<"ms";
    #else
        qDebug()<<"TIMERMESSAGE: "<<message<<"    DUR: "<<double(elapsed())/1.0e3<<"ms";
    #endif
}
