/*
    Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center

    

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



#include "qfrdrimagingfcscorrelationdialog.h"
#include "ui_qfrdrimagingfcscorrelationdialog.h"
#include "qfrdrimagingfcscorrelationjobthread.h"
#include<QtGlobal>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#include <QRegularExpression>
#else
#include <QtGui>
#include <QRegExp>
#endif


#include <QDebug>
#include "qmodernprogresswidget.h"
#include "qfrdrimagingfcsdataexplorer.h"
#include "qfrdrimagingfcstools.h"
#include "qfstyledbutton.h"
#include "qfcompleterfromfile.h"

#define UPDATE_TIMEOUT 250

QFRDRImagingFCSCorrelationDialog::QFRDRImagingFCSCorrelationDialog(QFPluginServices* pluginservices, ProgramOptions* opt, QWidget *parent) :
    QFDialog(parent),
    ui(new Ui::QFRDRImagingFCSCorrelationDialog)
{
    frame_data=NULL;
    this->pluginServices=pluginservices;
    closing=false;
    image_width=image_height=1;
    this->options=opt;
    lastImagefileDir="";
    filenameDisplayed="";

    ui->setupUi(this);

    ui->scrollAreaParams->setMaximumHeight(int(1.1*ui->widDetails->height()));
    //qDebug()<<ui->scrollAreaParams->maximumHeight()<<ui->widDetails->height()<<int(1.1*ui->widDetails->height());
    QRegExp rxv("\\s*([\\+\\-]?\\d+\\s*\\,\\s*[\\+\\-]?\\d+(\\s*;\\s*[\\+\\-]?\\d+\\s*\\,\\s*[\\+\\-]?\\d+)*)");
    ui->edtDCCF->setValidator(new QRegExpValidator(rxv, ui->edtDCCF));
    ui->edtFrameTime->setRange(1e-10,1e10);
    ui->edtFrameTime->setValue(10000);
    ui->edtFrameRate->setRange(1e-10,1e10);
    ui->edtFrameRate->setValue(100);
    ui->edtOffset->setRange(-1e10,1e10);
    ui->edtOffset->setValue(0);
    //ui->edtDecayA->setCheckBounds(false, false);
    //ui->edtDecayB->setCheckBounds(false, false);
    //ui->spinDecay->setCheckBounds(false, false);
    //ui->edtDecayA->setValue(1000);
    //ui->edtDecayB->setValue(1000);
    //ui->spinDecay->setValue(10000);
    ui->edtPostfix->addInsertContextMenuEntry(tr("insert %counter%"), QString("%counter%"));
    ui->edtPostfix->addInsertContextMenuEntry(tr("insert %s%"), QString("%s%"));
    ui->edtPostfix->addInsertContextMenuEntry(tr("insert %p%"), QString("%p%"));
    ui->edtPostfix->addInsertContextMenuEntry(tr("insert %m%"), QString("%m%"));
    ui->edtPostfix->addInsertContextMenuEntry(tr("insert %framerate%"), QString("%framerate%"));
    ui->edtPostfix->addInsertContextMenuEntry(tr("insert %frametime%"), QString("%frametime%"));
    ui->edtPostfix->addInsertContextMenuEntry(tr("insert %first%"), QString("%first%"));
    ui->edtPostfix->addInsertContextMenuEntry(tr("insert %last%"), QString("%last%"));
    ui->edtPostfix->addInsertContextMenuEntry(tr("insert %backoffset%"), QString("%backoffset%"));
    ui->edtPostfix->addInsertContextMenuEntry(tr("insert %segments%"), QString("%segments%"));
    ui->edtPostfix->addInsertContextMenuEntry(tr("insert %backcorrectionid%"), QString("%backcorrectionid%"));
    ui->edtPostfix->addInsertContextMenuEntry(tr("insert %correlatorid%"), QString("%correlatorid%"));
    ui->edtPostfix->addInsertContextMenuEntry(tr("insert %binning%"), QString("%binning%"));
    ui->edtPostfix->addInsertContextMenuEntry(tr("insert %backcorrection%"), QString("%backcorrection%"));
    ui->edtPostfix->addInsertContextMenuEntry(tr("insert %correlator%"), QString("%correlator%"));
    ui->edtPostfix->addInsertContextMenuEntry(tr("insert %bleach%"), QString("%bleach%"));
    //ui->edtPostfix->addInsertContextMenuEntry(tr("insert %%"), QString("%%"));

    ui->edtDCCF->addInsertContextMenuEntry(tr("4 neighbors"), QString("1,0;-1,0;0,1;0,-1;"));
    ui->edtDCCF->addInsertContextMenuEntry(tr("8 neighbors"), QString("1,0;1,1;0,1;-1,1;-1,0;-1,-1;0,-1;1,-1;"));
    ui->edtDCCF->addInsertContextMenuEntry(tr("4 to the right"), QString("1,0;2,0;3,0;4,0;"));
    ui->edtDCCF->addInsertContextMenuEntry(tr("8 to the right"), QString("1,0;2,0;3,0;4,0;5,0;6,0;7,0;8,0;"));
    ui->edtDCCF->addButton(new QFStyledButton(QFStyledButton::SelectFromCompleter, ui->edtDCCF, ui->edtDCCF));
    ui->edtDCCF->addButton(new QFStyledButton(QFStyledButton::ClearLineEdit, ui->edtDCCF, ui->edtDCCF));
    QFCompleterFromFile* cc=new QFCompleterFromFile(ui->edtDCCF);
    cc->setFilename(ProgramOptions::getInstance()->getConfigFileDirectory()+"plugins/imaging_fcs/completers/dccf.txt");
    ui->edtDCCF->setCompleter(cc);

    imageFilters.clear();
    ui->cmbFileformat->clear();

    actCropCenter=new QFActionWithNoMenuRole(tr("crop center ..."), this);
    connect(actCropCenter, SIGNAL(triggered()), this, SLOT(cropCenter()));
    ui->btnSelCrop->addAction(actCropCenter);

    actCropLeftCenter=new QFActionWithNoMenuRole(tr("crop left center ..."), this);
    connect(actCropLeftCenter, SIGNAL(triggered()), this, SLOT(cropLeftCenter()));
    ui->btnSelCrop->addAction(actCropLeftCenter);

    actCropRightCenter=new QFActionWithNoMenuRole(tr("crop right center ..."), this);
    connect(actCropRightCenter, SIGNAL(triggered()), this, SLOT(cropRightCenter()));
    ui->btnSelCrop->addAction(actCropRightCenter);

    actCropTopCenter=new QFActionWithNoMenuRole(tr("crop top center ..."), this);
    connect(actCropTopCenter, SIGNAL(triggered()), this, SLOT(cropTopCenter()));
    ui->btnSelCrop->addAction(actCropTopCenter);

    actCropBottomCenter=new QFActionWithNoMenuRole(tr("crop bottom center ..."), this);
    connect(actCropBottomCenter, SIGNAL(triggered()), this, SLOT(cropBottomCenter()));
    ui->btnSelCrop->addAction(actCropBottomCenter);


    actCropLeftHalf=new QFActionWithNoMenuRole(tr("crop left half-image ..."), this);
    connect(actCropLeftHalf, SIGNAL(triggered()), this, SLOT(cropLeftHalf()));
    ui->btnSelCrop->addAction(actCropLeftHalf);

    actCropRightHalf=new QFActionWithNoMenuRole(tr("crop right half-image ..."), this);
    connect(actCropRightHalf, SIGNAL(triggered()), this, SLOT(cropRightHalf()));
    ui->btnSelCrop->addAction(actCropRightHalf);

    actCropTopHalf=new QFActionWithNoMenuRole(tr("crop top half-image ..."), this);
    connect(actCropTopHalf, SIGNAL(triggered()), this, SLOT(cropTopHalf()));
    ui->btnSelCrop->addAction(actCropTopHalf);

    actCropBottomHalf=new QFActionWithNoMenuRole(tr("crop bottom half-image ..."), this);
    connect(actCropBottomHalf, SIGNAL(triggered()), this, SLOT(cropBottomHalf()));
    ui->btnSelCrop->addAction(actCropBottomHalf);

    //ui->cmbCorrelator->addItem(tr("Multi-Tau 2 (one monitor for all lags)"));
    //ui->cmbCorrelator->addItem(tr("!!! Experimental Corelator from Shared Library [don't use]"));



    // add more fileformats here
    imageFilters=QFRDRImagingFCSCorrelationJobThread::getImageFilterList(pluginServices);
    imageFormatNames=QFRDRImagingFCSCorrelationJobThread::getImageFormatNameList(pluginServices);
    ui->cmbFileformat->clear();
    ui->cmbFileformat->addItems(imageFormatNames);

    lastImagefileFilter=imageFilters[0];
    connect(ui->edtFrameRate, SIGNAL(valueChanged(double)), this, SLOT(frameRateChanged(double)));
    connect(ui->edtFrameTime, SIGNAL(valueChanged(double)), this, SLOT(frameTimeChanged(double)));
    connect(ui->spinFirstFrame, SIGNAL(valueChanged(int)), this, SLOT(updateFrameCount()));
    connect(ui->spinLastFrame, SIGNAL(valueChanged(int)), this, SLOT(updateFrameCount()));
    connect(ui->spinSegments, SIGNAL(valueChanged(int)), this, SLOT(updateFrameCount()));
    connect(ui->spinSegments, SIGNAL(valueChanged(int)), this, SLOT(updateBlocking()));
    connect(ui->spinStatistics, SIGNAL(valueChanged(int)), this, SLOT(updateFrameCount()));
    connect(ui->spinBackStatistics, SIGNAL(valueChanged(int)), this, SLOT(updateFrameCount()));
    connect(ui->spinVideoFrames, SIGNAL(valueChanged(int)), this, SLOT(updateFrameCount()));
    connect(ui->btnAddSeriesJob, SIGNAL(clicked()), this, SLOT(onAddSeriesJobClicked()));
    if (opt) readSettings();
    filesToAdd.clear();
    setEditControlsEnabled(false);
    ui->btnLoadNoCount->setEnabled(false);
    ui->btnSelectImageFileNoCount->setEnabled(false);
    ui->btnNextCell->setEnabled(false);
    ui->btnAddCellsAndJobs->setEnabled(false);
    on_cmbDualView_currentIndexChanged(ui->cmbDualView->currentIndex());
    QTimer::singleShot(UPDATE_TIMEOUT, this, SLOT(updateProgress()));


}

QFRDRImagingFCSCorrelationDialog::~QFRDRImagingFCSCorrelationDialog() {
    delete ui;
    if (frame_data) qfFree(frame_data);
    frame_data=NULL;
}

void  QFRDRImagingFCSCorrelationDialog::setEditControlsEnabled(bool enabled) {
    ui->widDetails->setEnabled(enabled);
    ui->widAddJob->setEnabled(enabled);
    ui->labDetails->setEnabled(enabled);
    ui->spinFirstFrame->setEnabled(enabled && !ui->chkFirstFrame->isChecked());
    ui->spinLastFrame->setEnabled(enabled && !ui->chkLastFrame->isChecked());
}

void QFRDRImagingFCSCorrelationDialog::on_btnDataExplorer_clicked() {
    QFRDRImagingFCSDataExplorer* explorer=new QFRDRImagingFCSDataExplorer(this);
    explorer->readSettings(*(pluginServices->getOptions()->getQSettings()), "imFCS/dataExplorer/");
    QFImporterImageSeries* reader=NULL;
    QFImporterImageSeries* readerRaw=NULL;
    if (ui->cmbFileformat->currentIndex()>=0 && ui->cmbFileformat->currentIndex()<QFRDRImagingFCSCorrelationJobThread::getImageReaderCount(pluginServices)) {
        reader=QFRDRImagingFCSCorrelationJobThread::getImageReader(ui->cmbFileformat->currentIndex(), pluginServices);
        readerRaw=QFRDRImagingFCSCorrelationJobThread::getImageReader(ui->cmbFileformat->currentIndex(), pluginServices);
    }
    if (reader) {
        //explorer->setBleachDecay1(ui->spinDecay->value());
        //explorer->setBleachDecay2(ui->spinDecay2->value());
        //explorer->setBleachA1(ui->edtDecayA->value());
        //explorer->setBleachA2(ui->edtDecayA2->value());
        //explorer->setBleachB(ui->edtDecayB->value());
        explorer->init(reader, readerRaw, ui->edtImageFile->text(), ui->chkFirstFrame->isChecked(), ui->spinFirstFrame->value(), ui->chkLastFrame->isChecked(), ui->spinLastFrame->value(), ui->chkCrop->isChecked(), ui->spinXFirst->value(), ui->spinXLast->value(), ui->spinYFirst->value(), ui->spinYLast->value(), ui->spinBinning->value(), ui->chkInterleavedBinning->isChecked(), ui->chkAverageBinning->isChecked());
        if (explorer->exec()==QDialog::Accepted) {
            //ui->spinDecay->setValue(explorer->getBleachDecay1());
            //ui->spinDecay2->setValue(explorer->getBleachDecay2());
            ui->chkCrop->setChecked(explorer->getUseCropping());
            ui->spinXFirst->setValue(explorer->getCropX0());
            ui->spinXLast->setValue(explorer->getCropX1());
            ui->spinYFirst->setValue(explorer->getCropY0());
            ui->spinYLast->setValue(explorer->getCropY1());
            ui->spinBinning->setValue(explorer->getBinning());
            ui->chkInterleavedBinning->setChecked(explorer->getInterleavedBinning());
            ui->chkAverageBinning->setChecked(explorer->getBinAverage());
            //ui->edtDecayA->setValue(explorer->getBleachA1());
            //ui->edtDecayA2->setValue(explorer->getBleachA2());
            //ui->edtDecayB->setValue(explorer->getBleachB());
            ui->chkFirstFrame->setChecked(explorer->getUseFirst());
            ui->chkLastFrame->setChecked(explorer->getUseLast());
            ui->spinFirstFrame->setValue(explorer->getFirst());
            ui->spinLastFrame->setValue(explorer->getLast());
        }
        explorer->writeSettings(*(pluginServices->getOptions()->getQSettings()), "imFCS/dataExplorer/");
        reader->close();
        readerRaw->close();
        delete reader;
        delete readerRaw;
    } else {
        QMessageBox::critical(this, explorer->windowTitle(), tr("Could not read input file!"));
    }
    delete explorer;
}

void QFRDRImagingFCSCorrelationDialog::readBackgroundFramesize()
{
    readBackgroundProperties(ui->edtBackgroundFile->text(), ui->cmbFileformat->currentIndex(), this,  &background_width, &background_height, &background_frame_count);
    QString err="";
    if (background_frame_count>0) {
        if ((image_width!=background_width) || (image_height!=background_height)) {
            err=tr("<br><font color=\"red\"><b>background and image file have different sizes!!!</b></font>");
        }
        ui->labBackgroundProps->setText(tr("size: %1&times;%2 pixels, frames: %3%4").arg(background_width).arg(background_height).arg(background_frame_count).arg(err));

    } else {
        ui->labBackgroundProps->setText("");
    }
}

void QFRDRImagingFCSCorrelationDialog::cropCenter()
{
    int cwidth=ProgramOptions::getConfigValue("imaging_fcs/dlg_correlate/crop_settings/width_center", qMin(image_width, image_height)).toInt();
    bool ok=false;
    cwidth=QInputDialog::getInt(this, tr("crop center"), tr("width of crop region:"), cwidth, 1, qMax(image_width, image_height), 1, &ok);
    if (ok) {
        ui->spinXFirst->setValue(image_width/2-cwidth/2);
        ui->spinXLast->setValue(image_width/2-cwidth/2+cwidth-1);
        ui->spinYFirst->setValue(image_height/2-cwidth/2);
        ui->spinYLast->setValue(image_height/2-cwidth/2+cwidth-1);
        ProgramOptions::setConfigValue("imaging_fcs/dlg_correlate/crop_settings/width_center", cwidth);
    }
}

void QFRDRImagingFCSCorrelationDialog::cropLeftCenter()
{
    int cwidth=ProgramOptions::getConfigValue("imaging_fcs/dlg_correlate/crop_settings/width_dvcenter", qMin(image_width, image_height)).toInt();
    bool ok=false;
    cwidth=QInputDialog::getInt(this, tr("crop left center"), tr("width of crop region:"), cwidth, 1, qMax(image_width, image_height), 1, &ok);
    if (ok) {
        ui->spinXFirst->setValue(image_width/4-cwidth/2);
        ui->spinXLast->setValue(image_width/4-cwidth/2+cwidth-1);
        ui->spinYFirst->setValue(image_height/2-cwidth/2);
        ui->spinYLast->setValue(image_height/2-cwidth/2+cwidth-1);
        ProgramOptions::setConfigValue("imaging_fcs/dlg_correlate/crop_settings/width_dvcenter", cwidth);
    }
}

void QFRDRImagingFCSCorrelationDialog::cropRightCenter()
{
    int cwidth=ProgramOptions::getConfigValue("imaging_fcs/dlg_correlate/crop_settings/width_dvcenter", qMin(image_width, image_height)).toInt();
    bool ok=false;
    cwidth=QInputDialog::getInt(this, tr("crop right center"), tr("width of crop region:"), cwidth, 1, qMax(image_width, image_height), 1, &ok);
    if (ok) {
        ui->spinXFirst->setValue(3*image_width/4-cwidth/2);
        ui->spinXLast->setValue(3*image_width/4-cwidth/2+cwidth-1);
        ui->spinYFirst->setValue(image_height/2-cwidth/2);
        ui->spinYLast->setValue(image_height/2-cwidth/2+cwidth-1);
        ProgramOptions::setConfigValue("imaging_fcs/dlg_correlate/crop_settings/width_dvcenter", cwidth);
    }
}

void QFRDRImagingFCSCorrelationDialog::cropTopCenter()
{
    int cwidth=ProgramOptions::getConfigValue("imaging_fcs/dlg_correlate/crop_settings/width_dvcenter", qMin(image_width, image_height)).toInt();
    bool ok=false;
    cwidth=QInputDialog::getInt(this, tr("crop top center"), tr("width of crop region:"), cwidth, 1, qMax(image_width, image_height), 1, &ok);
    if (ok) {
        ui->spinXFirst->setValue(image_width/2-cwidth/2);
        ui->spinXLast->setValue(image_width/2-cwidth/2+cwidth-1);
        ui->spinYFirst->setValue(3*image_height/4-cwidth/2);
        ui->spinYLast->setValue(3*image_height/4-cwidth/2+cwidth-1);
        ProgramOptions::setConfigValue("imaging_fcs/dlg_correlate/crop_settings/width_dvcenter", cwidth);
    }

}

void QFRDRImagingFCSCorrelationDialog::cropBottomCenter()
{
    int cwidth=ProgramOptions::getConfigValue("imaging_fcs/dlg_correlate/crop_settings/width_dvcenter", qMin(image_width, image_height)).toInt();
    bool ok=false;
    cwidth=QInputDialog::getInt(this, tr("crop bottom center"), tr("width of crop region:"), cwidth, 1, qMax(image_width, image_height), 1, &ok);
    if (ok) {
        ui->spinXFirst->setValue(image_width/2-cwidth/2);
        ui->spinXLast->setValue(image_width/2-cwidth/2+cwidth-1);
        ui->spinYFirst->setValue(image_height/4-cwidth/2);
        ui->spinYLast->setValue(image_height/4-cwidth/2+cwidth-1);
        ProgramOptions::setConfigValue("imaging_fcs/dlg_correlate/crop_settings/width_dvcenter", cwidth);
    }


}

void QFRDRImagingFCSCorrelationDialog::cropLeftHalf()
{
    int cwidth=image_width/2;
    ui->spinXFirst->setValue(0);
    ui->spinXLast->setValue(cwidth-1);
    ui->spinYFirst->setValue(0);
    ui->spinYLast->setValue(image_height-1);
}

void QFRDRImagingFCSCorrelationDialog::cropRightHalf()
{
    int cwidth=image_width/2;
    ui->spinXFirst->setValue(cwidth);
    ui->spinXLast->setValue(image_width-1);
    ui->spinYFirst->setValue(0);
    ui->spinYLast->setValue(image_height-1);
}

void QFRDRImagingFCSCorrelationDialog::cropTopHalf()
{
    int cwidth=image_height/2;
    ui->spinXFirst->setValue(0);
    ui->spinXLast->setValue(image_width-1);
    ui->spinYFirst->setValue(0);
    ui->spinYLast->setValue(cwidth-1);

}

void QFRDRImagingFCSCorrelationDialog::cropBottomHalf()
{
    int cwidth=image_height/2;
    ui->spinXFirst->setValue(0);
    ui->spinXLast->setValue(image_width-1);
    ui->spinYFirst->setValue(cwidth);
    ui->spinYLast->setValue(image_height-1);
}


bool QFRDRImagingFCSCorrelationDialog::allThreadsDone() const  {
    for (int i=0; i<jobs.size(); i++) {
        if ((jobs[i].thread->isRunning()) || (jobs[i].thread->status()==1)) return false;
    }
    return true;
}

int QFRDRImagingFCSCorrelationDialog::runningThreads() const  {
    int cnt=0;
    for (int i=0; i<jobs.size(); i++) {
        if ((jobs[i].thread->status()==1)||(jobs[i].thread->isRunning())) cnt++;
    }
    return cnt;
}

int QFRDRImagingFCSCorrelationDialog::waitingThreads() const  {
    int cnt=0;
    for (int i=0; i<jobs.size(); i++) {
        if (jobs[i].thread->status()==0) cnt++;
    }
    return cnt;
}

void QFRDRImagingFCSCorrelationDialog::ensureTiffReader() {
    if (!ui->cmbFileformat->currentText().toUpper().contains("TINYTIFF")) {
        for (int i=0; i<ui->cmbFileformat->currentIndex(); i++) {
            if (ui->cmbFileformat->itemText(i).toUpper().contains("TINYTIFF")) {
                ui->cmbFileformat->setCurrentIndex(i);
                break;
            }
        }
    }
    if (!ui->cmbFileformat->currentText().toUpper().contains("TIFF")) {
        for (int i=0; i<ui->cmbFileformat->currentIndex(); i++) {
            if (ui->cmbFileformat->itemText(i).toUpper().contains("TIFF")) {
                ui->cmbFileformat->setCurrentIndex(i);
                break;
            }
        }
    }
}


void QFRDRImagingFCSCorrelationDialog::startNextWaitingThread()   {
    for (int i=0; i<jobs.size(); i++) {
        if (jobs[i].thread->status()==0) {
            jobs[i].thread->start();
            break;
        }
    }
}

void QFRDRImagingFCSCorrelationDialog::done(int status)  {
    closing=true;
    bool allOK=allThreadsDone();
    bool someError=false;
    for (int i=0; i<jobs.size(); i++) {
        if (jobs[i].thread->status()<0) someError=true;
    }

    if (allOK) {
    } else {
        if (QMessageBox::warning(this, tr("imFCS Correlator"), tr("Some of the correlation jobs are not done yet.\nClose anyways (unfinished jobs will be canceled)!"), QMessageBox::Yes|QMessageBox::No, QMessageBox::No)==QMessageBox::Yes) {
            // cancel all unfinished threads
            for (int i=0; i<jobs.size(); i++) {
                if (!jobs[i].progress->isDone()) {
                    jobs[i].thread->cancel();
                }
            }
            setEnabled(false);

            QModernProgressDialog prg(this);
            prg.setLabelText(tr("Canceling remaining threads ..."));
            prg.open();
            // wait until all are OK
            while (!allThreadsDone()) {
                QApplication::processEvents();
            }
            prg.close();
            allOK=true;

        } else {
            closing=false;
        }
    }

    if (closing) {
        // add jobs to project
        QModernProgressDialog prg(this);
        prg.setLabelText(tr("add job results to project ..."));
        prg.open();
        for (int i=0; i<jobs.size(); i++) {
            if (jobs[i].addToProject && (jobs[i].thread->status()==2)) {
                for (int j=0; j<jobs[i].thread->getAddFiles().size(); j++) {
                    filesToAdd.append(jobs[i].thread->getAddFiles().at(j));
                }
            }
            jobs[i].progress->close();
            delete jobs[i].progress;
            delete jobs[i].thread;
        }
        prg.close();
        QFDialog::done(status);
        close();
    }

    writeSettings();
}

void QFRDRImagingFCSCorrelationDialog::setProject(QFProject* project) {
    this->project=project;
}

void QFRDRImagingFCSCorrelationDialog::on_spinP_valueChanged(int /*val*/) {
    updateCorrelator(true);
}

