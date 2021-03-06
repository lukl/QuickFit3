/*
    Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center (DKFZ) & IWR, University of Heidelberg

    last modification: $LastChangedDate$  (revision $Rev:$)

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


#ifndef QFGETPLOTDATAINTERFACE_H
#define QFGETPLOTDATAINTERFACE_H

#include "qfplugin.h"

/*! \brief interface for returning data for a plot
    \ingroup qf3lib_tools

    Data for plots is returned in the form of a QFGetPlotdataInterface::evalPlotData struct,
    which defines data and properties for a single graph.

    The availability of the feature can be acquired using getPlotDataSpecs(), which return \c true, if the feature is actually
    implemented. In addition it returns a list of optionNames (QStringlList, call-by-reference), which can be used to let the user decide
    on the type of returned data (usually you can present the data in a QComboBox for the user to choose the type of returned data.

    The actual data is returned (appended to a list) by the function getPlotData(). The argument option specifies, which data to return and
    is interpreted as index in the QStringList returned by getPlotDataSpecs() (see above).

*/
#include <QVector>
#include <QString>

class QFRawDataRecord; // forward

class QFGetPlotdataInterface
{
    public:
        struct GetPlotDataItem {
            bool majorPlot;
            /** \brief x-data for the plot */
            QVector<double> x;
            /** \brief optional x-data-error for the plot */
            QVector<double> xerrors;
            /** \brief y-data for the plot */
            QVector<double> y;
            /** \brief y-data-error for the plot */
            QVector<double> yerrors;
            /** \brief label of the plot */
            QString name;
            /** \brief the overview plot tool may supply the option to average over all curves that have the same positive value in averageGroupIndex. A value of \c averageGroupIndex=-1 [default] disables this bhaviour. */
            int averageGroupIndex;


            GetPlotDataItem() {
                majorPlot=true;
                name=QString();
                averageGroupIndex=-1;
            }
        };

        struct GetPlotPlotOptions {
            /** \brief should the x-axis be log.scaled? */
            bool logX;
            /** \brief should the y-axis be log.scaled? */
            bool logY;
            /** \brief x-axis label */
            QString xLabel;
            /** \brief y-axis label */
            QString yLabel;
            GetPlotPlotOptions(const QString& xLabel=QString("X"), const QString& yLabel=QString("Y"), bool logX=false, bool logY=false) {
                this->logX=logX;
                this->logY=logY;
                this->xLabel=xLabel;
                this->yLabel=yLabel;
            }
        };

        virtual ~QFGetPlotdataInterface() {}
        /** \brief returns plot data (appends to the list \a data ) from the given record and index therein. \a option specifies, which data to return. */
        virtual void getPlotData(QList<GetPlotDataItem>& data, int option, const QString& optionName)=0;
        /** \brief returns \c true, if the getPlotData Feature is available and (if lists are supplied) a list of optionNames. */
        virtual bool getPlotDataSpecs(QStringList* optionNames=NULL, QList<GetPlotPlotOptions>* listPlotOptions=NULL)=0;
};


class QFEvaluationGetPlotdataInterface
{
    public:

        virtual ~QFEvaluationGetPlotdataInterface() {}
        /** \brief returns plot data (appends to the list \a data ) from the given record and index therein. \a option specifies, which data to return. */
        virtual void getPlotData(QFRawDataRecord* rec, int index, QList<QFGetPlotdataInterface::GetPlotDataItem>& data, int option, const QString& optionName)=0;
        /** \brief returns \c true, if the getPlotData Feature is available and (if lists are supplied) a list of optionNames. */
        virtual bool getPlotDataSpecs(QStringList* optionNames=NULL, QList<QFGetPlotdataInterface::GetPlotPlotOptions>* listPlotOptions=NULL)=0;
};


Q_DECLARE_INTERFACE( QFGetPlotdataInterface,
                     "www.dkfz.de.b040.quickfit3.QFGetPlotdataInterface/1.0")


Q_DECLARE_INTERFACE( QFEvaluationGetPlotdataInterface,
                     "www.dkfz.de.b040.quickfit3.QFEvaluationGetPlotdataInterface/1.0")


#endif // QFGETPLOTDATAINTERFACE_H
