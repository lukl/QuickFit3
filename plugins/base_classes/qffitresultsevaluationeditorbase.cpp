/*
    Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center (DKFZ) & IWR, University of Heidelberg

    

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

#include "qffitresultsevaluationeditorbase.h"
#include "qffitresultsevaluation.h"
#include "qmodernprogresswidget.h"
#include "qftools.h"

/////////////////////////////////////////////////////////////////////
// CONSTRUCTOR/DESTRUCTOR/INITIALISATION
/////////////////////////////////////////////////////////////////////

QFFitResultsEvaluationEditorBase::QFFitResultsEvaluationEditorBase(QString iniPrefix, QFPluginServices* services, QFEvaluationPropertyEditor* propEditor, QWidget* parent) :
    QFEvaluationEditor(services, propEditor, parent)
{
    m_iniPrefix=iniPrefix;
    m_currentFPSSaveDir=ProgramOptions::getInstance()->getHomeQFDirectory();
    m_currentSaveDirectory="";
    actSaveReport=new QFActionWithNoMenuRole(QIcon(":/lib/savereport.png"), tr("Save Report"), this);
    connect(actSaveReport, SIGNAL(triggered()), this, SLOT(saveReport()));
    actPrintReport=new QFActionWithNoMenuRole(QIcon(":/lib/printreport.png"), tr("Print Report"), this);
    connect(actPrintReport, SIGNAL(triggered()), this, SLOT(printReport()));
}


QFFitResultsEvaluationEditorBase::~QFFitResultsEvaluationEditorBase() {

}








/////////////////////////////////////////////////////////////////////
// READING & WRITING PROPERTIES
/////////////////////////////////////////////////////////////////////

void QFFitResultsEvaluationEditorBase::setIniPrefix(QString p) {
    m_iniPrefix=p;
}

QString QFFitResultsEvaluationEditorBase::iniPrefix() const {
    return m_iniPrefix;
}

QString QFFitResultsEvaluationEditorBase::currentFPSSaveDir() const {
    return m_currentFPSSaveDir;
}

void QFFitResultsEvaluationEditorBase::setCurrentFPSSaveDir(QString d) {
    m_currentFPSSaveDir=d;
}

QString QFFitResultsEvaluationEditorBase::currentSaveDirectory() const {
    return m_currentSaveDirectory;
}

void QFFitResultsEvaluationEditorBase::setCurrentSaveDirectory(QString d) {
    m_currentSaveDirectory=d;
}













/////////////////////////////////////////////////////////////////////
// SETTINGS STORAGE
/////////////////////////////////////////////////////////////////////

void QFFitResultsEvaluationEditorBase::readSettings() {
    m_currentFPSSaveDir=settings->getQSettings()->value(m_iniPrefix+"lastFPSDirectory", m_currentFPSSaveDir).toString();
    m_currentSaveDirectory=settings->getQSettings()->value(m_iniPrefix+"lastSaveDirectory", m_currentSaveDirectory).toString();
}

void QFFitResultsEvaluationEditorBase::writeSettings() {
    settings->getQSettings()->setValue(m_iniPrefix+"lastFPSDirectory", m_currentFPSSaveDir);
    settings->getQSettings()->setValue(m_iniPrefix+"lastSaveDirectory", m_currentSaveDirectory);
}














/////////////////////////////////////////////////////////////////////
// READING & WRITING (CURRENT) FIT RESULTS TO INI
/////////////////////////////////////////////////////////////////////

void QFFitResultsEvaluationEditorBase::saveCurrentFitResults(QFRawDataRecord* rec) {
    if (!current) return;
    QFFitResultsEvaluation* eval=qobject_cast<QFFitResultsEvaluation*>(current);
    QFFitFunction* ffunc=eval->getFitFunction(rec);
    if (!ffunc || !eval) return;


    QString filter= tr("Fit Parameter Set (*.fps)");
    QString selectedFilter=filter;
    QString fileName = qfGetSaveFileName(this, tr("Save Fit Parameter Set as ..."), m_currentFPSSaveDir, filter, &selectedFilter);
    if ((!fileName.isEmpty())&&(!fileName.isNull())) {
        m_currentFPSSaveDir=QFileInfo(fileName).absolutePath();
        bool ok=true;
        if (QFile::exists(fileName)) {
            int ret = QMessageBox::question(this, tr("Save Fit Parameter Set as  ..."),
                            tr("A Configuration with the name '%1' already exists.\n"
                               "Do you want to overwrite?").arg(fileName),
                            QMessageBox::Yes | QMessageBox::No,  QMessageBox::No);
            if (ret==QMessageBox::No) ok=false;
        }
        if (ok) {
            QSettings settings(fileName, QSettings::IniFormat);
            settings.setValue("fit_function/id", eval->getFitFunction(rec)->id());

            QStringList pids=ffunc->getParameterIDs();
            double* fullParams=eval->allocFillParameters(ffunc);
            double* errors=eval->allocFillParameterErrors(ffunc);
            bool* paramsFix=eval->allocFillFix(ffunc);
            double* paramsMin=eval->allocFillParametersMin(NULL, ffunc);
            double* paramsMax=eval->allocFillParametersMax(NULL, ffunc);
            //ffunc->calcParameter(fullParams, errors);
            for (int i=0; i<pids.size(); i++) {
                QString id=pids[i];
                int num=ffunc->getParameterNum(id);
                if (!id.isEmpty()) {
                    bool visible=ffunc->isParameterVisible(num, fullParams);
                    QFFitFunction::ParameterDescription d=ffunc->getDescription(id);
                    if (!d.userEditable) visible=false;

                    if (visible) {
                        settings.setValue("fit_params/"+id+"/value", eval->getFitValue(id, rec));
                        if (d.displayError==QFFitFunction::EditError) settings.setValue("fit_params/"+id+"/error", eval->getFitError(id, rec));
                        if (d.fit) settings.setValue("fit_params/"+id+"/fix", eval->getFitFix(id, rec));
                        if (d.userRangeEditable) {
                           settings.setValue("fit_params/"+id+"/min", eval->getFitMin(id, rec));
                           settings.setValue("fit_params/"+id+"/max", eval->getFitMax(id, rec));
                        }
                    }
                }
            }
            qfFree(fullParams);
            qfFree(errors);
            qfFree(paramsFix);
            qfFree(paramsMin);
            qfFree(paramsMax);
        }
    }
}

void QFFitResultsEvaluationEditorBase::loadCurrentFitResults(QFRawDataRecord* rec) {
    if (!current) return;
    QFFitResultsEvaluation* eval=qobject_cast<QFFitResultsEvaluation*>(current);
    QFFitFunction* ffunc=eval->getFitFunction(rec);
    if (!ffunc || !eval) return;

    QString filter= tr("Fit Parameter Set (*.fps)");
    QString selectedFilter=filter;
    QString fileName = qfGetOpenFileName(this, tr("Load Fit Parameter Set ..."), m_currentFPSSaveDir, filter, &selectedFilter);
    if ((!fileName.isEmpty())&&(!fileName.isNull())) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        eval->set_doEmitPropertiesChanged(false);
        m_currentFPSSaveDir=QFileInfo(fileName).absolutePath();
        QSettings settings(fileName, QSettings::IniFormat);
        QString ffuncname=settings.value("fit_function/id", eval->getFitFunction(rec)->id()).toString();
        eval->setFitFunction(ffuncname);
        settings.beginGroup("fit_params");
        QStringList keys = settings.allKeys();
        for (int i=0; i<keys.size(); i++) {
            if (keys[i].endsWith("/error")) {
                QString paramname=keys[i].left(keys[i].length()-6);
                eval->setFitError(paramname, settings.value(keys[i]).toDouble(), rec);
            } else if (keys[i].endsWith("/value")) {
                QString paramname=keys[i].left(keys[i].length()-6);
                eval->setFitValue(paramname, settings.value(keys[i]).toDouble(), rec);
            } else if (keys[i].endsWith("/fix")) {
                QString paramname=keys[i].left(keys[i].length()-4);
                eval->setFitFix(paramname, settings.value(keys[i]).toBool(), rec);
            } else if (keys[i].endsWith("/min")) {
                QString paramname=keys[i].left(keys[i].length()-4);
                eval->setFitMin(paramname, settings.value(keys[i]).toDouble(), rec);
            } else if (keys[i].endsWith("/max")) {
                QString paramname=keys[i].left(keys[i].length()-4);
                eval->setFitMax(paramname, settings.value(keys[i]).toDouble(), rec);
            }
        }
        eval->set_doEmitPropertiesChanged(true);
        eval->emitPropertiesChanged();
        displayModel(true);
        replotData();
        QApplication::restoreOverrideCursor();
    }
}

void QFFitResultsEvaluationEditorBase::saveCurrentFitResults()
{
    QFFitResultsEvaluation* eval=qobject_cast<QFFitResultsEvaluation*>(current);
    if (eval) saveCurrentFitResults(eval->getHighlightedRecord());
}

void QFFitResultsEvaluationEditorBase::loadCurrentFitResults()
{
    QFFitResultsEvaluation* eval=qobject_cast<QFFitResultsEvaluation*>(current);
    if (eval) loadCurrentFitResults(eval->getHighlightedRecord());
}













/////////////////////////////////////////////////////////////////////
// REPORT GENERATION
/////////////////////////////////////////////////////////////////////


void QFFitResultsEvaluationEditorBase::createReportDoc(QTextDocument* /*document*/) {

}