void QFRDRImagingFCSCorrelationDialog::on_spinS_valueChanged(int /*val*/) {
    updateCorrelator();
}

void QFRDRImagingFCSCorrelationDialog::on_spinM_valueChanged(int /*val*/) {
    updateCorrelator(true);
}

//void QFRDRImagingFCSCorrelationDialog::on_spinDecay_valueChanged(double /*val*/) {
//    updateBleach();
//}

//void QFRDRImagingFCSCorrelationDialog::on_spinDecay2_valueChanged(double /*val*/) {
//    updateBleach();
//}
void QFRDRImagingFCSCorrelationDialog::on_cmbCorrelator_currentIndexChanged(int /*idx*/) {
    updateCorrelator(true);
    updateBlocking();
}

void QFRDRImagingFCSCorrelationDialog::on_cmbBackground_currentIndexChanged(int idx) {
    ui->label_16->setEnabled(idx>0);
    ui->edtOffset->setEnabled(idx>0);
    ui->edtBackgroundFile->setEnabled(idx>2);
    ui->label_31->setEnabled(idx>2);
    ui->btnSelectBackgroundFile->setEnabled(idx>2);
}

void QFRDRImagingFCSCorrelationDialog::on_cmbBleachType_currentIndexChanged(int /*idx*/) {
    //ui->widBleach->setEnabled(ui->cmbBleachType->currentIndex()>=2);
}

