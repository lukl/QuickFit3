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

#include "qfimfccsfitevaluation_item.h"
#include "qfimfccsfitevaluation_editor.h"
#include "qfrdrfcsdatainterface.h"
#include "qffitfunction.h"
#include "qftools.h"
#include "qffcstools.h"
#include "qfprojectrawdatamodel.h"
#include "qfglobalfittool.h"
#include "qfrdrrunselection.h"
#include "qffitfunctionmanager.h"

QFImFCCSMatchRDRFunctor::QFImFCCSMatchRDRFunctor()
{
}

bool QFImFCCSMatchRDRFunctor::matches(const QFRawDataRecord *record) const
{

    return record && record->inherits("QFRDRFCSDataInterface") && record->inherits("QFRDRImageToRunInterface");
}

QFImFCCSFitEvaluationItem::QFImFCCSFitEvaluationItem(QFProject* parent):
    QFFitResultsByIndexAsVectorEvaluation("fcs_,dls_,fccs_", parent, false, false),
    QFFitResultsByIndexMultiRDREvaluationFitTools(),
    QFFCSWeightingTools()
{
    connect(project, SIGNAL(recordAboutToBeDeleted(QFRawDataRecord*)), this, SLOT(checkAndCleanFitFileSets()));
    qRegisterMetaType<QList<QFRawDataRecord*> >("QList<QFRawDataRecord*>");
    getEvaluationResultIDUsesFitFunction=true;
    matchFunctor=new QFImFCCSMatchRDRFunctor();
    m_weighting=RunErrorWeighting;
    //m_currentIndex=-1;
    m_multiFitFunctions.clear();
    globalParamsChanged=true;
    lastGlobalParamsCount=0;
    doFitThread=new QFGlobalThreadedFit(this);
    //mutexThreadedFit=new QMutex(QMutex::Recursive);

    if (m_fitFunctions.contains("fcs_spim_diffc")) {
        m_fitFunction="fcs_spim_diff";
    }/* else if (m_fitFunctions.contains("fcs_diff1")) {
        m_fitFunction="fcs_diff1";
    }*/

    if (m_fitAlgorithms.contains("fit_lmfit")) {
        m_fitAlgorithm="fit_lmfit";
    }

    m_multiFitFunctions[0]=m_fitFunction;
    QList<QPointer<QFRawDataRecord> > recs=getApplicableRecords();
    if (recs.size()>0) fitFilesList.append(recs[0]);

    rdrProxy=new QFEvaluationRawDataModelProxy(this);
    rdrProxy->setSourceModel(project->getRawDataModel());
    rdrProxy->setEvaluation(this);

    paramTable=new QFImFCCSParameterInputTable(this);
    connect(this, SIGNAL(parameterStructureChanged()), paramTable, SLOT(rebuildModel()));
    connect(this, SIGNAL(currentIndexChanged(int)), paramTable, SLOT(rebuildModel()));

    fittedFileSetsModel=new QFImFCCSFileSetsModel(this);
    connect(this, SIGNAL(filesetsChanged()), fittedFileSetsModel, SLOT(rebuildModel()));
    ensureFitFiles();
}

QFImFCCSFitEvaluationItem::~QFImFCCSFitEvaluationItem() {
    delete matchFunctor;
}



QFEvaluationEditor* QFImFCCSFitEvaluationItem::createEditor(QFPluginServices* services,  QFEvaluationPropertyEditor *propEditor, QWidget* parent) {
    return new QFImFCCSFitEvaluationEditor(services, propEditor, parent);
}

int QFImFCCSFitEvaluationItem::getIndexMin(const QFRawDataRecord */*r*/) const
{
    return -1;
}

int QFImFCCSFitEvaluationItem::getIndexMax(const QFRawDataRecord *r) const
{
    if (!r) return -1;
    QFRDRFCSDataInterface* fcs=qobject_cast<QFRDRFCSDataInterface*>(r);
    if (fcs->getCorrelationRuns()<=0) return -1;
    else return fcs->getCorrelationRuns()-1;
}

QFFitStatistics QFImFCCSFitEvaluationItem::calcFitStatistics(bool storeAsResults, QFFitFunction *ffunc, long N, const double *tauvals, const double *corrdata, const double *weights, int datacut_min, int datacut_max, const double *fullParams, const double *errors, const bool *paramsFix, int runAvgWidth, int residualHistogramBins, QFRawDataRecord *record, int run, const QString &/*prefix*/, const QString &/*pgroup*/, const QVector<double>& COV, double paramrange_size, bool storeCOV, QStringList *returnFitParamNames)
{
    return QFFitResultsByIndexEvaluationFitToolsBase::calcFitStatistics(storeAsResults, ffunc,  N, tauvals, corrdata, weights,  datacut_min,  datacut_max, fullParams, errors, paramsFix,  runAvgWidth,  residualHistogramBins, record,  run, QString("fitstat_local_"), tr("local fit statistics"), COV, paramrange_size, storeCOV, returnFitParamNames);
}

QFEvaluationRawDataModelProxy *QFImFCCSFitEvaluationItem::getRawDataProxyModel() const
{
    return rdrProxy;
}

QFRawDataRecord *QFImFCCSFitEvaluationItem::getFitFile(int num) const
{
    return fitFilesList.value(num, NULL);
}

QString QFImFCCSFitEvaluationItem::getFitFunctionID(int num) const
{
    return m_multiFitFunctions.value(num, m_fitFunction);
}

QFFitFunction* QFImFCCSFitEvaluationItem::getFitFunction(int num) const
{
    return getFitFunction(getFitFunctionID(num));
}

QStringList QFImFCCSFitEvaluationItem::getFitFunctionIDs() const
{
    QStringList res;
    for (int i=0; i<getFitFileCount(); i++) {
        res.append(getFitFunctionID(i));
    }
    return res;
}

QFFitFunction *QFImFCCSFitEvaluationItem::getFitFunctionForID(QString id) const
{
    return getFitFunction(id);
}

void QFImFCCSFitEvaluationItem::setFitFunction(int num, QString fitFunction)
{
    m_multiFitFunctions[num]=fitFunction;
    emit parameterStructureChanged();
    setDataChanged();
}

void QFImFCCSFitEvaluationItem::setFitFunction(QString fitFunction)
{
    setFitFunction(0,fitFunction);
}

int QFImFCCSFitEvaluationItem::addFitFile()
{
    QList<QPointer<QFRawDataRecord> > recs=getApplicableRecords();
    if (fitFilesList.size()<=0)  {
        if (recs.size()>0) fitFilesList.append(recs[0]);
    } else {
        QFRawDataRecord* next=fitFilesList.last()->getNextOfSameType();
        if (!next) next=fitFilesList[0];
        if (next) fitFilesList.append(next);
    }
    setDataChanged();
    paramTable->rebuildModel();
    if (fitFilesList.size()>0) emit fileChanged(fitFilesList.size()-1, fitFilesList.last());

    return fitFilesList.size()-1;
}

void QFImFCCSFitEvaluationItem::removeFitFile()
{
    if (fitFilesList.size()>1) {
        fitFilesList.removeLast();
        if (fitFilesList.size()>0) emit fileChanged(0, fitFilesList.first());
        paramTable->rebuildModel();
        setDataChanged();
    }

}

void QFImFCCSFitEvaluationItem::removeFitFile(int i)
{
    if (i>=0 && i<fitFilesList.size() && fitFilesList.size()>1) {
        fitFilesList.removeAt(i);
        if (fitFilesList.size()>0) emit fileChanged(0, fitFilesList.first());
        paramTable->rebuildModel();
        setDataChanged();
    }
}

void QFImFCCSFitEvaluationItem::setCurrentIndex(int index)
{
    setCurrentIndexM(index);
    QFRawDataRecord* r=getFitFile(0);
    if (r) {
        if (index<getIndexMin(r)) setCurrentIndexM(getIndexMin(r));
        if (index>getIndexMax(r)) setCurrentIndexM(getIndexMax(r));
        r->setQFProperty(QString(getType()+QString::number(getID())+"_last_index"), getCurrentIndexM(), false, false);
        emit currentIndexChanged(getCurrentIndexM());
    }
}

void QFImFCCSFitEvaluationItem::setLinkParameter(int file, QString parameter, int global_param)
{
    if (global_param>=0) {
        int old=globalParams[file].value(parameter, -1);
        globalParams[file].insert(parameter, global_param);
        if (old!=global_param) setDataChanged();
    } else {
        bool change=globalParams[file].contains(parameter);
        globalParams[file].remove(parameter);
        if (change) setDataChanged();
    }
    globalParamsChanged=true;
}

void QFImFCCSFitEvaluationItem::unsetLinkParameter(int file, QString parameter)
{
    setLinkParameter(file, parameter, -1);
}

void QFImFCCSFitEvaluationItem::clearLinkParameters(int file)
{
    if (file<0) {
        globalParams.clear();
    } else {
        globalParams.remove(file);
    }
    globalParamsChanged=true;
    setDataChanged();
}


void QFImFCCSFitEvaluationItem::ensureFitFiles()
{
    if (fitFilesList.size()<=0)  {
        QList<QPointer<QFRawDataRecord> > recs=getApplicableRecords();
        if (recs.size()>0) {
            fitFilesList.append(recs[0]);
            paramTable->rebuildModel();
            setDataChanged();
        }
    }
}

void QFImFCCSFitEvaluationItem::checkAndCleanFitFileSets()
{
   bool removed=false;
    for (int i=fittedFileSets.size()-1; i>=0; i--) {
        bool ok=true, allNotOK=false;
        for (int j=0; j<fittedFileSets[i].size(); j++) {
            if (!fittedFileSets[i].at(j) || !isApplicable(fittedFileSets[i].at(j))) {
                ok=false;
            } else {
                allNotOK=true;
            }
        }
        if (!ok && allNotOK) {
            fittedFileSets.removeAt(i);
            removed=true;
        }
    }

    for (int i=guessedFileSets.size()-1; i>=0; i--) {
        bool ok=true, allNotOK=false;
        for (int j=0; j<guessedFileSets[i].size(); j++) {
            if (!guessedFileSets[i].at(j) || !isApplicable(guessedFileSets[i].at(j))) {
                ok=false;
            } else {
                allNotOK=true;
            }
        }
        if (!ok && allNotOK) {
            guessedFileSets.removeAt(i);
            removed=true;
        }
    }

    if (removed) emit filesetsChanged();
}

void QFImFCCSFitEvaluationItem::setFitFile(int num, QFRawDataRecord *record)
{
    QList<QFRawDataRecord*> fileset;
    if (num<0) return;
    else if (num>fitFilesList.size()) fitFilesList.append(record);
    else fitFilesList[num]=record;

    for (int i=0; i<fitFilesList.size(); i++) {
        fileset<<fitFilesList[i];
    }
    if (guessedFileSets.size()<=0) guessFileSets(fileset, false);
    emit parameterStructureChanged();

    emit parameterStructureChanged();
    setDataChanged();
    emit fileChanged(num, record);

}

void QFImFCCSFitEvaluationItem::setFitFiles(const QList<QFRawDataRecord *> &records)
{
    QList<QFRawDataRecord*> fileset;
    while (records.size()>fitFilesList.size()) fitFilesList.append(NULL);
    for (int i=0; i<records.size(); i++) {
        fitFilesList[i]=records[i];
    }
    for (int i=0; i<fitFilesList.size(); i++) {
        fileset<<fitFilesList[i];
    }
    if (guessedFileSets.size()<=0) guessFileSets(fileset, false);
    emit parameterStructureChanged();
    setDataChanged();
    for (int i=0; i<records.size(); i++) {
        emit fileChanged(i, records[i]);
    }
}

void QFImFCCSFitEvaluationItem::setFitFileSet(int idx)
{
    if (idx>=0 && idx<fittedFileSets.size())  {
        setFitFiles(fittedFileSets[idx]);
    } else if (idx-fittedFileSets.size()>=0 && idx-fittedFileSets.size()<guessedFileSets.size()) {
        setFitFiles(guessedFileSets[idx-fittedFileSets.size()]);
    }
}