void QFFitResultsEvaluationEditorBase::resetCurrent() {
    if (!current) return;
    QFFitResultsEvaluation* eval=qobject_cast<QFFitResultsEvaluation*>(current);
    if (!eval) return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    eval->resetAllFitResultsCurrent();
    displayModel(false);
    replotData();
    QApplication::restoreOverrideCursor();
}

void QFFitResultsEvaluationEditorBase::resetAll() {
    if (!current) return;
    QFFitResultsEvaluation* eval=qobject_cast<QFFitResultsEvaluation*>(current);
    if (!eval) return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    eval->resetAllFitResultsAllFiles();
    displayModel(false);
    replotData();
    QApplication::restoreOverrideCursor();
}



void QFFitResultsEvaluationEditorBase::copyToAll(QFRawDataRecord* rec) {

    if (!current) return;
    QFFitResultsEvaluation* eval=qobject_cast<QFFitResultsEvaluation*>(current);
    if (!eval) return;
    copyToInitial(false, rec);


    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    double* params=eval->allocFillParameters(eval->getFitFunction(rec));

    QList<QPointer<QFRawDataRecord> > recs=eval->getApplicableRecords();

    eval->set_doEmitResultsChanged(false);
    eval->set_doEmitPropertiesChanged(false);


    for (int i=0; i<recs.size(); i++) {
        QFRawDataRecord* record=recs[i];
        if (record) {
            QFFitFunction* ffunc=eval->getFitFunction(record);
            record->disableEmitResultsChanged();
            for (int i=0; i<ffunc->paramCount(); i++) {
                QString id=ffunc->getParameterID(i);
                double value=eval->getFitValue(id, record);
                double error=eval->getFitError(id, record);
                bool fix=eval->getFitFix(id, record);
                if (ffunc->isParameterVisible(i, params)) {
                    if (eval->hasFit(record)) {
                        eval->setFitResultValue(record, eval->getEvaluationResultID(record), id, value, error);
                        eval->setFitResultFix(record, eval->getEvaluationResultID(record), id, fix);
                    }
                }
            }
            record->enableEmitResultsChanged();
        };
    }

    eval->set_doEmitResultsChanged(true);
    eval->set_doEmitPropertiesChanged(true);
    eval->emitResultsChanged();

    qfFree(params);
    QApplication::restoreOverrideCursor();
}