void QFRDRImagingFCSCorrelationDialog::on_cmbDualView_currentIndexChanged(int index) {
    if (index==0) {
        ui->labDualView->setText("");
        ui->labDualView->setVisible(false);
        //ui->chkCrop->setEnabled(true);
        ui->chk2cFCCS->setEnabled(false);
        ui->chkAlex->setEnabled(false);
        ui->chkNondefaultShift->setEnabled(false);
        ui->widNondefaultShift->setEnabled(false);
        ui->chkSeparateColorChannels->setEnabled(false);
        //ui->lab2cFCCS->setText("");
        //ui->lab2cFCCS->setVisible(false);
    } else if (index==1 || index==2) {
        QString colChannels=tr("an ACF-calculation results in 1 RDR");
        if (ui->chkSeparateColorChannels->isChecked()) colChannels=tr("an ACF-calculation results in 2 RDRs");
        ui->labDualView->setText(tr("DualView mode actiavted:&nbsp;&nbsp;&nbsp;&bull; %1&nbsp;&nbsp;&nbsp;&bull; cropping should be done symmetrically").arg(colChannels));
        ui->labDualView->setVisible(true);
        //ui->chkCrop->setChecked(false);
        //ui->chkCrop->setEnabled(false);
        ui->chk2cFCCS->setEnabled(true);
        ui->chkAlex->setEnabled(true);
        ui->chkNondefaultShift->setEnabled(true);
        ui->widNondefaultShift->setEnabled(ui->chkNondefaultShift->isChecked());

        ui->chkSeparateColorChannels->setEnabled(true);
        //ui->lab2cFCCS->setText(tr("<b>Distance CCFs will be calculated for the full DualView image!</b>"));
        //ui->lab2cFCCS->setVisible(true);
    }
    ui->chkSeparateColorChannels->setChecked(true);
    ui->chkSeparateColorChannels->setVisible(false);
}

void QFRDRImagingFCSCorrelationDialog::on_chkSeparateColorChannels_toggled(bool /*value*/)
{
    on_cmbDualView_currentIndexChanged(ui->cmbDualView->currentIndex());
}

void QFRDRImagingFCSCorrelationDialog::on_btnSimulate_clicked()
{
    emit runSimulation();
}

void QFRDRImagingFCSCorrelationDialog::on_btnPreview_clicked()
{
    if (!image) {
        image=new QFImagePlot(0, "QFRDRImagingFCSCorrelationDialog/imagePlot/");
        image->setAttribute(Qt::WA_DeleteOnClose);
        connect(image, SIGNAL(destroyed(QObject*)), this, SLOT(previewDestroyed(QObject*)));
    }
    if (image) {
        image->show();
        image->raise();
        image->setImage(frame_data, image_width,image_height);
        if (ui->chkCrop->isChecked()) {
            image->setROI(ui->spinXFirst->value()-0.5,ui->spinYFirst->value()-0.5, ui->spinXLast->value()-ui->spinXFirst->value()+1, ui->spinYLast->value()-ui->spinYFirst->value()+1 );
        } else {
            image->resetROI();
        }
        disconnect(ui->btnPreview, SIGNAL(toggled(bool)), this, SLOT(on_btnPreview_clicked()));
        ui->btnPreview->setChecked(true);
        connect(ui->btnPreview, SIGNAL(toggled(bool)), this, SLOT(on_btnPreview_clicked()));
    } else {
        disconnect(ui->btnPreview, SIGNAL(toggled(bool)), this, SLOT(on_btnPreview_clicked()));
        ui->btnPreview->setChecked(false);
        connect(ui->btnPreview, SIGNAL(toggled(bool)), this, SLOT(on_btnPreview_clicked()));
    }
}

void QFRDRImagingFCSCorrelationDialog::previewDestroyed(QObject* object)
{
    disconnect(object, SIGNAL(destroyed(QObject*)), this, SLOT(previewDestroyed(QObject*)));
    disconnect(ui->btnPreview, SIGNAL(toggled(bool)), this, SLOT(on_btnPreview_clicked()));
    ui->btnPreview->setChecked(false);
    connect(ui->btnPreview, SIGNAL(toggled(bool)), this, SLOT(on_btnPreview_clicked()));
}


void QFRDRImagingFCSCorrelationDialog::on_chkFirstFrame_clicked(bool checked) {
    ui->spinFirstFrame->setEnabled(!checked);
    updateFrameCount();
}

void QFRDRImagingFCSCorrelationDialog::on_chkLastFrame_clicked(bool checked) {
    ui->spinLastFrame->setEnabled(!checked);
    updateFrameCount();
}



void QFRDRImagingFCSCorrelationDialog::on_btnSelectImageFile_clicked() {
    QString fileName = qfGetOpenFileName(this, tr("Select Image Series File ..."), lastImagefileDir, imageFilters.join(";;"), &lastImagefileFilter);
    if (!fileName.isEmpty()) {
        lastImagefileDir=QFileInfo(fileName).dir().absolutePath();
        ui->cmbFileformat->setCurrentIndex(imageFilters.indexOf(lastImagefileFilter));
        ui->edtImageFile->setText(fileName);
        ui->edtImageFile->setFocus(Qt::MouseFocusReason);
        on_btnLoad_clicked();
        writeSettings();
    }
}

void QFRDRImagingFCSCorrelationDialog::on_btnSelectImageFileNoCount_clicked()
{
    QString fileName = qfGetOpenFileName(this, tr("Select Image Series File ..."), lastImagefileDir, imageFilters.join(";;"), &lastImagefileFilter);
    if (!fileName.isEmpty()) {
        lastImagefileDir=QFileInfo(fileName).dir().absolutePath();
        ui->cmbFileformat->setCurrentIndex(imageFilters.indexOf(lastImagefileFilter));
        ui->edtImageFile->setText(fileName);
        ui->edtImageFile->setFocus(Qt::MouseFocusReason);
        on_btnLoadNoCount_clicked();
        writeSettings();
    }
}

void QFRDRImagingFCSCorrelationDialog::on_btnSelectBackgroundFile_clicked() {
    QString fileName = qfGetOpenFileName(this, tr("Select Background Image Series File ..."), lastImagefileDir, imageFilters.join(";;"), &lastImagefileFilter);
    if (!fileName.isEmpty()) {
        lastImagefileDir=QFileInfo(fileName).dir().absolutePath();
        ui->edtBackgroundFile->setText(fileName);
        ui->edtBackgroundFile->setFocus(Qt::MouseFocusReason);
        writeSettings();
        readBackgroundFramesize();
    }
}

void QFRDRImagingFCSCorrelationDialog::on_btnHelp_clicked() {
    pluginServices->displayHelpWindow(pluginServices->getPluginHelpDirectory("imaging_fcs")+"imfcs_usage.html");
}

void QFRDRImagingFCSCorrelationDialog::on_btnLoad_clicked() {
    QString oldFilename=filenameDisplayed;
    QString filename=ui->edtImageFile->text();
    if (QFile::exists(filename)) {
        int channels=1;
        if (oldFilename!=filename) updateFromFile(true, true, &channels);
        setEditControlsEnabled(true);
        ui->btnLoadNoCount->setEnabled(true);
        ui->btnSelectImageFileNoCount->setEnabled(true);
        ui->btnNextCell->setEnabled(true);
        ui->btnAddCellsAndJobs->setEnabled(true);
        if (channels>1) {

        }
    } else {
        setEditControlsEnabled(false);
        QMessageBox::critical(this, tr("imFCS Correlator"), tr("The file '%1' does not exist.\nPlease select an existing file!").arg(filename));
    }
    writeSettings();
}