void QFImFCCSFitEvaluationItem::intWriteData(QXmlStreamWriter &w) const
{
    QFFitResultsByIndexAsVectorEvaluation::intWriteData(w);

    w.writeStartElement("multifunctions");
    QMapIterator<int,QString> i(m_multiFitFunctions);
    while (i.hasNext()) {
        i.next();
        w.writeStartElement("multifunction");
        w.writeAttribute("num", QString::number(i.key()));
        w.writeAttribute("id", i.value());
        w.writeEndElement();
    }
    w.writeEndElement();

    w.writeStartElement("fitfiles");
    for (int ii=0; ii<fitFilesList.size(); ii++) {
        w.writeStartElement("fitfile");
        w.writeAttribute("num", QString::number(ii));
        if (fitFilesList[ii]) {
            w.writeAttribute("id", QString::number(fitFilesList[ii]->getID()));
        }
        w.writeEndElement();
    }
    w.writeEndElement();

    w.writeStartElement("fittedfilesets");
    for (int ii=0; ii<fittedFileSets.size(); ii++) {
        w.writeStartElement("set");
        QString l;
        for (int jj=0; jj<fittedFileSets[ii].size(); jj++) {
            if (jj>0)l+=",";
            l+=QString::number(fittedFileSets[ii].at(jj)->getID());
        }
        w.writeAttribute("records", l);
        w.writeEndElement();
    }
    w.writeEndElement();

    w.writeStartElement("globalparams");
    QMapIterator<int, QMap<QString, int> > it(globalParams);
    while (it.hasNext()) {
        it.next();
        QMapIterator<QString, int> it2(it.value());
        while (it2.hasNext()) {
            it2.next();
            w.writeStartElement("globalparam");
            w.writeAttribute("file", QString::number(it.key()));
            w.writeAttribute("parameter", it2.key());
            w.writeAttribute("global", QString::number(it2.value()));
            w.writeEndElement();
        }
    }
    w.writeEndElement();

}

void QFImFCCSFitEvaluationItem::intReadData(QDomElement *e)
{
    QFFitResultsByIndexAsVectorEvaluation::intReadData(e);
    m_multiFitFunctions.clear();
    QDomElement e1=e->firstChildElement("multifunctions").firstChildElement("multifunction");
    while (!e1.isNull()) {
        bool ok=false;
        int num=e1.attribute("num", "-1").toInt(&ok);
        QString id=e1.attribute("id", "");

        if (num>=0 && ok && m_fitFunctions.contains(id)) {
            m_multiFitFunctions[num]=id;
        }

        e1=e1.nextSiblingElement("multifunction");
    }

    fitFilesList.clear();
    e1=e->firstChildElement("fitfiles").firstChildElement("fitfile");
    while (!e1.isNull()) {
        bool ok=false, ok1=false;
        int num=e1.attribute("num", "-1").toInt(&ok);
        int id=e1.attribute("id", "-1").toInt(&ok1);
        QFRawDataRecord* rdr=project->getRawDataByID(id);
        if (num>=0) {
            fitFilesList.append(rdr);
        }

        e1=e1.nextSiblingElement("fitfile");
    }
    ensureFitFiles();

    fittedFileSets.clear();
    e1=e->firstChildElement("fittedfilesets").firstChildElement("set");
    while (!e1.isNull()) {
        QString recs=e1.attribute("records", "");
        if (!recs.isEmpty()) {
            QList<QFRawDataRecord*> fileSet;
            QStringList sl=recs.split(',');
            bool ok=true;
            //qDebug()<<"read set "<<recs<<"  ok="<<ok;
            for (int i=0; i<sl.size(); i++) {
                bool okI=false;
                int id=sl[i].toInt(&okI);
                //qDebug()<<"       id="<<id<<" ok="<<okI;
                if (okI && id>=0) {
                    QFRawDataRecord* rdr=project->getRawDataByID(id);                    
                    //qDebug()<<"       rdr="<<rdr<<" isApplicable="<<isApplicable(rdr);
                    if (rdr&&isApplicable(rdr)) {
                        fileSet.append(rdr);
                    } else {
                        ok=false;
                        break;
                    }
                } else {
                    ok=false;
                    break;
                }

            }
            //qDebug()<<"       ok="<<ok;
            if (ok) fittedFileSets.append(fileSet);
        }

        e1=e1.nextSiblingElement("set");
    }
    checkAndCleanFitFileSets();
    emit filesetsChanged();

    globalParams.clear();
    e1=e->firstChildElement("globalparams").firstChildElement("globalparam");
    while (!e1.isNull()) {
        bool ok=false, ok1=false;
        int file=e1.attribute("file", "-1").toInt(&ok);
        int global=e1.attribute("global", "-1").toInt(&ok1);
        QString param=e1.attribute("parameter", "");
        if (file>=0 && global>=0 && ok && ok1 && (!param.isEmpty())) {
            globalParams[file].insert(param, global);
        }

        e1=e1.nextSiblingElement("globalparam");
    }
    globalParamsChanged=true;
    lastGlobalParamsCount=0;

    paramTable->rebuildModel();
}




bool QFImFCCSFitEvaluationItem::isApplicable(const QFRawDataRecord *record) const
{
    //return record->inherits("QFRDRFCSDataInterface") && record->inherits("QFRDRImageToRunInterface");
    return matchFunctor->matches(record);
}

void QFImFCCSFitEvaluationItem::intWriteDataAlgorithm(QXmlStreamWriter &w) const
{
    w.writeAttribute("weighting", dataWeightToString(getFitDataWeighting()));
}

void QFImFCCSFitEvaluationItem::intReadDataAlgorithm(QDomElement &e)
{
    if (e.hasAttribute("weighting")) {
        QString a=e.attribute("weighting").toLower();
        setFitDataWeighting(stringToDataWeight(a));
    }
}

bool QFImFCCSFitEvaluationItem::hasSpecial(const QFRawDataRecord *r, const QString &id, const QString &paramid, double &value, double &error) const
{
    int run=getIndexFromEvaluationResultID(id);
    return qfFCSHasSpecial(r, run, paramid, value, error);
}

bool QFImFCCSFitEvaluationItem::hasSpecial(const QFRawDataRecord *r, int index, const QString &paramid, double &value, double &error) const
{
    return qfFCSHasSpecial(r, index, paramid, value, error);
}

bool QFImFCCSFitEvaluationItem::overrideFitFunctionPreset(const QFRawDataRecord* r, QString paramName, double &value) const
{
    if (qfFCSOverrideFitFunctionPreset(this, r, paramName, value)) {
        return true;
    }
    return QFFitResultsByIndexAsVectorEvaluation::overrideFitFunctionPreset(r, paramName, value);
}

bool QFImFCCSFitEvaluationItem::overrideFitFunctionPresetError(const QFRawDataRecord* r, QString paramName, double &value) const
{
    if (qfFCSOverrideFitFunctionPresetError(this, r, paramName, value)) {
        return true;
    }
    return QFFitResultsByIndexAsVectorEvaluation::overrideFitFunctionPresetError(r, paramName, value);
}

bool QFImFCCSFitEvaluationItem::overrideFitFunctionPresetFix(const QFRawDataRecord* r, QString paramName, bool &value) const
{
    if (qfFCSOverrideFitFunctionPresetFix(this, r, paramName, value)) {
        return true;
    }
    return QFFitResultsByIndexAsVectorEvaluation::overrideFitFunctionPresetFix(r, paramName, value);
}



QString QFImFCCSFitEvaluationItem::getFitFunctionID() const
{
    return getFitFunctionID(0);
}

QList<QFRawDataRecord *> QFImFCCSFitEvaluationItem::getFitFiles() const
{
    QList<QFRawDataRecord *> l;
    for (int i=0; i<fitFilesList.size(); i++) l<<fitFilesList[i];
    return l;
}

QFFitFunction *QFImFCCSFitEvaluationItem::getFitFunction(const QFRawDataRecord *rdr) const
{
    //if (rdr) qDebug()<<"QFImFCCSFitEvaluationItem::getFitFunction("<<rdr->getName()<<")";
    //else qDebug()<<"QFImFCCSFitEvaluationItem::getFitFunction("<<rdr<<")";
    return getFitFunctionForID(getFitFunctionID(rdr));
}

QString QFImFCCSFitEvaluationItem::getFitFunctionID(const QFRawDataRecord *rdr) const
{
    //if (rdr) qDebug()<<"QFImFCCSFitEvaluationItem::getFitFunctionID("<<rdr->getName()<<")";
    //else qDebug()<<"QFImFCCSFitEvaluationItem::getFitFunctionID("<<rdr<<")";
    int idx=-1;
    for (int i=0; i<fitFilesList.size(); i++) {
        if (fitFilesList[i]==rdr) {
            idx=i;
            break;
        }
    }
    if (rdr && idx>=0) {
        return getFitFunctionID(idx);
    } else if (rdr) {
        for (int i=0; i<fittedFileSets.size(); i++) {
            for (int j=0; j<fittedFileSets[i].size(); j++) {
                if (fittedFileSets[i].at(j)==rdr) {
                    idx=j;
                    break;
                }
            }
            if (idx>=0) break;
        }
        if (idx<0) {
            for (int i=0; i<guessedFileSets.size(); i++) {
                for (int j=0; j<guessedFileSets[i].size(); j++) {
                    if (guessedFileSets[i].at(j)==rdr) {
                        idx=j;
                        break;
                    }
                }
                if (idx>=0) break;
            }
        }
    }
    if (rdr && idx>=0) {
        return getFitFunctionID(idx);
    }
    return "";
}

QString QFImFCCSFitEvaluationItem::rdrPointerToParameterStoreID(const QFRawDataRecord *rdr) const
{
    int idx=-1;
    for (int i=0; i<fitFilesList.size(); i++) {
        if (fitFilesList[i]==rdr) {
            idx=i;
            break;
        }
    }
    if (rdr&&idx<0) {
        for (int i=0; i<fittedFileSets.size(); i++) {
            for (int j=0; j<fittedFileSets[i].size(); j++) {
                if (fittedFileSets[i].at(j)==rdr) {
                    idx=j;
                    break;
                }
            }
            if (idx>=0) break;
        }
        if (idx<0) {
            for (int i=0; i<guessedFileSets.size(); i++) {
                for (int j=0; j<guessedFileSets[i].size(); j++) {
                    if (guessedFileSets[i].at(j)==rdr) {
                        idx=j;
                        break;
                    }
                }
                if (idx>=0) break;
            }
        }
    }

    if (rdr && idx>=0) {
        return QString::number(idx);
    }
    return QString("");
}

QFImFCCSParameterInputTable *QFImFCCSFitEvaluationItem::getParameterInputTableModel() const
{
    return paramTable;
}

int QFImFCCSFitEvaluationItem::getFitFileCount() const
{
    return fitFilesList.size();
}

QFMatchRDRFunctor *QFImFCCSFitEvaluationItem::getMatchFunctor() const
{
    return matchFunctor;
}

int QFImFCCSFitEvaluationItem::getCurrentIndex() const
{
    QFRawDataRecord* r=getFitFile(0);
    int index=getCurrentIndexM();
    if (r!=NULL) {
        /*m_currentIndex=*/index=r->getProperty(QString(getType()+QString::number(getID())+"_last_index"), index).toInt();
    }
    if (index<getIndexMin(r)) {
        /*m_currentIndex=*/index=getIndexMin(r);
    }
    if (index>getIndexMax(r)) {
        /*m_currentIndex=*/index=getIndexMax(r);
    }
    return index;
}

int QFImFCCSFitEvaluationItem::getLinkParameterCount() const
{
    if (!globalParamsChanged) return qMax(0,lastGlobalParamsCount);
    int count=0;
    QMapIterator<int, QMap<QString, int> > it(globalParams);
    while (it.hasNext()) {
        it.next();
        QMapIterator<QString, int> it2(it.value());
        while (it2.hasNext()) {
            it2.next();
            if (it2.value()>count) count=it2.value();
        }
    }
    globalParamsChanged=false;
    lastGlobalParamsCount=count+1;
    return count+1;
}

int QFImFCCSFitEvaluationItem::getLinkParameter(int file, QString parameter) const
{
    if (globalParams.contains(file)) {
        return globalParams.value(file).value(parameter, -1);
    }
    return -1;
}

QList<QPair<int, QString> > QFImFCCSFitEvaluationItem::getLinkedParameterList(int file, QString parameter) const
{
    QList<QPair<int, QString> > l;
    int link=getLinkParameter(file, parameter);
    if (link<0) return l;

    QMapIterator<int, QMap<QString, int> > it(globalParams);
    while (it.hasNext()) {
        it.next();
        QMapIterator<QString, int> it2(it.value());
        while (it2.hasNext()) {
            it2.next();
            if (it2.value()==link && (it.key()!=file || it2.key()!=parameter)) l<<qMakePair(it.key(), it2.key());
        }
    }
    return l;
}

