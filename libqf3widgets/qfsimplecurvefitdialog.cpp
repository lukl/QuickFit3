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


#include "qfsimplecurvefitdialog.h"
#include "ui_qfsimplecurvefitdialog.h"
#include "dlgqfprogressdialog.h"
#include "qffitalgorithmthreaddedfit.h"
#include "programoptions.h"
#include "qftools.h"

QFSimpleCurveFitDialog::QFSimpleCurveFitDialog(const QVector<double>& dataXin, const QVector<double>& dataYin, const QVector<double>& dataWin, QWidget *parent, bool logX, bool logY) :
    QFDialog(parent),
    ui(new Ui::QFSimpleCurveFitDialog)
{
    ui->setupUi(this);
    ui->chkLogX->setChecked(logX);
    ui->chkLogY->setChecked(logY);


    ui->pltDistribution->getXAxis()->set_axisLabel(tr("x-data"));
    ui->pltDistribution->getYAxis()->set_axisLabel(tr("y-data"));


    dataX=dataXin;
    dataY=dataYin;
    //qDebug()<<"dataX.size="<<dataX.size();
    //qDebug()<<"dataY.size="<<dataX.size();
    datapoints=qMin(dataX.size(), dataY.size());
    if (dataWin.size()>0) {
        dataW=dataWin;
        //qDebug()<<"dataW.size="<<dataW.size();
        if (dataW.size()>0 && dataW.size()<datapoints) datapoints=dataW.size();
    } else {
        dataW=QVector<double>(datapoints, 1);
    }
    if (dataX.size()>datapoints) dataX.remove(dataX.size()-(dataX.size()-datapoints), (dataX.size()-datapoints));
    if (dataY.size()>datapoints) dataY.remove(dataY.size()-(dataY.size()-datapoints), (dataY.size()-datapoints));
    if (dataW.size()>datapoints) dataW.remove(dataW.size()-(dataW.size()-datapoints), (dataW.size()-datapoints));

    //qDebug()<<"datapoints="<<datapoints;
    for (int i=datapoints-1; i>=0; i--) {
        if (!QFFloatIsOK(dataX[i]) || !QFFloatIsOK(dataY[i]) || (dataW.size()==dataX.size() && !QFFloatIsOK(dataW[i]))) {
            dataX.remove(i, 1);
            dataY.remove(i, 1);
            dataW.remove(i, 1);
        }
    }
    datapoints=dataX.size();
    weights.clear();
    for (int i=0; i<dataW.size(); i++) weights<<1.0/qfSqr(dataW[i]);
    //qDebug()<<"datapoints_after_clean="<<datapoints;

    if (dataX.size()>0) {
        ui->datacut->set_min(qfstatisticsMin(dataX));
        ui->datacut->set_max(qfstatisticsMax(dataX));
    }
    ui->datacut->setLogScale(logX, 20);

    ui->tabParams->setItemDelegate(new QFFitFunctionValueInputDelegate(ui->tabParams));
    parameterTable=new QFFitFunctionValueInputTable(this);

    ui->tabParams->setModel(parameterTable);



    ui->cmbFitAlgorithm->setCurrentAlgorithm("fit_lmfit");
    ui->cmbFitFunction->setCurrentFitFunction("gen_line");






    ui->pltResiduals->set_displayToolbar(false);
    ui->pltResiduals->getXAxis()->set_axisLabel(tr("X"));
    //ui->pltResiduals->getXAxis()->set_logAxis(true);
    ui->pltResiduals->getYAxis()->set_axisLabel(tr("residuals"));
    ui->pltDistribution->getXAxis()->set_axisLabel("");
    ui->pltDistribution->getYAxis()->set_axisLabel(tr("Y"));
    ui->pltDistribution->getXAxis()->set_drawMode1(JKQTPCADMticks);
    ui->pltResiduals->getXAxis()->set_drawMode1(JKQTPCADMcomplete);
    ui->pltResiduals->get_plotter()->setBorder(1,1,1,1);
    ui->pltDistribution->get_plotter()->setBorder(1,1,1,1);
    ui->pltResiduals->synchronizeToMaster(ui->pltDistribution, true, false);
    ui->pltDistribution->get_plotter()->set_useAntiAliasingForSystem(true);
    ui->pltDistribution->get_plotter()->set_useAntiAliasingForGraphs(true);
    ui->pltResiduals->get_plotter()->set_useAntiAliasingForSystem(true);
    ui->pltResiduals->get_plotter()->set_useAntiAliasingForGraphs(true);
    ui->pltResiduals->set_displayMousePosition(false);
    ui->pltDistribution->set_displayMousePosition(false);
    ui->pltDistribution->get_plotter()->set_keyFontSize(9);
    ui->pltResiduals->get_plotter()->set_keyFontSize(9);
    ui->pltResiduals->useExternalDatastore(ui->pltDistribution->getDatastore());
    ui->pltResiduals->setMinimumHeight(75);
    ui->pltResiduals->get_plotter()->set_showKey(false);
    ui->pltDistribution->setMinimumHeight(75);
    ui->pltDistribution->get_plotter()->setBorder(1,1,1,1);
    ui->pltDistribution->get_plotter()->set_useAntiAliasingForSystem(true);
    ui->pltDistribution->get_plotter()->set_useAntiAliasingForGraphs(true);
    ui->pltDistribution->get_plotter()->set_keyFontSize(9);
    ui->pltDistribution->get_plotter()->set_keyPosition(JKQTPkeyInsideTopLeft);
    QColor cb("white");
    cb.setAlphaF(0.5);
    ui->pltDistribution->get_plotter()->set_keyBackgroundColor(cb);
    ui->pltDistribution->get_plotter()->set_keyPosition(JKQTPkeyInsideTopLeft);

    connect(ui->pltDistribution->get_plotter()->get_actZoomAll(), SIGNAL(triggered()), ui->pltResiduals, SLOT(zoomToFit()));
    connect(ui->pltResiduals->get_plotter(), SIGNAL(zoomChangedLocally(double,double,double,double,JKQtBasePlotter*)), ui->pltDistribution, SLOT(pzoomChangedLocally(double,double,double,double,JKQtBasePlotter*)));
    connect(ui->pltDistribution->get_plotter(), SIGNAL(zoomChangedLocally(double,double,double,double,JKQtBasePlotter*)), ui->pltResiduals, SLOT(pzoomChangedLocally(double,double,double,double,JKQtBasePlotter*)));


    QSettings* set=ProgramOptions::getInstance()->getQSettings();
    if (set) {
        loadWidgetGeometry(*set, this, pos(), size(), "QFSimpleCurveFitDialog/windowsize");
        ui->chkPlotErrors->setChecked(set->value("QFSimpleCurveFitDialog/ploterrors", ui->chkPlotErrors->isChecked()).toBool());
        ui->chkWeightedResiduals->setChecked(set->value("QFSimpleCurveFitDialog/weightedresiduals", ui->chkWeightedResiduals->isChecked()).toBool());
        ui->cmbFitAlgorithm->setCurrentAlgorithm(set->value("QFSimpleCurveFitDialog/algorithm", ui->cmbFitAlgorithm->currentFitAlgorithmID()).toString());
        ui->cmbFitFunction->setCurrentFitFunction(set->value("QFSimpleCurveFitDialog/model", ui->cmbFitFunction->currentFitFunctionID()).toString());
    }

    methodChanged(0);//ui->cmbFitFunction->currentIndex());
    connectSignals(true);
    on_btnFit_clicked();
    setWindowFlags(windowFlags()|Qt::WindowMinMaxButtonsHint);
}

