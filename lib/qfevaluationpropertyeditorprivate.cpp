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

#include "qfevaluationpropertyeditorprivate.h"
#include "qfevaluationpropertyeditor.h"
#include "qfprojectrawdatamodel.h"
#include "qfevaluationitemfactory.h"
#include "qfversion.h"
#include "qfproject.h"
#include "qfenhancedtabwidget.h"
#include "datatools.h"
#include "qftablemodel.h"
#include "qfrdrtableinterface.h"
#include "qfexporttotabledialog.h"
#include "qfrawdataeditor.h"
#include "qfrawdatapropertyeditor.h"
#include "qfrdrimagemask.h"
#include "qfrdrrunselection.h"

QFEvaluationPropertyEditorPrivate::QFEvaluationPropertyEditorPrivate(QFEvaluationPropertyEditor *parent) :
    QObject(parent)
{
    this->d=parent;
}
























void QFEvaluationPropertyEditorPrivate::resultsChanged(QFRawDataRecord* /*record*/, const QString& /*evalName*/, const QString& /*resultName*/) {
    if (!d->resultsModel) return;
    //resultsModel->resultsChanged();
    if (tvResults->model()->columnCount()*tvResults->model()->rowCount()<10000) tvResults->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    //tvResults->verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void QFEvaluationPropertyEditorPrivate::refreshResults()
{
    d->resultsModel->resultsChanged();
    resultsChanged();
}

void QFEvaluationPropertyEditorPrivate::displayHelp() {
    if (tabMain->currentIndex()==0 || tabMain->currentIndex()==tabMain->count()-1) {
        displayHelpEval();
    } else {
        displayHelpPlugin();
    }

}

void QFEvaluationPropertyEditorPrivate::displayHelpPlugin()
{
    QString dll=d->current->getProject()->getEvaluationItemFactory()->getPluginHelp(d->current->getType());
    services->displayHelpWindow(dll);
}

void QFEvaluationPropertyEditorPrivate::displayHelpPluginTutorial()
{
    QString dll=d->current->getProject()->getEvaluationItemFactory()->getPluginTutorialMain(d->current->getType());
    services->displayHelpWindow(dll);
}

void QFEvaluationPropertyEditorPrivate::displayHelpPluginCopyright()
{
    QString dll=d->current->getProject()->getEvaluationItemFactory()->getPluginCopyrightFile(d->current->getType());
    services->displayHelpWindow(dll);
}

void QFEvaluationPropertyEditorPrivate::displayHelpEval()
{
    QString dll=services->getOptions()->getAssetsDirectory()+QString("/help/qf3_evalscreen.html");
    services->displayHelpWindow(dll);
}

void QFEvaluationPropertyEditorPrivate::copyValErrResults() {
    tvResults->copySelectionAsValueErrorToExcel(QFEvaluationResultsModel::AvgRole, QFEvaluationResultsModel::SDRole);
}

void QFEvaluationPropertyEditorPrivate::copyValErrResultsNoHead() {
    tvResults->copySelectionAsValueErrorToExcel(QFEvaluationResultsModel::AvgRole, QFEvaluationResultsModel::SDRole, false);
}

void QFEvaluationPropertyEditorPrivate::copyMedianQuantilesResults(){
    tvResults->copySelectionAsMedianQuantilesToExcel(QFEvaluationResultsModel::MedianRole, QFEvaluationResultsModel::Quantile25Role, QFEvaluationResultsModel::Quantile75Role);
}

void QFEvaluationPropertyEditorPrivate::copyMedianQuantilesResultsNoHead() {
    tvResults->copySelectionAsMedianQuantilesToExcel(QFEvaluationResultsModel::MedianRole, QFEvaluationResultsModel::Quantile25Role, QFEvaluationResultsModel::Quantile75Role, false);
}

void QFEvaluationPropertyEditorPrivate::copyExpandedResults()
{
    tvResults->copySelectionToExcelExpanded(Qt::EditRole, true);
}

void QFEvaluationPropertyEditorPrivate::copyExpandedResultsNoHead()
{
    tvResults->copySelectionToExcelExpanded(Qt::EditRole, false);
}

void QFEvaluationPropertyEditorPrivate::copyExpandedResultsNoHeadMatlab()
{
    tvResults->copySelectionToMatlabExpandedNoHead(Qt::EditRole);
}

void QFEvaluationPropertyEditorPrivate::copyExpandedResultsFlipped()
{
    tvResults->copySelectionToExcelExpanded(Qt::EditRole, true, true);
}

void QFEvaluationPropertyEditorPrivate::copyExpandedResultsNoHeadFlipped()
{
    tvResults->copySelectionToExcelExpanded(Qt::EditRole, false, true);
}

void QFEvaluationPropertyEditorPrivate::copyExpandedResultsNoHeadMatlabFlipped()
{
    tvResults->copySelectionToMatlabExpandedNoHead(Qt::EditRole, true);
}
void QFEvaluationPropertyEditorPrivate::currentTabChanged(int tab)
{
    int idx=tab-1;
    for (int i=0; i<menus.size(); i++) {
        if (menus[i].first==-1 || (idx>=0 && menus[i].first==idx)) {
            menus[i].second->menuAction()->setVisible(true);
        } else {
            menus[i].second->menuAction()->setVisible(false);
        }
    }
    menuResults->menuAction()->setVisible(tab==tabMain->count()-1);
}

void QFEvaluationPropertyEditorPrivate::checkHelpAvailable()
{
    if (!d->current) {
        actHelpPlugin->setVisible(false);
        actHelpPluginTutorial->setVisible(false);
        actHelpPluginCopyright->setVisible(false);
    } else {
        QString dll=d->current->getProject()->getEvaluationItemFactory()->getPluginHelp(d->current->getType());
        actHelpPlugin->setVisible(QFile::exists(dll));
        dll=d->current->getProject()->getEvaluationItemFactory()->getPluginTutorialMain(d->current->getType());
        actHelpPluginTutorial->setVisible(QFile::exists(dll));
        dll=d->current->getProject()->getEvaluationItemFactory()->getPluginCopyrightFile(d->current->getType());
        actHelpPluginCopyright->setVisible(QFile::exists(dll));
    }

}

void QFEvaluationPropertyEditorPrivate::propertiesTextChanged(const QString &text) {
    if (!d->current) return;
    if (!d->resultsModel) return;
    if (text.isEmpty()) {
        d->resultsModel->setDisplayProperties(QStringList());
    } else {
        QStringList p=text.split(",");
        for (int i=0; i<p.size(); i++) {
            p[i]=p[i].trimmed();
        }
        d->resultsModel->setDisplayProperties(p);
    }
}

void QFEvaluationPropertyEditorPrivate::filterRecordsChanged()
{
    d->setFilesListFilteres(filesListFiltered);
}

void QFEvaluationPropertyEditorPrivate::deleteSelectedResults() {
    if (!d->current) return;
    QModelIndexList sel=tvResults->selectionModel()->selectedIndexes();
    if (sel.size()>0) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::question(d, tr("QuickFit %1").arg(qfInfoVersion()),
                     tr("Do you really want to delete the selected results?"),
                     QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (ret == QMessageBox::Yes) {

            QSet<QFRawDataRecord*> recs;
            for (int i=0; i<sel.size(); i++) {
                QString en=sel[i].data(QFEvaluationResultsModel::EvalNameRole).toString();
                QString rn=sel[i].data(QFEvaluationResultsModel::ResultNameRole).toString();
                int rid=sel[i].data(QFEvaluationResultsModel::ResultIDRole).toInt();
                if (rid>=0) {
                    QFRawDataRecord* r=d->current->getProject()->getRawDataByID(rid);
                    if (r) {
                        recs.insert(r);
                        r->disableEmitResultsChanged();
                        r->resultsRemove(en, rn, false);
                    }
                }
            }
            QSetIterator<QFRawDataRecord *> i(recs);
            while (i.hasNext()) {
                i.next()->enableEmitResultsChanged(true);
            }
        }
    }
}
void QFEvaluationPropertyEditorPrivate::deleteAllVisibleResults() {

    tvResults->selectAll();
    deleteSelectedResults();
}
void QFEvaluationPropertyEditorPrivate::showAvgClicked(bool checked)
{
    if (d->current && d->resultsModel) {
        d->resultsModel->setShowVectorMatrixAvg(checked);
    }
}

void QFEvaluationPropertyEditorPrivate::showStatistics()
{

    QFHistogramService* hs=QFHistogramService::getInstance();
    if (hs&&d->current) {
        QModelIndexList idxs=tvResults->selectionModel()->selectedIndexes();
        QMap<int, QFHistogramService::Histogram> hists;
        for (int i=0; i<idxs.size(); i++) {
            QFHistogramService::Histogram h;
            int col=idxs[i].column();
            if (!hists.contains(col)) {
                h.name=d->resultsModel->headerData(col, Qt::Horizontal).toString();
                hists[col]=h;
            }
            QString ename=d->resultsModel->data(idxs[i], QFEvaluationResultsModel::EvalNameRole).toString();
            QString rname=d->resultsModel->data(idxs[i], QFEvaluationResultsModel::ResultNameRole).toString();
            int rid=d->resultsModel->data(idxs[i], QFEvaluationResultsModel::ResultIDRole).toInt();

            QFRawDataRecord* record=d->current->getProject()->getRawDataByID(rid);

            if (record) hists[col].data<<record->resultsGetAsDoubleList(ename, rname);
        }

        bool onePerCol=true;
        if (hists.size()>1) {
            onePerCol=QMessageBox::question(d, tr("Data histogram"), tr("You selected %1 columns.\nShould QuickFit open\n   [Yes] one histogram window per column, or\n   [No]  combine all data into one window?").arg(hists.size()), QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes)==QMessageBox::Yes;
        }

        if (onePerCol) {
            QMapIterator<int, QFHistogramService::Histogram> ii(hists);
            while (ii.hasNext()) {
                ii.next();
                QString histID=QString("hist")+d->current->getType()+QString::number(d->current->getID())+"_"+QString::number(ii.key());
                hs->getCreateView(histID, tr("Histogram from %1").arg(d->current->getName()));
                hs->clearView(histID);
                hs->addHistogramToView(histID, ii.value());
            }
        } else {
            QString histID=QString("hist")+d->current->getType()+QString::number(d->current->getID());
            hs->getCreateView(histID, tr("Histogram from %1").arg(d->current->getName()));
            hs->clearView(histID);
            QMapIterator<int, QFHistogramService::Histogram> ii(hists);
            while (ii.hasNext()) {
                ii.next();
                hs->addHistogramToView(histID, ii.value());
            }
        }
    }

}

void QFEvaluationPropertyEditorPrivate::showStatisticsComparing()
{

    QFHistogramService* hs=QFHistogramService::getInstance();
    if (hs&&d->current) {
        QModelIndexList idxs=tvResults->selectionModel()->selectedIndexes();
        QMap<int, QList<QFHistogramService::Histogram> > hists;
        for (int i=0; i<idxs.size(); i++) {
            QFHistogramService::Histogram h;
            int col=idxs[i].column();
            if (!hists.contains(col)) {
                QList<QFHistogramService::Histogram> l;
                hists[col]=l;
            }
            QString ename=d->resultsModel->data(idxs[i], QFEvaluationResultsModel::EvalNameRole).toString();
            QString rname=d->resultsModel->data(idxs[i], QFEvaluationResultsModel::ResultNameRole).toString();
            int rid=d->resultsModel->data(idxs[i], QFEvaluationResultsModel::ResultIDRole).toInt();
            QFRawDataRecord* record=d->current->getProject()->getRawDataByID(rid);

            h.name=d->resultsModel->headerData(col, Qt::Vertical).toString();
            if (record) h.data<<record->resultsGetAsDoubleList(ename, rname);
            hists[col].append(h);
        }

        bool onePerCol=true;
        if (hists.size()>1) {
            onePerCol=QMessageBox::question(d, tr("Data histogram"), tr("You selected %1 columns.\nShould QuickFit open\n   [Yes] one histogram window per column, or\n   [No]  combine all data into one window?").arg(hists.size()), QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes)==QMessageBox::Yes;
        }

        if (onePerCol) {
            QMapIterator<int, QList<QFHistogramService::Histogram> > ii(hists);
            while (ii.hasNext()) {
                ii.next();
                QString histID=QString("hist")+d->current->getType()+QString::number(d->current->getID())+"_"+QString::number(ii.key());
                hs->getCreateView(histID, tr("Histogram from %1").arg(d->current->getName()));
                hs->clearView(histID);
                for (int i=0; i<ii.value().size(); i++) {
                    hs->addHistogramToView(histID, ii.value().at(i));
                }
            }
        } else {
            QString histID=QString("hist")+d->current->getType()+QString::number(d->current->getID());
            hs->getCreateView(histID, tr("Histogram from %1").arg(d->current->getName()));
            hs->clearView(histID);
            QMapIterator<int,  QList<QFHistogramService::Histogram> > ii(hists);
            while (ii.hasNext()) {
                ii.next();
                for (int i=0; i<ii.value().size(); i++) {
                    hs->addHistogramToView(histID, ii.value().at(i));
                }
            }
        }
    }

}

void QFEvaluationPropertyEditorPrivate::showData()
{
    QFTableService* hs=QFTableService::getInstance();
    if (hs&&d->current) {
        QModelIndexList idxs=tvResults->selectionModel()->selectedIndexes();
        QMap<int, QFTableService::TableColumn> cols;
        QString histID=QString("hist")+d->current->getType()+QString::number(d->current->getID());
        hs->getCreateTableView(histID, tr("Data Table from %1").arg(d->current->getName()));
        hs->clearTableView(histID);


        for (int i=0; i<idxs.size(); i++) {
            QFTableService::TableColumn h;
            int col=idxs[i].column();
            QString chead=d->resultsModel->headerData(col, Qt::Horizontal).toString();
            QString rhead=d->resultsModel->headerData(idxs[i].row(), Qt::Vertical).toString();

            QString ename=d->resultsModel->data(idxs[i], QFEvaluationResultsModel::EvalNameRole).toString();
            QString rname=d->resultsModel->data(idxs[i], QFEvaluationResultsModel::ResultNameRole).toString();
            int rid=d->resultsModel->data(idxs[i], QFEvaluationResultsModel::ResultIDRole).toInt();

            QFRawDataRecord* record=d->current->getProject()->getRawDataByID(rid);

            if (record) {
                h.name=chead+": "+rhead;
                h.data=record->resultsGetAsQVariantListNoError(ename, rname);
                hs->addColumnToTableView(histID, h);
                if (record->resultsHasError(ename, rname)) {
                    h.name=tr("ERROR: ")+chead+": "+rhead;
                    h.data=record->resultsGetAsQVariantListOnlyError(ename, rname);
                    hs->addColumnToTableView(histID, h);
                }
            }
        }
    }
}

void QFEvaluationPropertyEditorPrivate::storeSettings()
{
    if (!d->current) return;
    d->current->setQFProperty("RESULTS_PROPERTIES", edtDisplayProperties->text(), false, false);
    d->current->setQFProperty("FILES_FILTER", edtFilterFiles->text(), false, false);
    d->current->setQFProperty("FILES_FILTERNOT", edtFilterFilesNot->text(), false, false);
    d->current->setQFProperty("FILES_FILTER_REGEXP", chkFilterFilesRegExp->isChecked(), false, false);
    d->current->setQFProperty("RESULTS_FILTER", edtFilterResults->text(), false, false);
    d->current->setQFProperty("RESULTS_FILTERNOT", edtFilterResultsNot->text(), false, false);
    d->current->setQFProperty("RESULTS_FILTER_REGEXP", chkFilterResultsRegExp->isChecked(), false, false);


    d->current->setQFProperty("RESULTS_MIND_UNUSED_VECSTAT", chkMindUnusedForVectorStat->isChecked(), false, false);
    d->current->setQFProperty("RESULTS_EXTRACT_RUNS", chkExtractRuns->isChecked(), false, false);
    d->current->setQFProperty("RESULTS_REMOVE_UNUSED_RUNS", chkRemoveUnusedRuns->isChecked(), false, false);
    d->current->setQFProperty("RESULTS_SHOW_AVG", chkShowAvg->isChecked(), false, false);
}

void QFEvaluationPropertyEditorPrivate::loadSettings()
{
    if (!d->current) return;
    edtDisplayProperties->setText(d->current->getProperty("RESULTS_PROPERTIES", "").toString());
    edtFilterFiles->setText(d->current->getProperty("FILES_FILTER", "").toString());
    edtFilterFilesNot->setText(d->current->getProperty("FILES_FILTERNOT", "").toString());
    chkFilterFilesRegExp->setChecked(d->current->getProperty("FILES_FILTER_REGEXP", false).toBool());
    edtFilterResults->setText(d->current->getProperty("RESULTS_FILTER", "").toString());
    edtFilterResultsNot->setText(d->current->getProperty("RESULTS_FILTERNOT", "").toString());
    chkFilterResultsRegExp->setChecked(d->current->getProperty("RESULTS_FILTER_REGEXP", false).toBool());


    chkMindUnusedForVectorStat->setChecked(d->current->getProperty("RESULTS_MIND_UNUSED_VECSTAT", true).toBool());
    chkExtractRuns->setChecked(d->current->getProperty("RESULTS_EXTRACT_RUNS", true).toBool());
    chkRemoveUnusedRuns->setChecked(d->current->getProperty("RESULTS_REMOVE_UNUSED_RUNS", true).toBool());
    chkShowAvg->setChecked(d->current->getProperty("RESULTS_SHOW_AVG", true).toBool());
}


void QFEvaluationPropertyEditorPrivate::nameChanged(const QString& text) {
    if (d->current) {
        d->current->setName(text);
    }
}

void QFEvaluationPropertyEditorPrivate::descriptionChanged() {
    if (d->current) {
        d->current->setDescription(pteDescription->toPlainText());
    }
}

void QFEvaluationPropertyEditorPrivate::evaluationAboutToBeDeleted(QFEvaluationItem* r) {
    if ((d->current==r) && d->current) {
        d->close();
    }
}


void QFEvaluationPropertyEditorPrivate::propsChanged(const QString& /*property*/, bool /*visible*/) {
    if (d->current) {
        if (d->current->getName()!=edtName->text()) {
            edtName->setText(d->current->getName());
        }
        if (d->current->getDescription()!=pteDescription->toPlainText()) {
            pteDescription->setPlainText(d->current->getDescription());
        }
        /*labTopIcon->setPixmap(d->current->getSmallIcon().pixmap(16,16));
        labTop->setText(tr("<b>%1</b>").arg(d->current->getName()));*/
        QString prefix="";
        int whm=ProgramOptions::getConfigValue("quickfit/windowheadermode", 1).toInt();
        if (whm==1) {
            prefix=QFileInfo(d->current->getProject()->getFile()).fileName();
        } else if (whm==2) {
            prefix=d->current->getProject()->getName();
            if (prefix.isEmpty()) prefix=QFileInfo(d->current->getProject()->getFile()).fileName();
        }
        if (!prefix.isEmpty()) prefix += QString(": ");

        d->setWindowTitle(QString("%2Evaluation: %1").arg(d->current->getName()).arg(prefix));
        d->setWindowIcon(d->current->getSmallIcon());
        labID->setText(QString::number(d->current->getID()));
        labType->setText(d->current->getTypeDescription());
        labTypeIcon->setPixmap(d->current->getSmallIcon().pixmap(16,16));
    }
}


void QFEvaluationPropertyEditorPrivate::createWidgets() {
    QVBoxLayout* ml=new QVBoxLayout();
    d->setLayout(ml);
    ml->setContentsMargins(2,2,2,2);


    menuBar=new QMenuBar(d);
    menuBar->setVisible(true);
    menuBar->setNativeMenuBar(ProgramOptions::getConfigValue("quickfit/macxsubwinmenus_asmain", false).toBool());
    ml->addWidget(menuBar);
    menuResults=menuBar->addMenu("&Results");
    menuHelp=menuBar->addMenu("&Help");

    tabMain=new QFEnhancedTabWidget(d);
    ml->addWidget(tabMain);

    QWidget* w=new QWidget(tabMain);
    QFormLayout* fl=new QFormLayout();
    w->setLayout(fl);
    tabMain->addTab(w, tr("&Properties"));
    labID=new QLabel(w);
    fl->addRow(tr("ID:"), labID);
    labType=new QLabel(w);
    labTypeIcon=new QLabel(w);
    QHBoxLayout* ptl=new QHBoxLayout();
    ptl->setContentsMargins(0,0,0,0);
    ptl->addWidget(labTypeIcon);
    ptl->addWidget(labType);
    ptl->addStretch();
    fl->addRow(tr("Type:"), ptl);
    edtName=new QLineEdit(w);
    fl->addRow(tr("&Name:"), edtName);
    pteDescription=new QPlainTextEdit(w);
    fl->addRow(tr("&Description:"), pteDescription);

    splitMain=new QVisibleHandleSplitter(tabMain);
    lstRawData=new QListView(splitMain);
    //tabEditors=new QTabWidget(splitMain);
    layWidgets=new QHBoxLayout();
    QWidget* wl=new QWidget(d);
    wl->setLayout(layWidgets);
    //splitMain->addWidget(tabEditors);
    splitMain->addWidget(wl);
    //splitMain->addWidget(lstRawData);
    QVBoxLayout* lstvbl=new QVBoxLayout();
    widRDRList=new QWidget(splitMain);
    widRDRList->setLayout(lstvbl);

    widFilterRecords=new QWidget(d);
    QHBoxLayout* lFilterRecords=new QHBoxLayout();
    widFilterRecords->setLayout(lFilterRecords);
    lFilterRecords->addWidget(new QLabel("filter: "));
    edtFilterRecords=new QFEnhancedLineEdit(d);
    edtFilterRecords->addButton(new QFStyledButton(QFStyledButton::ClearLineEdit, edtFilterRecords, edtFilterRecords));
    edtFilterRecords->setCompleter(new QFCompleterFromFile(edtFilterRecords));
    compFilterRecords=new QFCompleterFromFile(d);
    edtFilterRecords->setCompleter(compFilterRecords);
    lFilterRecords->addWidget(edtFilterRecords);
    lFilterRecords->addWidget(new QLabel(tr(" <span style=\"text-decoration: overline\">filter</span>: "), d));
    edtFilterRecordsNot=new QFEnhancedLineEdit(d);
    edtFilterRecordsNot->addButton(new QFStyledButton(QFStyledButton::ClearLineEdit, edtFilterRecordsNot, edtFilterRecordsNot));
    compFilterRecordsNot=new QFCompleterFromFile(d);
    edtFilterRecordsNot->setCompleter(compFilterRecordsNot);
    lFilterRecords->addWidget(edtFilterRecordsNot);
    chkFilterRecordsRegExp=new QCheckBox(tr("RegExp"), d);
    chkFilterRecordsRegExp->setChecked(false);
    lFilterRecords->addWidget(chkFilterRecordsRegExp);
    edtFilterRecords->setToolTip(tr("use this to filter the contents of the rdr list<br><br>"
                                       "Simply enter a filter string and the list will only display those"
                                       "rows where the raw data record name contains a match to the filter string."
                                       "Depending on whether <i>RegExp</i> is checked or not, you can use"
                                       "either simple text with wildcards '<tt>*</tt>' (match any characters) and '<tt>?</tt>'"
                                       "(match a single character), or full regular expressions in the filter string."
                                  "<br><br><font color=\"darkred\">red text</font> means you entered an invalid regular expression"));
    edtFilterRecordsNot->setToolTip(tr("use this to filter the contents of the rdr list<br><br>"
                                       "Simply enter a filter string and the list will only display those"
                                       "rows where the raw data record name contains NO match to the filter string."
                                       "Depending on whether <i>RegExp</i> is checked or not, you can use"
                                       "either simple text with wildcards '<tt>*</tt>' (match any characters) and '<tt>?</tt>'"
                                       "(match a single character), or full regular expressions in the filter string."
                                     "<br><br><font color=\"darkred\">red text</font> means you entered an invalid regular expression"));
    connect(edtFilterRecordsNot, SIGNAL(textChanged(QString)), this, SLOT(filterRecordsTextChanged(QString)));
    connect(edtFilterRecords, SIGNAL(textChanged(QString)), this, SLOT(filterRecordsTextChanged(QString)));

    lstvbl->addWidget(widFilterRecords);

    lstvbl->addWidget(lstRawData, 10);

    btnRemoveRawData=new QPushButton(QIcon(":/lib/item_delete.png"), tr("remove RDR"), widRDRList);
    btnRemoveRawData->setToolTip(tr("remove the current raw data record (RDR) from the project"));
    connect(btnRemoveRawData, SIGNAL(clicked()), this, SLOT(removeRawData()));
    lstvbl->addWidget(btnRemoveRawData);

    btnRemoveRawDataGroup=new QPushButton(QIcon(":/lib/item_delete.png"), tr("remove RDR+group"), widRDRList);
    btnRemoveRawDataGroup->setToolTip(tr("remove the current raw data record (RDR) and its group files from the project"));
    connect(btnRemoveRawDataGroup, SIGNAL(clicked()), this, SLOT(removeRawDataGroup()));
    lstvbl->addWidget(btnRemoveRawDataGroup);

    splitMain->addWidget(widRDRList);

    splitMain->setCollapsible(0, false);
    splitMain->setCollapsible(1, false);
    splitMain->setStretchFactor(0,5);
    splitMain->setStretchFactor(1,1);

    //connect(lstRawData, SIGNAL(activated(const QModelIndex&)), this, SLOT(selectionChanged(const QModelIndex&)));
    //connect(lstRawData, SIGNAL(clicked(const QModelIndex&)), this, SLOT(selectionChanged(const QModelIndex&)));
    //connect(lstRawData, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(selectionChanged(const QModelIndex&)));
    //connect(lstRawData->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(selectionChanged(const QModelIndex&, const QModelIndex&)));

    //QItemSelectionModel::SelectCurrent

    tabMain->addTab(splitMain, tr("&Evaluation"));




    widResults=new QWidget(d);
    QVBoxLayout* rwvlayout=new QVBoxLayout();
    widResults->setLayout(rwvlayout);

    tvResults=new QEnhancedTableView(widResults);
    actCopyResults=tvResults->getActCopyExcel();
    actCopyResultsNoHead=tvResults->getActCopyExcelNoHead();
    tvResults->setAlternatingRowColors(true);
    tvResults->setItemDelegate(new QFHTMLDelegate(tvResults));
    QFontMetrics fm(d->font());
    tvResults->verticalHeader()->setDefaultSectionSize((int)round((double)fm.height()*1.5));
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    tvResults->verticalHeader()->setResizeMode(QHeaderView::Interactive);
#endif
    tvResults->verticalHeader()->setTextElideMode(Qt::ElideMiddle);
    tvResults->verticalHeader()->setMaximumWidth(QApplication::desktop()->screenGeometry(QApplication::desktop()->primaryScreen()).width()*0.75);

    tvResults->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    tvResults->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    tvResults->setModel(d->resultsModel);
    tvResults->setContextMenuPolicy(Qt::ActionsContextMenu);


    tbResults=new QToolBar("toolbar_eval_results", d);
    rwvlayout->addWidget(tbResults);
    tbResultsSettings=new QToolBar("toolbar_eval_results_settings", d);
    rwvlayout->addWidget(tbResultsSettings);


    actRefreshResults=new QFActionWithNoMenuRole(QIcon(":/lib/refresh16.png"), tr("Refresh results ..."), d);
    actRefreshResults->setShortcut(tr("F5"));
    tbResults->addAction(actRefreshResults);
    tbResults->addSeparator();
    actDeleteResults=new QFActionWithNoMenuRole(QIcon(":/lib/delete16.png"), tr("Delete selected results"), d);
    actDeleteResults->setShortcut(QKeySequence::Delete);
    tbResults->addAction(actDeleteResults);
    actDeleteAllVisibleResults=new QFActionWithNoMenuRole(tr("Delete all visible results"), d);
    tbResults->addSeparator();


    tvResults->addActionsToToolbar(tbResults);
    actCopyValErrResults=new QFActionWithNoMenuRole(QIcon(":/lib/copy16valerr.png"), tr("Copy Selection as value+error pairs"), this);
    tbResults->addAction(actCopyValErrResults);
    actCopyValErrResultsNoHead=new QFActionWithNoMenuRole(QIcon(":/lib/copy16valerr_nohead.png"), tr("Copy Selection as value+error pairs, w/o header"), d);
    tbResults->addAction(actCopyValErrResultsNoHead);

    actCopyMedianQuantilesResults=new QFActionWithNoMenuRole(QIcon(":/lib/copy16valerr.png"), tr("Copy Selection as median+q25+q75"), d);
    actCopyMedianQuantilesNoHead=new QFActionWithNoMenuRole(QIcon(":/lib/copy16valerr_nohead.png"), tr("Copy Selection as median+q25+q75, w/o header"), d);

    menuExpanded=new QMenu(tr("copy in expanded form"), tvResults);
    menuExpanded->setIcon(QIcon(":/lib/copy16.png"));
    tvResults->addAction(menuExpanded->menuAction());
    actCopyExpanded=new QFActionWithNoMenuRole( tr("Copy Selection in expanded form"), d);
    menuExpanded->addAction(actCopyExpanded);
    actCopyExpandedFlipped=new QFActionWithNoMenuRole( tr("Copy Selection in expanded form, flipped"), d);
    menuExpanded->addAction(actCopyExpandedFlipped);
    actCopyExpandedNoHead=new QFActionWithNoMenuRole( tr("Copy Selection in expanded form, w/o header"), d);
    menuExpanded->addAction(actCopyExpandedNoHead);
    actCopyExpandedNoHeadFlipped=new QFActionWithNoMenuRole( tr("Copy Selection in expanded form, w/o header, flipped"), d);
    menuExpanded->addAction(actCopyExpandedNoHeadFlipped);
    actCopyExpandedNoHeadMatlab=new QFActionWithNoMenuRole( tr("Copy Selection in expanded form, to Matlab"), d);
    menuExpanded->addAction(actCopyExpandedNoHeadMatlab);
    actCopyExpandedNoHeadMatlabFlipped=new QFActionWithNoMenuRole( tr("Copy Selection in expanded form, to Matlab, flipped"), d);
    menuExpanded->addAction(actCopyExpandedNoHeadMatlabFlipped);



    menuCopyIDs=new QMenu(tr("Copy Result/Evaluation IDs"), tvResults);
    menuCopyIDs->setIcon(QIcon(":/lib/copy_forparser.png"));
    tvResults->addAction(menuCopyIDs->menuAction());
    actCopyRDRID=new QFActionWithNoMenuRole( tr("Copy RDR ID"), d);
    menuCopyIDs->addAction(actCopyRDRID);;
    actCopyEvaluationID=new QFActionWithNoMenuRole( tr("Copy Evaluation ID"), d);
    menuCopyIDs->addAction(actCopyEvaluationID);;
    actCopyResultID=new QFActionWithNoMenuRole( tr("Copy Result ID"), d);
    menuCopyIDs->addAction(actCopyResultID);;
    actCopyResultAccessParserFunction=new QFActionWithNoMenuRole( tr("Copy Result Access Parser Function rdr_getresult(...)"), d);
    menuCopyIDs->addAction(actCopyResultAccessParserFunction);;

    actCopyResultAccessParserFunctionTable=new QFActionWithNoMenuRole(QIcon(":/table/table_insert.png"), tr("Copy selected cells as table RDR with result access parser functions"), d);
    tvResults->addAction(actCopyResultAccessParserFunctionTable);

    actSaveResults=new QFActionWithNoMenuRole(QIcon(":/lib/save16.png"), tr("Save all results to file"), d);
    tbResults->addAction(actSaveResults);
    actSaveResultsAveraged=new QFActionWithNoMenuRole(tr("Save all results to file, averaged vector/matrix results"), d);

    tbResults->addSeparator();
    actShowData=new QFActionWithNoMenuRole(QIcon(":/lib/result_table.png"), tr("show selected data as table"), d);
    tbResults->addAction(actShowData);
    actStatistics=new QFActionWithNoMenuRole(QIcon(":/lib/result_statistics.png"), tr("histogram: column-wise"), d);
    tbResults->addAction(actStatistics);
    actStatisticsComparing=new QFActionWithNoMenuRole(QIcon(":/lib/result_statistics_compare.png"), tr("histogram: cell-wise"), d);
    tbResults->addAction(actStatisticsComparing);

    //tbResults->addSeparator();
    chkShowAvg=new QCheckBox(tr("show Avg+/-SD for vector/matrix resuts"), d);
    tbResultsSettings->addWidget(chkShowAvg);
    chkShowAvg->setChecked(true);
    connect(chkShowAvg, SIGNAL(toggled(bool)), this, SLOT(showAvgClicked(bool)));
    connect(chkShowAvg, SIGNAL(toggled(bool)), this, SLOT(storeSettings()));
    tbResultsSettings->addSeparator();

    chkExtractRuns=new QCheckBox(tr("show runs/indices"), d);
    chkExtractRuns->setChecked(true);
    tbResultsSettings->addWidget(chkExtractRuns);
    connect(chkExtractRuns, SIGNAL(toggled(bool)), d->resultsModel, SLOT(setExtractIndexes(bool)));
    connect(chkExtractRuns, SIGNAL(toggled(bool)), this, SLOT(storeSettings()));
    chkRemoveUnusedRuns=new QCheckBox(tr("don't show removed runs"), d);
    chkRemoveUnusedRuns->setChecked(false);
    tbResultsSettings->addWidget(chkRemoveUnusedRuns);
    connect(chkExtractRuns, SIGNAL(toggled(bool)), chkRemoveUnusedRuns, SLOT(setEnabled(bool)));
    connect(chkRemoveUnusedRuns, SIGNAL(toggled(bool)), d->resultsModel, SLOT(setRemoveUnusedIndexes(bool)));
    connect(chkRemoveUnusedRuns, SIGNAL(toggled(bool)), this, SLOT(storeSettings()));

    chkMindUnusedForVectorStat=new QCheckBox(tr("mind excluded runs for in-cell statistics"), d);
    chkMindUnusedForVectorStat->setChecked(true);
    tbResultsSettings->addWidget(chkMindUnusedForVectorStat);
    connect(chkMindUnusedForVectorStat, SIGNAL(toggled(bool)), d->resultsModel, SLOT(setMinUnusedIndexesForStatistics(bool)));
    connect(chkMindUnusedForVectorStat, SIGNAL(toggled(bool)), this, SLOT(storeSettings()));


    tbResultsSettings->addSeparator();
    tbResultsSettings->addWidget(new QLabel(" display properties: "));
    edtDisplayProperties=new QFEnhancedLineEdit(d);
    edtDisplayProperties->addButton(new QFStyledButton(QFStyledButton::ClearLineEdit, edtDisplayProperties, edtDisplayProperties));
    edtDisplayProperties->addButton(new QFStyledButton(QFStyledButton::SelectFromCompleter, edtDisplayProperties, edtDisplayProperties));
    edtDisplayProperties->setToolTip(tr("put a comma ',' separated list of properties here that should be part of the results table. <br><b>Note:</b> These properties can NOT be filtered with the filters below."));
    compDisplayProperties=new QFCompleterProperties(d);
    edtDisplayProperties->setCompleter(compDisplayProperties);

    connect(edtDisplayProperties, SIGNAL(textChanged(QString)), this, SLOT(propertiesTextChanged(QString)));
    tbResultsSettings->addWidget(edtDisplayProperties);



    tbResultsFilter=new QToolBar("toolbar_eval_results2", d);
    rwvlayout->addWidget(tbResultsFilter);
    tbResultsFilter->addWidget(new QLabel(" filter: file contains "));
    edtFilterFiles=new QFEnhancedLineEdit(d);
    edtFilterFiles->addButton(new QFStyledButton(QFStyledButton::ClearLineEdit, edtFilterFiles, edtFilterFiles));
    compFilterFiles=new QFCompleterFromFile(d);
    edtFilterFiles->setCompleter(compFilterFiles);
    connect(edtFilterFiles, SIGNAL(textChanged(QString)), d->resultsModel, SLOT(setFilesFilter(QString)));
    tbResultsFilter->addWidget(edtFilterFiles);
    tbResultsFilter->addWidget(new QLabel(tr("  and not contains "), d));
    edtFilterFilesNot=new QFEnhancedLineEdit(d);
    edtFilterFilesNot->addButton(new QFStyledButton(QFStyledButton::ClearLineEdit, edtFilterFilesNot, edtFilterFilesNot));
    compFilterFilesNot=new QFCompleterFromFile(d);
    edtFilterFilesNot->setCompleter(compFilterFilesNot);
    connect(edtFilterFilesNot, SIGNAL(textChanged(QString)), d->resultsModel, SLOT(setFilesFilterNot(QString)));
    tbResultsFilter->addWidget(edtFilterFilesNot);
    chkFilterFilesRegExp=new QCheckBox(tr("RegExp"), d);
    connect(chkFilterFilesRegExp, SIGNAL(toggled(bool)), this, SLOT(storeSettings()));
    chkFilterFilesRegExp->setChecked(false);
    tbResultsFilter->addWidget(chkFilterFilesRegExp);
    connect(chkFilterFilesRegExp, SIGNAL(toggled(bool)), d->resultsModel, SLOT(setFilesFilterUsesRegExp(bool)));
    edtFilterFiles->setToolTip(tr("use this to filter the contents of the results table<br><br>"
                                       "Simply enter a filter string and the table will only display those"
                                       "rows where the raw data record name contains a match to the filter string."
                                       "Depending on whether <i>RegExp</i> is checked or not, you can use"
                                       "either simple text with wildcards '<tt>*</tt>' (match any characters) and '<tt>?</tt>'"
                                       "(match a single character), or full regular expressions in the filter string."
                                  "<br><br><font color=\"darkred\">red text</font> means you entered an invalid regular expression"));
    edtFilterFilesNot->setToolTip(tr("use this to filter the contents of the results table<br><br>"
                                       "Simply enter a filter string and the table will only display those"
                                       "rows where the raw data record name contains NO match to the filter string."
                                       "Depending on whether <i>RegExp</i> is checked or not, you can use"
                                       "either simple text with wildcards '<tt>*</tt>' (match any characters) and '<tt>?</tt>'"
                                       "(match a single character), or full regular expressions in the filter string."
                                     "<br><br><font color=\"darkred\">red text</font> means you entered an invalid regular expression"));

    tbResultsFilter->addSeparator();
    tbResultsFilter->addWidget(new QLabel(" filter: result contains: "));
    edtFilterResults=new QFEnhancedLineEdit(d);
    edtFilterResults->addButton(new QFStyledButton(QFStyledButton::ClearLineEdit, edtFilterResults, edtFilterResults));
    edtFilterResults->setToolTip(tr("use this to filter the contents of the results table<br><br>"
                                       "Simply enter a filter string and the table will only display those"
                                       "columns where the result name contains a match to the filter string."
                                       "Depending on whether <i>RegExp</i> is checked or not, you can use"
                                       "either simple text with wildcards '<tt>*</tt>' (match any characters) and '<tt>?</tt>'"
                                       "(match a single character), or full regular expressions in the filter string."
                                       "<br><br><font color=\"darkred\">red text</font> means you entered an invalid regular expression"));
    compFilterResults=new QFCompleterFromFile(d);
    edtFilterResults->setCompleter(compFilterResults);
    connect(edtFilterResults, SIGNAL(textChanged(QString)), d->resultsModel, SLOT(setResultFilter(QString)));
    tbResultsFilter->addWidget(edtFilterResults);
    tbResultsFilter->addWidget(new QLabel(tr("  and not contains "), d));
    edtFilterResultsNot=new QFEnhancedLineEdit(d);
    edtFilterResultsNot->addButton(new QFStyledButton(QFStyledButton::ClearLineEdit, edtFilterResultsNot, edtFilterResultsNot));
    compFilterResultsNot=new QFCompleterFromFile(d);
    edtFilterResultsNot->setCompleter(compFilterResultsNot);
    connect(edtFilterResultsNot, SIGNAL(textChanged(QString)), d->resultsModel, SLOT(setResultFilterNot(QString)));
    tbResultsFilter->addWidget(edtFilterResultsNot);
    chkFilterResultsRegExp=new QCheckBox(tr("RegExp"), d);
    chkFilterResultsRegExp->setChecked(false);
    tbResultsFilter->addWidget(chkFilterResultsRegExp);
    connect(chkFilterResultsRegExp, SIGNAL(toggled(bool)), d->resultsModel, SLOT(setResultFilterUsesRegExp(bool)));
    edtFilterResultsNot->setToolTip(tr("use this to filter the contents of the results table<br><br>"
                                       "Simply enter a filter string and the table will only display those"
                                       "columns where the result name contains NO match to the filter string."
                                       "Depending on whether <i>RegExp</i> is checked or not, you can use"
                                       "either simple text with wildcards '<tt>*</tt>' (match any characters) and '<tt>?</tt>'"
                                       "(match a single character), or full regular expressions in the filter string."
                                       "<br><br><font color=\"darkred\">red text</font> means you entered an invalid regular expression"));



    connect(edtFilterResultsNot, SIGNAL(textChanged(QString)), this, SLOT(filterResultsTextChanged(QString)));
    connect(edtFilterResults, SIGNAL(textChanged(QString)), this, SLOT(filterResultsTextChanged(QString)));
    connect(edtFilterFiles, SIGNAL(textChanged(QString)), this, SLOT(filterFilesTextChanged(QString)));
    connect(edtFilterFilesNot, SIGNAL(textChanged(QString)), this, SLOT(filterFilesTextChanged(QString)));


    rwvlayout->addWidget(tvResults);
    labAveragedresults=new QLabel(widResults);
    labAveragedresults->setTextInteractionFlags(Qt::TextSelectableByMouse);
    labAveragedresults->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    labAveragedresults->setMaximumHeight(200);
    labAveragedresults->setMaximumHeight(200);
    labAveragedresults->setSizePolicy(QSizePolicy::Ignored, labAveragedresults->sizePolicy().verticalPolicy());
    rwvlayout->addWidget(labAveragedresults);

    //connect(actCopyResults, SIGNAL(triggered()), tvResults, SLOT(copySelectionToExcel()));
    //connect(actCopyResultsNoHead, SIGNAL(triggered()), tvResults, SLOT(copySelectionToExcelNoHead()));
    connect(actCopyValErrResults, SIGNAL(triggered()), this, SLOT(copyValErrResults()));
    connect(actCopyValErrResultsNoHead, SIGNAL(triggered()), this, SLOT(copyValErrResultsNoHead()));
    connect(actCopyMedianQuantilesResults, SIGNAL(triggered()), this, SLOT(copyMedianQuantilesResults()));
    connect(actCopyMedianQuantilesNoHead, SIGNAL(triggered()), this, SLOT(copyMedianQuantilesResultsNoHead()));
    connect(actSaveResults, SIGNAL(triggered()), this, SLOT(saveResults()));
    connect(actSaveResultsAveraged, SIGNAL(triggered()), this, SLOT(saveResultsAveraged()));
    connect(actRefreshResults, SIGNAL(triggered()), this, SLOT(refreshResults()));
    connect(actDeleteResults, SIGNAL(triggered()), this, SLOT(deleteSelectedResults()));
    connect(actDeleteAllVisibleResults, SIGNAL(triggered()), this, SLOT(deleteAllVisibleResults()));
    connect(actStatistics, SIGNAL(triggered()), this, SLOT(showStatistics()));
    connect(actStatisticsComparing, SIGNAL(triggered()), this, SLOT(showStatisticsComparing()));
    connect(actShowData, SIGNAL(triggered()), this, SLOT(showData()));


    connect(actCopyExpanded, SIGNAL(triggered()), this, SLOT(copyExpandedResults()));
    connect(actCopyExpandedNoHead, SIGNAL(triggered()), this, SLOT(copyExpandedResultsNoHead()));
    connect(actCopyExpandedNoHeadMatlab, SIGNAL(triggered()), this, SLOT(copyExpandedResultsNoHeadMatlab()));
    connect(actCopyExpandedFlipped, SIGNAL(triggered()), this, SLOT(copyExpandedResultsFlipped()));
    connect(actCopyExpandedNoHeadFlipped, SIGNAL(triggered()), this, SLOT(copyExpandedResultsNoHeadFlipped()));
    connect(actCopyExpandedNoHeadMatlabFlipped, SIGNAL(triggered()), this, SLOT(copyExpandedResultsNoHeadMatlabFlipped()));

    connect(actCopyRDRID, SIGNAL(triggered()), this, SLOT(copyRDRID()));
    connect(actCopyEvaluationID, SIGNAL(triggered()), this, SLOT(copyEvaluationID()));
    connect(actCopyResultID, SIGNAL(triggered()), this, SLOT(copyResultID()));
    connect(actCopyResultAccessParserFunction, SIGNAL(triggered()), this, SLOT(copyResultAccessParserFunction()));
    connect(actCopyResultAccessParserFunctionTable, SIGNAL(triggered()), this, SLOT(copyResultAccessParserFunctionTable()));

    tabMain->addTab(widResults, tr("Evaluation &Results"));


    actHelp=new QFActionWithNoMenuRole(QIcon(":/lib/help.png"), tr("&Help"), d);
    actHelp->setToolTip(tr("display online-help"));
    connect(actHelp, SIGNAL(triggered()), this, SLOT(displayHelp()));

    actHelpPlugin=new QFActionWithNoMenuRole(QIcon(":/lib/help.png"), tr("&Plugin Help"), d);
    actHelpPlugin->setToolTip(tr("display online-help for the specific plugin"));
    connect(actHelpPlugin, SIGNAL(triggered()), this, SLOT(displayHelpPlugin()));

    actHelpPluginTutorial=new QFActionWithNoMenuRole(QIcon(":/lib/help/help_tutorial.png"), tr("&Plugin Tutorial"), d);
    actHelpPluginTutorial->setToolTip(tr("display the tutorial for the specific plugin"));
    connect(actHelpPluginTutorial, SIGNAL(triggered()), this, SLOT(displayHelpPluginTutorial()));

    actHelpPluginCopyright=new QFActionWithNoMenuRole(QIcon(":/lib/help/help_copyright.png"), tr("&Plugin Copyright"), d);
    actHelpPluginCopyright->setToolTip(tr("display copyright note for the specific plugin"));
    connect(actHelpPluginCopyright, SIGNAL(triggered()), this, SLOT(displayHelpPluginCopyright()));

    actHelpEval=new QFActionWithNoMenuRole(QIcon(":/lib/help_rdr.png"), tr("&Evaluation item help"), d);
    actHelpEval->setToolTip(tr("display online-help common to all plugins, i.e. for the basic evaluation editor dialog"));
    connect(actHelpEval, SIGNAL(triggered()), this, SLOT(displayHelpEval()));

    menuHelp->addAction(actHelpEval);
    menuHelp->addAction(actHelpPlugin);
    menuHelp->addAction(actHelpPluginTutorial);
    menuHelp->addAction(actHelpPluginCopyright);

    btnHelp=new QToolButton(d);
    btnHelp->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btnHelp->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btnHelp->setDefaultAction(actHelp);
    tabMain->setCornerWidget(btnHelp);

    connect(tabMain, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));

    tvResults->addAction(actCopyValErrResults);
    tvResults->addAction(actCopyValErrResultsNoHead);
    tvResults->addAction(actSaveResults);
    tvResults->addAction(actSaveResultsAveraged);
    tvResults->addAction(actRefreshResults);
    tvResults->addAction(actDeleteResults);
    tvResults->addAction(actDeleteAllVisibleResults);
    tvResults->addAction(actShowData);
    tvResults->addAction(actStatistics);
    tvResults->addAction(actStatisticsComparing);

    menuResults->addAction(actRefreshResults);
    menuResults->addSeparator();
    menuResults->addAction(actDeleteResults);
    menuResults->addAction(actDeleteAllVisibleResults);
    menuResults->addSeparator();
    menuResults->addAction(actSaveResults);
    menuResults->addAction(actSaveResultsAveraged);
    menuResults->addSeparator();
    tvResults->addActionsToMenu(menuResults);
    menuResults->addSeparator();
    //menuResults->addAction(actCopyResults);
    //menuResults->addAction(actCopyResultsNoHead);
    menuResults->addAction(actCopyValErrResults);
    menuResults->addAction(actCopyValErrResultsNoHead);
    menuResults->addSeparator();
    menuResults->addAction(actCopyMedianQuantilesResults);
    menuResults->addAction(actCopyMedianQuantilesNoHead);
    menuResults->addSeparator();
    menuResults->addAction(actCopyExpanded);
    menuResults->addAction(actCopyExpandedFlipped);
    menuResults->addAction(actCopyExpandedNoHead);
    menuResults->addAction(actCopyExpandedNoHeadFlipped);
    menuResults->addAction(actCopyExpandedNoHeadMatlab);
    menuResults->addAction(actCopyExpandedNoHeadMatlabFlipped);
    menuResults->addSeparator();
    menuResults->addAction(actShowData);
    menuResults->addAction(actStatistics);
    menuResults->addAction(actStatisticsComparing);

    /*QDir().mkpath(ProgramOptions::getInstance()->getConfigFileDirectory()+"/completers/");
    compFilterFiles->setFilename(ProgramOptions::getInstance()->getConfigFileDirectory()+"/completers/"+current->getType()+"_evalfilterfiles.txt");
    compFilterResults->setFilename(ProgramOptions::getInstance()->getConfigFileDirectory()+"/completers/"+current->getType()+"_evalfilterresults.txt");
    compFilterFilesNot->setFilename(ProgramOptions::getInstance()->getConfigFileDirectory()+"/completers/"+current->getType()+"_evalfilterfiles_not.txt");
    compFilterResultsNot->setFilename(ProgramOptions::getInstance()->getConfigFileDirectory()+"/completers/"+current->getType()+"_evalfilterresults_not.txt");
    compFilterRecords->setFilename(ProgramOptions::getInstance()->getConfigFileDirectory()+"/completers/"+current->getType()+"_evalfilterrecords.txt");
    compFilterRecordsNot->setFilename(ProgramOptions::getInstance()->getConfigFileDirectory()+"/completers/"+current->getType()+"_evalfilterrecords_not.txt");*/

    currentTabChanged(0);
}