const QList<QList<QFRawDataRecord *> > &QFImFCCSFitEvaluationItem::getFittedFiles() const
{
    return fittedFileSets;
}

const QList<QList<QFRawDataRecord *> > &QFImFCCSFitEvaluationItem::getGuessedFiles() const
{
    return guessedFileSets;
}

void QFImFCCSFitEvaluationItem::addFittedFileSet(const QList<QFRawDataRecord* > &fileset)
{
    if (!fittedFileSets.contains(fileset)) {
        fittedFileSets.append(fileset);
        guessFileSets(fileset, false);
        emit filesetsChanged();
    }
}

void QFImFCCSFitEvaluationItem::removeFittedFileSet(const QList<QFRawDataRecord* > &fileset)
{
    int oldS=fittedFileSets.size();
    fittedFileSets.removeAll(fileset);
    if (fittedFileSets.size()!=oldS) emit filesetsChanged();
}

QFImFCCSFileSetsModel *QFImFCCSFitEvaluationItem::getFileSetsModel() const
{
    return fittedFileSetsModel;
}

void QFImFCCSFitEvaluationItem::clearFittedFileSets()
{
    int oldS=fittedFileSets.size();
    fittedFileSets.clear();
    if (fittedFileSets.size()!=oldS) emit filesetsChanged();
}

void QFImFCCSFitEvaluationItem::guessFileSets(const QList<QFRawDataRecord *> &fileset, bool emitChangedSignal)
{
    int oldS=guessedFileSets.size();
    guessedFileSets.clear();
    //qDebug()<<"guessFileSets "<<fileset.size();

    if (fileset.size()>0) {
        QString groupName=fileset[0]->getGroupName();
        bool sameGroup=true;
        QString folderName=fileset[0]->getFolder();
        bool sameFolder=true;
        QStringList roles;
        bool allRolesOK=true;
        bool someRolesOK=false;
        for (int i=0; i<fileset.size(); i++) {
            if (fileset[i]->getGroupName()!=groupName) sameGroup=false;
            if (fileset[i]->getFolder()!=folderName) sameFolder=false;
            QString role=fileset[i]->getRole();
            roles<<role;
            if (role.isEmpty()) allRolesOK=false;
            else someRolesOK=true;
        }

        QFProject* project=QFPluginServices::getInstance()->getCurrentProject();
        //qDebug()<<"sameGroup "<<sameGroup<<" ("<<groupName<<")   sameFolder="<<sameFolder<<" ("<<folderName<<")   allRolesOK="<<allRolesOK<<"   someRolesOK="<<someRolesOK<<"   roles"<<roles;
        if (project && someRolesOK) {
            if (sameGroup && !groupName.isEmpty()) {
                //qDebug()<<"guess from group";
                for (int i=0; i<project->getRDRGroupCount(); i++) {
                    QList<QFRawDataRecord*> rlist=project->getRDRGroupMembers(i);
                    QList<QFRawDataRecord*> newset;
                    bool ok=false;
                    for (int j=0; j<roles.size(); j++) newset<<NULL;
                    //qDebug()<<"  checking group "<<project->getRDRGroupName(i)<<" with "<<rlist.size()<<" files";
                    for (int j=0; j<rlist.size(); j++) {
                        if (rlist[j] && isApplicable(rlist[j])) {
                            int idx=roles.indexOf(rlist[j]->getRole());
                            if (idx>=0 && idx<newset.size()) {
                                newset[idx]=rlist[j];
                                ok=true;
                            }
                        }
                    }
                    //qDebug()<<"    ok="<<ok<<" newset="<<newset<<" in_guessed="<<guessedFileSets.contains(newset)<<" in_fitted="<<fittedFileSets.contains(newset);
                    if (ok && !guessedFileSets.contains(newset) && !fittedFileSets.contains(newset) && !newset.contains(NULL)) guessedFileSets.append(newset);
                }
            } else if (sameFolder) {
                //qDebug()<<"guess from folder";
                for (int i=0; i<project->getRawDataCount(); i++) {
                    QList<QFRawDataRecord*> rlist=project->getRDRsInFolder(project->getRawDataByNum(i)->getFolder());
                    QList<QFRawDataRecord*> newset;
                    bool ok=false;
                    for (int j=0; j<roles.size(); j++) newset<<NULL;
                    for (int j=0; j<rlist.size(); j++) {
                        if (rlist[j] && isApplicable(rlist[j])) {
                            int idx=roles.indexOf(rlist[j]->getRole());
                            if (idx>=0 && idx<newset.size()) {
                                newset[idx]=rlist[j];
                                ok=true;
                            }
                        }
                    }
                    if (ok && !guessedFileSets.contains(newset) && !fittedFileSets.contains(newset) && !newset.contains(NULL)) guessedFileSets.append(newset);
                }
            } else {

            }
        }
    }
    //qDebug()<<"guessed:"<<guessedFileSets.size();

    if (emitChangedSignal || (emitChangedSignal && guessedFileSets.size()!=oldS)) emit filesetsChanged();
}









void QFImFCCSFitEvaluationItem::setupGlobalFitTool(QFGlobalFitTool& tool, QList<doFitData>* fitDataOut, QString& iparams, QList<double*>& paramsVector, QList<double*>& initialParamsVector, QList<double*>& errorsVector, QList<double*>& errorsVectorI, const QList<  QFRawDataRecord *> &records, int run, int rangeMinDatarange, int rangeMaxDatarange, bool doLog) const
{

    bool dontFitMasked=getProperty("dontFitMaskedPixels", true).toBool();
    QList<doFitData> fitData;
    //QFFitAlgorithm* falg=getFitAlgorithm();
    //if ((!falg)||records.size()<=0) return;
    if (records.size()<=0) return;
    const QFRDRFCSDataInterface* data0=dynamic_cast<const QFRDRFCSDataInterface*>(records.first());
    if (!data0) return;

    tool.clear();
    tool.setDoRecalculateInternals(false);

    const QFRDRRunSelectionsInterface* runsel=dynamic_cast<const QFRDRRunSelectionsInterface*>(records.first());
    if (runsel && dontFitMasked && run>=0 && runsel->leaveoutRun(run)) return;


    for (int r=0; r<records.size(); r++) {
         QFRawDataRecord* record=records[r];
          QFRDRFCSDataInterface* data=dynamic_cast<  QFRDRFCSDataInterface*>(record);
        if (data) {
            doFitData dfd;
            dfd.emitSignals=record->isEmitResultsChangedEnabled();
            record->disableEmitResultsChanged();

            QFFitFunction* ffunc=createFitFunction(record);
            if (!ffunc) ffunc=createFitFunction(r);
            if (doLog) QFPluginLogTools::log_text(tr("   - adding RDR '%1', model '%2'' ... \n").arg(record->getName()).arg(ffunc->name()));
            dfd.N=data->getCorrelationN();
            dfd.weights=NULL;
            dfd.taudata=data->getCorrelationT();
            dfd.corrdata=NULL;
            if (run<0) {
                dfd.corrdata=data->getCorrelationMean();
            } else {
                if (run<(int)data->getCorrelationRuns()) {
                    dfd.corrdata=data->getCorrelationRun(run);
                } else {
                    dfd.corrdata=data->getCorrelationMean();
                }
            }
            // we also have to care for the data cutting
            int cut_low=rangeMinDatarange;
            int cut_up=rangeMaxDatarange;
            if (cut_low<0) cut_low=0;
            if (cut_up>=dfd.N) cut_up=dfd.N-1;
            int cut_N=cut_up-cut_low+1;
            if (cut_N<0) {
                cut_low=0;
                cut_up=ffunc->paramCount()-1;
                if (cut_up>=dfd.N) cut_up=dfd.N-1;
                cut_N=cut_up+1;
            }
            if (doLog) QFPluginLogTools::log_text(tr("      - fit data range: %1...%2 (%3 datapoints)\n").arg(cut_low).arg(cut_up).arg(cut_N));
            dfd.weightsOK=false;
            dfd.taudata=&(dfd.taudata[cut_low]);
            dfd.corrdata=&(dfd.corrdata[cut_low]);
            dfd.weights=allocWeights(&(dfd.weightsOK), record, run);
            if (!dfd.weightsOK && doLog) QFPluginLogTools::log_warning(tr("      - weights have invalid values => setting all weights to 1\n"));
            // retrieve fit parameters and errors. run calcParameters to fill in calculated parameters and make sure
            // we are working with a complete set of parameters
            if (dfd.weights) dfd.weights=&(dfd.weights[cut_low]);
            dfd.params=allocFillParameters(record, run, ffunc);
            dfd.initialparams=duplicateArray(dfd.params, ffunc->paramCount());// allocFillParameters(record, run, ffunc);
            dfd.errors=allocFillParameterErrors(record, run, ffunc);
            dfd.errorsI=duplicateArray(dfd.errors, ffunc->paramCount());//allocFillParameterErrors(record, run, ffunc);
            dfd.paramsMin=allocFillParametersMin(record, ffunc);
            dfd.paramsMax=allocFillParametersMax(record, ffunc);
            dfd.paramsFix=allocFillFix(record, run, ffunc);
            dfd.ffunc=ffunc;
            dfd.cut_low=cut_low;
            dfd.cut_up=cut_up;
            dfd.cut_N=cut_N;

            //qDebug()<<"adding "<<r<<": ("<<record->getName()<<")   ffunc="<<ffunc->name();
            //qDebug()<<"  params="<<arrayToString(dfd.params, ffunc->paramCount());
            //qDebug()<<"  initialparams="<<arrayToString(dfd.initialparams, ffunc->paramCount());
            //qDebug()<<"  paramsMin="<<arrayToString(dfd.paramsMin, ffunc->paramCount());
            //qDebug()<<"  paramsMax="<<arrayToString(dfd.paramsMax, ffunc->paramCount());
            //qDebug()<<"  errors="<<arrayToString(dfd.errors, ffunc->paramCount());
            //qDebug()<<"  errorsI="<<arrayToString(dfd.errorsI, ffunc->paramCount());

            paramsVector.append(dfd.params);
            initialParamsVector.append(dfd.initialparams);
            errorsVector.append(dfd.errors);
            errorsVectorI.append(dfd.errorsI);

            ffunc->calcParameter(dfd.params, dfd.errors);
            ffunc->calcParameter(dfd.initialparams, dfd.errors);

            int fitparamcount=0;
            //qDebug()<<"record "<<r<<"  ("<<record->getName()<<"):";
            for (int i=0; i<ffunc->paramCount(); i++) {
                if (ffunc->isParameterVisible(i, dfd.params) && (!dfd.paramsFix[i]) && ffunc->getDescription(i).fit) {
                    if (!iparams.isEmpty()) iparams=iparams+";  ";
                    fitparamcount++;
                    iparams=iparams+QString("%1 = %2").arg(ffunc->getDescription(i).id).arg(dfd.params[i]);
                    //iparams=iparams+QString("%1 = %2 (%3..%4)").arg(ffunc->getDescription(i).id).arg(dfd.params[i]).arg(dfd.paramsMin[i]).arg(dfd.paramsMax[i]);
                }
                //qDebug("  before_fit: %s = %lf +/m %lf", ffunc->getDescription(i).id.toStdString().c_str(), dfd.params[i], dfd.errors[i]);
            }
            //qDebug()<<"\n\n";
            if (doLog) QFPluginLogTools::log_text(tr("      - initial params         (%1)\n").arg(iparams));


            fitData.append(dfd);
            tool.addTerm(ffunc, dfd.params, dfd.paramsFix, dfd.taudata, dfd.corrdata, dfd.weights, dfd.cut_N, dfd.paramsMin, dfd.paramsMax);
        }
    }

    if (doLog) QFPluginLogTools::log_text(tr("   - adding global links ... \n"));
    int cnt=0;
    tool.setGlobalParamCount(getLinkParameterCount());
    //QList<QList<QPair<int, int> > > links(getLinkParameterCount());
    tool.setDoRecalculateInternals(false);
    for (int r=0; r<records.size(); r++) {
         QFRawDataRecord* record=records[r];
         QFRDRFCSDataInterface* data=dynamic_cast< QFRDRFCSDataInterface*>(record);
        if (data) {
            QStringList paramIDs=fitData[cnt].ffunc->getParameterIDs();
            for (int p=0; p<paramIDs.size(); p++) {
                int l=getLinkParameter(r, paramIDs[p]);
                if (l>=0) {
                    tool.addToGlobalParam(l, r, p);
                }
            }
            cnt++;
        }
    }
    tool.setDoRecalculateInternals(true);
    tool.createLocalFitFunctors();

    if (fitDataOut) *fitDataOut=fitData;
}