QFSimpleCurveFitDialog::~QFSimpleCurveFitDialog()
{
    QSettings* set=ProgramOptions::getInstance()->getQSettings();
    if (set) {
        saveWidgetGeometry(*set, this, "QFSimpleCurveFitDialog/windowsize");
        set->setValue("QFSimpleCurveFitDialog/ploterrors", ui->chkPlotErrors->isChecked());
        set->setValue("QFSimpleCurveFitDialog/weightedresiduals", ui->chkWeightedResiduals->isChecked());
        set->setValue("QFSimpleCurveFitDialog/algorithm", ui->cmbFitAlgorithm->currentFitAlgorithmID());
        set->setValue("QFSimpleCurveFitDialog/model", ui->cmbFitFunction->currentFitFunctionID());
    }

    delete ui;
}

QVector<double> QFSimpleCurveFitDialog::getFitResults() const
{
    return lastResultD;
}

QString QFSimpleCurveFitDialog::getFitModelID() const
{
    return ui->cmbFitFunction->currentFitFunctionID();
}

void QFSimpleCurveFitDialog::saveResults() {
    accept();
}



void QFSimpleCurveFitDialog::on_btnFit_clicked()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    connectSignals(false);

    QFFitFunction* model=ui->cmbFitFunction->createCurrentInstance();
    QFFitAlgorithm* algorithm=ui->cmbFitAlgorithm->createCurrentInstance();

    int rmin=getRangeMin();
    int rmax=getRangeMax();

    QVector<double> datX;
    QVector<double> datY;
    QVector<double> datW;

    /*for (int i=rmin; i<=rmax; i++) {
        datX<<dataX[i];
        datY<<dataY[i];
        datW<<dataW[i];
    }*/
    for (int i=0; i<datapoints; i++) {
        if (dataX[i]>=ui->datacut->get_userMin() && dataX[i]<=ui->datacut->get_userMax()) {
            datX<<dataX[i];
            datY<<dataY[i];
            datW<<dataW[i];
        }
    }
    double* dx=datX.data();
    double* dy=datY.data();
    double* dw=datW.data();
    int items=datX.size();


    bool ok=false;
    QString error="";

    if (model && algorithm && items>1) {
        QVector<double> fitParamsIn, fitParamsOut, fitParamsErrOut, fitParamsErrIn, fitParamsMin, fitParamsMax;
        QVector<bool> fitFix;
        QStringList ids=model->getParameterIDs();
        for (int i=0; i<ids.size(); i++) {
            QFFitFunction::ParameterDescription d=model->getDescription(ids[i]);
            fitParamsIn<<getParamValue(ids[i], d.initialValue);
            fitParamsMin<<getParamMin(ids[i], d.minValue);
            fitParamsMax<<getParamMax(ids[i], d.maxValue);
            fitFix<<getParamFix(ids[i], d.initialFix);
            fitParamsErrOut<<0.0;
            //qDebug()<<"in:  "<<ids[i]<<" = "<<fitParamsIn.last()<<fitFix.last();
        }
        fitParamsOut=fitParamsIn;
        fitParamsErrIn=fitParamsErrOut;

        ok=false;




        dlgQFProgressDialog* progress=new dlgQFProgressDialog(this);
        dlgQFProgressDialogReporter* progressReporter=new dlgQFProgressDialogReporter(progress);
        progress->setWindowTitle(tr("Curve fit progress"));
        progress->reportTask(tr("fitting curve ..."));
        progress->reportStatus(tr("setting up curve fit ..."));
        progress->setProgressMax(100);
        progress->setProgress(0);
        progress->show();
        algorithm->setReporter(progressReporter);

        QFFitAlgorithm::FitResult res;
        try {


            model->calcParameter(fitParamsIn.data(), fitParamsErrOut.data());
            model->calcParameter(fitParamsIn.data(), fitParamsErrOut.data());


            if (!progress->isCanceled()) {

                progress->reportStatus(tr("fitting ..."));
                progress->setProgressMax(100);
                progress->setProgress(0);
                QFFitAlgorithmThreadedFit* doFitThread=new QFFitAlgorithmThreadedFit(this);
                doFitThread->init(algorithm, fitParamsOut.data(), fitParamsErrOut.data(), dx, dy, dw, items, model, fitParamsIn.data(), fitFix.data(), fitParamsMin.data(), fitParamsMax.data());
                doFitThread->start(QThread::HighPriority);
                QTime t;
                t.start();
                while (!doFitThread->isFinished()) {
                    if (t.elapsed()>10) {
                        QApplication::processEvents(QEventLoop::AllEvents, 20);
                        if (progress->isCanceled()) {
                          doFitThread->terminate();
                          break;
                        }
                        t.start();
                    }
                }
                progress->setProgressFull();
                progress->reportStatus(tr("calculating parameters and errors ..."));
                ok=!progress->isCanceled();
                if (progress->isCanceled()) {
                    error=tr("fit canceled by user");
                }
                res=doFitThread->getResult();
                delete doFitThread;
            }



            for (int i=0; i<model->paramCount(); i++) {
                if (!(model->isParameterVisible(i, fitParamsOut.data()) && (!fitFix[i]) && model->getDescription(i).fit)) {
                    fitParamsErrOut[i]=fitParamsErrIn[i];
                }
            }

            model->calcParameter(fitParamsOut.data(), fitParamsErrOut.data());
            model->sortParameter(fitParamsOut.data(), fitParamsErrOut.data(), fitFix.data());
            model->calcParameter(fitParamsOut.data(), fitParamsErrOut.data());



        } catch(std::exception& E) {
            QFPluginLogTools::log_error(tr("error during curve fitting, error message: %1\n").arg(E.what()));
            error=E.what();
            ok=false;
        }




        lastResults.clear();
        lastResultD.clear();
        resultComment="";
        if (ok && res.fitOK) {
            for (int i=0; i<ids.size(); i++) {
                paramMap[ids[i]].value=fitParamsOut[i];
                paramMap[ids[i]].error=fitParamsErrOut[i];
                paramMap[ids[i]].fix=fitFix[i];
                lastResults<<fitParamsOut[i];
                lastResultD<<fitParamsOut[i];
            }
            resultComment=tr("fit result for: %1").arg(model->shortName());
            ok=true;
        } else {
            error=tr("Fit failed.\n  Reason: %1.").arg(res.messageSimple);
            ok=false;
        }


        if (progress) {
            algorithm->setReporter(NULL);
            delete progressReporter;
            progress->close();
            progress->deleteLater();
        }





    }

    updateFitStatistics();

    QApplication::restoreOverrideCursor();
    parameterTable->rebuildModel();
    replotGraph();
    if (!ok) {
        QMessageBox::critical(this, tr("Fit Error"), tr("an error occured during the model fit.\nerror message:\n    %1").arg(error));
    }
    connectSignals(true);
    if (model) delete model;
    if (algorithm) delete algorithm;
}