void QFEvaluationPropertyEditorPrivate::selectionChanged(const QModelIndex& index, const QModelIndex& /*oldindex*/) {
    if (d->rdrProxy!=NULL) {
        QFRawDataRecord* rec=d->current->getProject()->getRawDataByID(d->rdrProxy->data(index, Qt::UserRole).toInt());
        if (rec!=NULL) {
            d->current->setHighlightedRecord(rec);
        } else {
            //std::cout<<"!!!   QFEvaluationPropertyEditorPrivate::selectionChanged() with NULL new record\n";
        }

        if (lstRawData->model()->rowCount()<=0) {
            d->closeBecauseNoRDRs();
        }
    }
}

void QFEvaluationPropertyEditorPrivate::rdrModelReset() {
    //std::cout<<"!!!   QFEvaluationPropertyEditorPrivate::rdrModelReset() called\n";
    if (!d->current) return;
    if (!d->current->getHighlightedRecord()) return;
    if ((!d->rdrModel) || (!d->rdrProxy)) return;
    QModelIndex idx=d->rdrModel->index(d->current->getProject()->getRawDataIndex(d->current->getHighlightedRecord()));
    QModelIndex pidx=d->rdrProxy->mapFromSource(idx);
    lstRawData->selectionModel()->select(pidx, QItemSelectionModel::SelectCurrent);
}



