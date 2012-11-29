#include "qfrdrploteditor.h"
#include "qfrdrtable.h"
#include "dlgcsvparameters.h"


QFRDRPlotEditor::QFRDRPlotEditor(QFPluginServices* services,  QFRawDataPropertyEditor* propEditor, QWidget* parent):
    QFRawDataEditor(services, propEditor, parent)
{
    //std::cout<<"QFRDRPlotEditor() ...\n";
    updating=true;
    createWidgets();
    updating=false;
    //std::cout<<"QFRDRPlotEditor() ... done\n";
}

QFRDRPlotEditor::~QFRDRPlotEditor()
{
    writeSettings();
}

void QFRDRPlotEditor::createWidgets() {
    QVBoxLayout* l=new QVBoxLayout(this);
    setLayout(l);

    tbMain=new QToolBar("tbPlotMain", this);
    l->addWidget(tbMain);

    tabPlots=new QTabWidget(this);
    tabPlots->setTabsClosable(true);
    connect(tabPlots, SIGNAL(tabCloseRequested(int)), this, SLOT(deletePlot(int)));
    l->addWidget(tabPlots);

    actAddPlot=new QAction(QIcon(":/table/plot_add.png"), tr("add plot"), this);
    connect(actAddPlot, SIGNAL(triggered()), this, SLOT(addPlot()));
    actDeletePlot=new QAction(QIcon(":/table/plot_delete.png"), tr("delete current plot"), this);
    connect(actDeletePlot, SIGNAL(triggered()), this, SLOT(deleteCurrentPlot()));
    actDuplicatePlot=new QAction(QIcon(":/table/copy.png"), tr("duplicate the current plot"), this);
    connect(actDuplicatePlot, SIGNAL(triggered()), this, SLOT(duplicatePlot()));
    actMovePlotLeft=new QAction(QIcon(":/table/leftarrow.png"), tr("move current plot to the left"), this);
    connect(actMovePlotLeft, SIGNAL(triggered()), this, SLOT(movePlotUp()));
    actMovePlotRight=new QAction(QIcon(":/table/rightarrow.png"), tr("move current plot to the right"), this);
    connect(actMovePlotRight, SIGNAL(triggered()), this, SLOT(movePlotDown()));

    tbMain->addAction(actAddPlot);
    tbMain->addAction(actDuplicatePlot);
    tbMain->addSeparator();
    tbMain->addAction(actDeletePlot);
    tbMain->addSeparator();
    tbMain->addAction(actMovePlotLeft);
    tbMain->addAction(actMovePlotRight);


    menuEdit=propertyEditor->addMenu("&Edit", 1);


    menuPlot=propertyEditor->addMenu("&Plots", 1);
    menuPlot->addAction(actAddPlot);
    menuPlot->addAction(actDuplicatePlot);
    menuPlot->addSeparator();
    menuPlot->addAction(actDeletePlot);
    menuPlot->addSeparator();
    menuPlot->addAction(actMovePlotLeft);
    menuPlot->addAction(actMovePlotRight);

}

void QFRDRPlotEditor::connectWidgets(QFRawDataRecord* current, QFRawDataRecord* old) {
    updating=true;
    if (old) {
        QFRDRTable* m=qobject_cast<QFRDRTable*>(old);
        if (m && m->model()) {

            //disconnect(m->model(), SIGNAL(notReadonlyChanged(bool)), actPaste, SLOT(setEnabled(bool)));
        }
        clearPlotWidgets();
    }
    //std::cout<<"qobject_cast ... ";
    QFRDRTable* m=qobject_cast<QFRDRTable*>(current);
    this->current=m;
    //std::cout<<m<<" ... ";
    //if (m) std::cout<<m->model()<<" ... ";
    rebuildPlotWidgets();
    //std::cout<<m<<" done\n";
    updating=false;
}

void QFRDRPlotEditor::rawDataChanged() {
    for (int i=0; i<plotWidgets.size(); i++)         {
        if (plotWidgets[i]) {
            plotWidgets[i]->rawDataChanged();
        }
    }
}

void QFRDRPlotEditor::readSettings() {
    if (settings) {
        //std::cout<<"QFRDRPlotEditor::readSettings()\n";
        currentPlotDir=settings->getQSettings()->value("rawtableeditor/currentPlotDir", currentPlotDir).toString();
        for (int i=0; i<plotWidgets.size(); i++)         {
            if (plotWidgets[i]) {
                plotWidgets[i]->readSettings(*(settings->getQSettings()), QString("rawtableeditor/plotwidget%1/").arg(i));
            }
        }
    }
}

