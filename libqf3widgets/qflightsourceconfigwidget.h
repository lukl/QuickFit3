#ifndef QFLIGHTSOURCECONFIGWIDGET_H
#define QFLIGHTSOURCECONFIGWIDGET_H

#include <QWidget>


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
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include "programoptions.h"
#include "jkqttools.h"
#include "qtlogfile.h"
#include "qfextensionmanager.h"
#include "qfextensionlightsource.h"
#include "qfextension.h"
#include "tools.h"
#include "qflightsourcecombobox.h"
#include "filters.h"
#include "libwid_imexport.h"
#include "qflightsourceconfigwidgetthread.h"




/*! \brief QWidget with a set of controls that allow to control a LightSource
    \ingroup qf3ext_spimb040


 */
class QFWIDLIB_EXPORT QFLightSourceConfigWidget : public QFrame {
        Q_OBJECT
    public:
        explicit QFLightSourceConfigWidget(QWidget *parent = 0);
        ~QFLightSourceConfigWidget();
        

        void init(QFPluginLogService* log, QFPluginServices* pluginServices);
        void setLog(QFPluginLogService* log);

        /** \brief load settings */
        void loadSettings(QSettings& settings, QString prefix);
        /** \brief save settings */
        void saveSettings(QSettings& settings, QString prefix);

        /** \brief connect to all selected LightSources */
        void connectLightSource();
        /** \brief disconnect from all selected LightSources */
        void disconnectLightSource();

        /*! \brief lock access to stages: stop the thread used for stage access by this widget

            \note call this, if you want to access the stage from any other method outside this widget!!! otherwise concurrent thread accesses are possible!!!
            \note You can release the lock y calling unlockStages().
          */
        void lockLightSource();
        /** \brief unlock access to stages: restart the thread used for stage access by this widget  */
        void unlockLightSource();

        /** \brief return a pointer to the LightSource */
        QFExtensionLightSource* getLightSource() const;

        /** \brief return a pointer to the ahutter QFExtension class */
        QFExtension* getLightSourceExtension() const;


        /** \brief get the LightSource number of the given axis */
        int getLightSourceID() const;

        bool isLightSourceConnected() const;

        bool isLastActionComplete() const;

        int getLineCount() const;
        QString getLineDescription(int line) const;
        double getSetPower(int line) const;
        double getMeasuredPower(int line) const;
        bool isLineEnabled(int line) const;
        QString getLineUnit(int line) const;


    public slots:
        void setReadOnly(bool readonly);
        /** \brief switch the given line on or off. While the widget is NOT locked, this function is blocking until the action completes  */
        void setLineEnabled(int line, bool enabled);
        /** \brief set the given line power. While the widget is NOT locked, this function is blocking until the action completes  */
        void setLinePower(int line, double power);

    protected:
        QFPluginLogService* m_log;
        QFPluginServices* m_pluginServices;
        QString m_filterconfig;

        QFLightSourceConfigWidgetThread* lsthread;


        void createWidgets();
        void createActions();

        /** \brief combobox to select a LightSource */
        QFLightSourceComboBox* cmbLightSource;

        /** \brief tool button to connect to a LightSource */
        QToolButton* btnConnect;

        /** \brief tool button to configure a LightSource */
        QToolButton* btnConfigure;

        QSpacerItem* stretch;

        /** \brief action to connect to a LightSource */
        QAction* actConnect;

        /** \brief action to configure to a LightSource */
        QAction* actConfigure;

        QTimer* timUpdate;

        bool locked;

        bool moving;

        double stateUpdateInterval;


        QTimer timerDisplayUpdate;

        struct LSLineWidgets {
            QPointer<QCheckBox> chkEnable;
            QPointer<QDoubleSpinBox> spinPower;
            QPointer<QLabel> labPower;
        };

        QList<LSLineWidgets> lineWidgets;

        QGridLayout* widgetLayout;
        QGridLayout* linesLayout;
        QWidget* linesLayoutWidget;

        void updateLSLinesWidgets();

        int getLineByWidget(QObject *widget);

        bool useThread;
        bool dontAccessWidgets;


    protected slots:
        void disConnect();
        void configure();
        void displayStates();
        void updateStates();
        void setPowerChanged(double value);
        void lineEnabledToggled(bool enabled);
        void linesChanged(QTime time, QList<bool> lineenabled, QList<double> setValues, QList<double> measuredValues, QStringList powerUnits, QStringList lineNames, QList<bool> widgetsEnabled);

        
};

#endif // QFLIGHTSOURCECONFIGWIDGET_H