void QFEvaluationPropertyEditorPrivate::tvResultsSelectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/) {
    QModelIndexList sel=tvResults->selectionModel()->selectedIndexes();
    QMap<int, QString> names;
    QMap<int, double> sum, sum2, count;
    for (int i=0; i<sel.size(); i++) {
        //int c=sel[i].row();
        int r=sel[i].column();
        QVariant dataS=sel[i].data(QFEvaluationResultsModel::SumRole);
        QVariant dataS2=sel[i].data(QFEvaluationResultsModel::Sum2Role);
        QVariant dataC=sel[i].data(QFEvaluationResultsModel::CountRole);
        QString name=sel[i].data(QFEvaluationResultsModel::NameRole).toString();
        double d=0;
        bool ok=false;
        if (dataS.canConvert(QVariant::Double) && dataS2.canConvert(QVariant::Double) && dataC.canConvert(QVariant::LongLong)) {
            if (names.contains(r)) {
                sum[r] = sum[r]+dataS.toDouble();
                sum2[r] = sum2[r]+dataS2.toDouble();
                count[r] = count[r]+dataC.toLongLong();
            } else {
                sum[r] = dataS.toDouble();
                sum2[r] = dataS2.toDouble();
                count[r] = dataC.toLongLong();
                names[r] = name;
            }

        }
        /*if (data.canConvert(QVariant::Double)) {
            d=data.toDouble();
            ok=true;
        }
        if (data.canConvert(QVariant::PointF)) {
            d=data.toPointF().x();
            ok=true;
        }
        if (data.type()==QVariant::String) { ok=false; }
        if (ok) {
            if (names.contains(r)) {
                sum[r] = sum[r]+d;
                sum2[r] = sum2[r]+d*d;
                count[r] = count[r]+1;
            } else {
                sum[r] = d;
                sum2[r] = d*d;
                count[r] = 1;
                names[r] = name;
            }
        }*/
    }

    int lineHeight=labAveragedresults->fontMetrics().lineSpacing();
    int maxheight=labAveragedresults->maximumHeight();
    int linespercol=maxheight/(2*lineHeight)-1;
    QStringList datalist;
    QMapIterator<int, QString> it(names);
    while (it.hasNext()) {
        it.next();
        QString name=it.value();
        int i=it.key();
        if (count[i]>0) {
            double avg=sum[i]/count[i];
            double error=sqrt(sum2[i]/count[i]-sum[i]*sum[i]/count[i]/count[i]);
            datalist.append(QString("<td align=\"right\"><b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;%1:&nbsp;</b></td><td>&nbsp;&nbsp;%2 &plusmn; %3&nbsp;</td>").arg(name).arg(roundWithError(avg, error)).arg(roundError(error)));
        }
    }
    QString results=tr("<table border=\"1\" cellspacing=\"0\"><tr>");
    int cols=(int)ceil((double)datalist.size()/(double)linespercol);
    for (int i=0; i<cols; i++) {
        results+=tr("<th align=\"right\"><b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Field:&nbsp;</b></th><th><b>&nbsp;&nbsp;Average &plusmn; StdDev&nbsp;</b></th>");
    }
    results+=tr("</th>");
    for (int r=0; r<linespercol; r++) {
        results+=tr("<tr>");
        for (int c=0; c<cols; c++) {
            int idx=c*linespercol+r;
            if ((idx>=0)&&(idx<datalist.size())) {
                results+=datalist[idx];
            } else {
                results+=tr("<td></td><td></td>");
            }
        }
        results+=tr("</tr>");
    }
    results+=tr("</table>");
    labAveragedresults->setText(results);
}