void QFFitResultsEvaluationEditorBase::copyToInitial(bool emitSignals, QFRawDataRecord *rec) {
    if (!current) return;
    QFFitResultsEvaluation* eval=qobject_cast<QFFitResultsEvaluation*>(current);
    if (!eval) return;

    QFFitFunction* ffunc=eval->getFitFunction(rec);

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    double* params=eval->allocFillParameters(ffunc);

    eval->set_doEmitResultsChanged(false);
    eval->set_doEmitPropertiesChanged(false);

    for (int i=0; i<ffunc->paramCount(); i++) {
        QString id=ffunc->getParameterID(i);
        double value=eval->getFitValue(id);
        double error=eval->getFitError(id);
        bool fix=eval->getFitFix(id);
        if (ffunc->isParameterVisible(i, params)) {
            eval->setInitFitFix(id, fix);
            eval->setInitFitValue(id, value, error);
        };
    }


    eval->set_doEmitResultsChanged(true);
    eval->set_doEmitPropertiesChanged(true);
    if (emitSignals) eval->emitResultsChanged();

    qfFree(params);
    QApplication::restoreOverrideCursor();
}






int QFFitResultsEvaluationEditorBase::getUserMin(QFRawDataRecord* rec, int defaultMin) {
    QFFitResultsEvaluation* data=qobject_cast<QFFitResultsEvaluation*>(current);
    if (!data) return defaultMin;
    const QString resultID=data->getEvaluationResultID(rec);

    // WORKROUND FOR OLD PROPERTY NAMES
    int defaultM=rec->getProperty(QString(resultID+"_datacut_min").replace(QString("_")+data->getFitFunctionID()+QString("_run"), "_r").replace(data->getType()+"_", data->getType()), defaultMin).toInt();

    return rec->getProperty(resultID+"_datacut_min", defaultM).toInt();
}