void QFRDRImagingFCSCorrelationDialog::on_btnNextCell_clicked() {
    QString fileName=ui->edtImageFile->text();
    QString OldfileName=fileName;

    // Some Pattern magic finding "_c[0-9][0-9]__"

    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        QRegularExpression re("_c([0-9][0-9])__");
        int cellno = re.match(fileName).captured(1).toInt();
        //QString OldCellName = re.match(filename).captured(0);
    #else
        QRegExp re("_c([0-9][0-9])__");
        int cellno = re.match(fileName).cap(1).toInt();
    #endif

    cellno++;
    QString NextCellNoStr = QString("_c%1__").arg(cellno, 2, 10, QChar('0'));
    fileName.replace(re, NextCellNoStr);

    if (!fileName.isEmpty() && (OldfileName!=fileName)) {
        lastImagefileDir=QFileInfo(fileName).dir().absolutePath();
        ui->cmbFileformat->setCurrentIndex(imageFilters.indexOf(lastImagefileFilter));
        ui->edtImageFile->setText(fileName);
        ui->edtImageFile->setFocus(Qt::MouseFocusReason);
        on_btnLoad_clicked();
        writeSettings();
    }
}

void QFRDRImagingFCSCorrelationDialog::on_btnAddCellsAndJobs_clicked() {

    QString fileName=ui->edtImageFile->text();
    QString OldfileName=fileName;
    QString InitialFileName=fileName;

    // Add first cell
    if (QFile::exists(fileName)) {
        lastImagefileDir=QFileInfo(fileName).dir().absolutePath();
        ui->cmbFileformat->setCurrentIndex(imageFilters.indexOf(lastImagefileFilter));
        ui->edtImageFile->setFocus(Qt::MouseFocusReason);
        on_btnLoadNoCount_clicked();
        writeSettings();
        on_btnAddJob_clicked();
    }

    // Some Pattern magic finding "_c[0-9][0-9]__"

    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        QRegularExpression re("_c([0-9][0-9])__");
        int cellno = re.match(fileName).captured(1).toInt();
        //QString OldCellName = re.match(filename).captured(0);
    #else
        QRegExp re("_c([0-9][0-9])__");
        int cellno = re.match(fileName).cap(1).toInt();
    #endif

    // Add All other cells
    while (cellno<99) {

        OldfileName=fileName;
        cellno++;
        QString NextCellNoStr = QString("_c%1__").arg(cellno, 2, 10, QChar('0'));
        fileName.replace(re, NextCellNoStr);

        if (!fileName.isEmpty() && (OldfileName!=fileName)) {
            lastImagefileDir=QFileInfo(fileName).dir().absolutePath();
            ui->cmbFileformat->setCurrentIndex(imageFilters.indexOf(lastImagefileFilter));
            ui->edtImageFile->setText(fileName);
            //ui->edtImageFile->setFocus(Qt::MouseFocusReason);
            if (QFile::exists(fileName)) {
                on_btnLoad_clicked();
                writeSettings();
                on_btnAddJob_clicked();
            }
        }
    }
    ui->edtImageFile->setText(InitialFileName);
}

void QFRDRImagingFCSCorrelationDialog::on_btnLoadNoCount_clicked()
{
    QString oldFilename=filenameDisplayed;
    QString filename=ui->edtImageFile->text();
    if (QFile::exists(filename)) {
        if (oldFilename!=filename) updateFromFile(true, false);
        setEditControlsEnabled(true);
    } else {
        setEditControlsEnabled(false);
        QMessageBox::critical(this, tr("imFCS Correlator"), tr("The file '%1' does not exist.\nPlease select an existing file!").arg(filename));
    }
    writeSettings();
}

void QFRDRImagingFCSCorrelationDialog::writeSettings() {
    if (!options) return;
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/last_imagefile_dir", lastImagefileDir);
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/last_imagefile_filter", lastImagefileFilter);
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/correlator", ui->cmbCorrelator->currentIndex());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/prefix", ui->edtPrefix->text());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/postfix", ui->edtPostfix->text());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/background", ui->cmbBackground->currentIndex());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/S", ui->spinS->value());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/P", ui->spinP->value());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/m", ui->spinM->value());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/parallel_threads", ui->spinProcesses->value());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/add_to_project", ui->chkAddToProject->isChecked());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/statistics", ui->spinStatistics->value());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/backStatistics", ui->spinBackStatistics->value());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/video", ui->chkVideo->isChecked());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/video_frames", ui->spinVideoFrames->value());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/acf", ui->chkACF->isChecked());
    //options->getQSettings()->setValue("imaging_fcs/dlg_correlate/ccf", ui->chkCCF->isChecked());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/segments", ui->spinSegments->value());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/binning", ui->spinBinning->value());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/interleavedbinning", ui->chkInterleavedBinning->isChecked());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/averagebinning", ui->chkAverageBinning->isChecked());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/crop", ui->chkCrop->isChecked());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/crop_x0", ui->spinXFirst->value());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/crop_x1", ui->spinXLast->value());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/crop_y0", ui->spinYFirst->value());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/crop_y1", ui->spinYLast->value());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/bleach_frames", ui->spinBleachAvgFrames->value());
    //options->getQSettings()->setValue("imaging_fcs/dlg_correlate/dccf_deltax", ui->spinDistanceCCFDeltaX->value());
    //options->getQSettings()->setValue("imaging_fcs/dlg_correlate/dccf_deltay", ui->spinDistanceCCFDeltaY->value());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/dccf_exp", ui->edtDCCF->text());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/dccf", ui->chkDistanceCCD->isChecked());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/bleach", ui->cmbBleachType->currentIndex());
    //options->getQSettings()->setValue("imaging_fcs/dlg_correlate/bleachConst", ui->spinDecay->value());
    //options->getQSettings()->setValue("imaging_fcs/dlg_correlate/bleachA", ui->edtDecayA->value());
    //options->getQSettings()->setValue("imaging_fcs/dlg_correlate/bleachConst2", ui->spinDecay2->value());
    //options->getQSettings()->setValue("imaging_fcs/dlg_correlate/bleachA2", ui->edtDecayA2->value());
    //options->getQSettings()->setValue("imaging_fcs/dlg_correlate/bleachB", ui->edtDecayB->value());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/pixel_width", ui->spinPixelWidth->value());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/pixel_height", ui->spinPixelHeight->value());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/camera", ui->chkCamera->isChecked());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/FCCS_2color", ui->chk2cFCCS->isChecked());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/chkNondefaultShift", ui->chkNondefaultShift->isChecked());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/spinFCCSShiftX", ui->spinFCCSShiftX->value());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/spinFCCSShiftY", ui->spinFCCSShiftY->value());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/chkSeparateColorChannels", ui->chkSeparateColorChannels->isChecked());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/chkAddNB", ui->chkAddNB->isChecked());
    options->getQSettings()->setValue("imaging_fcs/dlg_correlate/chkBlocking", ui->chkBlocking->isChecked());
}

void QFRDRImagingFCSCorrelationDialog::readSettings() {
    if (!options) return;
    lastImagefileDir=options->getQSettings()->value("imaging_fcs/dlg_correlate/last_imagefile_dir", lastImagefileDir).toString();
    lastImagefileFilter=options->getQSettings()->value("imaging_fcs/dlg_correlate/last_imagefile_filter", lastImagefileFilter).toString();

    ui->cmbCorrelator->setCurrentIndex(options->getQSettings()->value("imaging_fcs/dlg_correlate/correlator", ui->cmbCorrelator->currentIndex()).toInt());
    ui->cmbBackground->setCurrentIndex(options->getQSettings()->value("imaging_fcs/dlg_correlate/background", ui->cmbBackground->currentIndex()).toInt());
    ui->spinS->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/S", ui->spinS->value()).toInt());
    ui->spinP->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/P", ui->spinP->value()).toInt());
    ui->spinM->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/m", ui->spinM->value()).toInt());
    ui->spinProcesses->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/parallel_threads", ui->spinProcesses->value()).toInt());
    ui->chkAddToProject->setChecked(options->getQSettings()->value("imaging_fcs/dlg_correlate/add_to_project", ui->chkAddToProject->isChecked()).toBool());
    ui->edtPrefix->setText(options->getQSettings()->value("imaging_fcs/dlg_correlate/prefix", ui->edtPrefix->text()).toString());
    ui->edtPostfix->setText(options->getQSettings()->value("imaging_fcs/dlg_correlate/postfix", ui->edtPostfix->text()).toString());
    ui->chkVideo->setChecked(options->getQSettings()->value("imaging_fcs/dlg_correlate/video", ui->chkVideo->isChecked()).toBool());
    ui->spinVideoFrames->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/video_frames", ui->spinVideoFrames->value()).toInt());
    ui->spinStatistics->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/statistics", ui->spinStatistics->value()).toInt());
    ui->spinBackStatistics->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/backStatistics", ui->spinBackStatistics->value()).toInt());
    ui->chkACF->setChecked(options->getQSettings()->value("imaging_fcs/dlg_correlate/acf", ui->chkACF->isChecked()).toBool());
    //ui->chkCCF->setChecked(options->getQSettings()->value("imaging_fcs/dlg_correlate/ccf", ui->chkCCF->isChecked()).toBool());
    ui->spinSegments->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/segments", ui->spinSegments->value()).toInt());
    ui->spinBinning->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/binning", ui->spinBinning->value()).toInt());
    ui->chkInterleavedBinning->setChecked(options->getQSettings()->value("imaging_fcs/dlg_correlate/interleavedbinning", ui->chkInterleavedBinning->isChecked()).toBool());
    ui->chkAverageBinning->setChecked(options->getQSettings()->value("imaging_fcs/dlg_correlate/averagebinning", ui->chkAverageBinning->isChecked()).toBool());
    ui->chkCrop->setChecked(options->getQSettings()->value("imaging_fcs/dlg_correlate/crop", ui->chkCrop->isChecked()).toBool());
    ui->spinXFirst->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/crop_x0", ui->spinXFirst->value()).toInt());
    ui->spinXLast->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/crop_x1", ui->spinXLast->value()).toInt());
    ui->spinYFirst->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/crop_y0", ui->spinYFirst->value()).toInt());
    ui->spinYLast->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/crop_y1", ui->spinYLast->value()).toInt());
    ui->spinBleachAvgFrames->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/bleach_frames", ui->spinBleachAvgFrames->value()).toInt());
    //ui->spinDistanceCCFDeltaX->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/dccf_deltax", ui->spinDistanceCCFDeltaX->value()).toInt());
    //ui->spinDistanceCCFDeltaY->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/dccf_deltay", ui->spinDistanceCCFDeltaY->value()).toInt());
    ui->edtDCCF->setText(options->getQSettings()->value("imaging_fcs/dlg_correlate/dccf_exp", ui->edtDCCF->text()).toString());
    ui->chkDistanceCCD->setChecked(options->getQSettings()->value("imaging_fcs/dlg_correlate/dccf", ui->chkDistanceCCD->isChecked()).toBool());
    ui->cmbBleachType->setCurrentIndex(options->getQSettings()->value("imaging_fcs/dlg_correlate/bleach", ui->cmbBleachType->currentIndex()).toInt());
    //ui->spinDecay->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/bleachConst", ui->spinDecay->value()).toDouble());
    //ui->edtDecayA->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/bleachA", ui->edtDecayA->value()).toDouble());
    //ui->spinDecay2->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/bleachConst2", ui->spinDecay2->value()).toDouble());
    //ui->edtDecayA2->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/bleachA2", ui->edtDecayA2->value()).toDouble());
    //ui->edtDecayB->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/bleachB", ui->edtDecayB->value()).toDouble());
    ui->spinPixelWidth->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/pixel_width", ui->spinPixelWidth->value()).toDouble());
    ui->spinPixelHeight->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/pixel_height", ui->spinPixelHeight->value()).toDouble());
    ui->chkCamera->setChecked(options->getQSettings()->value("imaging_fcs/dlg_correlate/camera", ui->chkCamera->isChecked()).toBool());
    ui->chk2cFCCS->setChecked(options->getQSettings()->value("imaging_fcs/dlg_correlate/FCCS_2color", ui->chk2cFCCS->isChecked()).toBool());
    ui->chkSeparateColorChannels->setChecked(options->getQSettings()->value("imaging_fcs/dlg_correlate/chkSeparateColorChannels", ui->chkSeparateColorChannels->isChecked()).toBool());


    ui->spinFCCSShiftX->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/spinFCCSShiftX", ui->spinFCCSShiftX->value()).toInt());
    ui->spinPixelHeight->setValue(options->getQSettings()->value("imaging_fcs/dlg_correlate/spinFCCSShiftY", ui->spinFCCSShiftY->value()).toInt());
    ui->chkNondefaultShift->setChecked(options->getQSettings()->value("imaging_fcs/dlg_correlate/chkNondefaultShift", ui->chkNondefaultShift->isChecked()).toBool());

    ui->chkSeparateColorChannels->setChecked(true);
    ui->chkSeparateColorChannels->setVisible(false);

    ui->chkAddNB->setChecked(options->getQSettings()->value("imaging_fcs/dlg_correlate/chkAddNB", ui->chkAddNB->isChecked()).toBool());
    ui->chkBlocking->setChecked(options->getQSettings()->value("imaging_fcs/dlg_correlate/chkBlocking", ui->chkBlocking->isChecked()).toBool());
    //ui->chkCCF->setChecked(false);
    //ui->chkCCF->setVisible(false);
}