void QFImFCCSFitEvaluationItem::setupGlobalFitTool(QFGlobalFitTool& tool, QList<doFitData>* fitDataOut, QString& iparams, QList<double*>& paramsVector, QList<double*>& initialParamsVector, QList<double*>& errorsVector, QList<double*>& errorsVectorI, const QList<const  QFRawDataRecord *> &records, int run, int rangeMinDatarange, int rangeMaxDatarange, bool doLog, const QStringList& fitfunctionids) const
{

    bool dontFitMasked=getProperty("dontFitMaskedPixels", true).toBool();
    QList<doFitData> fitData;
    //QFFitAlgorithm* falg=getFitAlgorithm();
    //if ((!falg)||records.size()<=0) return;
    if (records.size()<=0) return;
    const QFRDRFCSDataInterface* data0=dynamic_cast<const QFRDRFCSDataInterface*>(records.first());
    if (!data0) return;

    tool.clear();
    tool.setDoRecalculateInternals(false);

    const QFRDRRunSelectionsInterface* runsel=dynamic_cast<const QFRDRRunSelectionsInterface*>(records.first());
    if (runsel && dontFitMasked && run>=0 && runsel->leaveoutRun(run)) return;


    for (int r=0; r<records.size(); r++) {
        const QFRawDataRecord* record=records[r];
        const  QFRDRFCSDataInterface* data=dynamic_cast<const  QFRDRFCSDataInterface*>(record);
        if (data) {
            doFitData dfd;
            dfd.emitSignals=record->isEmitResultsChangedEnabled();
            //record->disableEmitResultsChanged();

            QFFitFunction* ffunc=NULL;
            if (r<fitfunctionids.size()) {
                ffunc=QFFitFunctionManager::getInstance()->createFunction(fitfunctionids[r]);
            }
            if (!ffunc) ffunc=createFitFunction(record);
            if (!ffunc) ffunc=createFitFunction(r);
            if (!ffunc) qDebug()<<"did not find fit function for record "<<r<<record->getName();
            if (ffunc) {
                if (doLog) QFPluginLogTools::log_text(tr("   - adding RDR '%1', model '%2'' ... \n").arg(record->getName()).arg(ffunc->name()));
                dfd.N=data->getCorrelationN();
                dfd.weights=NULL;
                dfd.taudata=data->getCorrelationT();
                dfd.corrdata=NULL;
                if (run<0) {
                    dfd.corrdata=data->getCorrelationMean();
                } else {
                    if (run<(int)data->getCorrelationRuns()) {
                        dfd.corrdata=data->getCorrelationRun(run);
                    } else {
                        dfd.corrdata=data->getCorrelationMean();
                    }
                }
                // we also have to care for the data cutting
                int cut_low=rangeMinDatarange;
                int cut_up=rangeMaxDatarange;
                if (cut_low<0) cut_low=0;
                if (cut_up>=dfd.N) cut_up=dfd.N-1;
                int cut_N=cut_up-cut_low+1;
                if (cut_N<0) {
                    cut_low=0;
                    cut_up=ffunc->paramCount()-1;
                    if (cut_up>=dfd.N) cut_up=dfd.N-1;
                    cut_N=cut_up+1;
                }
                if (doLog) QFPluginLogTools::log_text(tr("      - fit data range: %1...%2 (%3 datapoints)\n").arg(cut_low).arg(cut_up).arg(cut_N));
                dfd.weightsOK=false;
                dfd.taudata=&(dfd.taudata[cut_low]);
                dfd.corrdata=&(dfd.corrdata[cut_low]);
                dfd.weights=allocWeights(&(dfd.weightsOK), record, run);
                if (!dfd.weightsOK && doLog) QFPluginLogTools::log_warning(tr("      - weights have invalid values => setting all weights to 1\n"));
                // retrieve fit parameters and errors. run calcParameters to fill in calculated parameters and make sure
                // we are working with a complete set of parameters
                if (dfd.weights) dfd.weights=&(dfd.weights[cut_low]);
                record->readLock();
                dfd.params=allocFillParameters(record, run, ffunc);
                //if (r==0) qDebug()<<record->getName()<<run<<arrayToString(dfd.params, ffunc->paramCount(), false);
                dfd.initialparams=duplicateArray(dfd.params, ffunc->paramCount());
                dfd.errors=allocFillParameterErrors(record, run, ffunc);
                dfd.errorsI=duplicateArray(dfd.errors, ffunc->paramCount());
                dfd.paramsMin=allocFillParametersMin(record, ffunc);
                dfd.paramsMax=allocFillParametersMax(record, ffunc);
                dfd.paramsFix=allocFillFix(record, run, ffunc);
                record->readUnLock();
                dfd.ffunc=ffunc;
                dfd.cut_low=cut_low;
                dfd.cut_up=cut_up;
                dfd.cut_N=cut_N;

                //qDebug()<<"adding "<<r<<": ("<<record->getName()<<")   ffunc="<<ffunc->name();
                //qDebug()<<"  params="<<arrayToString(dfd.params, ffunc->paramCount());
                //qDebug()<<"  initialparams="<<arrayToString(dfd.initialparams, ffunc->paramCount());
                //qDebug()<<"  paramsMin="<<arrayToString(dfd.paramsMin, ffunc->paramCount());
                //qDebug()<<"  paramsMax="<<arrayToString(dfd.paramsMax, ffunc->paramCount());
                //qDebug()<<"  errors="<<arrayToString(dfd.errors, ffunc->paramCount());
                //qDebug()<<"  errorsI="<<arrayToString(dfd.errorsI, ffunc->paramCount());

                paramsVector.append(dfd.params);
                initialParamsVector.append(dfd.initialparams);
                errorsVector.append(dfd.errors);
                errorsVectorI.append(dfd.errorsI);

                ffunc->calcParameter(dfd.params, dfd.errors);
                ffunc->calcParameter(dfd.initialparams, dfd.errors);

                int fitparamcount=0;
                //qDebug()<<"record "<<r<<"  ("<<record->getName()<<"):";
                for (int i=0; i<ffunc->paramCount(); i++) {
                    if (ffunc->isParameterVisible(i, dfd.params) && (!dfd.paramsFix[i]) && ffunc->getDescription(i).fit) {
                        if (!iparams.isEmpty()) iparams=iparams+";  ";
                        fitparamcount++;
                        iparams=iparams+QString("%1 = %2").arg(ffunc->getDescription(i).id).arg(dfd.params[i]);
                        //iparams=iparams+QString("%1 = %2 (%3..%4)").arg(ffunc->getDescription(i).id).arg(dfd.params[i]).arg(dfd.paramsMin[i]).arg(dfd.paramsMax[i]);
                    }
                    //qDebug("  before_fit: %s = %lf +/m %lf", ffunc->getDescription(i).id.toStdString().c_str(), dfd.params[i], dfd.errors[i]);
                }
                //qDebug()<<"\n\n";
                if (doLog) QFPluginLogTools::log_text(tr("      - initial params         (%1)\n").arg(iparams));


                fitData.append(dfd);
                tool.addTerm(ffunc, dfd.params, dfd.paramsFix, dfd.taudata, dfd.corrdata, dfd.weights, dfd.cut_N, dfd.paramsMin, dfd.paramsMax);
            }
        }
    }

    if (doLog) QFPluginLogTools::log_text(tr("   - adding global links ... \n"));
    int cnt=0;
    tool.setGlobalParamCount(getLinkParameterCount());
    //QList<QList<QPair<int, int> > > links(getLinkParameterCount());
    tool.setDoRecalculateInternals(false);
    for (int r=0; r<records.size(); r++) {
         const QFRawDataRecord* record=records[r];
         const QFRDRFCSDataInterface* data=dynamic_cast<const  QFRDRFCSDataInterface*>(record);
        if (data) {
            QStringList paramIDs=fitData[cnt].ffunc->getParameterIDs();
            for (int p=0; p<paramIDs.size(); p++) {
                int l=getLinkParameter(r, paramIDs[p]);
                if (l>=0) {
                    tool.addToGlobalParam(l, r, p);
                }
            }
            cnt++;
        }
    }
    tool.setDoRecalculateInternals(true);
    tool.createLocalFitFunctors();

    if (fitDataOut) *fitDataOut=fitData;
}

void QFImFCCSFitEvaluationItem::calcChi2Landscape(double *chi2Landscape, int paramXFile, int paramXID, const QVector<double> &paramXValues, int paramYFile, int paramYID, const QVector<double> &paramYValues, const QList<QFRawDataRecord *> &records, int run, int defaultMinDatarange, int defaultMaxDatarange)
{
    QString iparams="";
    QList<double*> paramsVector;
    QList<double*> initialParamsVector;
    QList<double*> errorsVector, errorsVectorI;

    bool dontFitMasked=getProperty("dontFitMaskedPixels", true).toBool();

    QFFitAlgorithm* falg=getFitAlgorithm();
    if ((!falg)||records.size()<=0) return;
    QFRDRFCSDataInterface* data0=qobject_cast<QFRDRFCSDataInterface*>(records.first());
    if (!data0) return;
    QFGlobalFitTool tool(falg);
    tool.clear();
    tool.setRepeats(1);
    tool.setDoRecalculateInternals(false);

    QFRDRRunSelectionsInterface* runsel=qobject_cast<QFRDRRunSelectionsInterface*>(records.first());
    if (runsel && dontFitMasked && run>=0 && runsel->leaveoutRun(run)) return;


    QString runname=tr("average");
    if (run>=0) runname=QString::number(run);
    int rangeMinDatarange=0;
    int rangeMaxDatarange=data0->getCorrelationN();
    if (defaultMinDatarange>=0) rangeMinDatarange=defaultMinDatarange;
    if (defaultMaxDatarange>=0) rangeMaxDatarange=defaultMaxDatarange;

    restoreQFFitAlgorithmParameters(falg);
    falg->readErrorEstimateParametersFit(this);


    QList<doFitData> fitData;


    setupGlobalFitTool(tool, &fitData, iparams, paramsVector, initialParamsVector, errorsVector, errorsVectorI, records, run, rangeMinDatarange, rangeMaxDatarange, false);


    tool.evalueChi2Landscape(chi2Landscape, paramXFile, paramXID, paramXValues, paramYFile, paramYID, paramYValues, initialParamsVector);



    freeAndClearDoFitDataList(fitData);
    enableAndEmitResultsSignals();
}

void QFImFCCSFitEvaluationItem::freeAndClearDoFitDataList(QList<doFitData>& fitData) const {
    int cnt=0;
    for (int r=0; r<fitData.size(); r++) {
        //QFRawDataRecord* record=getFitFile(r);

        //if (fitData[cnt].emitSignals && record) record->enableEmitResultsChanged(true);
        if (fitData[cnt].weights) qfFree(fitData[cnt].weights);
        if (fitData[cnt].params) qfFree(fitData[cnt].params);
        if (fitData[cnt].initialparams) qfFree(fitData[cnt].initialparams);
        if (fitData[cnt].errors) qfFree(fitData[cnt].errors);
        if (fitData[cnt].errorsI) qfFree(fitData[cnt].errorsI);
        if (fitData[cnt].paramsFix) qfFree(fitData[cnt].paramsFix);
        if (fitData[cnt].paramsMax) qfFree(fitData[cnt].paramsMax);
        if (fitData[cnt].paramsMin) qfFree(fitData[cnt].paramsMin);
        if (fitData[cnt].ffunc) delete fitData[cnt].ffunc;

        cnt++;
    }
    fitData.clear();
}


void QFImFCCSFitEvaluationItem::enableAndEmitResultsSignals() const {
    int cnt=0;
    for (int r=0; r<getFitFileCount(); r++) {
        QFRawDataRecord* record=getFitFile(r);

        if (record) record->enableEmitResultsChanged(true);

        cnt++;
    }
}





