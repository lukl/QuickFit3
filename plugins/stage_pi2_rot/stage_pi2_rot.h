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

#ifndef STAGE_PI2_ROT_H
#define STAGE_PI2_ROT_H

#include <QObject>
#include <time.h>
#include <QVector>
#include "qfextension.h"
#include "qfextensionlinearstage.h"
#include "qfserialconnection.h"
#include "qf3comportmanager.h"
#include "qfextensionlinearstagepi2rotprothandler.h"
#include "pimercury863calibrationdialog.h"
#include <unistd.h>

/*!
    \defgroup qf3ext_StagePIRot QFExtensionLinearStage implementation for PI mercury stages with Rotational Stage
    \ingroup qf3extensionplugins
*/

/*! \brief QFExtensionLinearStage implementation for PI mercury stages with Rotational Stage PI RS-40
    \ingroup qf3ext_StagePI2rot
    The Stage is basically handled the same way as a linear stage would be, since otherwise the existing class may not be used.
 */
class QFExtensionLinearStagePI2Rot : public QObject, public QFExtensionBase, public QFExtensionLinearStage {
        Q_OBJECT
        Q_INTERFACES(QFExtension QFExtensionLinearStage)
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        Q_PLUGIN_METADATA(IID "www.dkfz.de.QuickFit3.Plugins.QFExtensionLinearStagePI2Rot")
#endif

    public:
        /** Default constructor */
        QFExtensionLinearStagePI2Rot(QObject* parent=NULL);
        /** Default destructor */
        virtual ~QFExtensionLinearStagePI2Rot();


    /////////////////////////////////////////////////////////////////////////////
    // QFExtension routines
    /////////////////////////////////////////////////////////////////////////////
        /** \copydoc QFExtension::getID() */
        virtual QString getID() const  { return QString("stage_pi863_2_rot"); };
        /** \copydoc QFExtension::getName() */
        virtual QString getName() const  { return tr("Rot.: PI Merc. 863 v2 Rot"); };
        /** \copydoc QFExtension::getDescription() */
        virtual QString getDescription() const  { return tr("Rotational Stage: PI Mercury 863 v2 Rot"); };
        /** \copydoc QFExtension::getAuthor() */
        virtual QString getAuthor() const  { return tr("Jan Krieger, Lukas Lau"); };
        /** \copydoc QFExtension::getCopyright() */
        virtual QString getCopyright() const  { return tr("(c) 2013 by Jan Krieger, 2016 Lukas Lau"); };
        /** \copydoc QFExtension::getWeblink() */
        virtual QString getWeblink() const  { return tr(""); };
        /** \copydoc QFExtension::getIconFilename() */
        virtual QString getIconFilename() const  { return QString(":/stage_pi2_logo.png"); };
        /** \copydoc QFExtension::deinit() */
        virtual void deinit();
        /** \brief plugin version  */
        virtual void getVersion(int& major, int& minor) const {
            major=1;
            minor=0;
        };

    protected:
        /** \copydoc QFExtensionBase::projectChanged() */
        virtual void projectChanged(QFProject* oldProject, QFProject* project);
        /** \copydoc QFExtensionBase::initExtension() */
        virtual void initExtension();
        /** \copydoc QFExtensionBase::loadSettings() */
        virtual void loadSettings(ProgramOptions* settings);
        /** \copydoc QFExtensionBase::storeSettings() */
        virtual void storeSettings(ProgramOptions* settings);


    /////////////////////////////////////////////////////////////////////////////
    // QFExtensionLinearStage routines
    /////////////////////////////////////////////////////////////////////////////
    public:
        /** \copydoc QFExtensionLinearStage::) */
		virtual unsigned int getAxisCount();


        /** \copydoc QFExtensionLinearStage::showSettingsDialog() */
        virtual void showSettingsDialog(unsigned int axis, QWidget* parent=NULL);
        /** \copydoc QFExtensionLinearStage::isConnected() */
        virtual bool isConnected(unsigned int axis);
        /** \copydoc QFExtensionLinearStage::connectDevice() */
        virtual void connectDevice(unsigned int axis);
        /** \copydoc QFExtensionLinearStage::disconnectDevice() */
        virtual void disconnectDevice(unsigned int axis);
        /** \copydoc QFExtensionLinearStage::setLogging() */
        virtual void setLogging(QFPluginLogService* logService);