int QFRDRImagingFCSCorrelationDialog::getIDForProgress(const QFRDRImagingFCSThreadProgress* w) const {
    for (int i=0; i<jobs.size(); i++) {
        if (w == jobs[i].progress) return i;
    }
    return -1;
}

int QFRDRImagingFCSCorrelationDialog::getLayoutIDForProgress(const QWidget* w) const {
    for (int i=0; i<ui->layProgress->count(); i++) {
        if (w == ui->layProgress->itemAt(i)->widget()) return i;
    }
    return -1;
}

void QFRDRImagingFCSCorrelationDialog::updateProgress() {
    if (!closing) {
        ////////////////////////////////////////////////////////////////////////////////////
        // SET OVERALL PROGRESS
        ////////////////////////////////////////////////////////////////////////////////////
        int max=1;
        int p=0;
        bool allDone=(jobs.size()>0);
        for (int i=0; i<jobs.size(); i++) {
            if (jobs[i].progress) {
                //max+=jobs[i].progress->getRangeMax()-jobs[i].progress->getRangeMin();
                max+=100;
                p+=round(100*(double)jobs[i].progress->getProgress()/((double)(jobs[i].progress->getRangeMax()-jobs[i].progress->getRangeMin())));
                allDone=allDone&&jobs[i].progress->isDone();
            }
        }
        max+=100*waitingThreads();
        ui->progressBar->setRange(0,max);
        ui->progressBar->setValue(p);

        if (allDone) {
            ui->progressBar->setValue(ui->progressBar->maximum());
        }


        ////////////////////////////////////////////////////////////////////////////////////
        // CHECK WHETHER WE HAVE TO START SOME MORE THREADS
        ////////////////////////////////////////////////////////////////////////////////////
        //int started=0;
        while ((waitingThreads()>0)&&(runningThreads()<ui->spinProcesses->value()) /*&& (started<=0)*/) {
            startNextWaitingThread();
            //started++;
        }


    }
    QTimer::singleShot(UPDATE_TIMEOUT, this, SLOT(updateProgress()));
}



IMFCSJob QFRDRImagingFCSCorrelationDialog::initJob(int biningForFCCS) {

    //updateFromFile(false); // make sure that inputconfigfile cintains the settings file for the input (if it exists)
    IMFCSJob job;
    job.progress=NULL;
    job.thread=NULL;
    job.filename=ui->edtImageFile->text();
    job.descriptionFilename=inputconfigfile;
    job.correlator=ui->cmbCorrelator->currentIndex();
    job.fileFormat=ui->cmbFileformat->currentIndex();
    //qDebug()<<"job.fileFormat="<<job.fileFormat<<"  "<<ui->cmbFileformat->count();
    job.backgroundCorrection=ui->cmbBackground->currentIndex();
    job.S=ui->spinS->value();
    job.P=ui->spinP->value();
    job.m=ui->spinM->value();
    job.frameTime=ui->edtFrameTime->value()*1e-6;
    job.addToProject=ui->chkAddToProject->isChecked();
    job.prefix=ui->edtPrefix->text();
    job.postfix=ui->edtPostfix->text();
    job.backgroundOffset=ui->edtOffset->value();
    job.filenameBackground=ui->edtBackgroundFile->text();
    //qDebug()<<job.filenameBackground;
    job.range_min=-1;
    if (!ui->chkFirstFrame->isChecked()) {
        job.range_min=ui->spinFirstFrame->value();
    }
    job.range_max=-1;
    if (!ui->chkLastFrame->isChecked()) {
        job.range_max=ui->spinLastFrame->value();
    }
    job.acf=ui->chkACF->isChecked();
    job.video=ui->chkVideo->isChecked();
    job.video_frames=qMax(2,ui->spinVideoFrames->value());
    job.statistics=true;
    job.statistics_frames=qMax(2, ui->spinStatistics->value());
    job.backstatistics_frames=qMax(2, ui->spinBackStatistics->value());
    job.segments=ui->spinSegments->value();
    job.binning=ui->spinBinning->value();
    if (biningForFCCS<0) biningForFCCS=job.binning;
    job.interleaved_binning=ui->chkInterleavedBinning->isChecked();
    job.binAverage=ui->chkAverageBinning->isChecked();
    job.use_cropping=ui->chkCrop->isChecked();
    job.crop_x0=ui->spinXFirst->value();
    job.crop_x1=ui->spinXLast->value();
    job.crop_y0=ui->spinYFirst->value();
    job.crop_y1=ui->spinYLast->value();
    job.distanceCCF=ui->chkDistanceCCD->isChecked();
    job.DCCFDeltaX.clear();
    job.DCCFDeltaY.clear();
    job.DCCFrole.clear();

    if (ui->chk2cFCCS->isChecked()) {
        if (ui->chkNondefaultShift->isChecked()) {
            job.DCCFDeltaX << ui->spinFCCSShiftX->value();
            job.DCCFDeltaY << ui->spinFCCSShiftY->value();
            job.DCCFrole<<QString("FCCS");
            job.distanceCCF=true;
        } else {
            if (ui->cmbDualView->currentIndex()==1) {
                job.DCCFDeltaX << image_width/2/biningForFCCS;
                job.DCCFDeltaY << 0;
                job.DCCFrole<<QString("FCCS");
                job.distanceCCF=true;
                job.isAlex=ui->chkAlex->isChecked();
                //qDebug()<<"added DV_H FCCS";
            } else if (ui->cmbDualView->currentIndex()==2) {
                job.DCCFDeltaX << 0;
                job.DCCFDeltaY << image_height/2/biningForFCCS;
                job.DCCFrole<<QString("FCCS");
                job.distanceCCF=true;
                job.isAlex=ui->chkAlex->isChecked();
                //qDebug()<<"added DV_V FCCS";
            }
        }
    }

    if (ui->chkDistanceCCD->isChecked()) {
        QString dccfs=ui->edtDCCF->text()+";";
        QRegExp rxdccf("([\\+\\-]?\\d+)\\s*\\,\\s*([\\+\\-]?\\d+)");
        rxdccf.setCaseSensitivity(Qt::CaseInsensitive);
        int pos = 0;

        while ((pos = rxdccf.indexIn(dccfs, pos)) != -1) {
            int dx=rxdccf.cap(1).toInt();
            int dy=rxdccf.cap(2).toInt();
            if (dx<image_width && dy<image_height) {
                job.addDCCF(dx, dy);
            }
            pos += rxdccf.matchedLength();
        }
    }
    job.bleach=ui->cmbBleachType->currentIndex();
    job.bleachAvgFrames=ui->spinBleachAvgFrames->value();
    job.cameraSettingsGiven=ui->chkCamera->isChecked();
    job.cameraPixelWidth=ui->spinPixelWidth->value();
    job.cameraPixelHeight=ui->spinPixelHeight->value();
    job.addFCCSSeparately=ui->chkSeparateColorChannels->isChecked();
    job.dualViewMode=ui->cmbDualView->currentIndex();
    job.addNandB=ui->chkAddNB->isChecked();
    job.useBlockingErrorEstimate=ui->chkBlocking->isChecked();
    //job.bleachDecay=ui->spinDecay->value();
    //job.bleachA=ui->edtDecayA->value();
    //job.bleachDecay2=ui->spinDecay2->value();
    //job.bleachA2=ui->edtDecayA2->value();
    //job.bleachB=0;//ui->edtDecayB->value();
    //job.bleachAvgFrames=100;
    writeSettings();
    return job;
}

void QFRDRImagingFCSCorrelationDialog::addJob(IMFCSJob jobin, bool /*ignoreDualView*/) {
    IMFCSJob job=jobin;
    job.progress=new QFRDRImagingFCSThreadProgress(this);
    job.thread=new QFRDRImagingFCSCorrelationJobThread(pluginServices, this);
    connect(job.thread, SIGNAL(messageChanged(QString)), job.progress, SLOT(setMessage(QString)));
    connect(job.thread, SIGNAL(statusChanged(int)), job.progress, SLOT(setStatus(int)));
    connect(job.thread, SIGNAL(rangeChanged(int, int)), job.progress, SLOT(setRange(int, int)));
    connect(job.thread, SIGNAL(progressChanged(int)), job.progress, SLOT(setProgress(int)));
    connect(job.thread, SIGNAL(progressIncrement(int)), job.progress, SLOT(incProgress(int)));
    connect(job.progress, SIGNAL(cancelClicked()), job.thread, SLOT(cancel()));

    setEditControlsEnabled(false);
    ui->layProgress->insertWidget(0, job.progress);

    job.progress->setName(tr("correlating '%1'").arg(job.filename));
    job.thread->init(job);
    jobs.append(job);

    /*if (ignoreDualView || (ui->cmbDualView->currentIndex()==0)) {

        job.progress=new QFRDRImagingFCSThreadProgress(this);
        job.thread=new QFRDRImagingFCSCorrelationJobThread(pluginServices, this);
        connect(job.thread, SIGNAL(messageChanged(QString)), job.progress, SLOT(setMessage(QString)));
        connect(job.thread, SIGNAL(statusChanged(int)), job.progress, SLOT(setStatus(int)));
        connect(job.thread, SIGNAL(rangeChanged(int, int)), job.progress, SLOT(setRange(int, int)));
        connect(job.thread, SIGNAL(progressChanged(int)), job.progress, SLOT(setProgress(int)));
        connect(job.thread, SIGNAL(progressIncrement(int)), job.progress, SLOT(incProgress(int)));
        connect(job.progress, SIGNAL(cancelClicked()), job.thread, SLOT(cancel()));


        setEditControlsEnabled(false);
        ui->layProgress->insertWidget(0, job.progress);

        job.progress->setName(tr("correlating '%1'").arg(job.filename));
        job.thread->init(job);
        jobs.append(job);
    } else if (!ignoreDualView) {
        if (ui->cmbDualView->currentIndex()==1) {
            job.use_cropping=true;
            job.crop_x0=0;
            job.crop_x1=image_width/2-1;
            job.crop_y0=0;
            job.crop_y1=image_height-1;
            job.role=tr("left");
            addJob(job, true);
            IMFCSJob job2=jobin;
            job2.use_cropping=true;
            job2.crop_x0=image_width/2;
            job2.crop_x1=image_width-1;
            job2.crop_y0=0;
            job2.crop_y1=image_height-1;
            job2.role=tr("right");
            addJob(job2, true);
        } else if (ui->cmbDualView->currentIndex()==2) {
            job.use_cropping=true;
            job.crop_x0=0;
            job.crop_x1=image_width-1;
            job.crop_y0=0;
            job.crop_y1=image_height/2-1;
            job.role=tr("bottom");
            addJob(job, true);
            IMFCSJob job2=jobin;
            job2.use_cropping=true;
            job2.crop_x0=0;
            job2.crop_x1=image_width-1;
            job2.crop_y0=image_height/2;
            job2.crop_y1=image_height-1;
            job.role=tr("top");
            addJob(job2, true);
        }
    }*/
}

