#ifndef QFESPIMB040ACQUISITIONCONFIGWIDGET2_H
#define QFESPIMB040ACQUISITIONCONFIGWIDGET2_H

#include <QWidget>
#include <QSettings>
#include <QAction>
#include <QToolButton>
#include <QWidget>
#include "../interfaces/qfextensionlinearstage.h"
#include "qfespimb040opticssetup.h"
#include "qfespimb040filenametool.h"
#include "qtriple.h"

class QFESPIMB040MainWindow; // forward
class QFPluginServices; // forward

namespace Ui {
    class QFESPIMB040AcquisitionConfigWidget2;
}


/*! \brief widget that allows to configure an image acquisition
    \ingroup qf3ext_spimb040
 */
class QFESPIMB040AcquisitionConfigWidget2 : public QWidget, public QFESPIMB040FilenameTool {
        Q_OBJECT

    public:
        explicit QFESPIMB040AcquisitionConfigWidget2(QWidget* parent, QFPluginServices* pluginServices, QFESPIMB040OpticsSetup* opticsSetup, QString configDirectory);
        ~QFESPIMB040AcquisitionConfigWidget2();

        /** \brief return the filename for the currently selected camera configuration */
        QString currentConfigFilename(int camera) const;
        /** \brief return the name (not the full filename with path) for the currently selected camera configuration */
        QString currentConfigName(int camera) const;
        /** \brief return the filename for the currently selected camera preview configuration */
        QString currentPreviewConfigFilename(int camera) const;
        /** \brief return the name (not the full filename with path) for the currently selected camera preview configuration */
        QString currentPreviewConfigName(int camera) const;
        /** \brief return the number of background frames */
        int currentBackgroundFrames(int camera) const;
        /** \brief return whether to save background frames */
        bool saveBackground() const;
    public slots:
        void lightpathesChanged(QFESPIMB040OpticsSetupItems lightpathes);

    signals:
        void doAcquisition();

    public:
        void loadSettings(QSettings& settings, QString prefix);
        void storeSettings(QSettings& settings, QString prefix) const;


        QString prefix1() ;
        QString prefix2() ;
        bool use1() const;
        bool use2() const;
        bool overview() const;

        bool lightpathActivated() const;
        QString lightpathFilename() const;
        QString lightpath() const;
        int repeats() const;


    protected slots:
        void on_btnAcquire_clicked();
        void on_chkUse1_toggled(bool enabled);
        void on_chkUse2_toggled(bool enabled);
        void on_chkBackground_toggled(bool enabled);
        void updateBackgroundWidgets();
        void updateReplaces();
    private:
        QFPluginServices* m_pluginServices;
        Ui::QFESPIMB040AcquisitionConfigWidget2 *ui;
        QFESPIMB040OpticsSetup* opticsSetup;
};

#endif // QFESPIMB040ACQUISITIONCONFIGWIDGET2_H
