/*
Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center

    last modification: $LastChangedDate: 2014-09-02 10:24:20 +0200 (Di, 02 Sep 2014) $  (revision $Rev: 3422 $)

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

#include "qffitfunctionsspimfccsdiffflowce2_newVeff_2.h"
#include "imfcstools.h"
#include <cmath>
#define sqr(x) ((x)*(x))
#define cube(x) ((x)*(x)*(x))
#define pow4(x) ((x)*(x)*(x)*(x))
#define pow5(x) ((x)*(x)*(x)*(x)*(x))
#define NAVOGADRO (6.02214179e23)

QFFitFunctionsSPIMFCCSDiffFlowCENewVeff2Global::QFFitFunctionsSPIMFCCSDiffFlowCENewVeff2Global() {
    //           type,         id,                        name,                                                    label,                      unit,          unitlabel,               fit,       userEditable, userRangeEditable, displayError,                initialValue, minValue, maxValue, inc, absMin, absMax
    addParameter(FloatNumber,  "n_particle",              "particle number N",                                     "N",                        "",            "",                      true,      true,         true,              QFFitFunction::DisplayError, false, 1,            1e-10,    1e5,      1,   0);
    #define FCCSDiff_n_particle 0
    addParameter(FloatNumber,  "1n_particle",             "1/particle number N",                                   "1/N",                      "",            "",                      false,     false,        false,             QFFitFunction::DisplayError, false, 0.01,         1e-10,    1e5,      0.1, 0);
    #define FCCSDiff_1n_particle 1
    addParameter(FloatNumber,  "diff_rho1",               "fraction of first component",                           "&rho;<sub>1</sub>",        "",            "",                      false,     false,        false,             QFFitFunction::DisplayError, false, 0.8,          0,        0.99999,  0.1, 0,      1);
    #define FCSSDiff_diff_rho1 2
    addParameter(FloatNumber,  "diff_coeff1",             "diffusion coefficient of species 1",                    "D<sub>1</sub>",            "micron^2/s", "&mu;m<sup>2</sup>/s",    true,      true,         true,              QFFitFunction::DisplayError, false, 10,           1e-10,    1e50,     1    );
    #define FCCSDiff_diff_coeff1 3
    addParameter(FloatNumber,  "vflowx",                  "flow speed in x-direction",                  "v<sub>flow, x</sub>",      "micron/s", "&mu;m/s",                  true,    true,        true,                 QFFitFunction::EditError, false, 100,          -1e50,        1e50,     1    );
    #define FCCSDiff_vflowx 4
    addParameter(FloatNumber,  "vflowy",                  "flow speed in y-direction",                  "v<sub>flow, y</sub>",      "micron/s", "&mu;m/s",                  true,    true,        true,                 QFFitFunction::EditError, false, 0,          -1e50,        1e50,     1    );
    #define FCCSDiff_vflowy 5
    addParameter(FloatNumber,  "vflow",                   "flow speed",                               "v<sub>flow</sub>",         "micron/s", "&mu;m/s",                  false,    false,        false,              QFFitFunction::DisplayError, false, 500,          0,        1e50,     1    );
    #define FCCSDiff_vflow 6
    addParameter(FloatNumber,  "vflow_angle",             "angle of the flow speed",                               "&alpha;<sub>flow</sub>",         QLatin1String("\xB0"), QLatin1String("\xB0"),                  false,    false,        false,              QFFitFunction::DisplayError, false, 500,          0,        1e50,     1    );
    #define FCCSDiff_alphaflow 7
    addParameter(FloatNumber,  "vflow_angle_centered",             "centered angle of the flow speed",                               "&alpha;<sub>flow</sub>'",         QLatin1String("\xB0"), QLatin1String("\xB0"),                  false,    false,        false,              QFFitFunction::DisplayError, false, 0,          0,        1e50,     1    );
    #define FCCSDiff_alphaflow_centered 8

    addParameter(FloatNumber,  "diff_rho2",               "fraction of second component",                          "&rho;<sub>2</sub>",        "",            "",                      true,      true,         true,              QFFitFunction::DisplayError, false, 0.2,          0,        0.99999,  0.1, 0,      1  );
    #define FCSSDiff_diff_rho2 9
    addParameter(FloatNumber,  "diff_coeff2",             "diffusion coefficient of species 2",                    "D<sub>2</sub>",            "micron^2/s", "&mu;m<sup>2</sup>/s",    true,      true,         true,              QFFitFunction::DisplayError, false, 10,           1e-10,    1e50,     1    );
    #define FCCSDiff_diff_coeff2 10


    addParameter(FloatNumber,  "offset",                  "correlation offset",                                    "G<sub>&infin;</sub>",      "",           "",                       true,      true,         true,              QFFitFunction::DisplayError, true, 0,            -10,      10,       0.1  );
    #define FCCSDiff_offset 11
    addParameter(FloatNumber,  "focus_distance_x",         "foci: lateral distance in x-direction",                 "d<sub>x</sub>",            "nm",         "nm",                     true,      true,         true,              QFFitFunction::EditError,    true, 1000,           -1e6,     1e6,      10  );
    #define FCCSDiff_focus_distancex 12
    addParameter(FloatNumber,  "focus_distance_y",         "foci: lateral distance in y-direction",                 "d<sub>y</sub>",            "nm",         "nm",                     true,      true,         true,              QFFitFunction::EditError,    true, 0,              -1e6,     1e6,      10  );
    #define FCCSDiff_focus_distancey 13
    addParameter(FloatNumber,  "focus_distance",         "foci: lateral distance",                                  "d<sub>xy</sub>",            "nm",         "nm",                     false,      false,         false,              QFFitFunction::DisplayError,    true, 1000,              0,     1e6,      10  );
    #define FCCSDiff_focus_distance 14
    addParameter(FloatNumber,  "focus_height",            "PSF: axial radius (1/e^2 radius)",                        "w<sub>z</sub>",      "nm",         "nm",                     true,      true,         true,              QFFitFunction::EditError,    true, 1240,         0.01,     1e5,      10  );
    #define FCCSDiff_focus_height 15
    addParameter(FloatNumber,  "focus_width",             "PSF: lateral radius (1/e^2 radius)",                      "w<sub>x,y</sub>",    "nm",         "nm",                     true,      true,         true,              QFFitFunction::EditError,    true, 600,          0,        1e4,      10    );
    #define FCCSDiff_focus_width 16
    addParameter(FloatNumber,  "pixel_width",             "pixel width",                                           "a",                        "nm",         "nm",                     true,      true,         true,              QFFitFunction::EditError,    true, 400,          0,        1e4,      10    );
    #define FCCSDiff_pixel_width 17
    addParameter(FloatNumber,  "focus_volume",            "focus: effective volume",                               "V<sub>eff</sub>",          "fl",         "fl",                     false,    false,        false,              QFFitFunction::DisplayError, false, 0.5,          0,        1e50,     1    );
    #define FCCSDiff_focus_volume 18
    addParameter(FloatNumber,  "effective_area",            "focus: effective area",                               "A<sub>eff</sub>",          "micron^2",         "&mu;m<sup>2</sup>",                     false,    false,        false,              QFFitFunction::DisplayError, false, 0.5,          0,        1e50,     1    );
    #define FCSSDiff_focus_area 19
    addParameter(FloatNumber,  "concentration",           "particle concentration in focus",                       "C<sub>all</sub>",          "nM",         "nM",                     false,    false,        false,              QFFitFunction::DisplayError, false, 0.5,          0,        1e50,     1    );
    #define FCCSDiff_concentration 20
    addParameter(FloatNumber,  "count_rate",              "count rate during measurement",                         "count rate",               "Hz",         "Hz",                     false,    true,         false,              QFFitFunction::EditError,    false, 0,            0,        1e50,     1    );
    #define FCCSDiff_count_rate 21
    addParameter(FloatNumber,  "background",              "background count rate during measurement",              "background",               "Hz",         "Hz",                     false,    true,         false,              QFFitFunction::EditError  ,  false, 0,            0,        1e50,     1    );
    #define FCCSDiff_background 22
    addParameter(FloatNumber,  "cpm",                     "photon counts per molecule",                            "cnt/molec",                "Hz",         "Hz",                     false,    false,        false,              QFFitFunction::DisplayError, false, 0,            0,        1e50,     1    );
    #define FCCSDiff_cpm 23

}

double QFFitFunctionsSPIMFCCSDiffFlowCENewVeff2Global::evaluate(double t, const double* data) const {
    const double N=data[FCCSDiff_n_particle];
    const double D1=data[FCCSDiff_diff_coeff1];

    const double D2=data[FCCSDiff_diff_coeff2];
    const double r2=data[FCSSDiff_diff_rho2];
    const double r1=1.0-qBound(0.0,r2,1.0);

    const double dx=data[FCCSDiff_focus_distancex]/1000.0;
    const double dy=data[FCCSDiff_focus_distancey]/1000.0;
    const double vx=data[FCCSDiff_vflowx];
    const double vy=data[FCCSDiff_vflowy];

    const double wz=data[FCCSDiff_focus_height]/1.0e3;
    const double wxy=data[FCCSDiff_focus_width]/1.0e3;
    const double a=data[FCCSDiff_pixel_width]/1.0e3;
    const double offset=data[FCCSDiff_offset];
    const double background=data[FCCSDiff_background];
    const double cr=data[FCCSDiff_count_rate];
    double backfactor=sqr(cr-background)/sqr(cr);
    if (fabs(cr)<1e-15 || fabs(background)<1e-10) backfactor=1;




    double cfac=r1*QFFitFunctionsSPIMFCCSFWDiff2ColorCCF_corrfactor(a, dx-vx*t, dy-vy*t, 0, D1, t, wxy, wxy, wz, wz)+
                r2*QFFitFunctionsSPIMFCCSFWDiff2ColorCCF_corrfactor(a, 0, 0, 0, D2, t, wxy, wxy, wz, wz);

    const double Veff=SPIMFCS_newVeff(a, wxy, wz);
    const double pre=1.0/sqr(a);
    return offset+pre/(N/Veff)*cfac*backfactor;
}

void QFFitFunctionsSPIMFCCSDiffFlowCENewVeff2Global::calcParameter(double* data, double* error) const {
    double N=data[FCCSDiff_n_particle];
    double eN=0;
    //double D1=data[FCCSDiff_diff_coeff1];
    double eD1=0;
    double wz=data[FCCSDiff_focus_height]/1.0e3;
    double ewz=0;
    double wxy=data[FCCSDiff_focus_width]/1.0e3;
    double ewxy=0;
    double a=data[FCCSDiff_pixel_width]/1.0e3;
    double ea=0;
    //double offset=data[FCCSDiff_offset];
    double eoffset=0;

    double dx=data[FCCSDiff_focus_distancex];
    double edx=0;
    double dy=data[FCCSDiff_focus_distancey];
    double edy=0;
    double vx=data[FCCSDiff_vflowx];
    double evx=0;
    double vy=data[FCCSDiff_vflowy];
    double evy=0;

    double r2=data[FCSSDiff_diff_rho2];
    double er2=0;

    double cps=data[FCCSDiff_count_rate];
    double ecps=0;
    double ecpm=0;
    double background=data[FCCSDiff_background];
    double ebackground=0;

    if (error) {
        eN=error[FCCSDiff_n_particle];
        er2=error[FCSSDiff_diff_rho2];
        eD1=error[FCCSDiff_diff_coeff1];
        ewz=error[FCCSDiff_focus_height]/1.0e3;
        ewxy=error[FCCSDiff_focus_width]/1.0e3;
        ea=error[FCCSDiff_pixel_width]/1.0e3;
        eoffset=error[FCCSDiff_offset];
        ecps=error[FCCSDiff_count_rate];
        ecpm=error[FCCSDiff_cpm];
        ebackground=error[FCCSDiff_background];
        edx=error[FCCSDiff_focus_distancex];
        edy=error[FCCSDiff_focus_distancey];
        evx=error[FCCSDiff_vflowx];
        evy=error[FCCSDiff_vflowy];
    }



    // calculate dxy=sqrt(dx^2+dy^2)
    data[FCCSDiff_focus_distance]=sqrt(dx*dx+dy*dy);
    if (error) {
        error[FCCSDiff_focus_distance]=sqrt(sqr(edx*2.0*dx)/(dx*dx+dy*dy)+sqr(edy*2.0*dy)/(dx*dx+dy*dy));
        if (!QFFloatIsOK(error[FCCSDiff_focus_distance])) error[FCCSDiff_focus_distance]=0;
    }

    // calculate vxy=sqrt(vx^2+vy^2)
    data[FCCSDiff_vflow]=sqrt(vx*vx+vy*vy);
    if (error) {
        error[FCCSDiff_vflow]=sqrt(sqr(evx*2.0*vx)/(vx*vx+vy*vy)+sqr(evy*2.0*vy)/(vx*vx+vy*vy));
        if (!QFFloatIsOK(error[FCCSDiff_vflow])) error[FCCSDiff_vflow]=0;
    }


    // calculate valpha=(atan(vy/vx)+pi)/pi*180
    double an;
    data[FCCSDiff_alphaflow]=an=(atan2(vy,vx)+M_PI)/M_PI*180.0;
    if (error) {
        error[FCCSDiff_alphaflow]=0;
        if (!QFFloatIsOK(error[FCCSDiff_alphaflow])) error[FCCSDiff_alphaflow]=0;
    }

    // calculate centered valpha=(atan(vy/vx)+pi)/pi*180
    data[FCCSDiff_alphaflow_centered]=an;
    if (an>180.0) data[FCCSDiff_alphaflow_centered]=an-360.0;
    if (error) {
        error[FCCSDiff_alphaflow_centered]=error[FCCSDiff_alphaflow];
    }

    // calculate 1/N
    data[FCCSDiff_1n_particle]=1.0/N;
    if (error) error[FCCSDiff_1n_particle]=fabs(eN/N/N);

    // calculate Veff
    data[FCCSDiff_focus_volume]=SPIMFCS_newVeff(a, wxy, wz);;
    if (error) error[FCCSDiff_focus_volume]=SPIMFCS_newVeffError(a, ea, wxy, ewxy, wz, ewz);

    // calculate Aeff
    data[FCSSDiff_focus_area]=SPIMFCS_newAeff(a, wxy);
    if (error) error[FCSSDiff_focus_area]=SPIMFCS_newAeffError(a, ea, wxy, ewxy);


    // calculate C = N / Veff
    if (data[FCCSDiff_focus_volume]!=0) {
        data[FCCSDiff_concentration]=N/data[FCCSDiff_focus_volume];
    } else {
        data[FCCSDiff_concentration]=0;
    }
    if (data[FCCSDiff_focus_volume]!=0 && error) {
        error[FCCSDiff_concentration]=sqrt(qfSqr(eN/data[FCCSDiff_focus_volume])+qfSqr(error[FCCSDiff_focus_volume]*N/qfSqr(data[FCCSDiff_focus_volume])));//N/data[FCCSDiff_focus_volume]
    }


    // calculate CPM = (CPS-background)/N
    data[FCCSDiff_cpm]=(cps-background)/N;
    error[FCCSDiff_cpm]=sqrt(sqr(ecps/N)+sqr(ebackground/N)+sqr(eN*(cps-background)/sqr(N)));

    // calculate rho1
    data[FCSSDiff_diff_rho1]=1.0-qBound(0.0, r2, 1.0);
    error[FCSSDiff_diff_rho1]=er2;

}

bool QFFitFunctionsSPIMFCCSDiffFlowCENewVeff2Global::isParameterVisible(int parameter, const double* data) const {

    Q_UNUSED(parameter);
    Q_UNUSED(data);
    return true;
}

unsigned int QFFitFunctionsSPIMFCCSDiffFlowCENewVeff2Global::getAdditionalPlotCount(const double* params) const {

    Q_UNUSED(params);
    return 0;
}

QString QFFitFunctionsSPIMFCCSDiffFlowCENewVeff2Global::transformParametersForAdditionalPlot(int plot, double* params) const {


    Q_UNUSED(plot);
    Q_UNUSED(params);
    return "";
}
