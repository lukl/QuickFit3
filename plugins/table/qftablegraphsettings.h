#ifndef QFTABLEGRAPHSETTINGS_H
#define QFTABLEGRAPHSETTINGS_H

#include <QWidget>
#include "qfrawdatapropertyeditor.h"
#include "qfrawdatarecord.h"
#include <cmath>
#include "tools.h"
#include "qfrdrtable.h"
#include "QToolTip"
#include "qfmathparser.h"
#include "qffunctionreferencetool.h"
namespace Ui {
    class QFTableGraphSettings;
}

class QFTableGraphSettings : public QWidget
{
        Q_OBJECT
        
    public:
        explicit QFTableGraphSettings(QWidget *parent = 0);
        ~QFTableGraphSettings();
        void setRecord(QFRDRTable* record, int plot);

        void rawDataChanged();

        /** \brief read the settings */
        virtual void readSettings(QSettings& settings, const QString& prefix=QString(""));
        /** \brief write the settings */
        virtual void writeSettings(QSettings& settings, const QString& prefix=QString(""));

        void writeGraphData(QFRDRTable::GraphInfo &graph);
        void loadGraphData(const QFRDRTable::GraphInfo &graph);
        void updatePlotWidgetVisibility(const QFRDRTable::GraphInfo &graph);
        QIcon getGraphIcon(int i) const;
    signals:
        void graphDataChanged();
    public slots:
        void initFocus();

    protected slots:
        void on_btnFunctionHelp_clicked();
        void on_edtFunction_textChanged(const QString &text);

        void writeGraphData();
        void reloadColumns(QComboBox* combo);

        void connectWidgets();
        void disconnectWidgets();
    protected:

    private:
        Ui::QFTableGraphSettings *ui;
        QFRDRTable* current;
        int plot;
        bool updating;
        QFFunctionReferenceTool* functionRef;
};

#endif // QFTABLEGRAPHSETTINGS_H