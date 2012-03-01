#ifndef QFSIMPLEFITPARAMETERINTERFACE_H
#define QFSIMPLEFITPARAMETERINTERFACE_H

#include <QString>
#include <QtPlugin>
#include "libwid_imexport.h"

/*! \brief generic interface that allows to access values of fit parameters, even simpler than QFFitParameterBasicInterface
    \ingroup qf3lib_widgets

 */
class QFWIDLIB_EXPORT QFSimpleFitParameterInterface {
    public:
        virtual ~QFSimpleFitParameterInterface() {};


        /*! \brief set a fit parameter for the current record and resultID

            The value may be stored at different positions:
              - If no item is highlighted the parameter is not stored at all
              - If a current raw data record is highlighted and no fit has been carried out so far for this record, the value
                is stored in the preset value store of this QFEvaluationItem
              - If a current raw data record is highlighted and a fit has already been carried out for this record, the value
                is stored as a result in the highlighted QFRawDataRecord
            .
            This function also checks whether the value is to be stored (i.e. it's userEditable property is set)

            \param parameterID set the value of the parameter with this id (see QFFitFunction)
            \param value value to be stored
            \param unit unit to be stored

         */
        virtual void setFitValue(const QString& parameterID, double value)=0;

        /*! \brief return the value of a given parameter
            \param parameterID the parameter id
            \return the value of the given parameter \a id

            This function looks for the value at several positions:
              -# if a fit has already been performed for the current record, the value is read from raw data record
              -# if no fit has been performed, the value is taken from this evaluation (i.e. the preset value)
              -# if also locally no value is available, the value is looked up in the application INI
              -# if this also fails, the value is taken from the initial value stored in the fitFunction
            .
        */
        virtual double getFitValue(const QString& parameterID) const=0;
        /*! \brief return the fit error of a given parameter
            \param parameterID the parameter id
            \return the error associated with the given parameter.
        */
        virtual double getFitError(const QString& parameterID) const=0;

        /*! \brief set the error of a given parameter
            \param parameterID set the value of the parameter with this id (see QFFitFunction)
            \param error error to be set
        */
        virtual void setFitError(const QString& parameterID, double error)=0;

        /*! \brief set the fix property of a fit parameter of the current fit function (see m_fitFunction)

            For a description of when data is stored, see setFitValue()

            \param parameterID set the value of the parameter with this id (see QFFitFunction)
            \param fix fix to be stored

         */
        virtual void setFitFix(const QString& parameterID, bool fix)=0;

        /*! \brief return the fix property of a given parameter

            \param parameterID the parameter id
            For a detailed description of where the value is searched, see getFitValue()
        */
        virtual bool getFitFix(const QString& parameterID) const=0;

};


Q_DECLARE_INTERFACE(QFSimpleFitParameterInterface,
                     "www.dkfz.de.b040.quickfit3.QFSimpleFitParameterInterface/1.0");


#endif // QFSIMPLEFITPARAMETERINTERFACE_H