void QFSimpleCurveFitDialog::on_btnGuess_clicked()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    connectSignals(false);

    QFFitFunction* model=ui->cmbFitFunction->createCurrentInstance();

    int rmin=getRangeMin();
    int rmax=getRangeMax();

    QVector<double> datX;
    QVector<double> datY;
    QVector<double> datW;

    for (int i=0; i<datapoints; i++) {
        if (dataX[i]>=ui->datacut->get_userMin() && dataX[i]<=ui->datacut->get_userMax()) {
            datX<<dataX[i];
            datY<<dataY[i];
            datW<<dataW[i];
        }
    }
    double* dx=datX.data();
    double* dy=datY.data();
    double* dw=datW.data();
    int items=datX.size();


    bool ok=false;
    QString error="";

    if (model && items>1) {
        QVector<double> fitParams, fitParamsErrs, fitParamsMin, fitParamsMax;
        QVector<bool> fitFix;
        QStringList ids=model->getParameterIDs();
        bool hasFixed=false;
        for (int i=0; i<ids.size(); i++) {
            QFFitFunction::ParameterDescription d=model->getDescription(ids[i]);
            fitParams<<getParamValue(ids[i], d.initialValue);
            fitParamsMin<<getParamMin(ids[i], d.minValue);
            fitParamsMax<<getParamMax(ids[i], d.maxValue);
            fitFix<<getParamFix(ids[i], d.initialFix);
            hasFixed=hasFixed||fitFix.last();
            fitParamsErrs<<0;
        }
        QVector<double> fitParamsInit=fitParams;
        ok=model->estimateInitial(fitParams.data(), dx, dy, items, fitFix.data());

        model->calcParameter(fitParams.data(), fitParamsErrs.data());
        model->sortParameter(fitParams.data(), fitParamsErrs.data(), fitFix.data());
        model->calcParameter(fitParams.data(), fitParamsErrs.data());








        lastResults.clear();
        lastResultD.clear();
        resultComment="";

        bool replaceFixed=true;
        if (hasFixed) {
            bool fixedChanged=false;
            for (int i=0; i<ids.size(); i++) {
                if (fitFix[i] && fitParamsInit[i]!=fitParams[i]) {
                    fixedChanged=true;
                    break;
                }
            }
            if (fixedChanged) {
                replaceFixed=(QMessageBox::question(this, tr("Parameter guess"), tr("The fit function returned guessed values for fixed parameters.\nDo you want to overwrite this fixed parameter values?"), QMessageBox::Yes|QMessageBox::No, QMessageBox::No)==QMessageBox::Yes);
            }

        }

        if (ok) {
            for (int i=0; i<ids.size(); i++) {
                if (!fitFix[i] || (fitFix[i] && replaceFixed)) {
                    paramMap[ids[i]].value=fitParams[i];
                    paramMap[ids[i]].error=0;
                    paramMap[ids[i]].fix=fitFix[i];
                    lastResults<<fitParams[i];
                    lastResultD<<fitParams[i];
                } else {
                    paramMap[ids[i]].value=fitParamsInit[i];
                    paramMap[ids[i]].error=0;
                    paramMap[ids[i]].fix=fitFix[i];
                    lastResults<<fitParamsInit[i];
                    lastResultD<<fitParamsInit[i];
                }
            }
            resultComment=tr("guess result for: %1").arg(model->shortName());
            ok=true;
        } else {
            error=tr("Guess failed.");
            ok=false;
        }





    }

    updateFitStatistics();

    QApplication::restoreOverrideCursor();
    parameterTable->rebuildModel();
    replotGraph();
    if (!ok) {
        QMessageBox::critical(this, tr("Guess Error"), tr("an error occured during the model parameter guess.\nerror message:\n    %1").arg(error));
    }

    connectSignals(true);
    if (model) delete model;
}


