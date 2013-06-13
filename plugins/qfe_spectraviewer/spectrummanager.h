#ifndef SPECTRUMMANAGER_H
#define SPECTRUMMANAGER_H

#include <cmath>
#include <iostream>
#include <string>
#include <map>
#include <exception>
#include <gsl/gsl_math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_interp.h>
#include <cstdlib>
#include <stdexcept>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <QVector>
#include <QString>
#include <QMap>
#include "tools.h"
#include "jkiniparser2.h"


/** \brief this class is used to manage fluorophor properties and spectra
 *  \ingroup diff4_tools
 *
 * There is also a system in this class that allows to lookup a spectral prefactor [0..1] in a known set of spectra.
 * The known spectra are loaded in the constructor and more may be added by calling add_spectrum(). The lookup
 * is implemented in get_spectral_efficiency(). It is sufficient to give some points of the spectrum, as
 * get_spectral_efficiency() interpolates between them. Spectra should be given in \c *.spec files that contain
 * two columns of data: The first is the wavelength in [nm] and the second is the absorption efficiency [0..1]
 * where 1 is maximum absorptiona and 0 is no absorption. The highest value in the spectrum should be 1.
 */

class SpectrumManager {
    public:
        /** \brief this struct contains all data neede to describe a spectrum */
        struct Spectrum {
            Spectrum();
            /** \brief the file the absorption spectrum was loaded from */
            QString filename;
            /** \brief number of spectrum inside file */
            int spectrumIDInFile;
            /** \brief if \c true the CSV file contains one wavelength column, otherwise the layoutout is wavelength, spectrum, wavelength, spectrum, ... */
            bool separateWavelengthsInFile;
            /** \brief the wavelengths from the file [nanometers] */
            double* wavelength;
            /** \brief the absorption efficiencies from the file [0..1] */
            double* spectrum;
            /** \brief the fluorescence from the file [0..1] */
            int N;
            /** \brief GSL lookup accelerator for absorbance */
            gsl_interp_accel *accel;
            /** \brief this stores the spline calculated from the data in the file for absorbance */
            gsl_spline *spline;
            /** \brief reference/source for absorption spectrum */
            QString reference;

            void init(const QVector<double>& wavelength, const QVector<double>& spectrum);
            void init(const QString& filename, int ID=0, bool separateWavelengths=false);
            void clear();
            double getSpectrumAt(double wavelength) const;
            double getSpectrumIntegral(double lambda_start, double lambda_end) const;
        };

        /** \brief specifies the properties of one fluorophor */
        struct FluorophorData {
            FluorophorData();
            QString name;
            QString description;
            QString manufacturer;
            /** \brief the fluorescence efficiency of the fluorophor [0..1] */
            double fluorescence_efficiency;
            /** \brief fluorescence lifetime [sec] */
            double fluorescence_lifetime;
            /** \brief absorption cross section [meters^2] */
            double absorption_crosssection;
            /** \brief wavelength at which the absorption cross section is given [nanometers] */
            double absorption_crosssection_wavelength;
            /** \brief absorption specturm */
            int spectrum_abs;
            /** \brief fluorophor */
            int spectrum_fl;
            /** \brief reference/source of data */
            QString reference;
            /** \brief conditions of measurements for the above given properties */
            QString condition;
            QString inifile;
        };

        /** \brief specifies the properties of a lightsource */
        struct LightSourceData {
            LightSourceData();
            QString name;
            QString description;
            QString manufacturer;
            QString orderNo;
            /** \brief absorption specturm */
            int spectrum;
            /** \brief reference/source of data */
            QString reference;
            /** \brief conditions of measurements for the above given properties */
            QString condition;
            QString inifile;
        };

        /** \brief specifies the properties of a filter */
        struct FilterData {
            FilterData();
            QString name;
            QString description;
            QString manufacturer;
            QString orderNo;
            /** \brief absorption specturm */
            int spectrum;
            /** \brief reference/source of data */
            QString reference;
            /** \brief conditions of measurements for the above given properties */
            QString condition;
            QString inifile;
        };