void QFEvaluationPropertyEditorPrivate::saveResults() {
    if (d->current) {

        bool exportAll=true;
        QAbstractItemModel* model=tvResults->model();
        QModelIndexList indxs=tvResults->selectionModel()->selectedIndexes();
        bool all=true;
        for (int c=0; c<model->columnCount(); c++) {
            if (!tvResults->selectionModel()->isColumnSelected(c, QModelIndex())) {
                all=false;
                break;
            }
        }
        if (all) {
            for (int r=0; r<model->rowCount(); r++) {
                if (!tvResults->selectionModel()->isRowSelected(r, QModelIndex())) {
                    all=false;
                    break;
                }
            }
        }

        if (!all && indxs.size()>0) {
            exportAll=QMessageBox::question(d, tr("Save Results ..."), tr("You selected a range of cells.\nDo you want to export this range only?"), QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes)==QMessageBox::No;

        }

        //qDebug()<<all<<exportAll<<indxs.size();

        QStringList evalIDs;
        QList<QPair<QPointer<QFRawDataRecord>, QString> > records;
        if (!exportAll) {
            for (int i=0; i<indxs.size(); i++) {
                evalIDs.append(indxs[i].data(QFEvaluationResultsModel::ResultNameRole).toString());
                QPointer<QFRawDataRecord>  pp=d->current->getProject()->getRawDataByID(indxs[i].data(QFEvaluationResultsModel::ResultIDRole).toInt());
                records.append(qMakePair(pp, indxs[i].data(QFEvaluationResultsModel::EvalNameRole).toString()));
                //qDebug()<<evalIDs.last()<<records.last().first<<records.last().second;
            }
        }


        QString evalfilter=d->current->getResultsDisplayFilter();
        QStringList f=QFDataExportHandler::getFormats();
        QString lastDir=ProgramOptions::getConfigValue("QFEvaluationPropertyEditorPrivate/lastDataDir", "").toString();
        QString selFilter=ProgramOptions::getConfigValue("QFEvaluationPropertyEditorPrivate/lastDataFilter", "").toString();
        QString fn=qfGetSaveFileName(d, tr("Save results to file ..."), lastDir, f.join(";;"), &selFilter);
        if (fn.size()>0) {
            int flip=QMessageBox::question(d, tr("Save results to file ..."), tr("Flip table?"), QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel, QMessageBox::No);
            if (flip==QMessageBox::Yes || flip==QMessageBox::No) {
                d->current->getProject()->rdrResultsSave(evalfilter, fn, f.indexOf(selFilter), false, flip==QMessageBox::Yes, evalIDs, records);
                ProgramOptions::setConfigValue("QFEvaluationPropertyEditorPrivate/lastDataDir", lastDir);
                ProgramOptions::setConfigValue("QFEvaluationPropertyEditorPrivate/lastDataFilter", selFilter);
            }
        }

        /*
        QString evalfilter=d->current->getResultsDisplayFilter();
        QString selectedFilter="";
        QString filter= tr("Comma Separated Values (*.csv *.dat);;Semicolon Separated Values [for german Excel] (*.dat *.txt *.csv);;SYLK dataformat (*.slk *.sylk);;SYLK dataformat, flipped (*.slk *.sylk);;Tab separated (*.dat *.txt *.tsv);;Semicolon Separated Values (*.dat *.txt *.csv)");
        QString fileName = qfGetSaveFileName(d, tr("Save Results ..."), currentSaveDir, filter, &selectedFilter);
        if ((!fileName.isEmpty())&&(!fileName.isNull())) {
            int f=filter.split(";;").indexOf(selectedFilter);
            bool ok=false;
            if (f==1) {
                ok=d->current->getProject()->rdrResultsSaveToCSV(evalfilter, fileName, false, ';', ',', '"', evalIDs, records);
            } else if (f==2) {
                ok=d->current->getProject()->rdrResultsSaveToSYLK(evalfilter, fileName, false, false, evalIDs, records);
            } else if (f==3) {
                ok=d->current->getProject()->rdrResultsSaveToSYLK(evalfilter, fileName, false, true, evalIDs, records);
            } else if (f==4) {
                ok=d->current->getProject()->rdrResultsSaveToCSV(evalfilter, fileName, false, '\t', '.', '"', evalIDs, records);
            } else if (f==4) {
                ok=d->current->getProject()->rdrResultsSaveToCSV(evalfilter, fileName, false, ';', '.', '"', evalIDs, records);
            } else {
                ok=d->current->getProject()->rdrResultsSaveToCSV(evalfilter, fileName, false, ',', '.', '"', evalIDs, records);
            }
            currentSaveDir=QFileInfo(fileName).absolutePath();
            if (!ok) QMessageBox::critical(NULL, tr("QuickFit"), tr("Could not save file '%1'.").arg(fileName));
        }*/
    }
}