void QFImFCCSFitEvaluationItem::doFit(const QList<QFRawDataRecord *> &records, int run, int defaultMinDatarange, int defaultMaxDatarange, QFFitAlgorithmReporter *dlgFitProgress, bool doLog)
{
    QString iparams="";
    QString oparams="";
    QString orparams="";
    QList<double*> paramsVector;
    QList<double*> initialParamsVector;
    QList<double*> errorsVector, errorsVectorI;
    QFBasicFitStatistics fitstat;

    bool saveLongStrings=!getProperty("dontSaveFitResultMessage", true).toBool();
    bool dontFitMasked=getProperty("dontFitMaskedPixels", true).toBool();

    QFFitAlgorithm* falg=getFitAlgorithm();
    if ((!falg)||records.size()<=0) return;
    QFRDRFCSDataInterface* data0=qobject_cast<QFRDRFCSDataInterface*>(records.first());
    if (!data0) return;
    QFGlobalFitTool tool(falg);
    tool.clear();
    tool.setRepeats(qMax(1,getProperty("repeatFit", 1).toInt()));
    tool.setGlobalLocalRepeats(qMax(0,getProperty("localGlobalFitIterations", 0).toInt()));
    tool.setDoRecalculateInternals(false);

    QFRDRRunSelectionsInterface* runsel=qobject_cast<QFRDRRunSelectionsInterface*>(records.first());
    if (runsel && dontFitMasked && run>=0 && runsel->leaveoutRun(run)) return;

    if (dlgFitProgress) {
        dlgFitProgress->reportStatus(tr("setting up ..."));
        dlgFitProgress->setProgressMax(100);
        dlgFitProgress->setProgress(0);
    }

    QString runname=tr("average");
    if (run>=0) runname=QString::number(run);
    int rangeMinDatarange=0;
    int rangeMaxDatarange=data0->getCorrelationN();
    if (defaultMinDatarange>=0) rangeMinDatarange=defaultMinDatarange;
    if (defaultMaxDatarange>=0) rangeMaxDatarange=defaultMaxDatarange;

    restoreQFFitAlgorithmParameters(falg);
    falg->readErrorEstimateParametersFit(this);
    falg->setReporter(dlgFitProgress);

    if (doLog) QFPluginLogTools::log_text(tr("setting up global fit for run %1 ... \n").arg(runname));

    bool epc=get_doEmitPropertiesChanged();
    bool erc=get_doEmitResultsChanged();
    QList<doFitData> fitData;
    set_doEmitPropertiesChanged(false);
    set_doEmitResultsChanged(false);

    QString egroup=QString("%1%2__%3").arg(getType()).arg(getID()).arg(falg->id());//.arg(dfd.ffunc->id());
    QString egrouplabel=QString("#%3 \"%1\": %2").arg(getName()).arg(falg->shortName()).arg(getID());//.arg(dfd.ffunc->shortName());
    for (int r=0; r<records.size(); r++) {
        QFRawDataRecord* record=records[r];
        QFRDRFCSDataInterface* data=qobject_cast<QFRDRFCSDataInterface*>(record);
        if (data) {
            QFFitFunction* ffunc=getFitFunction(record);
            egroup+=QString("__"+ffunc->id());
            egrouplabel+=QString(", "+ffunc->shortName());
        }
    }


    setupGlobalFitTool(tool, &fitData, iparams, paramsVector, initialParamsVector, errorsVector, errorsVectorI, records, run, rangeMinDatarange, rangeMaxDatarange, doLog);



    bool OK=true;
    try {
        if (!dlgFitProgress->isCanceled()) {

            if (dlgFitProgress) {
                dlgFitProgress->reportStatus(tr("fitting ..."));
                dlgFitProgress->setProgressMax(100);
                dlgFitProgress->setProgress(0);
            }
            //qDebug()<<"### before thread->init(): paramsVector.size="<<paramsVector.size();
            //qDebug()<<"  params[0]="<<arrayToString(paramsVector[0], fitData[0].ffunc->paramCount());
            doFitThread->init(&tool, paramsVector, errorsVector, initialParamsVector, errorsVectorI, NULL, &fitstat);
            //qDebug()<<"### after thread->init(): paramsVector.size="<<paramsVector.size();
            //qDebug()<<"  params[0]="<<arrayToString(paramsVector[0], fitData[0].ffunc->paramCount());
            //doFitThread->init(falg, params, errors, &taudata[cut_low], &corrdata[cut_low], &weights[cut_low], cut_N, ffunc, initialparams, paramsFix, paramsMin, paramsMax);
            doFitThread->start(QThread::HighPriority);
            QTime t;
            t.start();
            while (!doFitThread->isFinished()) {
                if (t.elapsed()>10) {
                    QApplication::processEvents(QEventLoop::AllEvents, 10);
                    if (dlgFitProgress && dlgFitProgress->isCanceled()) {
                      //doFitThread->terminate();
                      //break;
                    }
                    t.start();
                }
            }
            if (dlgFitProgress) {
                dlgFitProgress->setProgressFull();
                dlgFitProgress->reportStatus(tr("calculating parameters, errors and storing data ..."));
                OK=!dlgFitProgress->isCanceled();
            } else {
                OK=true;
            }
        }

        if (OK) {
            QFFitAlgorithm::FitResult result=doFitThread->getResults();
            int cnt=0;
            addFittedFileSet(records);
            for (int r=0; r<records.size(); r++) {
                QFRawDataRecord* record=records[r];
                QFRDRFCSDataInterface* data=qobject_cast<QFRDRFCSDataInterface*>(record);
                if (data) {
                    const doFitData& dfd=fitData[cnt];
                    double* errors=errorsVector[cnt];
                    double* params=paramsVector[cnt];
                    for (int i=0; i<dfd.ffunc->paramCount(); i++) {
                        if (!(dfd.ffunc->isParameterVisible(i, params) && (!dfd.paramsFix[i]) && dfd.ffunc->getDescription(i).fit)) {
                            errors[i]=dfd.errorsI[i];
                        }
                    }

                    dfd.ffunc->calcParameter(params, errors);
                    dfd.ffunc->sortParameter(params, errors, dfd.paramsFix);
                    dfd.ffunc->calcParameter(params, errors);

                    //qDebug()<<"record "<<r<<"  ("<<record->getName()<<"):  errors="<<errors<<"  params="<<params;
                    for (int i=0; i<dfd.ffunc->paramCount(); i++) {
                        if (dfd.ffunc->isParameterVisible(i, params) && (!dfd.paramsFix[i]) && dfd.ffunc->getDescription(i).fit) {
                            if (!oparams.isEmpty()) oparams=oparams+";  ";

                            oparams=oparams+QString("%1 = %2+/-%3").arg(dfd.ffunc->getDescription(i).id).arg(params[i]).arg(errors[i]);
                        }
                        //qDebug("  after_fit: %s = %lf +/- %lf", dfd.ffunc->getDescription(i).id.toStdString().c_str(), dfd.params[i], dfd.errors[i]);
                    }
                    //qDebug()<<"\n\n";


                    // round errors and values
                    for (int i=0; i<dfd.ffunc->paramCount(); i++) {
                        errors[i]=roundError(errors[i], 2);
                        params[i]=roundWithError(params[i], errors[i], 2);
                    }

                    //qDebug()<<"record "<<r<<"  ("<<record->getName()<<"):";
                    for (int i=0; i<dfd.ffunc->paramCount(); i++) {
                        if (dfd.ffunc->isParameterVisible(i, params) && (!dfd.paramsFix[i]) && dfd.ffunc->getDescription(i).fit) {
                            if (!orparams.isEmpty()) orparams=orparams+";  ";
                            orparams=orparams+QString("%1 = %2+/-%3").arg(dfd.ffunc->getDescription(i).id).arg(params[i]).arg(errors[i]);
                        }
                        //qDebug("  rounded_after_fit: %s = %lf +/- %lf", dfd.ffunc->getDescription(i).id.toStdString().c_str(), params[i], errors[i]);
                    }
                    //qDebug()<<"\n\n";
                    setFitResultValuesVisibleWithGroupAndLabel(record, run, params, errors, tr("fit results"), dfd.paramsFix, tr("fit results"), true);



                    //qDebug()<<"record "<<r<<"  ("<<record->getName()<<"):";
                    for (int i=0; i<dfd.ffunc->paramCount(); i++) {
                        //qDebug("  rounded_after_fit_after_save: %s = %lf +/- %lf", dfd.ffunc->getDescription(i).id.toStdString().c_str(), params[i], errors[i]);
                    }
                    //qDebug()<<"\n\n";

                    if (doLog) QFPluginLogTools::log_text(tr("   - fit completed after %1 msecs with result %2\n").arg(doFitThread->getDeltaTime()).arg(result.fitOK?tr("success"):tr("no convergence")));
                    if (doLog) QFPluginLogTools::log_text(tr("   - result-message: %1\n").arg(result.messageSimple));
                    if (doLog) QFPluginLogTools::log_text(tr("   - initial params         (%1)\n").arg(iparams));
                    if (doLog) QFPluginLogTools::log_text(tr("   - output params          (%1)\n").arg(oparams));
                    if (doLog) QFPluginLogTools::log_text(tr("   - output params, rounded (%1)\n").arg(orparams));


                    QString evalID=transformResultID(getEvaluationResultID(run, record));
                    QString param;
                    QString group="fit properties";

                    if (run<0) record->resultsSetNumber(evalID, "fitparam_g0", dfd.ffunc->evaluate(0, dfd.params));
                    else record->resultsSetInNumberList(evalID, "fitparam_g0", run, dfd.ffunc->evaluate(0, dfd.params));
                    record->resultsSetGroup(evalID, "fitparam_g0", tr("fit results"));
                    record->resultsSetLabel(evalID, "fitparam_g0", tr("g(0)"));

                    record->resultsSetEvaluationGroup(evalID, egroup);
                    record->resultsSetEvaluationGroupLabel(egroup, egrouplabel);
                    record->resultsSetEvaluationGroupIndex(evalID, run);
                    record->resultsSetEvaluationDescription(evalID, QString(""));


                    if (run<0) record->resultsSetString(evalID, "fit_model_name", dfd.ffunc->id());
                    else record->resultsSetInStringList(evalID, "fit_model_name", run, dfd.ffunc->id());
                    //else record->resultsSetString(evalID, "fit_model_name", dfd.ffunc->id());
                    record->resultsSetGroup(evalID, param, group);
                    record->resultsSetLabel(evalID, param, tr("fit: model"));

                    if (run<0) record->resultsSetString(evalID, "fitalg_name", falg->id());
                    else if (saveLongStrings) record->resultsSetInStringList(evalID, "fitalg_name", run, falg->id());
                    else  record->resultsSetString(evalID, "fitalg_name", falg->id());
                    record->resultsSetGroup(evalID, param, group);
                    record->resultsSetLabel(evalID, param, tr("fit: algorithm"));

                    if (run<0) record->resultsSetNumber(evalID, "fitalg_runtime", doFitThread->getDeltaTime(), "msecs");
                    else record->resultsSetInNumberList(evalID, "fitalg_runtime", run, doFitThread->getDeltaTime(), "msecs");
                    record->resultsSetGroup(evalID, param, group);
                    record->resultsSetLabel(evalID, param, tr("fit: runtime"));

                    if (run<0) record->resultsSetBoolean(evalID, "fitalg_success", result.fitOK);
                    else record->resultsSetInBooleanList(evalID, "fitalg_success", run, result.fitOK);
                    record->resultsSetGroup(evalID, param, group);
                    record->resultsSetLabel(evalID, param, tr("fit: success"));

                    if (run<0) record->resultsSetString(evalID, "fitalg_message", result.messageSimple);
                    else if (saveLongStrings) record->resultsSetInStringList(evalID, "fitalg_message", run, result.messageSimple);
                    record->resultsSetGroup(evalID, param, group);
                    record->resultsSetLabel(evalID, param, tr("fit: message"));

                    if (run<0) record->resultsSetString(evalID, "fitalg_messageHTML", result.message);
                    else if (saveLongStrings) record->resultsSetInStringList(evalID, "fitalg_messageHTML", run, result.message);
                    record->resultsSetGroup(evalID, param, group);
                    record->resultsSetLabel(evalID, param, tr("fit: message (markup)"));

                    if (run<0) record->resultsSetInteger(evalID, "fit_datapoints", dfd.cut_N);
                    else record->resultsSetInIntegerList(evalID, "fit_datapoints", run, dfd.cut_N);
                    record->resultsSetGroup(evalID, param, group);
                    record->resultsSetLabel(evalID, param, tr("fit: datapoints"));

                    if (run<0) record->resultsSetInteger(evalID, "fit_cut_low", dfd.cut_low);
                    else record->resultsSetInIntegerList(evalID, "fit_cut_low", run, dfd.cut_low);
                    record->resultsSetGroup(evalID, param, group);
                    record->resultsSetLabel(evalID, param, tr("fit: first point"));

                    if (run<0) record->resultsSetInteger(evalID, "fit_cut_up", dfd.cut_up);
                    else record->resultsSetInIntegerList(evalID, "fit_cut_up", run, dfd.cut_up);
                    record->resultsSetGroup(evalID, param, group);
                    record->resultsSetLabel(evalID, param, tr("fit: last point"));


                    QMapIterator<QString, QFRawDataRecord::evaluationResult> it(result.params);
                    while (it.hasNext()) {
                        it.next();
                        param="";
                        //record->resultsSet(evalID, param=("fitalg_"+it.key()), it.value());
                        switch(it.value().type) {
                            case QFRawDataRecord::qfrdreNumber:
                                if (run<0) record->resultsSetNumber(evalID, param=("fitalg_"+it.key()), it.value().dvalue, it.value().unit);
                                else record->resultsSetInNumberList(evalID, param=("fitalg_"+it.key()), run, it.value().dvalue, it.value().unit);
                                break;
                            case QFRawDataRecord::qfrdreNumberError:
                                if (run<0) record->resultsSetNumberError(evalID, param=("fitalg_"+it.key()), it.value().dvalue, it.value().derror, it.value().unit);
                                else record->resultsSetInNumberErrorList(evalID, param=("fitalg_"+it.key()), run, it.value().dvalue, it.value().derror, it.value().unit);
                                break;
                            case QFRawDataRecord::qfrdreInteger:
                                if (run<0) record->resultsSetInteger(evalID, param=("fitalg_"+it.key()), it.value().ivalue, it.value().unit);
                                else record->resultsSetInIntegerList(evalID, param=("fitalg_"+it.key()), run, it.value().ivalue, it.value().unit);
                                break;
                            case QFRawDataRecord::qfrdreBoolean:
                                if (run<0) record->resultsSetBoolean(evalID, param=("fitalg_"+it.key()), it.value().bvalue);
                                else record->resultsSetInBooleanList(evalID, param=("fitalg_"+it.key()), run, it.value().bvalue, it.value().unit);
                                break;
                            case QFRawDataRecord::qfrdreString:
                                if (run<0) record->resultsSetString(evalID, param=("fitalg_"+it.key()), it.value().svalue);
                                else if (saveLongStrings) record->resultsSetInStringList(evalID, param=("fitalg_"+it.key()), run, it.value().svalue, it.value().unit);
                                break;

                            case QFRawDataRecord::qfrdreBooleanVector:
                            case QFRawDataRecord::qfrdreBooleanMatrix:
                            case QFRawDataRecord::qfrdreNumberVector:
                            case QFRawDataRecord::qfrdreNumberMatrix:
                            case QFRawDataRecord::qfrdreNumberErrorVector:
                            case QFRawDataRecord::qfrdreNumberErrorMatrix:
                            case QFRawDataRecord::qfrdreIntegerVector:
                            case QFRawDataRecord::qfrdreIntegerMatrix:
                                if (run<0) record->resultsSet(evalID, param=("fitalg_"+it.key()), it.value());
                                break;
                            default:
                                break;
                        }

                        if (!param.isEmpty()) {
                            record->resultsSetGroup(evalID, param, group);
                            record->resultsSetLabel(evalID, param, it.value().label, it.value().label_rich);
                        }
                    }


                    {
                        QFFitStatistics fit_stat=calcFitStatistics(true, dfd.ffunc, dfd.N, dfd.taudata, dfd.corrdata, dfd.weights, dfd.cut_low, dfd.cut_up, params, errors, dfd.paramsFix, 11, 25, record, run);
                        fit_stat.free();
                    }
                    cnt++;
                }
            }
        } else {
            if (doLog) QFPluginLogTools::log_warning(tr("   - fit canceled by user!!!\n"));
        }
    } catch(std::exception& E) {
        if (doLog) QFPluginLogTools::log_error(tr("error during fitting, error message: %1\n").arg(E.what()));
    }

    if (dlgFitProgress) dlgFitProgress->reportStatus(tr("clean up ..."));
    if (doLog) QFPluginLogTools::log_text(tr("   - clean up ... \n"));
    if (epc) set_doEmitPropertiesChanged(true);
    if (erc) set_doEmitResultsChanged(true);

    freeAndClearDoFitDataList(fitData);
    enableAndEmitResultsSignals();
    fitData.clear();
    falg->setReporter(NULL);

}