bool QFRDRImagingFCSCorrelationDialog::correctJobCropAndDV()
{
    bool ok=false;
    QStringList sl;
    sl<<tr("add job as set");
    sl<<tr("set DualView mode to none (click again to add job)");
    sl<<tr("switch off cropping (click again to add job)");
    int what=sl.indexOf(QInputDialog::getItem(this, tr("add correlation job"), tr("You selected cropping and a DualView mode. Is this correct?\n    Note: This is a usually arising mistake when setting the parameters, so I ask!\n\nWhat should be done? Please select:"), sl, 0, false, &ok));
    if (ok) {
        if (what<=0) return true;
        else if (what==1) {
            ui->cmbDualView->setCurrentIndex(0);
            /*QApplication::processEvents();
            job=initJob();
            addJob(job);*/
        } else if (what==2) {
            ui->chkCrop->setChecked(false);
            /*QApplication::processEvents();
            job=initJob();
            addJob(job);*/
        }
    }
    return false;
}

void QFRDRImagingFCSCorrelationDialog::on_btnAddJob_clicked() {
    IMFCSJob job=initJob();
    if (job.use_cropping && job.dualViewMode>0) {
        if (correctJobCropAndDV()) addJob(job);
    } else {
        addJob(job);
    }
}

void QFRDRImagingFCSCorrelationDialog::onAddSeriesJobClicked(const QString &parameter, double start, double end, double inc) {
    IMFCSJob job=initJob();
    bool ok=true;
    if (job.use_cropping && job.dualViewMode>0) {
        ok=correctJobCropAndDV();
    }

    if (ok){
        QFRDRImagingFCSSeriesDialog* dlg=new QFRDRImagingFCSSeriesDialog(this);
        if (!parameter.isEmpty()) {
            dlg->setParameter(parameter);
            dlg->setRange(start, end, inc);
        }

        if (dlg->exec()) {
            QList<double> vals=dlg->getValues();

            setEditControlsEnabled(false);
            for (int i=0; i<vals.size(); i++) {
                if (dlg->getParameter()==0) { //binning
                    IMFCSJob job=initJob(vals[i]);
                    job.binning=vals[i];
                    addJob(job);
                }
            }
        }
        delete dlg;
    }

}

void QFRDRImagingFCSCorrelationDialog::on_btnAddBin12Job_clicked()
{
    IMFCSJob job=initJob();
    bool ok=true;
    if (job.use_cropping && job.dualViewMode>0) {
        ok=correctJobCropAndDV();
    }
    if (ok){
        setEditControlsEnabled(false);
        for (int i=0; i<2; i++) {
            IMFCSJob job=initJob(i+1);
            job.binning=i+1;
            addJob(job);
        }
    }

}

void QFRDRImagingFCSCorrelationDialog::frameTimeChanged(double value) {
    if (value!=0) {
        disconnect(ui->edtFrameRate, SIGNAL(valueChanged(double)), this, SLOT(frameRateChanged(double)));
        ui->edtFrameRate->setValue(1.0/(value/1e6));
        connect(ui->edtFrameRate, SIGNAL(valueChanged(double)), this, SLOT(frameRateChanged(double)));
    }
    updateCorrelator(true);
    updateFrameCount();
    updateBleach();
}

void QFRDRImagingFCSCorrelationDialog::frameRateChanged(double value) {
    if (value!=0) {
        disconnect(ui->edtFrameTime, SIGNAL(valueChanged(double)), this, SLOT(frameTimeChanged(double)));
        ui->edtFrameTime->setValue((1.0/value)*1e6);
        connect(ui->edtFrameTime, SIGNAL(valueChanged(double)), this, SLOT(frameTimeChanged(double)));
    }
    updateCorrelator(true);
    updateFrameCount();
    updateBleach();
}

void QFRDRImagingFCSCorrelationDialog::updateImageSize() {
    int w=image_width;
    int h=image_height;

    if (image_width-1>0) ui->spinXFirst->setMaximum(image_width-1);
    if (image_width-1>0) ui->spinXLast->setMaximum(image_width-1);
    if (image_height-1>0) ui->spinYFirst->setMaximum(image_height-1);
    if (image_height-1>0) ui->spinYLast->setMaximum(image_height-1);
    if (image_width-1>0) ui->spinFCCSShiftX->setMaximum(image_width-1);
    if (image_height-1>0) ui->spinFCCSShiftY->setMaximum(image_height-1);
    //if (image_width>0) ui->spinDistanceCCFDeltaX->setRange(-1*image_width, image_width);
    //if (image_height>0) ui->spinDistanceCCFDeltaY->setRange(-1*image_height, image_height);

    if (ui->chkCrop->isChecked()) {
        w=fabs(ui->spinXLast->value()-ui->spinXFirst->value())+1;
        h=fabs(ui->spinYLast->value()-ui->spinYFirst->value())+1;
    }

    if (ui->chkInterleavedBinning->isChecked()) {
        w=w-(ui->spinBinning->value()-1);
        h=h-(ui->spinBinning->value()-1);
    } else {
        w=w/ui->spinBinning->value();
        h=h/ui->spinBinning->value();
    }
    ui->labSize->setText(tr("input: %1&times;%2   output: %3&times;%4").arg(image_width).arg(image_height).arg(w).arg(h));

    if (image) {
        if (ui->chkCrop->isChecked()) {
            image->setROI(ui->spinXFirst->value()-0.5,ui->spinYFirst->value()-0.5, ui->spinXLast->value()-ui->spinXFirst->value()+1, ui->spinYLast->value()-ui->spinYFirst->value()+1 );
        } else {
            image->resetROI();
        }
    }
}

void QFRDRImagingFCSCorrelationDialog::updateFrameCount() {
    double taumin=ui->edtFrameTime->value()*1e-6;
    double frames=frame_count;
    int64_t first=0;
    int64_t last=frame_count-1;
    if (!ui->chkFirstFrame->isChecked()) first= ui->spinFirstFrame->value();
    if (!ui->chkLastFrame->isChecked()) last= ui->spinLastFrame->value();
    frames=last-first+1;

    if (frames>0) {
        ui->labInputLength->setText(tr("length: %1 = %2 s").arg((int64_t)round(frames)).arg(frames*taumin));
        ui->labRange->setText(tr("= %1 frames").arg(round(frames)));
        segment_length=frames/ui->spinSegments->value();
        if (ui->spinSegments->value()!=0) ui->labSegments->setText(tr("segments of length: %1 (%2 each)").arg(frames/ui->spinSegments->value()).arg(taumin*(double)(frames/ui->spinSegments->value())));
        else ui->labSegments->setText("");
        if (ui->spinStatistics->value()!=0) ui->labStatistics->setText(tr("&Delta;<sub>Statistics</sub>= %1 &mu;s  => %2 values").arg(1e6*taumin*(double)ui->spinStatistics->value()).arg(frames/ui->spinStatistics->value()));
        else ui->labStatistics->setText("");
        if (ui->spinVideoFrames->value()!=0) ui->labVideo->setText(tr("&Delta;<sub>Video</sub>= %1 &mu;s  => %2 frames").arg(1e6*taumin*(double)ui->spinVideoFrames->value()).arg(frames/ui->spinVideoFrames->value()));
        else ui->labVideo->setText(QString(""));
    } else {
        segment_length=0;
        ui->labInputLength->setText(QString(""));
        ui->labRange->setText(QString(""));
        ui->labSegments->setText(QString(""));
        ui->labStatistics->setText(QString(""));
        ui->labVideo->setText(QString(""));
    }

}

void QFRDRImagingFCSCorrelationDialog::updateBleach() {
    //ui->labDecay->setTextFormat(Qt::RichText);
    //ui->labDecay->setText(tr("&tau;<sub>Bleach,1</sub> = %1 s<br>&tau;<sub>Bleach,2</sub> = %2 s").arg((double)ui->spinDecay->value()*ui->edtFrameTime->value()/1e6).arg((double)ui->spinDecay2->value()*ui->edtFrameTime->value()/1e6));
    //ui->labDecay->setText(tr("&tau;<sub>Bleach,1</sub> = %1 s").arg((double)ui->spinDecay->value()*ui->edtFrameTime->value()/1e6));
}

void QFRDRImagingFCSCorrelationDialog::updateBlocking()
{
    ui->chkBlocking->setEnabled((ui->spinSegments->value()<=1) && ((ui->cmbCorrelator->currentIndex()==CORRELATOR_DIRECT) || (ui->cmbCorrelator->currentIndex()==CORRELATOR_DIRECT_INT) || (ui->cmbCorrelator->currentIndex()==CORRELATOR_DIRECTAVG) || (ui->cmbCorrelator->currentIndex()==CORRELATOR_DIRECTAVG_INT)));
}


void QFRDRImagingFCSCorrelationDialog::updateCorrelator(bool setS) {
    int corrType=ui->cmbCorrelator->currentIndex();
    if (corrType==CORRELATOR_MTAUONEMON) {
        ui->spinM->setEnabled(false);
        ui->labM->setEnabled(false);
        ui->spinM->setValue(2);
    } else {
        ui->spinM->setEnabled(true);
        ui->labM->setEnabled(true);
    }

    int S=ui->spinS->value();
    double P=ui->spinP->value();
    double m=ui->spinM->value();
    double taumin=ui->edtFrameTime->value();
    double taumax=taumin;

    int idealS=1;
    while (idealS<200 && taumax<double(segment_length)*taumin && segment_length>0) {
        taumax=QFRDRImagingFCSCorrelation_getCorrelatorTauMax(corrType, taumin, idealS, m, P);
        //qDebug()<<idealS<<taumax/1e6<<double(segment_length)*taumin/1e6<<taumin<<(taumax<double(segment_length)*taumin);
        idealS++;
    }
    if (setS) {
        S=idealS-1;
        ui->spinS->setValue(S);
    }
    taumax=QFRDRImagingFCSCorrelation_getCorrelatorTauMax(corrType, taumin, S, m, P);
    ui->labCorrelator->setText(tr("<i>correlator lags:</i> &tau;<sub>min</sub> = %1&mu;s ...&tau;<sub>max</sub><i> = %2s</i>").arg(taumin).arg(taumax/1e6));
}