void QFEvaluationPropertyEditorPrivate::saveResultsAveraged()
{
    if (d->current) {


        bool exportAll=true;
        QAbstractItemModel* model=tvResults->model();
        QModelIndexList indxs=tvResults->selectionModel()->selectedIndexes();
        bool all=true;
        for (int c=0; c<model->columnCount(); c++) {
            if (!tvResults->selectionModel()->isColumnSelected(c, QModelIndex())) {
                all=false;
                break;
            }
        }
        if (all) {
            for (int r=0; r<model->rowCount(); r++) {
                if (!tvResults->selectionModel()->isRowSelected(r, QModelIndex())) {
                    all=false;
                    break;
                }
            }
        }

        if (!all && indxs.size()>0) {
            exportAll=QMessageBox::question(d, tr("Save Results ..."), tr("You selected a range of cells.\nDo you want to export this range only?"), QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes)==QMessageBox::No;

        }

        QStringList evalIDs;
        QList<QPair<QPointer<QFRawDataRecord>, QString> > records;
        if (!exportAll) {
            for (int i=0; i<indxs.size(); i++) {
                evalIDs.append(indxs[i].data(QFEvaluationResultsModel::EvalNameRole).toString());
                QPointer<QFRawDataRecord> pp=d->current->getProject()->getRawDataByID(indxs[i].data(QFEvaluationResultsModel::ResultIDRole).toInt());
                records.append(qMakePair(pp, indxs[i].data(QFEvaluationResultsModel::ResultNameRole).toString()));
            }
        }


        QString evalfilter=d->current->getResultsDisplayFilter();
        QStringList f=QFDataExportHandler::getFormats();
        QString lastDir=ProgramOptions::getConfigValue("QFEvaluationPropertyEditorPrivate/lastDataDir", "").toString();
        QString selFilter=ProgramOptions::getConfigValue("QFEvaluationPropertyEditorPrivate/lastDataFilter", "").toString();
        QString fn=qfGetSaveFileName(d, tr("Save results to file ..."), lastDir, f.join(";;"), &selFilter);
        if (fn.size()>0) {
            int flip=QMessageBox::question(d, tr("Save results to file ..."), tr("Flip table?"), QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel, QMessageBox::No);
            if (flip==QMessageBox::Yes || flip==QMessageBox::No) {
                d->current->getProject()->rdrResultsSave(evalfilter, fn, f.indexOf(selFilter), true, flip==QMessageBox::Yes, evalIDs, records);
                ProgramOptions::setConfigValue("QFEvaluationPropertyEditorPrivate/lastDataDir", lastDir);
                ProgramOptions::setConfigValue("QFEvaluationPropertyEditorPrivate/lastDataFilter", selFilter);
            }
        }


        /*
        QString selectedFilter="";
        QString filter= tr("Comma Separated Values (*.csv *.dat);;Semicolon Separated Values [for german Excel] (*.dat *.txt *.csv);;SYLK dataformat (*.slk *.sylk);;SYLK dataformat, flipped (*.slk *.sylk);;Tab separated (*.dat *.txt *.tsv);;Semicolon Separated Values [for english Excel] (*.dat *.txt *.csv)");
        QString fileName = qfGetSaveFileName(d, tr("Save Results ..."), currentSaveDir, filter, &selectedFilter);
        if ((!fileName.isEmpty())&&(!fileName.isNull())) {
            int f=filter.split(";;").indexOf(selectedFilter);
            bool ok=false;
            if (f==1) {
                ok=d->current->getProject()->rdrResultsSaveToCSV(evalfilter, fileName, true, ';', ',', '"', evalIDs, records);
            } else if (f==2) {
                ok=d->current->getProject()->rdrResultsSaveToSYLK(evalfilter, fileName, true, false, evalIDs, records);
            } else if (f==3) {
                ok=d->current->getProject()->rdrResultsSaveToSYLK(evalfilter, fileName, true, true, evalIDs, records);
            } else if (f==4) {
                ok=d->current->getProject()->rdrResultsSaveToCSV(evalfilter, fileName, true, '\t', '.', '"', evalIDs, records);
            } else if (f==5) {
                ok=d->current->getProject()->rdrResultsSaveToCSV(evalfilter, fileName, true, '\t', '.', '"', evalIDs, records);
            } else {
                ok=d->current->getProject()->rdrResultsSaveToCSV(evalfilter, fileName, true, ';', '.', '"', evalIDs, records);
            }
            currentSaveDir=QFileInfo(fileName).absolutePath();
            if (!ok) QMessageBox::critical(NULL, tr("QuickFit"), tr("Could not save file '%1'.").arg(fileName));
        }*/
    }
}


