/*
    Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center (DKFZ) & IWR, University of Heidelberg

    

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


#ifndef QFSHUTTERCONFIGWIDGET_H
#define QFSHUTTERCONFIGWIDGET_H

#include <QGroupBox>


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
#include "qfextensionshutter.h"
#include "qfextension.h"
#include "cpptools.h"
#include "qfshuttercombobox.h"
#include "libwid_imexport.h"
#include "qfshutterconfigwidgetthread.h"
#include "qfmanyfilessettings.h"

/*! \brief SPIM Control Extension (B040, DKFZ Heidelberg) QWidget with a set of controls that allow to control a shutter
    \ingroup qf3lib_widgets


 */
class QFWIDLIB_EXPORT QFShutterConfigWidget : public QWidget {
        Q_OBJECT
    public:
        /** Default constructor */
        QFShutterConfigWidget(QWidget* parent=NULL);
        /** Default destructor */
        virtual ~QFShutterConfigWidget();

        void init(QFPluginLogService* log, QFPluginServices* pluginServices);
        void setLog(QFPluginLogService* log);

        /** \brief load settings */
        void loadSettings(QSettings& settings, QString prefix);
        void loadSettings(QFManyFilesSettings& settings, QString prefix);
        /** \brief save settings */
        void saveSettings(QSettings& settings, QString prefix);
        void saveSettings(QFManyFilesSettings& settings, QString prefix);

        /** \brief connect to all selected Shutters */
        void connectShutter();
        /** \brief disconnect from all selected Shutters */
        void disconnectShutter();

        /*! \brief lock access to stages: stop the thread used for stage access by this widget

            \note call this, if you want to access the stage from any other method outside this widget!!! otherwise concurrent thread accesses are possible!!!
            \note You can release the lock y calling unlockStages().
          */
        void lockShutters();
        /** \brief unlock access to stages: restart the thread used for stage access by this widget  */
        void unlockShutters();

        bool getShutterState();

        QAction* getStateAction() const;


    public slots:
        void setReadOnly(bool readonly);

    protected slots:
        void updateStates();

    protected:
        QFPluginLogService* m_log;
        QFPluginServices* m_pluginServices;

        QFShutterConfigWidgetThread* m_thread;


        void createWidgets();
        void createActions();

        /** \brief combobox to select a shutter */
        QFShutterComboBox* cmbShutter;

        /** \brief tool button to connect to a shutter */
        QToolButton* btnConnect;

        /** \brief tool button to configure a shutter */
        QToolButton* btnConfigure;

        /** \brief tool button to configure a shutter */
        QToolButton* btnAlexOnOff;

        /** \brief button to display/change the shutter state */
        QToolButton* btnState;

        /** \brief action to connect to a shutter */
        QAction* actConnect;

        /** \brief action to configure to a shutter */
        QAction* actConfigure;
        QAction* actState;

        /** \brief action to switch ALEX on or off on a shutter driver */
        QAction* actAlexOnOff;

        QTimer* timUpdate;

        bool locked;

        double shutterStateUpdateInterval;


        QTimer timerDisplayUpdate;

        bool useThread;

    public:

        /** \brief return a pointer to the shutter */
        QFExtensionShutter* getShutter() const;

        /** \brief return a pointer to the ahutter QFExtension class */
        QFExtension* getShutterExtension() const;
        QObject* getShutterExtensionObject() const;

        QPixmap iconOpened;
        QPixmap iconClosed;
        QPixmap iconAlex;

        bool moving;


        /** \brief get the shutter number of the given axis */
        int getShutterID() const;

        bool isShutterConnected() const;

        bool isShutterDone() const;

    public slots:
        void setShutter(bool shutterOpened);
        void setAlex(bool shutterAlexOnOff);

        void toggleShutter();
        void shutterOff();
        void shutterOn();

    protected slots:
        void disConnect();
        void configure();
        void displayShutterStates();
        void updateActionProperties();
        void updateActionProperties(bool opened);
        void shutterActionAlexClicked(bool AlexOnOff);
    public slots:
        void shutterActionClicked(bool shutterOpened);
        void shutterStateChanged(bool state);
};

#endif // QFSHUTTERCONFIGWIDGET_H