void QFRDRImagingFCSCorrelationDialog::updateFromFile(bool readFiles, bool countFrames, int *channels) {

    QModernProgressDialog prg(this);
    prg.setWindowTitle(tr("imFCS: Correlator"));
    prg.setLabelText(tr("Reading image series information ... reading config file ..."));
    prg.open();
    QApplication::processEvents();
    QApplication::processEvents();
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));



    filenameDisplayed=QFileInfo(ui->edtImageFile->text()).absoluteFilePath();
    qint64 filesize=QFileInfo(ui->edtImageFile->text()).size();
    if (filesize>1024*1024*1024) {
        ui->labWarning->setText(tr("<table border=\"0\"><tr><td><img src=\":/lib/help/helpboxlogo_warning.png\"></td><td><b><font color=\"red\">WARNING! The file you are about to correlate has a size of %1GB. Make sure, that your computer has enough free memory for this correlation!</font></b></td></tr></table>").arg(double(filesize)/double(1024*1024*1024)));
    } else {
        ui->labWarning->clear();
    }
    QApplication::processEvents();

    double frametime=ui->edtFrameTime->value();
    double baseline_offset=ui->edtOffset->value();
    QString backgroundF=ui->edtBackgroundFile->text();

    double pixel_width=ui->spinPixelWidth->value();
    double pixel_height=ui->spinPixelHeight->value();
    bool hasPixel=ui->chkCamera->isChecked();

    int dualViewMode=0;

    //////////////////////////////////////////////////////////////////////////////////
    // now we search for a .configuration.ini file describing the selected file
    //////////////////////////////////////////////////////////////////////////////////
    QDir d=QFileInfo(filenameDisplayed).absoluteDir();
    QStringList nameFilters;
    nameFilters<<"*.ini";
    nameFilters<<"*.cfg";
    nameFilters<<"*.txt";
    inputconfigfile="";
    d.setNameFilters(nameFilters);
    // could be that the absolute pathes are wrong. In this case we try to get a second guess by finding a file which
    // equals in the name, but not the directory ... the user can correct the data anyways. This second guess is stored
    // in these variables:
    double sframetime=frametime;
    double sbaseline_offset=baseline_offset;
    QString sbackgroundF=backgroundF;
    bool hasSecond=false;
    bool hasFirst=false;
    bool sIsTiff=false;
    foreach (QString iniFile, d.entryList(QDir::Files)) {
        //QApplication::processEvents();
        inputconfigfile=d.absoluteFilePath(iniFile);
        QSettings set(d.absoluteFilePath(iniFile), QSettings::IniFormat);
        int fcnt=set.value("files/count", 0).toInt();
        if (fcnt>0) {
            bool foundSecond=false;
            for (int f=0; f<fcnt; f++) {
                QString fn=set.value("files/name"+QString::number(f), "").toString();
                QString ft=set.value("files/type"+QString::number(f), "").toString();
                QString fd=set.value("files/description"+QString::number(f), "").toString();
                if (!fn.isEmpty()) {
                    QString fnAbs=d.absoluteFilePath(fn);
                    //qDebug()<<"fn="<<fn<<"  fnAbs="<<fnAbs<<"  fn(filename)="<<QFileInfo(filename).fileName()<<"  fn(fnAbs)="<<QFileInfo(fnAbs).fileName();
                    if (fnAbs==filenameDisplayed) {
                        hasFirst=true;
                        if (ft.toLower().simplified().startsWith("tiff")) ensureTiffReader();
                    } else if (QFileInfo(fnAbs).fileName()==QFileInfo(filenameDisplayed).fileName()) {
                        foundSecond=true;
                        if (ft.toLower().simplified().startsWith("tiff")) sIsTiff=true;
                    }
                }
            }

            if (hasFirst) {
                readB040SPIMExperimentConfigFile(set, frametime, baseline_offset, backgroundF, image_width, image_height, hasPixel, pixel_width, pixel_height, dualViewMode);
                //qDebug()<<"read first  "<<frametime<<baseline_offset<<backgroundF<<image_width<<image_height;
            } else if (foundSecond) {
                readB040SPIMExperimentConfigFile(set, sframetime, sbaseline_offset, sbackgroundF, image_width, image_height, hasPixel, pixel_width, pixel_height, dualViewMode);
                //qDebug()<<"read second  "<<sframetime<<sbaseline_offset<<sbackgroundF<<image_width<<image_height;
                hasSecond=true;
            }
        }
        if (hasFirst) break;
    }

    // if we didn't find a second guess, we try to find a config file with the same
    // basename + one of a set of extensions (newsuffix list) and try to read info from
    // that.
    if (!hasSecond) {
        QString suffix=QFileInfo(filenameDisplayed).suffix();
        QString cfgname;

        QStringList newsuffix;
        newsuffix<<"ini"
                 <<"configuration.ini"
                 <<"settings.ini"
                 <<"settings.txt"
                 <<"cfg";
        for (int i=0; i<newsuffix.size(); i++) {
            cfgname=filenameDisplayed.left(filenameDisplayed.size()-suffix.size())+newsuffix[i];
            if (QFile::exists(cfgname)) {
                QSettings set(cfgname, QSettings::IniFormat);
                readB040SPIMExperimentConfigFile(set, sframetime, sbaseline_offset, sbackgroundF, image_width, image_height, hasPixel, pixel_width, pixel_height, dualViewMode);
                //qDebug()<<"read third  "<<sframetime<<sbaseline_offset<<sbackgroundF<<image_width<<image_height;
                break;
            }
        }


    }

    // if there is no direct match, we take the second best, if there is one
    if (!hasFirst && hasSecond)  {
        frametime=sframetime;
        baseline_offset=sbaseline_offset;
        backgroundF=sbackgroundF;
        if (sIsTiff) ensureTiffReader();//ui->cmbFileformat->setCurrentIndex(0);
    }


    // SET THE FRAMETIME/RATE
    ui->edtFrameTime->setValue(frametime);
    frameTimeChanged(frametime);
    ui->edtOffset->setValue(baseline_offset);
    ui->edtBackgroundFile->setText(backgroundF);
    ui->spinPixelWidth->setValue(pixel_width);
    ui->spinPixelHeight->setValue(pixel_height);
    ui->chkCamera->setChecked(!hasPixel);
    ui->cmbDualView->setCurrentIndex(dualViewMode);



    if (readFiles) {

        //////////////////////////////////////////////////////////////////////////////////
        // we also try to open the file with an appropriate reader and read th number of frames in it
        //////////////////////////////////////////////////////////////////////////////////
        prg.setLabelText(tr("Reading image series information ... counting frames ..."));
        QApplication::processEvents();
        QApplication::processEvents();
        QFImporterImageSeries* reader=NULL;
        bool OK=false;
        if (ui->cmbFileformat->currentIndex()>=0 && ui->cmbFileformat->currentIndex()<QFRDRImagingFCSCorrelationJobThread::getImageReaderCount(pluginServices)) {
            reader=QFRDRImagingFCSCorrelationJobThread::getImageReader(ui->cmbFileformat->currentIndex(), pluginServices);
        }
        if (reader) {
            OK=reader->open(filenameDisplayed);
            if (OK)  {
                QApplication::processEvents();
                image_width=reader->frameWidth();
                image_height=reader->frameHeight();
                if (channels) *channels=reader->frameChannels();
                if (reader->frameChannels()>1) {
                    ui->labWarning->setText(ui->labWarning->text()+tr("<br><table border=\"0\"><tr><td><img src=\":/lib/help/helpboxlogo_warning.png\"></td><td><b><font color=\"red\">WARNING! The file you are about to correlate contains %1 color channels. Only the first channel will be correlated!!!</font></b></td></tr></table>").arg(reader->frameChannels()));
                }
                if (frame_data) qfFree(frame_data);
                frame_data=NULL;
                frame_data=qfMallocT<double>(image_width*image_height);
                reader->readFrameDouble(frame_data);
                QVariant v=reader->getFileProperty("FRAMERATE");
                if (v.canConvert(QVariant::Double) && v.toDouble()>0) {
                    frametime=1.0/v.toDouble()*1e6;
                    ui->edtFrameTime->setValue(frametime);
                    frameTimeChanged(frametime);

                }
                v=reader->getFileProperty("FRAMETIME");
                if (v.canConvert(QVariant::Double) && v.toDouble()>0) {
                    double frametime=v.toDouble()*1e6;
                    ui->edtFrameTime->setValue(frametime);
                    frameTimeChanged(frametime);

                }
                v=reader->getFileProperty("PIXEL_SIZE");
                if (v.canConvert(QVariant::Double) && v.toDouble()>0) {
                    ui->spinPixelWidth->setValue(v.toDouble());
                    ui->spinPixelHeight->setValue(v.toDouble());
                }
                v=reader->getFileProperty("PIXEL_WIDTH");
                if (v.canConvert(QVariant::Double) && v.toDouble()>0) {
                    ui->spinPixelWidth->setValue(v.toDouble());
                }
                v=reader->getFileProperty("PIXEL_HEIGHT");
                if (v.canConvert(QVariant::Double) && v.toDouble()>0) {
                    ui->spinPixelHeight->setValue(v.toDouble());
                }
                if (countFrames) frame_count=reader->countFrames();
                if (frame_count>0) {
                    ui->spinLastFrame->setMaximum(frame_count-1);
                    ui->spinFirstFrame->setMaximum(frame_count-1);
                    if (ui->chkLastFrame->isChecked()) ui->spinLastFrame->setValue(frame_count-1);
                    if (ui->chkFirstFrame->isChecked()) ui->spinFirstFrame->setValue(0);
                    ui->spinVideoFrames->setMaximum(frame_count-1);
                    ui->spinVideoFrames->setValue(qMax(2.0,round(double(frame_count)/1000.0)));
                    ui->spinStatistics->setMaximum(frame_count-1);
                    ui->spinStatistics->setValue(ui->spinVideoFrames->value());
                    ui->spinBleachAvgFrames->setValue(qMax(1,frame_count/2500));
                } else {
                    ui->spinLastFrame->setMaximum(10000000);
                    ui->spinFirstFrame->setMaximum(10000000);
                    if (ui->chkLastFrame->isChecked()) ui->spinLastFrame->setValue(10000000);
                    ui->spinVideoFrames->setMaximum(10000000);
                    ui->spinVideoFrames->setValue(100);
                    ui->spinStatistics->setMaximum(10000000);
                }
            }
            reader->close();

        }

        readBackgroundFramesize();
    }

    prg.close();
    QApplication::restoreOverrideCursor();

    if (image_width<=1) image_width=1;
    if (image_height<=1) image_height=1;

    updateFrameCount();
    updateImageSize();
    updateCorrelator(true);
    if (ui->btnPreview->isChecked()) on_btnPreview_clicked();
}


QList<QFRDRImagingFCSCorrelationJobThread::Fileinfo> QFRDRImagingFCSCorrelationDialog::getFilesToAdd() const {
    return filesToAdd;
}

