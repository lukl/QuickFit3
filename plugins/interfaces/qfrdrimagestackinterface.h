#ifndef QFRDRIMAGESTACKINTERFACE_H
#define QFRDRIMAGESTACKINTERFACE_H




#include <QtPlugin>
#include <stdint.h>
#include <QString>
#include <QList>

/*! \brief represents a common data interface for access to image stacks (1D, 2D, 3D, 4D)
    \ingroup qf3rdrdp_imaging_fcs

    The functions in this interface give access to a 4D image stack of type double. The
    axes are named:
      - x [0..width-1]
      - y [0..height-1]
      - time t [0..frames-1]
      - channel [0..channels-1]
    .
    Each of the axes may be transformed into a real coordinate value by multiplying with a given
    factor. The name of the resultant unit can also be extracted from this interface.


    The data is stored in row-major format, so the layout lookslike this:
    \verbatim
+-------------------------------------+           ****
| row 1               [0 ... width]   | I         *
| row 2               [0 ... width]   | M         * C
|  .                                  | A         * H
|  .                                  | G         * A
|  .                                  | E         * N
| row height-1        [0 ... width]   | 0         * N
+-------------------------------------+           * E
| row 1               [0 ... width]   | I         * L
| row 2               [0 ... width]   | M         *
|  .                                  | A         * 0
|  .                                  | G         *
|  .                                  | E         *
| row height-1        [0 ... width]   | 1         *
+-------------------------------------+           *
.                                     .           *
.                                     .           *
.                                     .           *
+-------------------------------------+           *
| row 1               [0 ... width-1] | I         *
| row 2               [0 ... width-1] | M         *
|  .                                  | A         *
|  .                                  | G         *
|  .                                  | E         *
| row height-1        [0 ... width-1] | frames-1  *
+-------------------------------------+           ****

+-------------------------------------+           ****
| row 1               [0 ... width-1] | I         *
| row 2               [0 ... width-1] | M         * C
|  .                                  | A         * H
|  .                                  | G         * A
|  .                                  | E         * N
| row height-1        [0 ... width-1] | 0         * N
+-------------------------------------+           * E
| row 1               [0 ... width-1] | I         * L
| row 2               [0 ... width-1] | M         *
|  .                                  | A         * 1
|  .                                  | G         *
|  .                                  | E         *
| row height-1        [0 ... width-1] | 1         *
+-------------------------------------+           *
.                                     .           *
.                                     .           *
.                                     .           *
+-------------------------------------+           *
| row 1               [0 ... width-1] | I         *
| row 2               [0 ... width-1] | M         *
|  .                                  | A         *
|  .                                  | G         *
|  .                                  | E         *
| row height-1        [0 ... width-1] | frames-1  *
+-------------------------------------+           ****

+-------------------------------------+           ****
.                                     .           .
.                                     .           .
.                                     .           .
.                                     .           .
    \endverbatim

*/
class QFRDRImageStackInterface {
    public:
        virtual ~QFRDRImageStackInterface() {}

        /** \brief returns the number of image stacks available through this interface */
        virtual int getImageStackCount() const =0;

        /** \brief returns the number of available preview images */
        virtual uint32_t getImageStackFrames(int stack) const =0;
        /** \brief returns the width of the given image */
        virtual int getImageStackWidth(int stack) const=0;
        /** \brief returns the height of the given image */
        virtual int getImageStackHeight(int stack) const=0;
        /** \brief returns the height of the given image */
        virtual int getImageStackChannels(int stack) const=0;
        /** \brief returns a pointer to the given frame in the video */
        virtual double* getImageStack(int stack, uint32_t frame=0, uint32_t channel=0) const=0;

        /** \brief returns a factor that scales the image stack x-dimension [0..width-1] to real coordinates */
        virtual double getImageStackXUnitFactor(int stack) const=0;
        /** \brief returns the name of the image stack x-dimension */
        virtual QString getImageStackXUnitName(int stack) const=0;

        /** \brief returns a factor that scales the image stack y-dimension [0..height-1] to real coordinates */
        virtual double getImageStackYUnitFactor(int stack) const=0;
        /** \brief returns the name of the image stack y-dimension */
        virtual QString getImageStackYUnitName(int stack) const=0;

        /** \brief returns a factor that scales the image stack t-dimension [0..frames-1] to real coordinates */
        virtual double getImageStackTUnitFactor(int stack) const=0;
        /** \brief returns the name of the image stack t-dimension */
        virtual QString getImageStackTUnitName(int stack) const=0;

        /** \brief returns a factor that scales the image stack channel-dimension [0..channels-1] to real coordinates */
        virtual double getImageStackCUnitFactor(int stack) const=0;
        /** \brief returns the name of the image stack channel-dimension */
        virtual QString getImageStackCUnitName(int stack) const=0;


        /** \brief returns a description for the video */
        virtual QString getImageStackDescription(int stack) const=0;

};

Q_DECLARE_INTERFACE( QFRDRImageStackInterface,
                     "www.dkfz.de.b040.quickfit3.fcsplugin.QFRDRImageStackInterface/1.0")

#endif // QFRDRIMAGESTACKINTERFACE_H