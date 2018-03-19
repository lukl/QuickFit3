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

#ifndef PIMERCURY663CALIBRATIONDIALOG_H
#define PIMERCURY663CALIBRATIONDIALOG_H

#include "qfdialog.h"
#include <QProgressBar>
#include <QPushButton>
#include <QLabel>
#include <stdint.h>
#include <QCheckBox>
#include "qfserialconnection.h"
#include "qfextensionlinearstagepi663protocolhandler.h"

class QFExtensionLinearStagePI663; // forward


class PIMercury663CalibrationDialog : public QFDialog {
        Q_OBJECT
    public:
        /** Default constructor */
        PIMercury663CalibrationDialog(QWidget* parent, QFExtensionLinearStagePI663* stage, int axis);
        /** Default destructor */
        virtual ~PIMercury663CalibrationDialog();
    protected slots:
        void getJoystickValues();
        void acceptCal();
        void calibrate();
        void center();
        void resetCal();
    protected:
        QFExtensionLinearStagePI663* stage;
        QFSerialConnection* com;
        QFExtensionLinearStagePI663ProtocolHandler* serial;
        QProgressBar* prgX;
        QPushButton* btnOK;
        QPushButton* btnCancel;
        QPushButton* btnCentered;
        QPushButton* btnCalibrated;
        QPushButton* btnResetCal;
        QLabel* labX;
        QLabel* labMinX;
        QLabel* labMaxX;
        QCheckBox* chkInvert;

        int32_t calX[256];

        int32_t maxX;
        int32_t minX;
        int32_t zeroX;

        bool accepted;

        int axis;

    private:
};

#endif // PIMERCURY663CALIBRATIONDIALOG_H