void QFEvaluationPropertyEditorPrivate::removeRawData() {
    if (d->current) {
        QFRawDataRecord* r=d->current->getHighlightedRecord();
        if (r) {
            int ret = QMessageBox::question(d, tr("QuickFit %1").arg(qfInfoVersionFull()),
                                    tr("Do you really want to delete the current record?\n   '%1'").arg(r->getName()),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::No);
            if (ret==QMessageBox::Yes) {
                d->deselectCurrent();
                d->current->getProject()->deleteRawData(r->getID());
            }
        }
    }
}

void QFEvaluationPropertyEditorPrivate::removeRawDataGroup() {
    if (d->current) {
        QFRawDataRecord* r=d->current->getHighlightedRecord();
        if (r) {
            int ret = QMessageBox::question(d, tr("QuickFit %1").arg(qfInfoVersionFull()),
                                    tr("Do you really want to delete the current record?\n   '%1'").arg(r->getName()),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::No);
            if (ret==QMessageBox::Yes) {
                d->deselectCurrent();
                d->current->getProject()->deleteRawDataGroup(r->getID());
            }
        }
    }
}

void QFEvaluationPropertyEditorPrivate::recordAboutToBeDeleted(QFRawDataRecord* record) {
    if (d->current) {
        if (d->current->isFilteredAndApplicable(record)) {
            int cnt=0;
            QList<QFRawDataRecord*> list=d->current->getProject()->getRawDataList();
            for (int i=0; i<list.size(); i++) {
                if ((record!=list[i])&&(d->current->isFilteredAndApplicable(list[i]))) {
                    cnt++;
                }
            }
            if (cnt<=0) {
                d->closeBecauseNoRDRs();
            }
        }
    }
}


void QFEvaluationPropertyEditorPrivate::filterFilesTextChanged(const QString &text)
{
    bool error=false;
    if (chkFilterFilesRegExp->isChecked()) {
        QRegExp rx(text);
        error=!rx.isValid();
    }
    QWidget* s=qobject_cast<QWidget*>(sender());
    if (s) {
       QPalette p=s->palette();
       if (error) {
           p.setColor(QPalette::Text, QColor("darkred"));
       } else {
           p.setColor(QPalette::Text, edtName->palette().color(QPalette::Text));
       }
       s->setPalette(p);
    }
    storeSettings();
}

void QFEvaluationPropertyEditorPrivate::filterResultsTextChanged(const QString &text)
{
    bool error=false;
    if (chkFilterResultsRegExp->isChecked()) {
        QRegExp rx(text);
        error=!rx.isValid();
    }
    QWidget* s=qobject_cast<QWidget*>(sender());
    if (s) {
       QPalette p=s->palette();
       if (error) {
           p.setColor(QPalette::Text, QColor("darkred"));
       } else {
           p.setColor(QPalette::Text, edtName->palette().color(QPalette::Text));
       }
       s->setPalette(p);
    }
    storeSettings();
}

void QFEvaluationPropertyEditorPrivate::filterRecordsTextChanged(const QString &text)
{
    bool error=false;
    if (chkFilterRecordsRegExp->isChecked()) {
        QRegExp rx(text);
        error=!rx.isValid();
    }
    QWidget* s=qobject_cast<QWidget*>(sender());
    if (s) {
       QPalette p=s->palette();
       if (error) {
           p.setColor(QPalette::Text, QColor("darkred"));
       } else {
           p.setColor(QPalette::Text, edtName->palette().color(QPalette::Text));
       }
       s->setPalette(p);
    }
    storeSettings();
}


void QFEvaluationPropertyEditorPrivate::copyRDRID()
{
    if (d->current) {
        QClipboard* clp=QApplication::clipboard();
        QModelIndex c=tvResults->currentIndex();
        if (c.isValid()) clp->setText(tvResults->model()->data(c, QFEvaluationResultsModel::ResultIDRole).toString());
    }
}

void QFEvaluationPropertyEditorPrivate::copyEvaluationID()
{
    if (d->current) {
        QClipboard* clp=QApplication::clipboard();
        QModelIndex c=tvResults->currentIndex();
        if (c.isValid()) clp->setText(tvResults->model()->data(c, QFEvaluationResultsModel::EvalNameRole).toString());
    }
}

void QFEvaluationPropertyEditorPrivate::copyResultID()
{
    if (d->current) {
        QClipboard* clp=QApplication::clipboard();
        QModelIndex c=tvResults->currentIndex();
        if (c.isValid()) clp->setText(tvResults->model()->data(c, QFEvaluationResultsModel::ResultNameRole).toString());
    }
}

void QFEvaluationPropertyEditorPrivate::copyResultAccessParserFunction()
{
    if (d->current) {
        QClipboard* clp=QApplication::clipboard();
        QModelIndex c=tvResults->currentIndex();
        if (c.isValid()) clp->setText(QString("rdr_getresult(%1, \"%2\", \"%3\")").arg(tvResults->model()->data(c, QFEvaluationResultsModel::ResultIDRole).toInt()).arg(tvResults->model()->data(c, QFEvaluationResultsModel::EvalNameRole).toString()).arg(tvResults->model()->data(c, QFEvaluationResultsModel::ResultNameRole).toString()));
    }
}

