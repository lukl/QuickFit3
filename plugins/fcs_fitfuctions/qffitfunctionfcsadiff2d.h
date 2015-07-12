/*
Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center (DKFZ) & IWR, University of Heidelberg

    last modification: $LastChangedDate: 2014-09-26 12:40:44 +0200 (Fr, 26 Sep 2014) $  (revision $Rev: 3508 $)

    This file is part of QuickFit 3 (http://www.dkfz.de/Macromol/quickfit).

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

#ifndef QFFitFunctionFCSADiff2D_H
#define QFFitFunctionFCSADiff2D_H
#include "qfpluginfitfunction.h"
#include "qffitfunction.h"



/*! \brief plugin class for FCS Anomalous Diffusion Fit Functions
    \ingroup qf3fitfunp_fcs


    This class implements a 3D anomalous diffusion model for the fitting routines in QuickFit2. The model may be denoted as:
      \f[ G(\tau)=G_\infty+\frac{1}{N}\cdot\frac{1}{(1+b/\mbox{cps})^2}\cdot\left(\frac{1-\Theta_{non}+\Theta_{non}\mathrm{e}^{-\tau/\tau_{non}}-\Theta_{trip}+\Theta_{trip}\mathrm{e}^{-\tau/\tau_{trip}}}{1-\Theta_{non}-\Theta_{trip}}\right)\cdot\left[(1-\rho_2-\rho_3)\cdot g_1(\tau)+\rho_2\cdot g_2(\tau)+\rho_3\cdot g_3(\tau)\right] \f]
    with
      \f[ g_i(\tau)=\left(1+\left(\frac{\tau}{\tau_{diff,i}}\right)^{\alpha_i}\right)^{-1}\cdot\left(1+\frac{1}{\gamma^2}\cdot\left(\frac{\tau}{\tau_{diff,i}}\right)^{\alpha_i}\right)^{-1/2} \f]
    The parameters are:
      - \f$ G_\infty \f$: offset of the correlation function
      - \f$ N \f$: overall particle number (including currently dark particles, e.g. in triplet state)
      - \f$ \Theta_{trip}, \Theta_{non} \f$: fraction of the particles in one of the first two non-fluorescent states (triplet, ...)
      - \f$ \tau_{trip}, \tau_{non} \f$: decay time of the first two non-fluorescent states (triplet, ...)
      - \f$ \rho_1=1-\rho2-\rho3, \rho_2, \rho_3 \f$: fractions of the three diffusing components
      - \f$ \tau_{diff,i} \f$: diffusion decay time of the i-th diffusing component
      - \f$ \alpha_i \f$: anomality exponent of the i-th diffusing fraction
      - \f$ \gamma=\frac{z_0}{w_{xy}} \f$: aspect ratio of the gaussian used to approximate the focus
      - \f$ w_{xy} \f$: lateral half axis of the focus gaussian
      - \f$ z_0 \f$: longitudinal half axis of the focus gaussian
    .


    This model also calculates the effective focal volume:
      \f[ A_{eff}=\pi\cdot w_{xy}^2 \f]

    Given the count rate cps and the background count rate \f$ b \f$, we can calculate the counts per molecule:
      \f[ \mbox{cpm}=\frac{\mbox{cps}-b}{N} \f]

    The plugin also calculates some parameter errors:
      \f[ \Delta \left(\frac{1}{N}\right)=\left|\frac{\Delta N}{N^2}\right| \f]
      \f[ \Delta A_{eff}=\sqrt{\left(\Delta w_{xy}\cdot 2\pi\cdot \cdot w_{xy}\right)^2} \f]
      \f[ \Delta \mbox{cpm}=\sqrt{\left(\Delta\mbox{cps}\cdot\frac{1}{N}\right)^2+\left(\Delta b\cdot\frac{1}{N}\right)^2+\left(\Delta N\cdot\frac{\mbox{cps}-b}{N^2}\right)^2} \f]


*/
class QFFitFunctionFCSADiff2D: public QFFitFunction {
    public:
        QFFitFunctionFCSADiff2D(bool isTIRF=false);
        virtual ~QFFitFunctionFCSADiff2D() {}
        /*! \copydoc QFFitFunction::name()   */
        virtual QString name() const {
            if (!isTIRF) return QObject::tr("FCS: 2D Anomalous Diffusion");
            else return QObject::tr("TIR-FCS: 2D Anomalous Diffusion (Gaussian, 1/e^2 radii)");
        }
        /*! \copydoc QFFitFunction::id()   */
        virtual QString id() const {
            if (!isTIRF) return QString("fcs_adiff2d");
            else return QString("fcs_tir_adiff2d");
        }

        /*! \copydoc QFFitFunction::category()   */
        virtual QString category() const {
            if (!isTIRF) return QObject::tr("Confocal FCS");
            else return QObject::tr("Confocal TIR-FCS");
        }


        /*! \copydoc QFFitFunction::evaluate()   */
        virtual double evaluate(double x, const double* parameters) const;

        /*! \copydoc QFFitFunction::calcParameter()   */
        virtual void calcParameter(double* parameterValues, double* error=NULL) const;

        /*! \copydoc QFFitFunction::isParameterVisible()   */
        virtual bool isParameterVisible(int parameter, const double* parameterValues) const;
        /*! \copydoc QFFitFunction::getAdditionalPlotCount()   */
        virtual unsigned int getAdditionalPlotCount(const double* params) const;

        /*! \copydoc QFFitFunction::transformParametersForAdditionalPlot()   */
        virtual QString transformParametersForAdditionalPlot(int plot, double* params) const;
    private:
        bool isTIRF;
};




#endif // QFFitFunctionFCSADiff2D_H