void QFRDRPlotEditor::writeSettings() {
    if (settings) {
        settings->getQSettings()->setValue("rawtableeditor/currentPlotDir", currentPlotDir);
        for (int i=0; i<plotWidgets.size(); i++)         {
            if (plotWidgets[i]) {
                plotWidgets[i]->writeSettings(*(settings->getQSettings()), QString("rawtableeditor/plotwidget%1/").arg(i));
            }
        }
        settings->getQSettings()->sync();
    }
}

void QFRDRPlotEditor::addPlot() {
    QFRDRTable* m=qobject_cast<QFRDRTable*>(current);
    if (m) {
        updating=true;
        //qDebug()<<"adding plot";
        m->addPlot();
        rebuildPlotWidgets();
        updating=false;
    }

}

void QFRDRPlotEditor::deleteCurrentPlot() {
    deletePlot(tabPlots->currentIndex());
}

void QFRDRPlotEditor::duplicatePlot()
{
    QFRDRTable* m=qobject_cast<QFRDRTable*>(current);
    if (m) {
        updating=true;
        QFRDRTable::PlotInfo p=m->getPlot(tabPlots->currentIndex());
        p.title=tr("copy of \"%1\"").arg(p.title);
        m->addPlot(p);
        rebuildPlotWidgets();
        updating=false;
    }
}

void QFRDRPlotEditor::movePlotUp()
{
    QFRDRTable* m=qobject_cast<QFRDRTable*>(current);
    if (m) {
        updating=true;
        int current=tabPlots->currentIndex();
        if (current>0) {
            m->swapPlots(current, current-1);
            rebuildPlotWidgets();
            tabPlots->setCurrentIndex(current-1);
        }
        updating=false;
    }
}

void QFRDRPlotEditor::movePlotDown()
{
    QFRDRTable* m=qobject_cast<QFRDRTable*>(current);
    if (m) {
        updating=true;
        int current=tabPlots->currentIndex();
        if (current<m->getPlotCount()-1) {
            m->swapPlots(current, current+1);
            rebuildPlotWidgets();
            tabPlots->setCurrentIndex(current+1);
        }
        updating=false;
    }
}

void QFRDRPlotEditor::deletePlot(int i) {
    QFRDRTable* m=qobject_cast<QFRDRTable*>(current);
    if (m) {
        updating=true;
        m->deletePlot(i);
        rebuildPlotWidgets();
        updating=false;
    }
}

void QFRDRPlotEditor::rebuildPlotWidgets(bool keepPosition) {
    //qDebug()<<"rebuilding plot widgets";
    bool updt=updating;
    updating=true;
    int pos=tabPlots->currentIndex();

    if (!current) {
        //qDebug()<<"   clearPlotWIdgets";
        clearPlotWidgets();
    } else {
        while (plotWidgets.size()<current->getPlotCount()) {
            QFRDRTablePlotWidget* w=new QFRDRTablePlotWidget(tabPlots);
            connect(w, SIGNAL(plotTitleChanged(int,QString)), this, SLOT(updatePlotName(int,QString)));
            plotWidgets.append(w);
            tabPlots->addTab(plotWidgets[plotWidgets.size()-1], "plot");
        }
        while (plotWidgets.size()>current->getPlotCount()) {
            int idx=tabPlots->indexOf(plotWidgets.last());
            if (idx>=0) {
                tabPlots->removeTab(idx);
            }
            if (plotWidgets.last()) delete plotWidgets.last();
        }
        for (int i=0; i<current->getPlotCount(); i++) {
            //qDebug()<<"   setRecord "<<i;
            QString pn=current->getPlot(i).title;
            if (pn.isEmpty()) pn=tr("plot %1").arg(i+1);
            tabPlots->setTabText(i, pn);
            plotWidgets[i]->setRecord(current, i);
        }
    }

    if (keepPosition) {
        tabPlots->setCurrentIndex(pos);
    }
    updating=updt;
}

void QFRDRPlotEditor::clearPlotWidgets()
{
    for (int i=0; i<plotWidgets.size(); i++) {
        int idx=tabPlots->indexOf(plotWidgets[i]);
        if (idx>=0) {
            tabPlots->removeTab(idx);
        }
        if (plotWidgets[i]) delete plotWidgets[i];
    }
    plotWidgets.clear();
}

void QFRDRPlotEditor::updatePlotName(int plot, QString title) {
    if (plot>=0 && plot<tabPlots->count()) tabPlots->setTabText(plot, title);
}






