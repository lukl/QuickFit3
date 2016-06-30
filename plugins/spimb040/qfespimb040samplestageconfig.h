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

#ifndef QFESPIMB040SAMPLESTAGECONFIG_H
#define QFESPIMB040SAMPLESTAGECONFIG_H

#include <QGroupBox>

class QFESPIMB040MainWindow; // forward

#include <QWidget>
#include "qvisiblehandlesplitter.h"
#include <QSplitter>
#include <QComboBox>
#include <QSettings>
#include <QMenu>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <cmath>
#include <QLabel>
#include <QPushButton>
#include <QToolBar>
#include <QTime>
#include <QTabWidget>
#include <QGroupBox>
#include <QTimer>
#include <QThread>
#include <QQueue>
#include <QMutex>

#include "programoptions.h"
#include "jkqttools.h"
#include "qtlogfile.h"
#include "qfextensionmanager.h"
#include "qfextensionlinearstage.h"
#include "qfextension.h"
#include "cpptools.h"
#include "qfstagecombobox.h"
#include "qfmanyfilessettings.h"

class QFESPIMB040SampleStageConfigThread; // forward

/*! \brief SPIM Control Extension (B040, DKFZ Heidelberg) QGropBox with a set of controls that allow to control a sample translation stage
    \ingroup qf3ext_spimb040


 */
class QFESPIMB040SampleStageConfig : public QGroupBox {
        Q_OBJECT
    public:
        /** Default constructor */
        QFESPIMB040SampleStageConfig(QWidget* parent, bool useThread=true);
        //QFESPIMB040SampleStageConfig(QFESPIMB040MainWindow* parent, QFPluginServices* pluginServices);
        /** Default destructor */
        virtual ~QFESPIMB040SampleStageConfig();

        void init(QFPluginLogService* log, QFPluginServices* pluginServices);
        void setLog(QFPluginLogService* log);

        /** \brief load settings */
        void loadSettings(QSettings& settings, QString prefix, bool dontLoadDevices=false);
        void loadSettings(QFManyFilesSettings& settings, QString prefix, bool dontLoadDevices=false);
        /** \brief save settings */
        void saveSettings(QSettings& settings, QString prefix);
        void saveSettings(QFManyFilesSettings& settings, QString prefix);

        /** \brief connect to all selected stages */
        void connectStages();
        /** \brief disconnect from all selected stages */
        void disconnectStages();

        /*! \brief lock access to stages: stop the thread used for stage access by this widget

            \note call this, if you want to access the stage from any other method outside this widget!!! otherwise concurrent thread accesses are possible!!!
            \note You can release the lock y calling unlockStages().
          */
        void lockStages();
        /** \brief unlock access to stages: restart the thread used for stage access by this widget  */
        void unlockStages();

        bool isJoystickChecked() const;
        double joystickMaxSpeed();

    public slots:
        void setReadOnly(bool readonly);

        void speedX2();
        void speedX10();
        void speedD2();
        void speedD10();
        void toggleJoystick();
        void joystickOn();
        void joystickOff();

        void stepX();
        void stepY();
        void stepZ();
        void stepR();
        void stepMinusX();
        void stepMinusY();
        void stepMinusZ();
        void stepMinusR();

    protected slots:
        void updateStates();

        void stageXMoved(QFExtensionLinearStage::AxisState state, double position, double velocity);
        void stageYMoved(QFExtensionLinearStage::AxisState state, double position, double velocity);
        void stageZMoved(QFExtensionLinearStage::AxisState state, double position, double velocity);
        void stageRMoved(QFExtensionLinearStage::AxisState state, double position, double velocity);
        void joystickStateChanged(bool enabled);
        void stagesConnectedChanged(bool connX, bool connY, bool connZ, bool connR);
        void threadStarted();
        void threadFinished();
    protected:
        //QFESPIMB040MainWindow* m_parent;
        QFPluginLogService* m_log;
        QFPluginServices* m_pluginServices;
        /** \brief list of all available QFExtensionLinearStage plugins, initialized by findCameras() */
        //QList<QObject*> stages;

        /** \brief fill stages */
        //void findStages(QFExtensionManager* extManager);

        void createWidgets();
        void createActions();

        /** \brief combobox to select a x-stage */
        QFStageComboBox* cmbStageX;
        /** \brief combobox to select a y-stage */
        QFStageComboBox* cmbStageY;
        /** \brief combobox to select a z-stage */
        QFStageComboBox* cmbStageZ;
        /** \brief combobox to select a Rot-stage */
        QFStageComboBox* cmbStageR;

        /** \brief tool button to connect to stage for axis x */
        QToolButton* btnConnectX;
        /** \brief tool button to connect to stage for axis y */
        QToolButton* btnConnectY;
        /** \brief tool button to connect to stage for axis z */
        QToolButton* btnConnectZ;
        /** \brief tool button to connect to stage for axis rot */
        QToolButton* btnConnectR;

        /** \brief tool button to configure stage for axis x */
        QToolButton* btnConfigureX;
        /** \brief tool button to configure stage for axis y */
        QToolButton* btnConfigureY;
        /** \brief tool button to configure stage for axis z */
        QToolButton* btnConfigureZ;
        /** \brief tool button to configure stage for axis rot */
        QToolButton* btnConfigureR;

        /** \brief checkbox to en-/disable the joystic for all axes (but rot) at once */
        QCheckBox* chkJoystick;
        /** \brief edit to give the maximum speed of the joystick in micron/sec */
        QDoubleSpinBox* spinJoystickMaxSpeed;

