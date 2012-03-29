#include "qfusesresultsbyindexevaluation.h"

QFUsesResultsByIndexEvaluation::QFUsesResultsByIndexEvaluation(QFProject *parent, bool showRDRList, bool useSelection) :
    QFUsesResultsEvaluation(parent, showRDRList, useSelection)
{
    currentIndex=0;
}

QFUsesResultsByIndexEvaluation::~QFUsesResultsByIndexEvaluation()
{
}

bool QFUsesResultsByIndexEvaluation::hasResults(QFRawDataRecord *r1, int index) const {
    QFRawDataRecord* r=r1;
    QString rsid=getEvaluationResultID(index);
    return r->resultsExistsFromEvaluation(rsid);
}

QString QFUsesResultsByIndexEvaluation::getEvaluationResultID(int index) const {
    return QString("%1_%2_i%3").arg(getType()).arg(getID()).arg(index);
}

void QFUsesResultsByIndexEvaluation::setFitResultGroup(QFRawDataRecord *r, int index, const QString &parameterID, const QString &group) {
    setFitResultGroup(r, getEvaluationResultID(index), parameterID, group);
}

void QFUsesResultsByIndexEvaluation::setFitResultLabel(QFRawDataRecord *r, int index, const QString &parameterID, const QString &label, const QString &label_richtext)
{
    setFitResultLabel(r, getEvaluationResultID(index), parameterID, label, label_richtext);
}

void QFUsesResultsByIndexEvaluation::setFitResultEvaluationGroup(QFRawDataRecord *r, int index, const QString &group) {
    setFitResultEvaluationGroup(r, getEvaluationResultID(index), group);
}

void QFUsesResultsByIndexEvaluation::setFitResultEvaluationDescription(QFRawDataRecord *r, int index, const QString &description) {
    setFitResultEvaluationDescription(r, getEvaluationResultID(index), description);
}

void QFUsesResultsByIndexEvaluation::resetAllFitResults(QFRawDataRecord *r, int index) {
    resetAllFitResults(r, getEvaluationResultID(index));
}

void QFUsesResultsByIndexEvaluation::setFitValue(QFRawDataRecord *r, int index, const QString &parameterID, double value, const QString &unit) {
    setFitValue(r, getEvaluationResultID(index), parameterID, value, unit);
}

void QFUsesResultsByIndexEvaluation::setFitResultValue(QFRawDataRecord *r, int index, const QString &parameterID, double value, const QString &unit) {
    setFitResultValue(r, getEvaluationResultID(index), parameterID, value, unit);
}

void QFUsesResultsByIndexEvaluation::setFitResultValueString(QFRawDataRecord *r, int index, const QString &parameterID, const QString &value) {
    setFitResultValueString(r, getEvaluationResultID(index), parameterID, value);
}

void QFUsesResultsByIndexEvaluation::setFitResultValueInt(QFRawDataRecord *r, int index, const QString &parameterID, int64_t value, const QString &unit) {
    setFitResultValueInt(r, getEvaluationResultID(index), parameterID, value, unit);
}

void QFUsesResultsByIndexEvaluation::setFitResultValueBool(QFRawDataRecord *r, int index, const QString &parameterID, bool value) {
    setFitResultValueBool(r, getEvaluationResultID(index), parameterID, value);
}

void QFUsesResultsByIndexEvaluation::setFitResultValue(QFRawDataRecord *r, int index, const QString &parameterID, double value, double error, const QString &unit) {
    setFitResultValue(r, getEvaluationResultID(index), parameterID, value, error, unit);
}

void QFUsesResultsByIndexEvaluation::setFitResultError(QFRawDataRecord *r, int index, const QString &parameterID, double error) {
    setFitResultError(r, getEvaluationResultID(index), parameterID, error);
}

void QFUsesResultsByIndexEvaluation::setFitResultValueNumberArray(QFRawDataRecord *r, int index, const QString &parameterID, double *value, double *error, uint32_t N, const QString &unit) {
    setFitResultValueNumberArray(r, getEvaluationResultID(index), parameterID, value, error, N, unit);
}

void QFUsesResultsByIndexEvaluation::setFitResultValueNumberArray(QFRawDataRecord *r, int index, const QString &parameterID, double *value, uint32_t N, const QString &unit) {
    setFitResultValueNumberArray(r, getEvaluationResultID(index), parameterID, value, N, unit);
}

QVector<double> QFUsesResultsByIndexEvaluation::getFitValueNumberArray(QFRawDataRecord *r, int index, const QString &parameterID) const {
    return getFitValueNumberArray(r, getEvaluationResultID(index), parameterID);
}

QVector<double> QFUsesResultsByIndexEvaluation::getFitValueErrorArray(QFRawDataRecord *r, int index, const QString &parameterID) const {
    return getFitValueErrorArray(r, getEvaluationResultID(index), parameterID);
}

double QFUsesResultsByIndexEvaluation::getFitValue(QFRawDataRecord *r, int index, const QString &parameterID) const {
    return getFitValue(r, getEvaluationResultID(index), parameterID);
}

double QFUsesResultsByIndexEvaluation::getFitError(QFRawDataRecord *r, int index, const QString &parameterID) const {
    return getFitError(r, getEvaluationResultID(index), parameterID);
}

void QFUsesResultsByIndexEvaluation::setFitError(QFRawDataRecord *r, int index, const QString &parameterID, double error) {
    setFitError(r, getEvaluationResultID(index), parameterID, error);
}

void QFUsesResultsByIndexEvaluation::setFitFix(QFRawDataRecord *r, int index, const QString &parameterID, bool fix) {
    setFitFix(r, getEvaluationResultID(index), parameterID, fix);
}

void QFUsesResultsByIndexEvaluation::setFitResultFix(QFRawDataRecord *r, int index, const QString &parameterID, bool fix) {
    setFitResultFix(r, getEvaluationResultID(index), parameterID, fix);
}

bool QFUsesResultsByIndexEvaluation::getFitFix(QFRawDataRecord *r, int index, const QString &parameterID) const {
    return getFitFix(r, getEvaluationResultID(index), parameterID);
}

void QFUsesResultsByIndexEvaluation::intWriteData(QXmlStreamWriter &w) {
    QFUsesResultsEvaluation::intWriteData(w);
    w.writeTextElement("current_index", QString::number(currentIndex));
}

void QFUsesResultsByIndexEvaluation::intReadData(QDomElement *e) {
    QFUsesResultsEvaluation::intReadData(e);
    bool OK=false;
    currentIndex=e->firstChildElement("current_index").text().toInt(&OK);
    if (!OK) currentIndex=-1;
}

void QFUsesResultsByIndexEvaluation::setCurrentIndex(int index) {
    currentIndex=index;
}

int QFUsesResultsByIndexEvaluation::getCurrentIndex() const {
    if (currentIndex<getIndexMin(getHighlightedRecord())) return getIndexMin(getHighlightedRecord());
    if (currentIndex>=getIndexMax(getHighlightedRecord())) return getIndexMax(getHighlightedRecord());
    return currentIndex;
}

int QFUsesResultsByIndexEvaluation::getIndexMin(QFRawDataRecord *r) const {
    return 0;
}

int QFUsesResultsByIndexEvaluation::getIndexMax(QFRawDataRecord *r) const {
    return 0;
}

void QFUsesResultsByIndexEvaluation::setFitResultSortPriority(QFRawDataRecord *r, int index, const QString &parameterID, bool sortPriority) {
    setFitResultSortPriority(r, getEvaluationResultID(index), parameterID, sortPriority);
}