void QFSimpleCurveFitDialog::replotGraph()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QFFitFunction* model=ui->cmbFitFunction->createCurrentInstance();
    updateFitStatistics();

    ui->pltDistribution->set_doDrawing(false);
    ui->pltDistribution->set_emitSignals(false);
    ui->pltResiduals->set_doDrawing(false);
    ui->pltResiduals->set_emitSignals(false);
    ui->pltResiduals->getXAxis()->set_logAxis(ui->chkLogX->isChecked());
    ui->pltDistribution->getXAxis()->set_logAxis(ui->chkLogX->isChecked());
    ui->pltDistribution->getYAxis()->set_logAxis(ui->chkLogY->isChecked());
    ui->pltDistribution->get_plotter()->clearGraphs(true);
    ui->pltDistribution->get_plotter()->clearOverlayElement(true);
    ui->pltResiduals->get_plotter()->clearGraphs(true);
    ui->pltResiduals->get_plotter()->clearOverlayElement(true);

    JKQTPdatastore* ds=ui->pltDistribution->get_plotter()->getDatastore();
    ds->clear();
    size_t c_X=ds->addCopiedColumn(dataX.data(), dataX.size(), tr("x-data"));
    size_t c_Y=ds->addCopiedColumn(dataY.data(), dataY.size(), tr("y-data"));
    size_t c_W=ds->addCopiedColumn(dataW.data(), dataW.size(), tr("weight"));
    size_t c_ResY=ds->addCopiedColumn(residualsY.data(), residualsY.size(), tr("residuals"));
    size_t c_ResYW=ds->addCopiedColumn(residualsYW.data(), residualsYW.size(), tr("weighted residuals"));


    JKQTPxyLineErrorGraph* g_dist=new JKQTPxyLineErrorGraph(ui->pltDistribution->get_plotter());
    g_dist->set_drawLine(true);
    g_dist->set_title(tr("data"));
    g_dist->set_xColumn(c_X);
    g_dist->set_yColumn(c_Y);
    g_dist->set_yErrorColumn(c_W);
    g_dist->set_xErrorStyle(JKQTPnoError);
    if (ui->chkPlotErrors->isChecked()) g_dist->set_yErrorStyle(JKQTPerrorBars);
    else g_dist->set_yErrorStyle(JKQTPnoError);
    g_dist->set_color(QColor("blue"));
    g_dist->set_errorColor(g_dist->get_color().lighter());
    g_dist->set_drawLine(false);
    g_dist->set_symbol(JKQTPcross);
    g_dist->set_symbolSize(7);
    ui->pltDistribution->addGraph(g_dist);

    JKQTPoverlayVerticalRange* ovlRange=new JKQTPoverlayVerticalRange(ui->datacut->get_userMin(), ui->datacut->get_userMax(), ui->pltDistribution->get_plotter());
    ovlRange->set_inverted(true);
    QColor fillRange=QColor("grey");
    fillRange.setAlphaF(0.75);
    ovlRange->set_fillColor(fillRange);
    ui->pltDistribution->get_plotter()->addOverlayElement(ovlRange);

    if (model) {
        JKQTPxQFFitFunctionLineGraph* g_fit=new JKQTPxQFFitFunctionLineGraph(ui->pltDistribution->get_plotter());
        g_fit->set_drawLine(true);
        g_fit->set_title(fitresult);
        g_fit->set_color(QColor("red"));
        g_fit->set_fitFunction(model, true);
        QStringList ids=model->getParameterIDs();
        QVector<double> vecP;
        for (int i=0; i<ids.size(); i++) {
            QFFitFunction::ParameterDescription d=model->getDescription(ids[i]);
            vecP<<getParamValue(ids[i], d.initialValue);
        }
        g_fit->set_params(vecP);
        ui->pltDistribution->addGraph(g_fit);
    }


    JKQTPxyLineGraph* g_res=new JKQTPxyLineGraph(ui->pltResiduals->get_plotter());
    g_res->set_drawLine(true);
    g_res->set_xColumn(c_X);
    if (ui->chkWeightedResiduals->isChecked()) {
        g_res->set_title(tr("weighted residuals"));
        g_res->set_yColumn(c_ResYW);
    } else {
        g_res->set_title(tr("residuals"));
        g_res->set_yColumn(c_ResY);
    }
    g_res->set_color(g_dist->get_color());
    g_res->set_symbol(JKQTPcross);
    g_res->set_symbolSize(7);
    ui->pltResiduals->addGraph(g_res);


    ui->pltResiduals->set_doDrawing(true);
    ui->pltResiduals->set_emitSignals(true);
    ui->pltResiduals->zoomToFit();
    ui->pltResiduals->update_plot();
    ui->pltDistribution->set_doDrawing(true);
    ui->pltDistribution->set_emitSignals(true);
    ui->pltDistribution->zoomToFit();
    ui->pltDistribution->update_plot();
    QApplication::restoreOverrideCursor();
}
void QFSimpleCurveFitDialog::showHelp()
{
    QFPluginServices::getInstance()->displayHelpWindow(QFPluginServices::getInstance()->getMainHelpDirectory()+"curvefitdlg.html");
}