        /** \copydoc QFExtensionLinearStage::setJoystickActive() */
        virtual void setJoystickActive(unsigned int axis, bool enabled, double maxVelocity=100000);
        /** \copydoc QFExtensionLinearStage::isJoystickActive() */
        virtual bool isJoystickActive(unsigned int axis);

        /** \copydoc QFExtensionLinearStage::setRefMoveActive() */
        virtual void setRefMoveActive(unsigned int axis, bool enabled);

        /** \copydoc QFExtensionLinearStage::getAxisState() */
        virtual AxisState getAxisState(unsigned int axis);

        /** \copydoc QFExtensionLinearStage::stop() */
        virtual void stop(unsigned int axis);

        /** \copydoc QFExtensionLinearStage::getPosition() */
        virtual double getPosition(unsigned int axis);
        /** \copydoc QFExtensionLinearStage::getSpeed() */
        virtual double getSpeed(unsigned int axis);

        /** \copydoc QFExtensionLinearStage::move() */
        virtual void move(unsigned int axis, double newPosition);


        /** \copydoc QFExtensionLinearStage::getStageName() */
        virtual QString getStageName(unsigned int axis) const;

        /** \copydoc QFExtensionLinearStage::getStageInfo() */
        virtual StageInfo getStageInfo(unsigned int axis) const;


        /** \brief log project text message
         *  \param message the message to log
         */
        virtual void log_text(QString message);
        /** \brief log project warning message
         *  \param message the warning message to log
         */
        virtual void log_warning(QString message);
        /** \brief log project error message
         *  \param message the error message to log
         */
        virtual void log_error(QString message);


    protected slots:
        void calibrateJoysticks();

	protected:
        QFPluginLogService* logService;
        QAction* actCalibrateJoysticks;

        struct AxisDescription {
            AxisDescription() {
                PTerm=1000;
                iTerm=500;
                DTerm=2000;
                iLimit=2000;
                acceleration=100;
                maxVelocity=3.99999;
                initVelocity=3;
                lengthFactor=1; //2.57e-5;
                velocityFactor=1; //2.57e-5;
                accelerationFactor=1; //2.57e-5;
                maxCoord=0;
                minCoord=0;
                backlashCorr=0.1;
                ms=0.1;
                doRefMove=false;
            }

            /** \brief ID of the Mercury C-863 controller for the axis
             *
             * This contains the address character (!!!) of the controller in a daisy chain. The character is either
             * \c '0' ... \c '9' or \c 'A' ... \c 'F'
             */
             QChar ID;

             int port;
             QFExtensionLinearStagePI2RotProtHandler* serial;

             QFExtensionLinearStage::AxisState state;
             /** \brief indicates whether the joystick is enabled or not */
             bool joystickEnabled;


             QString name;
             QString label;
             /** \brief settings of P-Term */
             unsigned int PTerm;
             /** \brief settings of i-Term */
             unsigned int iTerm;
             /** \brief settings of D-Term */
             unsigned int DTerm;
             /** \brief settings of i-Limit */
             unsigned int iLimit;
             /** \brief settings of acceleration */
             double acceleration;
             /** \brief the initial velocity of all axes */
             double initVelocity;
             /** \brief settings of max. velocity */
             double maxVelocity;
             /** \brief movement limitations for each axis */
             double maxCoord;
             double minCoord;
             /** \brief backlash correction for each axis in deg */
             double backlashCorr;
             /** \brief do Reference Movement on startup */
             bool doRefMove;

             /** \brief this factor is used to get the control electronics position from the position in degrees, given in degrees/unit */
             double lengthFactor;

             /** \brief this factor is used to get the control electronics velocity from the velocity in degrees/sec, given in (degrees/sec)/unit */
             double velocityFactor;

             /** \brief this factor is used to get the control electronics acceleration from the acceleration in degrees/sec^2, given in (degrees/sec^2)/unit */
             double accelerationFactor;

             /** \brief refresh rate for checking if stage is still moving */
             double ms;

        };

        QF3ComPortManager ports;

        QVector<AxisDescription> axes;

        friend class PIMercury863CalibrationDialog;
};

#endif // STAGE_PI2_ROT_H