void QFEvaluationPropertyEditorPrivate::copyResultAccessParserFunctionTable()
{
    if (d->current) {

        QList<QList<QVariant> > RDRIDs_tab, resultnames_tab, evalnames_tab, data_tab, props_tab;
        QStringList colnames, rownames;
        tvResults->getVariantDataTable(QFEvaluationResultsModel::RDRPropertyRole, props_tab, colnames, rownames);
        tvResults->getVariantDataTable(QFEvaluationResultsModel::ValueRole, data_tab, colnames, rownames);
        tvResults->getVariantDataTable(QFEvaluationResultsModel::ResultIDRole, RDRIDs_tab, colnames, rownames);
        tvResults->getVariantDataTable(QFEvaluationResultsModel::ResultNameRole, resultnames_tab, colnames, rownames);
        tvResults->getVariantDataTable(QFEvaluationResultsModel::EvalNameRole, evalnames_tab, colnames, rownames);
        QList<int> rdrids_int;
        QStringList resultnames, evalnames, propnames;
        //qDebug()<<rownames;
        //qDebug()<<colnames;
        for (int c=0; c<data_tab.size(); c++) {
            //qDebug()<<"data_tab["<<c<<"] = "<<data_tab[c];
        }

        for (int c=0; c<RDRIDs_tab.size(); c++) {
            //qDebug()<<"RDRIDs_tab["<<c<<"] = "<<RDRIDs_tab[c];
            for (int r=0; r<RDRIDs_tab[c].size(); r++) {
                bool oki=false;
                int id=RDRIDs_tab[c].value(r, -1).toInt(&oki);
                if (!oki || !RDRIDs_tab[c].value(r).isValid()) id=-1;
                if (r<rdrids_int.size()) {
                    if (id>=0 && rdrids_int[r]<0) rdrids_int[r]=id;
                } else rdrids_int.append(id);
            }

        }
        for (int c=0; c<evalnames_tab.size(); c++) {
            //qDebug()<<"evalnames_tab["<<c<<"] = "<<evalnames_tab[c];
            QString id;
            for (int r=0; r<evalnames_tab[c].size(); r++) {
                QString idd=evalnames_tab[c].value(r, QString()).toString();
                if (r<evalnames.size()) {
                    if (!idd.isEmpty() && evalnames[r].isEmpty()) evalnames[r]=idd;
                } else evalnames.append(idd);
            }
        }
        for (int c=0; c<resultnames_tab.size(); c++) {
            QString id;
            //qDebug()<<"resultnames_tab["<<c<<"] = "<<resultnames_tab[c];
            for (int r=0; r<resultnames_tab[c].size(); r++) {
                QString idd=resultnames_tab[c].value(r, QString()).toString();
                if (id.isEmpty() && !idd.isEmpty()) id=idd;
                if (!id.isEmpty()) break;
            }
            resultnames.append(id);
        }
        for (int c=0; c<props_tab.size(); c++) {
            QString id;
            //qDebug()<<"props_tab["<<c<<"] = "<<props_tab[c];
            for (int r=0; r<props_tab[c].size(); r++) {
                QString idd=props_tab[c].value(r, QString()).toString();
                if (id.isEmpty() && !idd.isEmpty()) id=idd;
                if (!id.isEmpty()) break;
            }
            propnames.append(id);
        }
        int datarows=0;
        bool hasMask=false;
        bool hasSelection=false;
        bool hasArray=false;
        for (int c=0; c<RDRIDs_tab.size(); c++) {
            datarows=qMax(datarows, RDRIDs_tab[c].size());
            for (int r=0; r<RDRIDs_tab[c].size(); r++) {
                int id=RDRIDs_tab[c].value(r, -1).toInt();
                if (d->current->getProject()->rawDataIDExists(id))  {
                    QFRawDataRecord* rdr=d->current->getProject()->getRawDataByID(id);
                    QFRDRRunSelectionsInterface* seli=dynamic_cast<QFRDRRunSelectionsInterface*>(rdr);
                    QFRDRImageMaskInterface* maski=dynamic_cast<QFRDRImageMaskInterface*>(rdr);
                    //qDebug()<<c<<r<<id<<rdr<<seli<<maski;
                    if (rdr) {
                        if (seli) hasSelection=true;
                        if (maski) hasMask=true;
                        QFRawDataRecord::evaluationResultType restype=rdr->resultsGetType(evalnames_tab[c].value(r, QString()).toString(), resultnames_tab[c].value(r, QString()).toString());
                        if (QFRawDataRecord::evaluationResultTypeIsVector(restype) && QFRawDataRecord::evaluationResultTypeIsNumeric(restype)) hasArray=true;
                    }
                }
            }
        }

        //qDebug()<<"\n\n\n"<<rdrids_int<<"\n\n"<<evalnames<<"\n\n"<<resultnames<<"\n\n"<<datarows<<hasMask<<hasSelection<<hasArray<<"\n\n\n";
        if (rdrids_int.size()>0) {
            QFExportToTableDialog* dlg=new QFExportToTableDialog(d);
            QCheckBox* chkOverwriteTable=new QCheckBox(tr("(unchecked: append)"), dlg);
            chkOverwriteTable->setEnabled(true);
            dlg->addWidget(tr("overwrite existing table:"), chkOverwriteTable);
            QCheckBox* chkUseColExpressions=new QCheckBox(tr(""), dlg);
            chkUseColExpressions->setEnabled(true);
            dlg->addWidget(tr("prefer column expressions where possible:"), chkUseColExpressions);

            QCheckBox* chkUseMask=new QCheckBox(QString(""), dlg);
            dlg->addWidget(tr("mind excluded indexes/mask"), chkUseMask);
            chkUseMask->setEnabled(hasArray&&(hasMask));//||hasSelection));

            QComboBox* cmbSingleLineMode=new QComboBox(dlg);
            cmbSingleLineMode->addItem(tr("add statistics to table"));
            cmbSingleLineMode->addItem(tr("add data as column to table"));
            cmbSingleLineMode->setEnabled(hasArray && (datarows==1));
            dlg->addWidget(tr("single row mode:"), cmbSingleLineMode);

            QCheckBox* chkAvg=new QCheckBox(tr("average"), dlg);
            chkAvg->setEnabled(hasArray);
            dlg->addWidget(tr("vector statistics:"), chkAvg);
            QCheckBox* chkMedian=new QCheckBox(tr("median"), dlg);
            chkMedian->setEnabled(hasArray);
            dlg->addWidget("", chkMedian);
            QCheckBox* chkStd=new QCheckBox(tr("standard deviation"), dlg);
            chkStd->setEnabled(hasArray);
            dlg->addWidget("", chkStd);
            QCheckBox* chkMin=new QCheckBox(tr("minimum"), dlg);
            chkMin->setEnabled(hasArray);
            dlg->addWidget("", chkMin);
            QCheckBox* chkMax=new QCheckBox(tr("maximum"), dlg);
            chkMax->setEnabled(hasArray);
            dlg->addWidget("", chkMax);
            QCheckBox* chkQ25=new QCheckBox(tr("25% quartile"), dlg);
            chkQ25->setEnabled(hasArray);
            dlg->addWidget("", chkQ25);
            QCheckBox* chkQ75=new QCheckBox(tr("75% quartile"), dlg);
            chkQ75->setEnabled(hasArray);
            dlg->addWidget("", chkQ75);

            ProgramOptions::getConfigQComboBox(cmbSingleLineMode, QString("evaleditor/%1/cmbSingleLineMode").arg(d->current->getType()), 1);
            ProgramOptions::getConfigQCheckBox(chkOverwriteTable, QString("evaleditor/%1/chkOverwriteTable").arg(d->current->getType()), false);
            ProgramOptions::getConfigQCheckBox(chkUseColExpressions, QString("evaleditor/%1/chkUseColExpressions").arg(d->current->getType()), false);
            ProgramOptions::getConfigQCheckBox(chkUseMask, QString("evaleditor/%1/chkUseMask").arg(d->current->getType()), true);
            ProgramOptions::getConfigQCheckBox(chkMedian, QString("evaleditor/%1/chkMedian").arg(d->current->getType()), false);
            ProgramOptions::getConfigQCheckBox(chkAvg, QString("evaleditor/%1/chkAvg").arg(d->current->getType()), true);
            ProgramOptions::getConfigQCheckBox(chkStd, QString("evaleditor/%1/chkStd").arg(d->current->getType()), true);
            ProgramOptions::getConfigQCheckBox(chkMin, QString("evaleditor/%1/chkMin").arg(d->current->getType()), false);
            ProgramOptions::getConfigQCheckBox(chkMax, QString("evaleditor/%1/chkMax").arg(d->current->getType()), false);
            ProgramOptions::getConfigQCheckBox(chkQ25, QString("evaleditor/%1/chkQ25").arg(d->current->getType()), false);
            ProgramOptions::getConfigQCheckBox(chkQ75, QString("evaleditor/%1/chkQ75").arg(d->current->getType()), false);
            if (dlg->exec()) {
                ProgramOptions::setConfigQComboBox(cmbSingleLineMode, QString("evaleditor/%1/cmbSingleLineMode").arg(d->current->getType()));
                ProgramOptions::setConfigQCheckBox(chkOverwriteTable, QString("evaleditor/%1/chkOverwriteTable").arg(d->current->getType()));
                ProgramOptions::setConfigQCheckBox(chkUseColExpressions, QString("evaleditor/%1/chkUseColExpressions").arg(d->current->getType()));
                ProgramOptions::setConfigQCheckBox(chkUseMask, QString("evaleditor/%1/chkUseMask").arg(d->current->getType()));
                ProgramOptions::setConfigQCheckBox(chkMedian, QString("evaleditor/%1/chkMedian").arg(d->current->getType()));
                ProgramOptions::setConfigQCheckBox(chkAvg, QString("evaleditor/%1/chkAvg").arg(d->current->getType()));
                ProgramOptions::setConfigQCheckBox(chkStd, QString("evaleditor/%1/chkStd").arg(d->current->getType()));
                ProgramOptions::setConfigQCheckBox(chkMin, QString("evaleditor/%1/chkMin").arg(d->current->getType()));
                ProgramOptions::setConfigQCheckBox(chkMax, QString("evaleditor/%1/chkMax").arg(d->current->getType()));
                ProgramOptions::setConfigQCheckBox(chkQ25, QString("evaleditor/%1/chkQ25").arg(d->current->getType()));
                ProgramOptions::setConfigQCheckBox(chkQ75, QString("evaleditor/%1/chkQ75").arg(d->current->getType()));


                QString tabname="";
                QFRDRTableInterface* tab=dlg->getTable();
                QFRawDataRecord* rdr=dlg->getRDR();
                if (dlg->getNewTable(tabname)) {
                    if (tabname.isEmpty()) tabname=tr("NEW_TABLE");
                    rdr=QFPluginServices::getInstance()->getCurrentProject()->addRawData("table", tabname, "");
                    tab=dynamic_cast<QFRDRTableInterface*>(rdr);
                }
                bool ok=true;


                if (tab && tab->tableSupportsExpressions()) {
                    QString preexpr=tab->tableGetPreEvaluationExpression();
                    bool emitSigT=tab->tableGetDoEmitSignals();
                    tab->tableSetDoEmitSignals(false);

                    int col=tab->tableGetColumnCount();
                    if (chkOverwriteTable->isChecked()) col=0;//

                    int singlerow_rdrid=-1;
                    for (int i=0; i<rdrids_int.size(); i++) {
                        singlerow_rdrid=rdrids_int.value(0, -1);
                        if (singlerow_rdrid>=0) break;
                    }
                    QString singlerow_evalid="";
                    for (int i=0; i<evalnames.size(); i++) {
                        singlerow_evalid=evalnames.value(0, "");
                        if (!singlerow_evalid.isEmpty()) break;
                    }
                    QString singlerow_evalidvar=QString("EVALID%1").arg(col);
                    QString singlerow_rdridvar=QString("RDRID%1").arg(col);



                    /*QString stat_expression=QString("result_id = \"%1\";\nfor(r, 1, collength(col-%3), %4(rdr_getresult(data(r, col-%3), data(r, col-%2), result_id)))");
                    //QString prop_expression=QString("prop_id = \"%1\";\nfor(r, 1, collength(col-%2), rdr_getproperty(data(r, col-%2), prop_id)))");
                    QString prop_expression=QString("prop_id = \"%1\";\nrdr_getproperty(data(row, col-%2), prop_id)");
                    QString data_expression=QString("result_id = \"%1\";\nfor(r, 1, collength(col-%3), firstinvector(rdr_getresult(data(r, col-%3), data(r, col-%2), result_id)))");
                    QString error_expression=QString("result_id = \"%1\";\nfor(r, 1, collength(col-%3), firstinvector(rdr_getresulterror(data(r, col-%3), data(r, col-%2), result_id)))");
                    QString simple_data_expression=QString("result_id = \"%1\";\nrdr_getresult(%3, %2, result_id)");
                    QString simple_error_expression=QString("result_id = \"%1\";\nrdr_getresulterror(%3, %2, result_id)");
                    if (chkUseMask->isChecked() && (hasMask)) {
                        stat_expression=QString("result_id = \"%1\";\nfor(r, 1, collength(col-%3), %4(itemorfirst(rdr_getresult(data(r, col-%3), data(r, col-%2), result_id), runex_mask(data(r, col-%3)))))");
                        data_expression=QString("result_id = \"%1\";\nfor(r, 1, collength(col-%3), itemorfirst(rdr_getresult(data(r, col-%3), data(r, col-%2), result_id), runex_mask(data(r, col-%3))))");
                        error_expression=QString("result_id = \"%1\";\nfor(r, 1, collength(col-%3), itemorfirst(rdr_getresulterror(data(r, col-%3), data(r, col-%2), result_id), runex_mask(data(r, col-%3))))");
                        simple_data_expression=QString("result_id = \"%1\";\nitemorfirst(rdr_getresult(%3, %2, result_id), runex_mask(%3))");
                        simple_error_expression=QString("result_id = \"%1\";\nitemorfirst(rdr_getresulterror(%3, %2, result_id), runex_mask(%3))");
                    }*/

                    bool useColExpressionsForData=false;
                    QString stat_expression=QString("result_id = \"%1\";\n%4(rdr_getresult(data(row, col-%3), data(row, col-%2), result_id))");
                    QString prop_expression=QString("prop_id = \"%1\";\nrdr_getproperty(data(row, col-%2), prop_id)");
                    QString data_expression=QString("result_id = \"%1\";\nfirstinvector(rdr_getresult(data(row, col-%3), data(row, col-%2), result_id))");
                    QString error_expression=QString("result_id = \"%1\";\nfirstinvector(rdr_getresulterror(data(row, col-%3), data(row, col-%2), result_id))");
                    QString simple_data_expression=QString("result_id = \"%1\";\nrdr_getresult(%3, %2, result_id)");
                    QString simple_error_expression=QString("result_id = \"%1\";\nrdr_getresulterror(%3, %2, result_id)");
                    if (chkUseMask->isChecked() && (hasMask/* || hasSelection*/)) {
                        stat_expression=QString("result_id = \"%1\";\ncollength(col-%3), %4(itemorfirst(rdr_getresult(data(row, col-%3), data(row, col-%2), result_id), runex_mask(data(row, col-%3))))");
                        data_expression=QString("result_id = \"%1\";\ncollength(col-%3), itemorfirst(rdr_getresult(data(row, col-%3), data(row, col-%2), result_id), runex_mask(data(row, col-%3)))");
                        error_expression=QString("result_id = \"%1\";\ncollength(col-%3), itemorfirst(rdr_getresulterror(data(row, col-%3), data(row, col-%2), result_id), runex_mask(data(row, col-%3)))");
                        simple_data_expression=QString("result_id = \"%1\";\nitemorfirst(rdr_getresult(%3, %2, result_id), runex_mask(%3))");
                        simple_error_expression=QString("result_id = \"%1\";\nitemorfirst(rdr_getresulterror(%3, %2, result_id), runex_mask(%3))");
                    } else {
                        if (chkUseColExpressions->isChecked()){
                            useColExpressionsForData=true;
                            prop_expression=QString("prop_id = \"%1\";\nfor(c, column(col-%2), rdr_getproperty(c, prop_id))");
                            data_expression=QString("result_id = \"%1\";\nfor(r, 1:length(column(col-%3)), firstinvector(rdr_getresult(data(r, col-%3), data(r, col-%2), result_id, NAN)))");
                            error_expression=QString("result_id = \"%1\";\nfor(r, 1:length(column(col-%3)), firstinvector(rdr_getresulterror(data(r, col-%3), data(r, col-%2), result_id, NAN)))");
                            //simple_data_expression=QString("result_id = \"%1\";\nrdr_getresult(%3, %2, result_id)");
                            //simple_error_expression=QString("result_id = \"%1\";\nrdr_getresulterror(%3, %2, result_id)");
                        }
                    }


                    // set list of RDR IDs as first column
                    int rdrid_col=-1, evalid_col=-1;
                    if (datarows>1 || cmbSingleLineMode->currentIndex()==0) {
                        rdrid_col=col;
                        evalid_col=col+2;
                        tab->tableSetColumnTitle(col, dlg->getPrefix()+"RDR_ID");
                        tab->tableSetColumnExpression(col, QString());
                        tab->tableSetColumnTitle(col+1, dlg->getPrefix()+"FILENAME");
                        tab->tableSetColumnExpression(col+1, QString());
                        tab->tableSetColumnTitle(col+2, dlg->getPrefix()+"EVAL_ID");
                        tab->tableSetColumnExpression(col+2, QString());
                        if (chkUseColExpressions->isChecked()) {
                            tab->tableSetColumnExpression(col+2);
                        }
                        for (int r=0; r<rdrids_int.size(); r++) {
                            tab->tableSetData(r, col, rdrids_int[r]);
                            tab->tableSetData(r, col+2, evalnames[r]);
                            if (!chkUseColExpressions->isChecked()) tab->tableSetExpression(r, col+1, QLatin1String("rdr_getname(data(row, col-1))"));
                        }
                        if (chkUseColExpressions->isChecked()) {
                            tab->tableSetColumnExpression(col+1, QLatin1String("for(r, column(col-1), rdr_getname(r))"));
                        }

                        //tab->tableSetColumnExpression(col+1, QLatin1String("for(r, column(col-1), rdr_getname(r))"));
                        col+=3;

                        for (int c=0; c<resultnames.size(); c++) {
                            bool thisHasVector=false;
                            bool thisHasError=false;
                            for (int r=0; r<evalnames.size(); r++) {
                                QString evalid=evalnames[r];
                                QString resid=resultnames[c];
                                QFRawDataRecord* rdr=d->current->getProject()->getRawDataByID(rdrids_int.value(r, -1));
                                if (rdr) {
                                    QFRawDataRecord::evaluationResultType restype=rdr->resultsGetType(evalid, resid);
                                    if (QFRawDataRecord::evaluationResultTypeIsVector(restype) && QFRawDataRecord::evaluationResultTypeIsNumeric(restype)) thisHasVector=true;
                                    if (QFRawDataRecord::evaluationResultTypeHasError(restype)) thisHasError=true;
                                }
                            }

                            if (resultnames[c].isEmpty()) {
                                tab->tableSetColumnTitle(col, dlg->getPrefix()+colnames.value(c, ""));
                                if (propnames.value(c, "").isEmpty()) {
                                    for (int r=0; r<datarows; r++) {
                                        tab->tableSetData(r, col, data_tab.value(c, QList<QVariant>()).value(r, QVariant()));
                                    }
                                } else {
                                    if (!useColExpressionsForData) {
                                        for (int r=0; r<datarows; r++) {
                                            tab->tableSetExpression(r, col, prop_expression.arg(propnames[c]).arg(col-rdrid_col));
                                        }
                                    } else {
                                        tab->tableSetColumnExpression(col, prop_expression.arg(propnames[c]).arg(col-rdrid_col));
                                    }
                                }
                                col++;
                            } else {
                                QString resname=resultnames[c];
                                if (!thisHasVector) {
                                    tab->tableSetColumnTitle(col, dlg->getPrefix()+colnames.value(c, ""));
                                    //tab->tableSetColumnExpression(col, data_expression.arg(resname).arg(col-evalid_col).arg(col-rdrid_col));
                                    if (!useColExpressionsForData) {
                                        for (int r=0; r<datarows; r++) {
                                            tab->tableSetExpression(r, col, data_expression.arg(resname).arg(col-evalid_col).arg(col-rdrid_col));
                                        }
                                    } else {
                                        tab->tableSetColumnExpression(col, data_expression.arg(resname).arg(col-evalid_col).arg(col-rdrid_col));
                                    }
                                    col++;
                                } else {
                                    if (chkAvg->isChecked()) { QString instr="mean";
                                        tab->tableSetColumnTitle(col, dlg->getPrefix()+QString("%2(%1)").arg(colnames.value(c, "")).arg(instr));
                                        //tab->tableSetColumnExpression(col, stat_expression.arg(resname).arg(col-evalid_col).arg(col-rdrid_col).arg(instr));
                                        for (int r=0; r<datarows; r++) {
                                            tab->tableSetExpression(r, col, stat_expression.arg(resname).arg(col-evalid_col).arg(col-rdrid_col).arg(instr));
                                        }
                                        col++;
                                    }
                                    if (chkStd->isChecked()) { QString instr="std";
                                        tab->tableSetColumnTitle(col, dlg->getPrefix()+QString("%2(%1)").arg(colnames.value(c, "")).arg(instr));
                                        //tab->tableSetColumnExpression(col, stat_expression.arg(resname).arg(col-evalid_col).arg(col-rdrid_col).arg(instr));
                                        for (int r=0; r<datarows; r++) {
                                            tab->tableSetExpression(r, col, stat_expression.arg(resname).arg(col-evalid_col).arg(col-rdrid_col).arg(instr));
                                        }
                                        col++;
                                    }
                                    if (chkMedian->isChecked()) { QString instr="median";
                                        tab->tableSetColumnTitle(col, dlg->getPrefix()+QString("%2(%1)").arg(colnames.value(c, "")).arg(instr));
                                        //tab->tableSetColumnExpression(col, stat_expression.arg(resname).arg(col-evalid_col).arg(col-rdrid_col).arg(instr));
                                        for (int r=0; r<datarows; r++) {
                                            tab->tableSetExpression(r, col, stat_expression.arg(resname).arg(col-evalid_col).arg(col-rdrid_col).arg(instr));
                                        }

                                        col++;
                                    }
                                    if (chkMin->isChecked()) { QString instr="min";
                                        tab->tableSetColumnTitle(col, dlg->getPrefix()+QString("%2(%1)").arg(colnames.value(c, "")).arg(instr));
                                        //tab->tableSetColumnExpression(col, stat_expression.arg(resname).arg(col-evalid_col).arg(col-rdrid_col).arg(instr));
                                        for (int r=0; r<datarows; r++) {
                                            tab->tableSetExpression(r, col, stat_expression.arg(resname).arg(col-evalid_col).arg(col-rdrid_col).arg(instr));
                                        }

                                        col++;
                                    }
                                    if (chkMax->isChecked()) { QString instr="max";
                                        tab->tableSetColumnTitle(col, dlg->getPrefix()+QString("%2(%1)").arg(colnames.value(c, "")).arg(instr));
                                        //tab->tableSetColumnExpression(col, stat_expression.arg(resname).arg(col-evalid_col).arg(col-rdrid_col).arg(instr));
                                        for (int r=0; r<datarows; r++) {
                                            tab->tableSetExpression(r, col, stat_expression.arg(resname).arg(col-evalid_col).arg(col-rdrid_col).arg(instr));
                                        }

                                        col++;
                                    }
                                    if (chkQ25->isChecked()) { QString instr="quantile25";
                                        tab->tableSetColumnTitle(col, dlg->getPrefix()+QString("%2(%1)").arg(colnames.value(c, "")).arg(instr));
                                        //tab->tableSetColumnExpression(col, stat_expression.arg(resname).arg(col-evalid_col).arg(col-rdrid_col).arg(instr));
                                        for (int r=0; r<datarows; r++) {
                                            tab->tableSetExpression(r, col, stat_expression.arg(resname).arg(col-evalid_col).arg(col-rdrid_col).arg(instr));
                                        }

                                        col++;
                                    }
                                    if (chkQ75->isChecked()) { QString instr="quantile75";
                                        tab->tableSetColumnTitle(col, dlg->getPrefix()+QString("%2(%1)").arg(colnames.value(c, "")).arg(instr));
                                        //tab->tableSetColumnExpression(col, stat_expression.arg(resname).arg(col-evalid_col).arg(col-rdrid_col).arg(instr));
                                        for (int r=0; r<datarows; r++) {
                                            tab->tableSetExpression(r, col, stat_expression.arg(resname).arg(col-evalid_col).arg(col-rdrid_col).arg(instr));
                                        }

                                        col++;
                                    }
                                }
                                if (thisHasError) {
                                    if (!thisHasVector) {
                                        tab->tableSetColumnTitle(col, dlg->getPrefix()+QString("error(%1)").arg(colnames.value(c, "")));
                                        //tab->tableSetColumnExpression(col, error_expression.arg(resname).arg(col-evalid_col).arg(col-rdrid_col));
                                        if (!useColExpressionsForData) {
                                            for (int r=0; r<datarows; r++) {
                                                tab->tableSetExpression(r, col, error_expression.arg(resname).arg(col-evalid_col).arg(col-rdrid_col));
                                            }
                                        } else {
                                            tab->tableSetColumnExpression(col, error_expression.arg(resname).arg(col-evalid_col).arg(col-rdrid_col));
                                        }

                                        col++;
                                    }
                                }
                            }
                        }

                    } else if (datarows==1){
                        if (!preexpr.isEmpty()) preexpr+=";\n";
                        preexpr+=QString("%1 = %2;\n%3 = \"%4\";\n").arg(singlerow_rdridvar).arg(singlerow_rdrid).arg(singlerow_evalidvar).arg(singlerow_evalid);

                        for (int c=0; c<resultnames.size(); c++) {
                            //bool thisHasVector=false;
                            bool thisHasError=false;
                            int r=0;
                            QString evalid=evalnames[r];
                            QString resid=resultnames[c];
                            QFRawDataRecord* rdr=d->current->getProject()->getRawDataByID(rdrids_int.value(0, -1));
                            if (rdr) {
                                QFRawDataRecord::evaluationResultType restype=rdr->resultsGetType(evalid, resid);
                                //if (QFRawDataRecord::evaluationResultTypeIsVector(restype) && QFRawDataRecord::evaluationResultTypeIsNumeric(restype)) thisHasVector=true;
                                if (QFRawDataRecord::evaluationResultTypeHasError(restype)) thisHasError=true;
                            }


                            if (resultnames[c].isEmpty()) {
                                tab->tableSetColumnTitle(col, dlg->getPrefix()+colnames.value(c, ""));
                                for (int r=0; r<datarows; r++) {
                                    tab->tableSetData(r, col, data_tab.value(c, QList<QVariant>()).value(r, QVariant()));
                                }
                                col++;
                            } else {
                                QString resname=resultnames[c];
                                tab->tableSetColumnTitle(col, dlg->getPrefix()+colnames.value(c, ""));
                                //tab->tableSetColumnExpression(col, simple_data_expression.arg(resname).arg(singlerow_evalidvar).arg(singlerow_rdridvar));
                                for (int r=0; r<datarows; r++) {
                                    tab->tableSetExpression(r, col, simple_data_expression.arg(resname).arg(singlerow_evalidvar).arg(singlerow_rdridvar));
                                }

                                col++;
                                if (thisHasError) {
                                    tab->tableSetColumnTitle(col, dlg->getPrefix()+colnames.value(c, ""));
                                   // tab->tableSetColumnExpression(col, simple_error_expression.arg(resname).arg(singlerow_evalidvar).arg(singlerow_rdridvar));
                                    for (int r=0; r<datarows; r++) {
                                        tab->tableSetExpression(r, col, simple_error_expression.arg(resname).arg(singlerow_evalidvar).arg(singlerow_rdridvar));
                                    }

                                    col++;
                                }
                            }
                        }
                    }

                    tab->tableSetPreEvaluationExpression(preexpr);
                    tab->tableReevaluateExpressions();
                    tab->tableSetDoEmitSignals(emitSigT);
                } else {
                    QMessageBox::critical(d, tr("Add data to table"), tr("No table selected or could not create table, or table does not support expressions!"));
                    ok=false;
                }

                if (ok && rdr && dlg->getShowEditor()) {
                    QFRawDataPropertyEditor* editor=QFPluginServices::getInstance()->openRawDataEditor(rdr, false);
                    editor->showTab(1);

                }
            }
            delete dlg;
        }

    }
}