void QFSimpleCurveFitDialog::methodChanged(int /*method*/)
{
    parameterTable->setDoRebuildModel(false);
    parameterTable->setEditErrors(true);
    parameterTable->setEditFix(true);
    parameterTable->setEditRanges(true);
    parameterNames.clear();
    parameterLabels.clear();
    //paramMap.clear();
    QFFitFunction* model=ui->cmbFitFunction->createCurrentInstance();
    QStringList ids=model->getParameterIDs();
    for (int i=0; i<ids.size(); i++) {
        QFFitFunction::ParameterDescription d=model->getDescription(ids[i]);
        if (!paramMap.contains(ids[i])) {
            paramMap[ids[i]].fix=d.initialFix;
            paramMap[ids[i]].error=0;
            paramMap[ids[i]].value=d.initialValue;
            paramMap[ids[i]].max=d.maxValue;
            paramMap[ids[i]].min=d.minValue;
        }
        paramMap[ids[i]].editable=d.userEditable;
    }

    ui->btnGuess->setEnabled(model->estimateInitial(NULL, NULL, NULL, 0));

    parameterTable->setWriteTo(&paramMap, model, true);
    parameterTable->setDoRebuildModel(true);
    ui->tabParams->setUpdatesEnabled(false);
    for (int i=0; i<ui->tabParams->model()->rowCount(); i++) {
        ui->tabParams->setRowHeight(i, QFontMetricsF(ui->tabParams->font()).height()*1.6);
    }
    ui->tabParams->verticalHeader()->setVisible(false);
    //ui->tabParams->horizontalHeader()
    for (int i=0; i<ui->tabParams->model()->columnCount(); i++) {
        if (i==0) ui->tabParams->setColumnWidth(i, 100);
        else if ((i-1)%parameterTable->getColsPerRDR()==0) ui->tabParams->setColumnWidth(i, 75);
        else if ((i-1)%parameterTable->getColsPerRDR()==1) ui->tabParams->setColumnWidth(i, 50);
        else if ((i-1)%parameterTable->getColsPerRDR()==2) ui->tabParams->setColumnWidth(i, 20);
        else if ((i-1)%parameterTable->getColsPerRDR()==3) ui->tabParams->setColumnWidth(i, 75);
        else if ((i-1)%parameterTable->getColsPerRDR()==4) ui->tabParams->setColumnWidth(i, 75);
    }
    ui->tabParams->setUpdatesEnabled(true);
    //on_btnFit_clicked();
}