void QFImFCCSFitEvaluationItem::doFitForMultithread(const QList<QFRawDataRecord *> &records, int run, int defaultMinDatarange, int defaultMaxDatarange, QFPluginLogService *logservice) const
{
    //qDebug()<<"fit: "<<records.size()<<run;
    QString iparams="";
    QString oparams="";
    QString orparams="";
    QList<double*> paramsVector;
    QList<double*> initialParamsVector;
    QList<double*> errorsVector, errorsVectorI;

    bool saveLongStrings=!getProperty("dontSaveFitResultMessage", true).toBool();
    bool dontFitMasked=getProperty("dontFitMaskedPixels", true).toBool();

    QFFitAlgorithm* falg=createFitAlgorithm();
    if ((!falg)||records.size()<=0) return;
    QFRDRFCSDataInterface* data0=qobject_cast<QFRDRFCSDataInterface*>(records.first());
    if (!data0) return;
    QFGlobalFitTool tool(falg);
    tool.clear();
    tool.setRepeats(qMax(1,getProperty("repeatFit", 1).toInt()));
    tool.setGlobalLocalRepeats(qMax(0,getProperty("localGlobalFitIterations", 0).toInt()));
    tool.setDoRecalculateInternals(false);

    QFRDRRunSelectionsInterface* runsel=qobject_cast<QFRDRRunSelectionsInterface*>(records.first());
    if (runsel && dontFitMasked && run>=0 && runsel->leaveoutRun(run)) return;


    QString runname=tr("average");
    if (run>=0) runname=QString::number(run);
    int rangeMinDatarange=0;
    int rangeMaxDatarange=data0->getCorrelationN();
    if (defaultMinDatarange>=0) rangeMinDatarange=defaultMinDatarange;
    if (defaultMaxDatarange>=0) rangeMaxDatarange=defaultMaxDatarange;

    restoreQFFitAlgorithmParameters(falg);
    falg->readErrorEstimateParametersFit(this);
    //falg->setReporter(dlgFitProgress);


    //bool epc=get_doEmitPropertiesChanged();
    //bool erc=get_doEmitResultsChanged();
    QList<doFitData> fitData;
    //set_doEmitPropertiesChanged(false);
    //set_doEmitResultsChanged(false);

    QString egroup=QString("%1%2__%3").arg(getType()).arg(getID()).arg(falg->id());//.arg(dfd.ffunc->id());
    QString egrouplabel=QString("#%3 \"%1\": %2").arg(getName()).arg(falg->shortName()).arg(getID());//.arg(dfd.ffunc->shortName());
    QString filesNames="";
    for (int r=0; r<records.size(); r++) {
        QFRawDataRecord* record=records[r];
        QFRDRFCSDataInterface* data=qobject_cast<QFRDRFCSDataInterface*>(record);
        if (data) {
            QFFitFunction* ffunc=getFitFunction(record);
            egroup+=QString("__"+ffunc->id());
            egrouplabel+=QString(", "+ffunc->shortName());
            if (!filesNames.isEmpty()) filesNames+=", ";
            filesNames+=record->getName();
        }
    }

    QMutexLocker locker(mutexThreadedFit);
    setupGlobalFitTool(tool, &fitData, iparams, paramsVector, initialParamsVector, errorsVector, errorsVectorI, records, run, rangeMinDatarange, rangeMaxDatarange, false);
    locker.unlock();


    bool OK=true;
    try {

        double deltaTime=0;
        QElapsedTimer tstart;
        tstart.start();

        QFFitAlgorithm::FitResult result=tool.fit(paramsVector, errorsVector, initialParamsVector, errorsVectorI);
        #if QT_VERSION >= 0x040800
            deltaTime=double(tstart.nsecsElapsed())/1.0e6;
        #else
            deltaTime=double(tstart.elapsed());
        #endif

        OK=true;
        if (OK) {
            int cnt=0;
            //addFittedFileSet(records);
            for (int r=0; r<records.size(); r++) {
                QFRawDataRecord* record=records[r];
                QFRDRFCSDataInterface* data=qobject_cast<QFRDRFCSDataInterface*>(record);
                if (data) {
                    const doFitData& dfd=fitData[cnt];
                    double* errors=errorsVector[cnt];
                    double* params=paramsVector[cnt];
                    for (int i=0; i<dfd.ffunc->paramCount(); i++) {
                        if (!(dfd.ffunc->isParameterVisible(i, params) && (!dfd.paramsFix[i]) && dfd.ffunc->getDescription(i).fit)) {
                            errors[i]=dfd.errorsI[i];
                        }
                    }

                    dfd.ffunc->calcParameter(params, errors);
                    dfd.ffunc->sortParameter(params, errors, dfd.paramsFix);
                    dfd.ffunc->calcParameter(params, errors);

                    //qDebug()<<"record "<<r<<"  ("<<record->getName()<<"):  errors="<<errors<<"  params="<<params;
                    for (int i=0; i<dfd.ffunc->paramCount(); i++) {
                        if (dfd.ffunc->isParameterVisible(i, params) && (!dfd.paramsFix[i]) && dfd.ffunc->getDescription(i).fit) {
                            if (!oparams.isEmpty()) oparams=oparams+";  ";

                            oparams=oparams+QString("%1 = %2+/-%3").arg(dfd.ffunc->getDescription(i).id).arg(params[i]).arg(errors[i]);
                        }
                        //qDebug("  after_fit: %s = %lf +/- %lf", dfd.ffunc->getDescription(i).id.toStdString().c_str(), dfd.params[i], dfd.errors[i]);
                    }
                    //qDebug()<<"\n\n";

                    // round errors and values
                    for (int i=0; i<dfd.ffunc->paramCount(); i++) {
                        errors[i]=roundError(errors[i], 2);
                        params[i]=roundWithError(params[i], errors[i], 2);
                    }


                    for (int i=0; i<dfd.ffunc->paramCount(); i++) {
                        if (dfd.ffunc->isParameterVisible(i, params) && (!dfd.paramsFix[i]) && dfd.ffunc->getDescription(i).fit) {
                            if (!orparams.isEmpty()) orparams=orparams+";  ";
                            orparams=orparams+QString("%1 = %2+/-%3").arg(dfd.ffunc->getDescription(i).id).arg(params[i]).arg(errors[i]);
                        }
                    }


                    QString evalID=transformResultID(getEvaluationResultID(run, record));
                    QString param;
                    QString group="fit properties";
                    bool* paramsFix=dfd.paramsFix;

                    for (int i=0; i<dfd.ffunc->paramCount(); i++) {
                        if (dfd.ffunc->isParameterVisible(i, params)) {
                            QString pid=dfd.ffunc->getParameterID(i);
                            QString unit=dfd.ffunc->getDescription(pid).unit;
                            QString fpid=getFitParamID(pid);
                            QString ffid= getFitParamFixID(pid);



                            if (run<0) record->resultsSetNumberError(evalID, fpid, params[i], errors[i], unit);
                            else {
                                record->resultsSetInNumberErrorListAndBool(evalID, fpid, run, params[i], errors[i], unit, getParamNameLocalStore(fpid), true);
                            }
                            record->resultsSetGroupLabelsAndSortPriority(evalID, fpid, tr("fit results"), pid+": "+dfd.ffunc->getDescription(pid).name, dfd.ffunc->getDescription(pid).label, true);

                            if (run<0) record->resultsSetBoolean(evalID, ffid, paramsFix[i]);
                            else {
                                record->resultsSetInBooleanListAndBool(evalID, ffid, run, paramsFix[i], QString(""), getParamNameLocalStore(ffid), true);
                            }
                            record->resultsSetGroupLabelsAndSortPriority(evalID,ffid, tr("fit results"), "fix_"+pid+": "+dfd.ffunc->getDescription(pid).name+tr(", fix"), dfd.ffunc->getDescription(pid).label+tr(", fix"), true);
                        }
                    }

                    if (run<0) record->resultsSetNumber(evalID, "fitparam_g0", dfd.ffunc->evaluate(0, dfd.params));
                    else record->resultsSetInNumberListAndBool(evalID, "fitparam_g0", run, dfd.ffunc->evaluate(0, dfd.params), "", getParamNameLocalStore("fitparam_g0"), true);
                    record->resultsSetGroupLabelsAndSortPriority(evalID, "fitparam_g0", tr("fit results"), tr("g(0)"), tr("g(0)"), true);


                    record->resultsSetEvaluationGroup(evalID, egroup);
                    record->resultsSetEvaluationGroupLabel(egroup, egrouplabel);
                    record->resultsSetEvaluationGroupIndex(evalID, run);
                    record->resultsSetEvaluationDescription(evalID, QString(""));


                    if (run<0) record->resultsSetString(evalID, "fit_model_name", dfd.ffunc->id());
                    else record->resultsSetInStringListAndBool(evalID, "fit_model_name", run, dfd.ffunc->id(), QString(""), getParamNameLocalStore("fit_model_name"), true);
                    record->resultsSetGroupAndLabels(evalID, param, group, tr("fit: model"));

                    if (run<0) record->resultsSetString(evalID, "fitalg_name", falg->id());
                    else if (saveLongStrings) record->resultsSetInStringListAndBool(evalID, "fitalg_name", run, falg->id(), QString(""), getParamNameLocalStore("fitalg_name"), true);
                    else  record->resultsSetString(evalID, "fitalg_name", falg->id());
                    record->resultsSetGroupAndLabels(evalID, param, group, tr("fit: algorithm"));

                    if (run<0) record->resultsSetNumber(evalID, "fitalg_runtime", deltaTime, "msecs");
                    else record->resultsSetInNumberListAndBool(evalID, "fitalg_runtime", run,deltaTime, "msecs", getParamNameLocalStore("fitalg_runtime"), true);
                    record->resultsSetGroupAndLabels(evalID, param, group, tr("fit: runtime"));

                    if (run<0) record->resultsSetBoolean(evalID, "fitalg_success", result.fitOK);
                    else record->resultsSetInBooleanListAndBool(evalID, "fitalg_success", run, result.fitOK, QString(""), getParamNameLocalStore("fitalg_success"), true);
                    record->resultsSetGroupAndLabels(evalID, param, group, tr("fit: success"));

                    if (run<0) record->resultsSetString(evalID, "fitalg_message", result.messageSimple);
                    else if (saveLongStrings) record->resultsSetInStringListAndBool(evalID, "fitalg_message", run, result.messageSimple, QString(""), getParamNameLocalStore("fitalg_message"), true);
                    record->resultsSetGroupAndLabels(evalID, param, group, tr("fit: message"));

                    if (run<0) record->resultsSetString(evalID, "fitalg_messageHTML", result.message);
                    else if (saveLongStrings) record->resultsSetInStringListAndBool(evalID, "fitalg_messageHTML", run, result.message, QString(""), getParamNameLocalStore("fitalg_messageHTML"), true);
                    record->resultsSetGroupAndLabels(evalID, param, group, tr("fit: message (markup)"));

                    if (run<0) record->resultsSetInteger(evalID, "fit_datapoints", dfd.cut_N);
                    else record->resultsSetInIntegerListAndBool(evalID, "fit_datapoints", run, dfd.cut_N, QString(""), getParamNameLocalStore("fit_datapoints"), true);
                    record->resultsSetGroupAndLabels(evalID, param, group, tr("fit: datapoints"));

                    if (run<0) record->resultsSetInteger(evalID, "fit_cut_low", dfd.cut_low);
                    else record->resultsSetInIntegerListAndBool(evalID, "fit_cut_low", run, dfd.cut_low, QString(""), getParamNameLocalStore("fit_cut_low"), true);
                    record->resultsSetGroupAndLabels(evalID, param, group, tr("fit: first point"));

                    if (run<0) record->resultsSetInteger(evalID, "fit_cut_up", dfd.cut_up);
                    else record->resultsSetInIntegerListAndBool(evalID, "fit_cut_up", run, dfd.cut_up, QString(""), getParamNameLocalStore("fit_cut_up"), true);
                    record->resultsSetGroupAndLabels(evalID, param, group, param, tr("fit: last point"));


                    QMapIterator<QString, QFRawDataRecord::evaluationResult> it(result.params);
                    while (it.hasNext()) {
                        it.next();
                        param="";
                        //record->resultsSet(evalID, param=("fitalg_"+it.key()), it.value());
                        switch(it.value().type) {
                            case QFRawDataRecord::qfrdreNumber:
                                if (run<0) record->resultsSetNumber(evalID, param=("fitalg_"+it.key()), it.value().dvalue, it.value().unit);
                                else record->resultsSetInNumberListAndBool(evalID, param=("fitalg_"+it.key()), run, it.value().dvalue, it.value().unit, getParamNameLocalStore(param), true);
                                break;
                            case QFRawDataRecord::qfrdreNumberError:
                                if (run<0) record->resultsSetNumberError(evalID, param=("fitalg_"+it.key()), it.value().dvalue, it.value().derror, it.value().unit);
                                else record->resultsSetInNumberErrorListAndBool(evalID, param=("fitalg_"+it.key()), run, it.value().dvalue, it.value().derror, it.value().unit, getParamNameLocalStore(param), true);
                                break;
                            case QFRawDataRecord::qfrdreInteger:
                                if (run<0) record->resultsSetInteger(evalID, param=("fitalg_"+it.key()), it.value().ivalue, it.value().unit);
                                else record->resultsSetInIntegerListAndBool(evalID, param=("fitalg_"+it.key()), run, it.value().ivalue, it.value().unit, getParamNameLocalStore(param), true);
                                break;
                            case QFRawDataRecord::qfrdreBoolean:
                                if (run<0) record->resultsSetBoolean(evalID, param=("fitalg_"+it.key()), it.value().bvalue);
                                else record->resultsSetInBooleanListAndBool(evalID, param=("fitalg_"+it.key()), run, it.value().bvalue, it.value().unit, getParamNameLocalStore(param), true);
                                break;
                            case QFRawDataRecord::qfrdreString:
                                if (run<0) record->resultsSetString(evalID, param=("fitalg_"+it.key()), it.value().svalue);
                                else if (saveLongStrings) record->resultsSetInStringListAndBool(evalID, param=("fitalg_"+it.key()), run, it.value().svalue, it.value().unit, getParamNameLocalStore(param), true);
                                break;

                            case QFRawDataRecord::qfrdreBooleanVector:
                            case QFRawDataRecord::qfrdreBooleanMatrix:
                            case QFRawDataRecord::qfrdreNumberVector:
                            case QFRawDataRecord::qfrdreNumberMatrix:
                            case QFRawDataRecord::qfrdreNumberErrorVector:
                            case QFRawDataRecord::qfrdreNumberErrorMatrix:
                            case QFRawDataRecord::qfrdreIntegerVector:
                            case QFRawDataRecord::qfrdreIntegerMatrix:
                                if (run<0) record->resultsSet(evalID, param=("fitalg_"+it.key()), it.value());
                                break;
                            default:
                                break;
                        }

                        if (!param.isEmpty()) {
                            record->resultsSetGroupAndLabels(evalID, param, group, it.value().label, it.value().label_rich);
                        }
                    }


                    {
                        QFFitStatistics fit_stat= dfd.ffunc->calcFitStatistics(dfd.N, dfd.taudata, dfd.corrdata, dfd.weights, dfd.cut_low, dfd.cut_up, params, errors, dfd.paramsFix, 11, 25);

                        setFitResultFitStatisticsInVector(record, evalID, run, fit_stat, "fitstat_local_", tr("local fit statistics"));

                        fit_stat.free();
                    }
                    cnt++;
                }
            }
        } else {
            //if (doLog) QFPluginLogTools::log_warning(tr("   - fit canceled by user!!!\n"));
        }
    } catch(std::exception& E) {
        if (logservice) logservice->log_error(tr("fitting file '%2', run %3:\n   error during fitting, error message: %1\n").arg(E.what()).arg(filesNames).arg(run));
    }

    //if (dlgFitProgress) dlgFitProgress->reportStatus(tr("clean up ..."));
    //if (doLog) QFPluginLogTools::log_text(tr("   - clean up ... \n"));
    //if (epc) set_doEmitPropertiesChanged(true);
    //if (erc) set_doEmitResultsChanged(true);

    freeAndClearDoFitDataList(fitData);
    fitData.clear();
    falg->setReporter(NULL);
    delete falg;
}

