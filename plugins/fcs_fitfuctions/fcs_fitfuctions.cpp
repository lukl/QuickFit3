#include "fcs_fitfuctions.h"


#include "qffitfunctionfcsmultidiffusion.h"
#include "qffitfunctionfcsadiff.h"
#include "qffitfunctionfcsdiff.h"
#include "qffitfunctionfcsdiff_relbright.h"
#include "qffitfunctionfcsadiff_relbright.h"
#include "qffitfunctionfcsexp.h"
#include "qffitfunctionfcssimplediff.h"
#include "qffitfunctionfcsflow.h"
#include "qffitfunctionfcsrot.h"
#include "qffitfunctionfcsoscillation.h"
#include "qffitfunctionfcsdiffD.h"
#include "qffitfunctionfcsflowv.h"
#include "qffitfunctioncsfcs.h"
#include "qffitfunctionfcspowerlaw.h"
#include "qffitfunctionfcssimpleexp.h"
#include "qffitfunctionfcsdiffDAfterpulse.h"







QStringList QFPFitFunctionFCS::getIDs() const {
    QStringList res;
    res<<"fcs_diff"<<"fcs_adiff_diffbright"<<"fcs_diff_diffbright"<<"fcs_diff_d"<<"fcs_diff_d_afterpulse"<<"fcs_diff_d_wz"<<"fcs_diff_d_wz_afterpulse"<<"fcs_diff1"<<"fcs_adiff"<<"fcs_multidiff"<<"fcs_diffflow"<<"fcs_diffflowv"<<"fcs_diffrot"<<"fcs_oscillation"<<"fcs_cs_diff_d"<<"fcs_diff_cs_d_wz"<<"fcs_simpleexp"<<"fcs_exp"<<"fcs_powerlaw";
    return res;
}

QFFitFunction* QFPFitFunctionFCS::get(QString id, QObject* parent) const  {
    if (id=="fcs_diff") {
        return new QFFitFunctionFCSDiff();
    } else if (id=="fcs_diff_d") {
        return new QFFitFunctionFCSDiffD(true);
    } else if (id=="fcs_exp") {
        return new QFFitFunctionFCSExp();
    } else if (id=="fcs_diff_d_wz") {
        return new QFFitFunctionFCSDiffD(false);
    } else if (id=="fcs_diff1") {
        return new QFFitFunctionFCSSimpleDiff();
    } else if (id=="fcs_adiff") {
        return new QFFitFunctionFCSADiff();
    } else if (id=="fcs_multidiff") {
        return new QFFitFunctionFCSMultiDiffusion();
    } else if (id=="fcs_diffflow") {
        return new QFFitFunctionFCSFlow();
    } else if (id=="fcs_diffrot") {
        return new QFFitFunctionFCSRot();
    } else if (id=="fcs_oscillation") {
        return new QFFitFunctionFCSOscillation();
    } else if (id=="fcs_diffflowv") {
        return new QFFitFunctionFCSFlowV();
    } else if (id=="fcs_cs_diff_d") {
        return new QFFitFunctionCSFCSDiffD(true);
    } else if (id=="fcs_diff_cs_d_wz") {
        return new QFFitFunctionCSFCSDiffD(false);
    } else if (id=="fcs_powerlaw") {
        return new QFFitFunctionFCSPowerLaw();
    } else if (id=="fcs_simpleexp") {
        return new QFFitFunctionFCSSimpleExp();
    } else if (id=="fcs_diff_d") {
        return new QFFitFunctionFCSDiffDAfterPulse(true);
    } else if (id=="fcs_diff_d_wz_afterpulse") {
        return new QFFitFunctionFCSDiffDAfterPulse(false);
    } else if (id=="fcs_diff_diffbright") {
        return new QFFitFunctionFCSDiffDifferentBrightness();
    } else if (id=="fcs_adiff_diffbright") {
        return new QFFitFunctionFCSADiffDifferentBrightness();
    }
    return NULL;
}

Q_EXPORT_PLUGIN2(qffcsfitfuncs, QFPFitFunctionFCS)
