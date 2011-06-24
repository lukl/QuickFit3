#ifndef QFRDRIMAGINGFCSDATA_H
#define QFRDRIMAGINGFCSDATA_H

#include <QList>
#include <QObject>
#include <QMap>
#include <QProgressBar>
#include <QStatusBar>
#include <QWidget>
#include <QMenu>
#include <QPair>
#include <QList>
#include <QAction>
#include "qfrawdatarecord.h"
#include "qfrdrimagingfcs_dataeditor.h"
#include "qfrawdatarecordfactory.h"
#include "../interfaces/qfrdrfcsdatainterface.h"



/*! \brief QFRawDataRecord implementation
    \ingroup qf3rdrdp_imaging_fcs

*/
class QFRDRImagingFCSData : public QFRawDataRecord, public QFRDRFCSDataInterface {
        Q_OBJECT
        Q_INTERFACES(QFRDRFCSDataInterface)
    public:
        /** Default constructor */
        QFRDRImagingFCSData(QFProject* parent);
        /** Default destructor */
        virtual ~QFRDRImagingFCSData();

        /** \brief return type (short type string) */
        virtual QString getType() const { return "imaging_fcs"; };
        /** \brief return type (longer type string, user readable) */
        virtual QString getTypeName() const { return tr("Imaging FCS Data"); };
        /** \brief return a small icon (16x16) */
        virtual QIcon getSmallIcon() const { return QIcon(":/imaging_fcs/qfrdrimagingfcs_small.png"); };
        /** \brief return type description */
        virtual QString getTypeDescription() const { return tr("manages FCS correlation curves from an imaging FCS experiment (SPIM-FCS, TIR-FCS ...)"); };
        /** \brief return a large icon (32x32) */
        virtual QIcon getLargeIcon() const { return QIcon(":/imaging_fcs/qfrdrimagingfcs.png"); };
        /** \brief returns the number of additional editor panes for this record */
        virtual int getEditorCount() { return 1; };
        /** \brief returns the name for the i-th editor pane */
        virtual QString getEditorName(int i) {
            if (i==0) return tr("Correlation Curves");
            return QString("");
        };
        /** \brief create an object for the i-th editor pane */
        virtual QFRawDataEditor* createEditor(QFPluginServices* services, int i=0, QWidget* parent=NULL) {
            if (i==0) return new QFRDRImagingFCSDataEditor(services, parent);
            return NULL;
        };
        /** \brief export the raw data into the specified format */
        virtual void exportData(const QString& format, const QString& filename)const ;
        /** \brief returns a list of filetypes which correspond to the filetypes returned by getExportFiletypes() */
        QStringList getExportFiletypes() {
            QStringList sl;
            return sl;
        };
        /** \brief returns the title of the Export file dialog */
        QString getExportDialogTitle() { return tr(""); };
        /** \brief returns the filetype of the Export file dialog */
        QString getExportDialogFiletypes() { return tr(""); };



        /** \copydoc QFRDRFCSDataInterface::getCorrelationRuns() */
        virtual int getCorrelationRuns();
        /** \copydoc QFRDRFCSDataInterface::getCorrelationN() */
        virtual long long getCorrelationN();
        /** \copydoc QFRDRFCSDataInterface::getCorrelationT() */
        virtual double* getCorrelationT();
        /** \copydoc QFRDRFCSDataInterface::getCorrelation() */
        virtual double* getCorrelation();
        /** \copydoc QFRDRFCSDataInterface::getCorrelationRun() */
        virtual double* getCorrelationRun(int run);
        /** \copydoc QFRDRFCSDataInterface::getCorrelationRunError() */
        virtual double* getCorrelationRunError(int run);
        /** \copydoc QFRDRFCSDataInterface::getCorrelationRunName() */
        virtual QString getCorrelationRunName(int run);
        /** \copydoc QFRDRFCSDataInterface::getCorrelationMean() */
        virtual double* getCorrelationMean();
        /** \copydoc QFRDRFCSDataInterface::getCorrelationStdDev() */
        virtual double* getCorrelationStdDev();
        /** \copydoc QFRDRFCSDataInterface::isCorrelationRunVisible() */
        virtual bool isCorrelationRunVisible(int run);
        /** \copydoc QFRDRFCSDataInterface::getCorrelationRunErrors() */
        virtual double* getCorrelationRunErrors();


		/** \brief return width of the image */
		virtual int getDataImageWidth() const { return width; }
		/** \brief return height of the image */
		virtual int getDataImageHeight() const { return height; }

    protected:
        /** \brief write the contents of the object to a XML file */
        virtual void intWriteData(QXmlStreamWriter& w);
        /** \brief read in external data files <b>and</b> data stored in the project file <b>IMPLEMENT IN CHILD CLASSES!</b>
         *
         * If \a e is \c NULL then this method should only read the datafiles already saved in the files property.
         */
        virtual void intReadData(QDomElement* e=NULL);

		/** \brief load data file */
        bool loadDataFile(QString filename);

    private:
		/** \brief width of the image */
		int width;
		/** \brief height of the image */
		int height;
		/** \brief number of points in correlation curve */
		int N;
		/** \brief points to the correlation curves */
		double* correlations;
		/** \brief average over all correlation curves */
		double* correlationMean;
		/** \brief average over all correlation curves */
		double* correlationStdDev;
		/** \brief points to the correlation curve erorrs */
		double* sigmas;
		/** \brief time axis [seconds] */
		double * tau;


        /** \brief convert a pixel coordinate to a rund index */
		inline int xyToRun(int x, int y) {
		    return y*width+x;
		}
		/** \brief convert a run to a pixel x-coordinate */
        inline int runToX(int run) {
            return run%width;
        }
		/** \brief convert a run to a pixel y-coordinate */
        inline int runToY(int run) {
            return run/width;
        }
        /** \brief convert a pixel coordinate to an array index (in correlations and sigmas) */
		inline int xyToIndex(int x, int y) {
		    return (y*width+x)*N;
		}
        /** \brief allocate memory to store a \a x by \a y set of correlation curves (+ additional data, like average and sigmas) with \a N datapoints each */
        void allocateContents(int x, int y, int N);
};


#endif // QFRDRIMAGINGFCSDATA_H