        /** \brief edit to give x coordinate for movement */
        QDoubleSpinBox* spinMoveX;
        /** \brief edit to give y coordinate for movement */
        QDoubleSpinBox* spinMoveY;
        /** \brief edit to give z coordinate for movement */
        QDoubleSpinBox* spinMoveZ;
        /** \brief edit to give Rot coordinate for movement */
        QDoubleSpinBox* spinMoveR;
        /** \brief button for absolute movement */
        QPushButton* btnMoveAbsolute;
        /** \brief button for relative movement */
        QPushButton* btnMoveRelative;



        /** \brief label for x-axis state */
        QLabel* labXState;
        /** \brief label for x-axis position */
        QLabel* labXPosition;
        /** \brief label for x-axis speed */
        QLabel* labXSpeed;

        /** \brief label for y-axis state */
        QLabel* labYState;
        /** \brief label for y-axis position */
        QLabel* labYPosition;
        /** \brief label for y-axis speed */
        QLabel* labYSpeed;

        /** \brief label for z-axis state */
        QLabel* labZState;
        /** \brief label for z-axis position */
        QLabel* labZPosition;
        /** \brief label for z-axis speed */
        QLabel* labZSpeed;

        /** \brief label for Rot-axis state */
        QLabel* labRState;
        /** \brief label for Rot-axis position */
        QLabel* labRPosition;
        /** \brief label for Rot-axis speed */
        QLabel* labRSpeed;

        /** \brief label to display joystick status */
        QLabel* labJoystick;
        //QLabel* labThread;
        QToolButton* btnX2;
        QToolButton* btnX10;
        QToolButton* btnD2;
        QToolButton* btnD10;


        /** \brief action to connect to stage for axis x */
        QAction* actConnectX;
        /** \brief action to connect to stage for axis y */
        QAction* actConnectY;
        /** \brief action to connect to stage for axis z */
        QAction* actConnectZ;
        /** \brief action to connect to stage for axis Rot */
        QAction* actConnectR;

        /** \brief action to configure stage for axis x */
        QAction* actConfigureX;
        /** \brief action to configure stage for axis y */
        QAction* actConfigureY;
        /** \brief action to configure stage for axis z */
        QAction* actConfigureZ;
        /** \brief action to configure stage for axis Rot */
        QAction* actConfigureR;

        QAction* actConfigSteps;
        QToolButton* btnConfigSteps;

        QTimer* timUpdate;

        bool locked;

        double stageStateUpdateInterval;

        QPixmap iconDisconnected;
        QPixmap iconReady;
        QPixmap iconError;
        QPixmap iconMoving;
        QPixmap iconMovingOpposite;
        QPixmap iconJoystick;
        QPixmap iconNoJoystick;

        QTimer timerDisplayUpdate;
        QFESPIMB040SampleStageConfigThread* stageThread;

        double m_stepX;
        double m_stepY;
        double m_stepZ;
        double m_stepR;

        bool useThread;

        void updateStageStateWidgets(QLabel* labPos, QLabel* labSpeed, QLabel* labState, bool present, QFExtensionLinearStage::AxisState state, double position, double speed);

    public:

        /** \brief return a pointer to the x-axis stage class */
        QFExtensionLinearStage* getXStage();

        /** \brief return a pointer to the y-axis stage class */
        QFExtensionLinearStage* getYStage();

        /** \brief return a pointer to the z-axis stage class */
        QFExtensionLinearStage* getZStage();

        /** \brief return a pointer to the Rot-axis stage class */
        QFExtensionLinearStage* getRStage();

        /** \brief return a pointer to the x-axis QFExtension class */
        QFExtension* getXStageExtension();

        /** \brief return a pointer to the y-axis QFExtension class */
        QFExtension* getYStageExtension();

        /** \brief return a pointer to the z-axis QFExtension class */
        QFExtension* getZStageExtension();

        /** \brief return a pointer to the Rot-axis QFExtension class */
        QFExtension* getRStageExtension();

        /** \brief return a pointer to the x-axis QObject class */
        QObject* getXStageExtensionObject();

        /** \brief return a pointer to the y-axis QObject class */
        QObject* getYStageExtensionObject();

        /** \brief return a pointer to the z-axis QObject class */
        QObject* getZStageExtensionObject();

        /** \brief return a pointer to the Rot-axis QObject class */
        QObject* getRStageExtensionObject();



        /** \brief get the axis number of x-axis stage inside its class */
        int getXStageAxis();

        /** \brief get the axis number of y-axis stage inside its class */
        int getYStageAxis();

        /** \brief get the axis number of z-axis stage inside its class */
        int getZStageAxis();

        /** \brief get the axis number of Rot-axis stage inside its class */
        int getRStageAxis();

        bool isXStageConnected() const;
        bool isYStageConnected() const;
        bool isZStageConnected() const;
        bool isRStageConnected() const;

        void moveRelative(double x, double y, double z, double R);
    protected slots:
        void disConnectX();
        void disConnectY();
        void disConnectZ();
        void disConnectR();
        void configureX();
        void configureY();
        void configureZ();
        void configureR();
        void updateJoystick();
        void displayAxisStates();
        void moveAbsolute();
        void moveRelative();
        void configSteps();


};

#endif // QFESPIMB040SAMPLESTAGECONFIG_H