int QFFitResultsEvaluationEditorBase::getUserMax(QFRawDataRecord* rec, int defaultMax) {
    QFFitResultsEvaluation* data=qobject_cast<QFFitResultsEvaluation*>(current);
    if (!data) return defaultMax;
    const QString resultID=data->getEvaluationResultID(rec);

    // WORKROUND FOR OLD PROPERTY NAMES
    int defaultM=rec->getProperty(QString(resultID+"_datacut_max").replace(QString("_")+data->getFitFunctionID()+QString("_run"), "_r").replace(data->getType()+"_", data->getType()), defaultMax).toInt();

    return rec->getProperty(resultID+"_datacut_max", defaultM).toInt();
}

void QFFitResultsEvaluationEditorBase::connectDefaultWidgets(QFEvaluationItem */*current*/, QFEvaluationItem */*old*/, bool /*updatePlots*/)
{

}

int QFFitResultsEvaluationEditorBase::getUserMin(int defaultMin) {
    QFFitResultsEvaluation* data=qobject_cast<QFFitResultsEvaluation*>(current);
    if (!data) return defaultMin;
    QFRawDataRecord* rdr=data->getHighlightedRecord();
    if (rdr) {
        const QString resultID=data->getEvaluationResultID(rdr);

        // WORKROUND FOR OLD PROPERTY NAMES
        int defaultM=rdr->getProperty(QString(resultID+"_datacut_min").replace(QString("_")+data->getFitFunctionID()+QString("_run"), "_r").replace(data->getType()+"_", data->getType()), defaultMin).toInt();

        return rdr->getProperty(resultID+"_datacut_min", defaultM).toInt();
    } else {
        return defaultMin;
    }
}

int QFFitResultsEvaluationEditorBase::getUserMax(int defaultMax) {
    QFFitResultsEvaluation* data=qobject_cast<QFFitResultsEvaluation*>(current);
    if (!data) return defaultMax;
    QFRawDataRecord* rdr=data->getHighlightedRecord();
    if (rdr) {
        const QString resultID=data->getEvaluationResultID(rdr);



        // WORKROUND FOR OLD PROPERTY NAMES
        int defaultM=rdr->getProperty(QString(resultID+"_datacut_max").replace(QString("_")+data->getFitFunctionID()+QString("_run"), "_r").replace(data->getType()+"_", data->getType()), defaultMax).toInt();

        return rdr->getProperty(resultID+"_datacut_max", defaultM).toInt();
    } else {
        return defaultMax;
    }
}


void QFFitResultsEvaluationEditorBase::setUserMin(int userMin) {
    QFFitResultsEvaluation* data=qobject_cast<QFFitResultsEvaluation*>(current);
    if (!data) return;
    QFRawDataRecord* rdr=data->getHighlightedRecord();
    const QString resultID=data->getEvaluationResultID(rdr);
    rdr->setQFProperty(resultID+"_datacut_min", userMin, false, false);
}

void QFFitResultsEvaluationEditorBase::setUserMax(int userMax) {
    if (!current) return;
    QFFitResultsEvaluation* data=qobject_cast<QFFitResultsEvaluation*>(current);
    if (!data) return;
    QFRawDataRecord* rdr=data->getHighlightedRecord();
    const QString resultID=data->getEvaluationResultID(rdr);
    rdr->setQFProperty(resultID+"_datacut_max", userMax, false, false);
}

void QFFitResultsEvaluationEditorBase::setUserMinMax(int userMin, int userMax) {
    if (!current) return;
    QFFitResultsEvaluation* data=qobject_cast<QFFitResultsEvaluation*>(current);
    if (!data) return;
    QFRawDataRecord* rdr=data->getHighlightedRecord();
    const QString resultID=data->getEvaluationResultID(rdr);
    rdr->disableEmitPropertiesChanged();
    rdr->setQFProperty(resultID+"_datacut_min", userMin, false, false);
    rdr->setQFProperty(resultID+"_datacut_max", userMax, false, false);
    rdr->enableEmitPropertiesChanged(true);
}
