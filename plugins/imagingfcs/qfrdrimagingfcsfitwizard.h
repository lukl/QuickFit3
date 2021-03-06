#ifndef QFRDRIMAGINGFCSFITWIZARD_H
#define QFRDRIMAGINGFCSFITWIZARD_H

#include <QPointer>
#include <QWidget>
#include "qfwizard.h"
#include "qfselectfileslistwidget.h"
#include "qfimageplot.h"
#include "qfrdrimagingfcscorrelationjobthread.h"
#include "qspecialtoolbutton.h"
#include "qfenhancedlineedit.h"
#include "qfstyledbutton.h"
#include "qfpixelsizeedit.h"
#include "qfframerangeedit.h"
#include "qfrdrimagingfcscorrelationdialog.h"
#include "qfrdrimagingfcspixelfromobjective.h"
#include "qfevaluationitemfactory.h"
#include "qfcroppixelsedit.h"
#include "qfpseudotreemodel.h"
#include "qffitfunctionconfigforglobalfitinterface.h"

class QFRDRImagingFCSPlugin; // forward
class QFRDRImagingFCFitSWizard_BackgroundIsValid;
class QFRDRImagingFCFitSWizard_ImagestackIsValid;
class QFRDRImagingFCFitSWizard_BackgroundNextId;
class QFRDRImagingFCFitSWizard_ProcessNextId;

class QFRDRImagingFCSFitWizard : public QFWizard {
        Q_OBJECT
    public:
        explicit QFRDRImagingFCSFitWizard(QWidget* parent=NULL);
        ~QFRDRImagingFCSFitWizard();
        enum Pages {
            InitPage=0,
            MicroscopyPage,
            LastPage,
        };

    public slots:
        void finalizeAndModifyProject(QFRDRImagingFCSPlugin* plugin);

    protected slots:

        void finishedIntro();
        void microscopyChoosen();
        void validateCorrelation();
    protected:

        QFTextWizardPage* wizIntro;
        QFFormWizardPage* wizMicroscopy;
        QDoubleSpinBox* spinWz;
        QFEnhancedComboBox* cmbMicroscopy;
        QLabel* labMicroscopy;
        QLabel* labWz;
        QDoubleSpinBox* spinWxy;
        QLabel* labWxy;
        QSpinBox* spinCCFs;

        QFFormWizardPage* wizFinalizePage;
        //QLabel* labFinal;
        QPointer<QCheckBox> chkLastImFCSFit1;
        QPointer<QCheckBox> chkLastIm2cFCCSFit;
        QPointer<QCheckBox> chkLastIm2fFCCSFit;
        QPointer<QFEnhancedComboBox> cmbImFCSFitMode;
        QPointer<QFEnhancedComboBox> cmbIm2cFCCSFitMode;
        QPointer<QFEnhancedComboBox> cmbIm2fFCCSFitMode;


        bool isCalibration;

};


#endif // QFRDRIMAGINGFCSFITWIZARD_H