void QFImFCCSFitEvaluationItem::doFitForMultithreadReturn(QList<QFRawDataRecord::QFFitFitResultsStore> &fitresult, const QList<const QFRawDataRecord *> &records, const QStringList &fitfunctionIDs, int run, int defaultMinDatarange, int defaultMaxDatarange, QFPluginLogService *logservice) const
{







    //qDebug()<<"fit: "<<records.size()<<run;
    QString iparams="";
    QString oparams="";
    QString orparams="";
    QList<double*> paramsVector;
    QList<double*> initialParamsVector;
    QList<double*> errorsVector, errorsVectorI;

    bool saveLongStrings=!getProperty("dontSaveFitResultMessage", true).toBool();
    bool dontFitMasked=getProperty("dontFitMaskedPixels", true).toBool();

    QFFitAlgorithm* falg=createFitAlgorithm();
    if ((!falg)||records.size()<=0) return;
    const QFRDRFCSDataInterface* data0=dynamic_cast<const QFRDRFCSDataInterface*>(records.first());
    if (!data0) return;
    QFGlobalFitTool tool(falg);
    tool.clear();
    tool.setRepeats(qMax(1,getProperty("repeatFit", 1).toInt()));
    tool.setGlobalLocalRepeats(qMax(0,getProperty("localGlobalFitIterations", 0).toInt()));
    tool.setDoRecalculateInternals(false);

    const QFRDRRunSelectionsInterface* runsel=dynamic_cast<const QFRDRRunSelectionsInterface*>(records.first());
    if (runsel && dontFitMasked && run>=0 && runsel->leaveoutRun(run)) return;


    QString runname=tr("average");
    if (run>=0) runname=QString::number(run);
    int rangeMinDatarange=0;
    int rangeMaxDatarange=data0->getCorrelationN();
    if (defaultMinDatarange>=0) rangeMinDatarange=defaultMinDatarange;
    if (defaultMaxDatarange>=0) rangeMaxDatarange=defaultMaxDatarange;

    restoreQFFitAlgorithmParameters(falg);
    falg->readErrorEstimateParametersFit(this);
    //falg->setReporter(dlgFitProgress);


    //bool epc=get_doEmitPropertiesChanged();
    //bool erc=get_doEmitResultsChanged();
    QList<doFitData> fitData;
    //set_doEmitPropertiesChanged(false);
    //set_doEmitResultsChanged(false);

    QString egroup=QString("%1%2__%3").arg(getType()).arg(getID()).arg(falg->id());//.arg(dfd.ffunc->id());
    QString egrouplabel=QString("#%3 \"%1\": %2").arg(getName()).arg(falg->shortName()).arg(getID());//.arg(dfd.ffunc->shortName());

    for (int i=0; i<records.size(); i++) {
        const QString evalID=transformResultID(getEvaluationResultID(run, records[i]));
        QFRawDataRecord::QFFitFitResultsStore r;
        r.index=run;
        r.rdr=records[i];
        r.rdrRecID=-1;
        if (records[i]) r.rdrRecID=records[i]->getID();
        r.evalID=evalID;
        r.evalgroup=egroup;
        r.egrouplabel=egrouplabel;
        r.egroupindex=run;
        r.egroupdescription=QString("");
        fitresult<<r;
    }
    QString filesNames="";
    for (int r=0; r<records.size(); r++) {
        const QFRawDataRecord* record=records[r];
        const QFRDRFCSDataInterface* data=dynamic_cast<const QFRDRFCSDataInterface*>(record);
        if (data) {
            const QFFitFunction* ffunc=getFitFunction(fitfunctionIDs.value(r));
            if (ffunc) {
                egroup+=QString("__"+ffunc->id());
                egrouplabel+=QString(", "+ffunc->shortName());
                if (!filesNames.isEmpty()) filesNames+=", ";
                filesNames+=record->getName();
            }
        }
    }

    //QMutexLocker locker(mutexThreadedFit);
    setupGlobalFitTool(tool, &fitData, iparams, paramsVector, initialParamsVector, errorsVector, errorsVectorI, records, run, rangeMinDatarange, rangeMaxDatarange, false, fitfunctionIDs);
    //locker.unlock();

    bool OK=true;
    try {

        double deltaTime=0;
        QElapsedTimer tstart;
        tstart.start();

        QFFitAlgorithm::FitResult result=tool.fit(paramsVector, errorsVector, initialParamsVector, errorsVectorI);
        #if QT_VERSION >= 0x040800
            deltaTime=double(tstart.nsecsElapsed())/1.0e6;
        #else
            deltaTime=double(tstart.elapsed());
        #endif

        OK=true;
        if (OK) {
            int cnt=0;
            //addFittedFileSet(records);
            for (int r=0; r<records.size(); r++) {
                const QFRawDataRecord* record=records[r];
                const QFRDRFCSDataInterface* data=dynamic_cast<const QFRDRFCSDataInterface*>(record);
                //const QString evalID=transformResultID(getEvaluationResultID(run, record));
                if (data) {
                    const doFitData& dfd=fitData[cnt];
                    double* errors=errorsVector[cnt];
                    double* params=paramsVector[cnt];
                    for (int i=0; i<dfd.ffunc->paramCount(); i++) {
                        if (!(dfd.ffunc->isParameterVisible(i, params) && (!dfd.paramsFix[i]) && dfd.ffunc->getDescription(i).fit)) {
                            errors[i]=dfd.errorsI[i];
                        }
                    }

                    dfd.ffunc->calcParameter(params, errors);
                    dfd.ffunc->sortParameter(params, errors, dfd.paramsFix);
                    dfd.ffunc->calcParameter(params, errors);

                    //qDebug()<<"record "<<r<<"  ("<<record->getName()<<"):  errors="<<errors<<"  params="<<params;
                    for (int i=0; i<dfd.ffunc->paramCount(); i++) {
                        if (dfd.ffunc->isParameterVisible(i, params) && (!dfd.paramsFix[i]) && dfd.ffunc->getDescription(i).fit) {
                            if (!oparams.isEmpty()) oparams=oparams+";  ";

                            oparams=oparams+QString("%1 = %2+/-%3").arg(dfd.ffunc->getDescription(i).id).arg(params[i]).arg(errors[i]);
                        }
                        //qDebug("  after_fit: %s = %lf +/- %lf", dfd.ffunc->getDescription(i).id.toStdString().c_str(), dfd.params[i], dfd.errors[i]);
                    }
                    //qDebug()<<"\n\n";

                    // round errors and values
                    for (int i=0; i<dfd.ffunc->paramCount(); i++) {
                        errors[i]=roundError(errors[i], 2);
                        params[i]=roundWithError(params[i], errors[i], 2);
                    }


                    for (int i=0; i<dfd.ffunc->paramCount(); i++) {
                        if (dfd.ffunc->isParameterVisible(i, params) && (!dfd.paramsFix[i]) && dfd.ffunc->getDescription(i).fit) {
                            if (!orparams.isEmpty()) orparams=orparams+";  ";
                            orparams=orparams+QString("%1 = %2+/-%3").arg(dfd.ffunc->getDescription(i).id).arg(params[i]).arg(errors[i]);
                        }
                    }



                    QString param;
                    QString group="fit properties";
                    bool* paramsFix=dfd.paramsFix;

                    for (int i=0; i<dfd.ffunc->paramCount(); i++) {
                        if (dfd.ffunc->isParameterVisible(i, params)) {
                            QString pid=dfd.ffunc->getParameterID(i);
                            QString unit=dfd.ffunc->getDescription(pid).unit;
                            QString fpid=getFitParamID(pid);
                            QString ffid= getFitParamFixID(pid);

                            QFRawDataRecord::evaluationResult res, resfix, reslocal, reslocalfix;
                            res.resultsSetNumberError(params[i], errors[i], unit, tr("fit results"), pid+": "+dfd.ffunc->getDescription(pid).name, dfd.ffunc->getDescription(pid).label, true);
                            resfix.resultsSetBoolean(paramsFix[i],  tr("fit results"), "fix_"+pid+": "+dfd.ffunc->getDescription(pid).name+tr(", fix"), dfd.ffunc->getDescription(pid).label+tr(", fix"), false);
                            reslocal.resultsSetBoolean(true);
                            reslocalfix.resultsSetBoolean(true);


                            fitresult[r].fitresults[fpid]=res;
                            fitresult[r].fitresults[ffid]=resfix;
                            fitresult[r].fitresults[getParamNameLocalStore(fpid)]=reslocal;
                            fitresult[r].fitresults[getParamNameLocalStore(ffid)]=reslocalfix;

//                            if (run<0) record->resultsSetNumberError(evalID, fpid, params[i], errors[i], unit);
//                            else {
//                                record->resultsSetInNumberErrorListAndBool(evalID, fpid, run, params[i], errors[i], unit, getParamNameLocalStore(fpid), true);
//                            }
//                            record->resultsSetGroupLabelsAndSortPriority(evalID, fpid, tr("fit results"), pid+": "+dfd.ffunc->getDescription(pid).name, dfd.ffunc->getDescription(pid).label, true);

//                            if (run<0) record->resultsSetBoolean(evalID, ffid, paramsFix[i]);
//                            else {
//                                record->resultsSetInBooleanListAndBool(evalID, ffid, run, paramsFix[i], QString(""), getParamNameLocalStore(ffid), true);
//                            }
//                            record->resultsSetGroupLabelsAndSortPriority(evalID,ffid, tr("fit results"), "fix_"+pid+": "+dfd.ffunc->getDescription(pid).name+tr(", fix"), dfd.ffunc->getDescription(pid).label+tr(", fix"), true);
                        }
                    }

                    fitresult[r].fitresults[param="fitparam_g0"].resultsSetNumber(dfd.ffunc->evaluate(0, dfd.params),"", tr("fit results"), tr("g(0)"), tr("g(0)"), true);
                    fitresult[r].fitresults[getParamNameLocalStore(param)].resultsSetBoolean(true);

                    /*if (run<0) record->resultsSetNumber(evalID, "fitparam_g0", dfd.ffunc->evaluate(0, dfd.params));
                    else record->resultsSetInNumberListAndBool(evalID, "fitparam_g0", run, dfd.ffunc->evaluate(0, dfd.params), "", getParamNameLocalStore("fitparam_g0"), true);
                    record->resultsSetGroupLabelsAndSortPriority(evalID, "fitparam_g0", tr("fit results"), tr("g(0)"), tr("g(0)"), true);*/


                    /*record->resultsSetEvaluationGroup(evalID, egroup);
                    record->resultsSetEvaluationGroupLabel(egroup, egrouplabel);
                    record->resultsSetEvaluationGroupIndex(evalID, run);
                    record->resultsSetEvaluationDescription(evalID, QString(""));*/



                    QString label;
                    fitresult[r].fitresults[param="fit_model_name"].resultsSetString(dfd.ffunc->id(), group, label=tr("fit results"), label, false);
                    fitresult[r].fitresults[getParamNameLocalStore(param)].resultsSetBoolean(true);

                    fitresult[r].fitresults[param="fitalg_name"].resultsSetString(falg->id(), group, label=tr("fit: algorithm"), label, false);
                    fitresult[r].fitresults[getParamNameLocalStore(param)].resultsSetBoolean(true);


                    fitresult[r].fitresults[param="fitalg_success"].resultsSetBoolean(result.fitOK, group, label=tr("fit: success"), label, false);
                    fitresult[r].fitresults[getParamNameLocalStore(param)].resultsSetBoolean(true);

                    fitresult[r].fitresults[param="fitalg_runtime"].resultsSetNumber(deltaTime, "msecs", group, label=tr("fit: runtime"), label, false);
                    fitresult[r].fitresults[getParamNameLocalStore(param)].resultsSetBoolean(true);

                     if (run<0 || saveLongStrings) {
                        fitresult[r].fitresults[param="fitalg_message"].resultsSetString(result.messageSimple, group, label=tr("fit: message"), label, false);
                        fitresult[r].fitresults[getParamNameLocalStore(param)].resultsSetBoolean(true);
                        fitresult[r].fitresults[param="fitalg_messageHTML"].resultsSetString(result.message, group, label=tr("fit: message (markup)"), label, false);
                        fitresult[r].fitresults[getParamNameLocalStore(param)].resultsSetBoolean(true);
                    }

                    fitresult[r].fitresults[param="fit_datapoints"].resultsSetInteger(dfd.cut_N, "", group, label=tr("fit: datapoints"), label, false);
                    fitresult[r].fitresults[getParamNameLocalStore(param)].resultsSetBoolean(true);
                    fitresult[r].fitresults[param="fit_cut_low"].resultsSetInteger(dfd.cut_low, "", group, label=tr("fit: datapoints"), label, false);
                    fitresult[r].fitresults[getParamNameLocalStore(param)].resultsSetBoolean(true);
                    fitresult[r].fitresults[param="fit_cut_up"].resultsSetInteger(dfd.cut_up, "", group, label=tr("fit: datapoints"), label, false);
                    fitresult[r].fitresults[getParamNameLocalStore(param)].resultsSetBoolean(true);





                    QMapIterator<QString, QFRawDataRecord::evaluationResult> it(result.params);
                    while (it.hasNext()) {
                        it.next();
                        param="";
                        //record->resultsSet(evalID, param=("fitalg_"+it.key()), it.value());
                        switch(it.value().type) {
                            case QFRawDataRecord::qfrdreNumber:
                            case QFRawDataRecord::qfrdreNumberError:
                            case QFRawDataRecord::qfrdreInteger:
                            case QFRawDataRecord::qfrdreBoolean:
                            case QFRawDataRecord::qfrdreString:
                                fitresult[r].fitresults[param="fitalg_"+it.key()]=it.value();
                                fitresult[r].fitresults[param="fitalg_"+it.key()].group=group;
                                fitresult[r].fitresults[getParamNameLocalStore(param)].resultsSetBoolean(true);
                                break;

                            case QFRawDataRecord::qfrdreBooleanVector:
                            case QFRawDataRecord::qfrdreBooleanMatrix:
                            case QFRawDataRecord::qfrdreNumberVector:
                            case QFRawDataRecord::qfrdreNumberMatrix:
                            case QFRawDataRecord::qfrdreNumberErrorVector:
                            case QFRawDataRecord::qfrdreNumberErrorMatrix:
                            case QFRawDataRecord::qfrdreIntegerVector:
                            case QFRawDataRecord::qfrdreIntegerMatrix:
                                if (run<0) {
                                    fitresult[r].fitresults[param="fitalg_"+it.key()]=it.value();
                                    fitresult[r].fitresults[param="fitalg_"+it.key()].group=group;
                                    fitresult[r].fitresults[getParamNameLocalStore(param)].resultsSetBoolean(true);
                                }
                                break;
                            default:
                                break;
                        }
                    }


                    {
                        QFFitStatistics fit_stat= dfd.ffunc->calcFitStatistics(dfd.N, dfd.taudata, dfd.corrdata, dfd.weights, dfd.cut_low, dfd.cut_up, params, errors, dfd.paramsFix, 11, 25);

                        setFitResultFitStatisticsInResultStore(fitresult[r], fit_stat, tr("local fit statistics"), "fitstat_local_");

                        fit_stat.free();
                    }
                    cnt++;
                }
            }
        } else {
            //if (doLog) QFPluginLogTools::log_warning(tr("   - fit canceled by user!!!\n"));
        }
    } catch(std::exception& E) {
        if (logservice) logservice->log_error(tr("fitting file '%2', run %3:\n   error during fitting, error message: %1\n").arg(E.what()).arg(filesNames).arg(run));
    }

    //if (dlgFitProgress) dlgFitProgress->reportStatus(tr("clean up ..."));
    //if (doLog) QFPluginLogTools::log_text(tr("   - clean up ... \n"));
    //if (epc) set_doEmitPropertiesChanged(true);
    //if (erc) set_doEmitResultsChanged(true);

    freeAndClearDoFitDataList(fitData);
    fitData.clear();
    falg->setReporter(NULL);
    delete falg;

}

