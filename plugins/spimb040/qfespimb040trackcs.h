#ifndef QFETRACKCS
#define QFETRACKCS

#include <QGroupBox>

#include "qfextensionmanager.h"
#include "qfextensionlinearstage.h"
#include "qfextension.h"

class QFESPIMB040MainWindow; // forward
class QFESPIMB040SimpleCameraConfig; //forward
class QFESPIMB040AcquisitionTools;//forward

class QFESPIMB040TrackCS: public QWidget {
        Q_OBJECT
    public:
        QFESPIMB040TrackCS(QWidget* parent=NULL);
        virtual ~QFESPIMB040TrackCS();


        void createWidgets();
        void setWidgetsInActive();
    protected:

        QToolButton* btnConfigSteps;

};


#endif // QFETRACKCS