void QFSimpleCurveFitDialog::updateFitStatistics()
{
    QFFitFunction* model=ui->cmbFitFunction->createCurrentInstance();
    if (model) {
        QStringList ids=model->getParameterIDs();
        QVector<double> vecP;
        for (int i=0; i<ids.size(); i++) {
            QFFitFunction::ParameterDescription d=model->getDescription(ids[i]);
            vecP<<getParamValue(ids[i], d.initialValue);
        }
        residualsY.clear();
        residualsYW.clear();

        for (int i=0; i<dataX.size(); i++) {
            double v=model->evaluate(dataX[i], vecP.data());
            if (i<dataY.size()) {
                residualsY<<dataY[i]-v;
            } else {
                break;
            }
            if (i<dataY.size() && i<dataW.size()) {
                residualsYW<<(dataY[i]-v)/dataW[i];
            }
        }

        int items=residualsY.size();

        // estimate goodness-of-fit
        resultStat="";
        if (residualsY.size()>0) {
            double chi2=0;
            double chi2w=0;
            double TSS=0;
            double obsMean=0;
            for (int i=0; i<items; i++) {
                obsMean+=dataY[i];
            }
            obsMean=obsMean/double(items);
            for (int i=0; i<items; i++) {
                chi2+=qfSqr(residualsY[i]);
                chi2w+=qfSqr(residualsYW[i]);
                TSS+=qfSqr(dataY[i]-obsMean);
            }
            double R2=1.0-chi2/TSS;
            paramMap["chi2"].value=chi2;
            paramMap["chi2"].error=0;
            paramMap["chi2"].fix=false;
            paramMap["chi2"].editable=false;
            resultStat+=tr("\\chi^2=%1").arg(chi2);
            paramMap["R2"].value=R2;
            paramMap["R2"].error=0;
            paramMap["R2"].fix=false;
            paramMap["R2"].editable=false;
            resultStat+=tr(", R^2=%1").arg(R2);

        }

        ui->labFitResult->setText(resultStat);


        delete model;
    }

}

