#include "qffitfunctionpsf3dgaussian.h"
#include <cmath>
#include "qftools.h"
#include "statistics_tools.h"

QFFitFunctionPSF3DGaussian::QFFitFunctionPSF3DGaussian()
{
    //           type,         id,                        name,                                                    label (HTML),                      unit,          unitlabel (HTML),        fit,       userEditable, userRangeEditable, displayError, initialFix,                initialValue, minValue, maxValue, inc, absMin, absMax
    addParameter(FloatNumber,  "offset",                  "offset",                                                "offset",                          "",            "",                      true,      true,         true,              QFFitFunctionBase::DisplayError,       false, 0.0,          -1e10,    1e10,  1  );
    #define PARAM_OFFSET 0
    addParameter(FloatNumber,  "amplitude",               "amplitude",                                             "amplitude",                       "",            "",                      true,      true,         true,              QFFitFunctionBase::DisplayError,       false, 1000.0,       0,    1e10,  1  );
    #define PARAM_AMPLITUDE 1
    addParameter(FloatNumber,  "position_x",              "x0",                                                    "x<sub>0</sub>",                   "pix",         "pix",                   true,      true,         true,              QFFitFunctionBase::DisplayError,       false, 0,            -1e10,    1e10,  1  );
    #define PARAM_POSITIONX 2
    addParameter(FloatNumber,  "position_y",              "y0",                                                    "y<sub>0</sub>",                   "pix",         "pix",                   true,      true,         true,              QFFitFunctionBase::DisplayError,       false, 0,            -1e10,    1e10,  1  );
    #define PARAM_POSITIONY 3
    addParameter(FloatNumber,  "position_z",              "z0",                                                    "z<sub>0</sub>",                   "pix",         "pix",                   true,      true,         true,              QFFitFunctionBase::DisplayError,       false, 0,            -1e10,    1e10,  1  );
    #define PARAM_POSITIONZ 4
    addParameter(FloatNumber,  "width1",                  "width1",                                                "width1",                          "pix",         "pix",                   true,      true,         true,              QFFitFunctionBase::DisplayError,       false, 1,            0,    1e10,  1  );
    #define PARAM_WIDTH1 5
    addParameter(FloatNumber,  "width2",                  "width2",                                                "width2",                          "pix",         "pix",                   true,      true,         true,              QFFitFunctionBase::DisplayError,       false, 1,            0,    1e10,  1  );
    #define PARAM_WIDTH2 6
    addParameter(FloatNumber,  "width3",                  "width3",                                                "width3",                          "pix",         "pix",                   true,      true,         true,              QFFitFunctionBase::DisplayError,       false, 5,            0,    1e10,  1  );
    #define PARAM_WIDTH3 7

    addParameter(FloatNumber,  "theta",                   "theta",                                                 "&theta;",                         "\xB0",            "&deg;",                   true,      true,         true,              QFFitFunctionBase::DisplayError,       false, 0,            -45,    45,  1  );
    #define PARAM_THETA 8
    addParameter(FloatNumber,  "phi",                     "phi",                                                   "&phi;",                           "\xB0",            "&deg;",                   true,      true,         true,              QFFitFunctionBase::DisplayError,       false, 0,            -45,    45,  1  );
    #define PARAM_PHI 9
    addParameter(FloatNumber,  "alpha",                   "alpha",                                                 "&alpha;",                         "\xB0",            "&deg;",                   true,      true,         true,              QFFitFunctionBase::DisplayError,       false, 0,            0,    45,  1  );
    #define PARAM_ALPHA 10


}

double QFFitFunctionPSF3DGaussian::evaluate(double x, double y, double z, const double *parameters) const
{
    const double offset=parameters[PARAM_OFFSET];
    const double amplitude=parameters[PARAM_AMPLITUDE];
    const double x0=parameters[PARAM_POSITIONX];
    const double y0=parameters[PARAM_POSITIONY];
    const double z0=parameters[PARAM_POSITIONZ];
    const double w1=parameters[PARAM_WIDTH1];
    const double w2=parameters[PARAM_WIDTH2];
    const double w3=parameters[PARAM_WIDTH3];
    const double theta=parameters[PARAM_THETA]/180.0*M_PI;
    const double phi=parameters[PARAM_PHI]/180.0*M_PI;
    const double alpha=parameters[PARAM_ALPHA]/180.0*M_PI;

    double er[3];
    double etheta[3];
    double ephi[3];

    linalgSphericalCoordinatesUnitVecPhi(ephi, theta, phi);
    linalgSphericalCoordinatesUnitVecTheta(etheta, theta, phi);
    linalgSphericalCoordinatesUnitVecR(er, theta, phi);

    linalgRotateVector3D(ephi, alpha, er);
    linalgRotateVector3D(etheta, alpha, er);

    const double xx[3]= {x-x0, y-y0, z-z0};

    return offset+amplitude*exp(-2.0*qfSqr(linalgDotProduct(xx,etheta,3))/qfSqr(w1)-2.0*qfSqr(linalgDotProduct(xx,ephi,3))/qfSqr(w2)-2.0*qfSqr(linalgDotProduct(xx,er,3))/qfSqr(w3));

}