    protected:
        /** \brief specifies the type of interpolation for the spectra */
        const gsl_interp_type *spectral_interpolation_type;


        /** \brief load all \c *.spec file from the current directory */
        void init_spectra();

        QList<Spectrum> spectra;

        /** \brief fluorophor database */
        QMap<QString, FluorophorData> fluorophores;
        /** \brief fluorophor database */
        QMap<QString, LightSourceData> lightsources;
        /** \brief fluorophor database */
        QMap<QString, FilterData> filters;

        /** \brief parse the \c fluorophors.ini file and fill fluorophor_database
         *
         * The \c fluorophors.ini file contains a list of the fluorescence parameters of the different fluorophors.
         * Each entry is characterized by a name. Note that if you also have a spectrum for the dye the name of the
         * spectrum file has to be \c name.spec where \c name is the same name as in \c fluorophors.ini.
         *
         * Each entry has this form:
         * \verbatim
[Rho6G]
q_fluor=0.94
tau_fluor=3.9e-9
sigma_abs=3.6e-20 \endverbatim
         * Here \ Rho6G is the name and the spectrum is saved as \c rho6g.spec (case is ignored!). The property q_fluor
         * give the fluorescence efficiency (i.e. \f$ \frac{\mbox{fluorescence photons}}{\mbox{absorbed photons}} \f$) in
         * the range [0..1], \c tau_fluor gives the fluorescence lifetime in seconds and \c sigma_abs (\f$ \sigma_{abs} \f$)
         * the absorption cross  section in \f$ \mbox{meters}^2 \f$. Instead of \c sigma_abs you can also provide
         * c molar_extinction which is the molar extinction coefficient \f$ \epsilon_{mol} \f$ in 1/(cm*Molar)=liters/(mol*cm).
         * These two properties are connected by \f[ \sigma_{abs}=\frac{\epsilon_{abs}}{N_A}=\frac{\epsilon_{abs}[l/(\mbox{cm}\cdot\mbox{mol})]\cdot 10^{-1}}{6.022\cdot10^{23}}[\mbox{meters}^2] \f]
         */
        void init_fluorophor_database();

    private:

    public:
        /** \brief class constructor, initialises the database from the provided path. */
        SpectrumManager();

        /** \brief class destructor */
        ~SpectrumManager();

        /** \brief get number of spectra */
        int getSpectraCount() const;
        /** \brief get number of fluorophores */
        int getFluorophoreCount() const;
        /** \brief get number of light sources */
        int getLightSourceCount() const;
        /** \brief get number of filters */
        int getFilterCount() const;
        /** \brief get number of fluorophores */
        QStringList getFluorophores() const;
        /** \brief get number of light sources */
        QStringList getLightSources() const;
        /** \brief get number of filters */
        QStringList getFilters() const;

        void clear();

        void loadFluorophoreDatabase(const QString& ininame, const QStringList& directories=QStringList() );


        /** \brief add a spectrum file to the internal database */
        int addSpectrum(const QString &filename, int ID=0, bool separateWavelengths=false);
        /** \brief return the i-th spectrum */
        Spectrum& getSpectrum(int i) const;
        /** \brief return the i-th spectrum */
        const Spectrum& getSpectrum(int i) const;


        /** \brief returns \c true if fluorophor exists in database */
        bool fluorophoreExists(const QString& f);
        /** \brief returns \c true if fluorophor exists in database */
        bool filterExists(const QString& f);
        /** \brief returns \c true if fluorophor exists in database */
        bool lightsourceExists(const QString& f);

        FluorophorData& getFluorophoreData(const QString& name);
        LightSourceData& getLightSourceData(const QString& name);
        FilterData& getFilterData(const QString& name);
        const FluorophorData& getFluorophoreData(const QString& name);
        const LightSourceData& getLightSourceData(const QString& name);
        const FilterData& getFilterData(const QString& name);

};
#endif // SPECTRUMMANAGER_H