void QFSimpleCurveFitDialog::connectSignals(bool connectS)
{
    if (connectS) {
        connect(parameterTable, SIGNAL(fitParamChanged()), this, SLOT(replotGraph()));
        connect(ui->cmbFitFunction, SIGNAL(currentIndexChanged(int)), this, SLOT(methodChanged(int)));
        connect(ui->chkLogX, SIGNAL(toggled(bool)), this, SLOT(replotGraph()));
        connect(ui->chkLogY, SIGNAL(toggled(bool)), this, SLOT(replotGraph()));
        connect(ui->chkPlotErrors, SIGNAL(toggled(bool)), this, SLOT(replotGraph()));
        connect(ui->datacut, SIGNAL(slidersChanged(double,double,double,double)), this, SLOT(replotGraph()));
        connect(ui->chkWeightedResiduals, SIGNAL(toggled(bool)), this, SLOT(replotGraph()));
    } else {
        disconnect(parameterTable, SIGNAL(fitParamChanged()), this, SLOT(replotGraph()));
        disconnect(ui->cmbFitFunction, SIGNAL(currentIndexChanged(int)), this, SLOT(methodChanged(int)));
        disconnect(ui->chkLogX, SIGNAL(toggled(bool)), this, SLOT(replotGraph()));
        disconnect(ui->chkLogY, SIGNAL(toggled(bool)), this, SLOT(replotGraph()));
        disconnect(ui->chkPlotErrors, SIGNAL(toggled(bool)), this, SLOT(replotGraph()));
        disconnect(ui->datacut, SIGNAL(slidersChanged(double,double,double,double)), this, SLOT(replotGraph()));
        disconnect(ui->chkWeightedResiduals, SIGNAL(toggled(bool)), this, SLOT(replotGraph()));
    }
}

