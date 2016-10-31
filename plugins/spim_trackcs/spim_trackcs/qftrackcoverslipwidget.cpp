#include "qftrackcoverslipwidget.h"


QFTrackCoverSlipWidget::QFTrackCoverSlipWidget(QObject *parent) :
    QGenericPlugin(parent)
{
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(spim_trackcs, QFTrackCoverSlipWidget)
#endif // QT_VERSION < 0x050000