void QFRDRImagingFCSCorrelationDialog::readStackProperties(const QString& filename, int fileFormat, bool readFiles, bool countFrames, QWidget *parentWid, int *channels, int *frame_count_io, qint64* filesize_io, double* frametime_io, double* baseline_offset_io, QString* backgroundF_io, double* pixel_width_io, double* pixel_height_io, bool *hasPixel_io, int* dualViewMode_io, int* image_width_io, int* image_height_io, QString *inputconfigfile_io, double **frame_data_io, int *background_width_io, int *background_height_io, int *background_frame_count_io)
{
    QModernProgressDialog prg(parentWid);
    prg.setWindowTitle(tr("imFCS: Correlator"));
    prg.setLabelText(tr("Reading image series information ... reading config file ..."));
    prg.open();
    QApplication::processEvents();
    QApplication::processEvents();
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    qint64 filesize=QFileInfo(filename).size();
    if (filesize_io) *filesize_io=filesize;
    QApplication::processEvents();

    double frametime=1;
    double baseline_offset=0;
    QString backgroundF="";
    QString inputconfigfile="";
    double* frame_data=NULL;

    double pixel_width=1;
    double pixel_height=1;
    bool hasPixel=false;

    int dualViewMode=0;

    int image_width=0;
    int image_height=0;
    int frame_count=0;

    if (frametime_io) frametime=*frametime_io;
    if (baseline_offset_io) baseline_offset=*baseline_offset_io;
    if (backgroundF_io) backgroundF=*backgroundF_io;
    if (pixel_width_io) pixel_width=*pixel_width_io;
    if (pixel_height_io) pixel_height=*pixel_height_io;
    if (dualViewMode_io) dualViewMode=*dualViewMode_io;
    if (hasPixel_io) hasPixel=*hasPixel_io;
    if (image_width_io) image_width=*image_width_io;
    if (image_height_io) image_height=*image_height_io;
    if (frame_count_io) frame_count=*frame_count_io;
    if (inputconfigfile_io) inputconfigfile=*inputconfigfile_io;
    if (frame_data_io) frame_data=*frame_data_io;

    //////////////////////////////////////////////////////////////////////////////////
    // now we search for a .configuration.ini file describing the selected file
    //////////////////////////////////////////////////////////////////////////////////
    QDir d=QFileInfo(filename).absoluteDir();
    QStringList nameFilters;
    nameFilters<<"*.ini";
    nameFilters<<"*.cfg";
    nameFilters<<"*.txt";
    inputconfigfile="";
    d.setNameFilters(nameFilters);
    // could be that the absolute pathes are wrong. In this case we try to get a second guess by finding a file which
    // equals in the name, but not the directory ... the user can correct the data anyways. This second guess is stored
    // in these variables:
    double sframetime=frametime;
    double sbaseline_offset=baseline_offset;
    QString sbackgroundF=backgroundF;
    bool hasSecond=false;
    bool hasFirst=false;
    //bool sIsTiff=false;

    foreach (QString iniFile, d.entryList(QDir::Files)) {
        //QApplication::processEvents();
        inputconfigfile=d.absoluteFilePath(iniFile);
        QSettings set(d.absoluteFilePath(iniFile), QSettings::IniFormat);
        int fcnt=set.value("files/count", 0).toInt();
        if (fcnt>0) {
            bool foundSecond=false;
            for (int f=0; f<fcnt; f++) {
                QString fn=set.value("files/name"+QString::number(f), "").toString();
                QString ft=set.value("files/type"+QString::number(f), "").toString();
                QString fd=set.value("files/description"+QString::number(f), "").toString();
                if (!fn.isEmpty()) {
                    QString fnAbs=d.absoluteFilePath(fn);
                    //qDebug()<<"fn="<<fn<<"  fnAbs="<<fnAbs<<"  fn(filename)="<<QFileInfo(filename).fileName()<<"  fn(fnAbs)="<<QFileInfo(fnAbs).fileName();
                    if (fnAbs==filename) {
                        hasFirst=true;
                        //if (ft.toLower().simplified().startsWith("tiff")) sIsTiff=true;
                    } else if (QFileInfo(fnAbs).fileName()==QFileInfo(filename).fileName()) {
                        foundSecond=true;
                        //if (ft.toLower().simplified().startsWith("tiff")) sIsTiff=true;
                    }
                }
            }

            if (hasFirst) {
                readB040SPIMExperimentConfigFile(set, frametime, baseline_offset, backgroundF, image_width, image_height, hasPixel, pixel_width, pixel_height, dualViewMode);
                //qDebug()<<"read first  "<<frametime<<baseline_offset<<backgroundF<<image_width<<image_height;
            } else if (foundSecond) {
                readB040SPIMExperimentConfigFile(set, sframetime, sbaseline_offset, sbackgroundF, image_width, image_height, hasPixel, pixel_width, pixel_height, dualViewMode);
                //qDebug()<<"read second  "<<sframetime<<sbaseline_offset<<sbackgroundF<<image_width<<image_height;
                hasSecond=true;
            }
        }
        if (hasFirst) break;
    }

    // if we didn't find a second guess, we try to find a config file with the same
    // basename + one of a set of extensions (newsuffix list) and try to read info from
    // that.
    if (!hasSecond) {
        QString suffix=QFileInfo(filename).suffix();
        QString cfgname;

        QStringList newsuffix;
        newsuffix<<"ini"
                 <<"configuration.ini"
                 <<"settings.ini"
                 <<"settings.txt"
                 <<"cfg";
        for (int i=0; i<newsuffix.size(); i++) {
            cfgname=filename.left(filename.size()-suffix.size())+newsuffix[i];
            if (QFile::exists(cfgname)) {
                QSettings set(cfgname, QSettings::IniFormat);
                readB040SPIMExperimentConfigFile(set, sframetime, sbaseline_offset, sbackgroundF, image_width, image_height, hasPixel, pixel_width, pixel_height, dualViewMode);
                //qDebug()<<"read third  "<<sframetime<<sbaseline_offset<<sbackgroundF<<image_width<<image_height;
                break;
            }
        }


    }

    // if there is no direct match, we take the second best, if there is one
    if (!hasFirst && hasSecond)  {
        frametime=sframetime;
        baseline_offset=sbaseline_offset;
        backgroundF=sbackgroundF;
        //if (sIsTiff) ensureTiffReader();//ui->cmbFileformat->setCurrentIndex(0);
    }








    if (readFiles) {

        //////////////////////////////////////////////////////////////////////////////////
        // we also try to open the file with an appropriate reader and read th number of frames in it
        //////////////////////////////////////////////////////////////////////////////////
        prg.setLabelText(tr("Reading image series information ... counting frames ..."));
        QApplication::processEvents();
        QApplication::processEvents();
        QFImporterImageSeries* reader=NULL;
        bool OK=false;
        if (fileFormat>=0 && fileFormat<QFRDRImagingFCSCorrelationJobThread::getImageReaderCount(QFPluginServices::getInstance())) {
            reader=QFRDRImagingFCSCorrelationJobThread::getImageReader(fileFormat, QFPluginServices::getInstance());
        }
        if (reader) {
            OK=reader->open(filename);
            if (OK)  {
                QApplication::processEvents();
                image_width=reader->frameWidth();
                image_height=reader->frameHeight();
                if (channels) *channels=reader->frameChannels();
                if (frame_data) qfFree(frame_data);
                frame_data=NULL;
                frame_data=qfMallocT<double>(image_width*image_height);
                reader->readFrameDouble(frame_data);
                QVariant v=reader->getFileProperty("FRAMERATE");
                if (v.canConvert(QVariant::Double) && v.toDouble()>0) {
                    frametime=1.0/v.toDouble()*1e6;

                }
                v=reader->getFileProperty("FRAMETIME");
                if (v.canConvert(QVariant::Double) && v.toDouble()>0) {
                    frametime=v.toDouble()*1e6;
                }
                v=reader->getFileProperty("PIXEL_SIZE");
                if (v.canConvert(QVariant::Double) && v.toDouble()>0) {
                    pixel_width=v.toDouble();
                    pixel_height=v.toDouble();
                }
                v=reader->getFileProperty("PIXEL_WIDTH");
                if (v.canConvert(QVariant::Double) && v.toDouble()>0) {
                    pixel_width=v.toDouble();
                }
                v=reader->getFileProperty("PIXEL_HEIGHT");
                if (v.canConvert(QVariant::Double) && v.toDouble()>0) {
                    pixel_height=v.toDouble();
                }
                if (countFrames) frame_count=reader->countFrames();
            }
            reader->close();

        }

        //readBackgroundFramesize();
        readBackgroundProperties(backgroundF, fileFormat, parentWid, background_width_io, background_height_io, background_frame_count_io);
    }

    prg.close();
    QApplication::restoreOverrideCursor();

    if (image_width<=1) image_width=1;
    if (image_height<=1) image_height=1;

    // SET THE FRAMETIME/RATE
    if (frametime_io) *frametime_io=frametime;
    if (baseline_offset_io) *baseline_offset_io=baseline_offset;
    if (backgroundF_io) *backgroundF_io=backgroundF;
    if (pixel_width_io) *pixel_width_io=pixel_width;
    if (pixel_height_io) *pixel_height_io=pixel_height;
    if (dualViewMode_io) *dualViewMode_io=dualViewMode;
    if (hasPixel_io) *hasPixel_io=hasPixel;
    if (image_width_io) *image_width_io=image_width;
    if (image_height_io) *image_height_io=image_height;
    if (frame_count_io) *frame_count_io=frame_count;
    if (inputconfigfile_io) *inputconfigfile_io=inputconfigfile;
    if (frame_data_io) *frame_data_io=frame_data;
    //if (_io) *_io=;
}

void QFRDRImagingFCSCorrelationDialog::readBackgroundProperties(const QString &backgroundF, int fileFormat, QWidget* parentWid, int* background_width_io, int* background_height_io, int* background_frame_count_io)
{
    QModernProgressDialog prg(parentWid);
    prg.setWindowTitle(tr("imFCS: Correlator"));
    prg.setLabelText(tr("Reading background image series information ... reading config file ..."));
    prg.open();
    QApplication::processEvents();
    QApplication::processEvents();
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));


    //////////////////////////////////////////////////////////////////////////////////
    // we also try to open the file with an appropriate reader and read th number of frames in it
    //////////////////////////////////////////////////////////////////////////////////
    prg.setLabelText(tr("Reading background image series information ... counting frames ..."));
    QApplication::processEvents();
    QApplication::processEvents();
    QFImporterImageSeries* reader=NULL;
    bool OK=false;
    if (fileFormat>=0 && fileFormat<QFRDRImagingFCSCorrelationJobThread::getImageReaderCount(QFPluginServices::getInstance())) {
        reader=QFRDRImagingFCSCorrelationJobThread::getImageReader(fileFormat, QFPluginServices::getInstance());
    }
    if (reader && QFile::exists(backgroundF)) {
        OK=reader->open(backgroundF);
        if (OK)  {
            QApplication::processEvents();
            if (background_width_io) *background_width_io=reader->frameWidth();
            if (background_height_io) *background_height_io=reader->frameHeight();
            if (background_frame_count_io) *background_frame_count_io=reader->countFrames();

        } else {
        }
        reader->close();
        delete reader;
    }


    prg.close();
    QApplication::restoreOverrideCursor();

}

void QFRDRImagingFCSCorrelationDialog::openFile(const QString &file, const QString &filter)
{
    ui->edtImageFile->setText(file);
    int idx=imageFilters.indexOf(filter);
    if (idx<0) idx=imageFormatNames.indexOf(filter);
    if (idx>=0) ui->cmbFileformat->setCurrentIndex(idx);
    on_btnLoad_clicked();
}

void QFRDRImagingFCSCorrelationDialog::userSelectFile()
{
    on_btnSelectImageFile_clicked();
}

void QFRDRImagingFCSCorrelationDialog::clickAddJob()
{
    on_btnAddJob_clicked();
}

void QFRDRImagingFCSCorrelationDialog::clickAddJobSeries(const QString &parameter, double start, double end, double inc)
{
    onAddSeriesJobClicked(parameter, start, end, inc);
}