int QFSimpleCurveFitDialog::getRangeMin()
{
    int rm=0;
    for (int i=0; i<datapoints; i++) {
        if (dataX[i]>=ui->datacut->get_userMin()) {
            rm=i;
            break;
        }
    }
    return qBound(rm, 0, dataX.size()-1);
}

int QFSimpleCurveFitDialog::getRangeMax()
{
/*    int rm=datapoints-1;
    for (int i=datapoints-1; i>=0; i--) {
        if (dataX[i]>ui->datacut->get_userMax()) {
            rm=i;
        } else {
            break;
        }
    }

    return qBound(0, rm, dataX.size()-1);*/
    int rm=getRangeMin();
    for (int i=rm; i<dataX.size(); i++) {
        if (dataX[i]<=ui->datacut->get_userMax()) {
            rm=i;
        } else {
            break;
        }
    }

    return qBound(0, rm, dataX.size()-1);

}



double QFSimpleCurveFitDialog::getParamValue(const QString &param, double defaultVal) const
{
    return paramMap.value(param, QFFitFunctionValueInputTableFitParamData(defaultVal,0)).value;
}

double QFSimpleCurveFitDialog::getParamError(const QString &param, double defaultVal) const
{
    return paramMap.value(param, QFFitFunctionValueInputTableFitParamData(0,defaultVal)).error;
}

bool QFSimpleCurveFitDialog::getParamFix(const QString &param, bool defaultVal) const
{
    return paramMap.value(param, QFFitFunctionValueInputTableFitParamData(0,0,defaultVal)).fix;
}

double QFSimpleCurveFitDialog::getParamMin(const QString &param, double defaultVal) const
{
    return paramMap.value(param, QFFitFunctionValueInputTableFitParamData(0,0,false,defaultVal)).min;
}

double QFSimpleCurveFitDialog::getParamMax(const QString &param, double defaultVal) const
{
    return paramMap.value(param, QFFitFunctionValueInputTableFitParamData(0,0,false,defaultVal)).max;
}

