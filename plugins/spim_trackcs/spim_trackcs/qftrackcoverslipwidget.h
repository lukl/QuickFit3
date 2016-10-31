#ifndef QFTRACKCOVERSLIPWIDGET_H
#define QFTRACKCOVERSLIPWIDGET_H

#include <QGenericPlugin>


class QFTrackCoverSlipWidget : public QGenericPlugin
{
    Q_OBJECT
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QGenericPluginFactoryInterface" FILE "spim_trackcs.json")
#endif // QT_VERSION >= 0x050000

public:
    QFTrackCoverSlipWidget(QObject *parent = 0);
};

#endif // QFTRACKCOVERSLIPWIDGET_H
