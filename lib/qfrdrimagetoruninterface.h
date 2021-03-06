/*
    Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center (DKFZ) & IWR, University of Heidelberg

    

    This file is part of QuickFit 3 (http://www.dkfz.de/Macromol/quickfit).

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

#ifndef QFRDRIMAGETORUNINTERFACE_H
#define QFRDRIMAGETORUNINTERFACE_H

#include "qfplugin.h"
#include <stdint.h>

/*! \brief represents a common data interface that allows to operate on image data (x,y) that is represented by an indexed list ("runs")
    \ingroup qf3rdrdp_imaging_fcs

*/
class QFRDRImageToRunInterface {
    public:
        virtual ~QFRDRImageToRunInterface() {}

        /** \brief return width of the image */
        virtual int getImageFromRunsWidth() const=0;
        /** \brief return height of the image */
        virtual int getImageFromRunsHeight() const=0;
        /** \brief return the number of color channels */
        virtual int getImageFromRunsChannels() const=0;
        /** \brief return the number of color channels that are advised to be displayed
         *
         *  This always returns a number \c >=0 and \c <=getImageFromRunsChannels() it is the number of preview images
         *  you should absolutely display, even if more are available. e.g.
         *    - For an imFCCS-record for the CCF getImageFromRunsChannels()=getImageFromRunsChannelsAdvised()=2 so the red and green channels are displayed
         *    - In the same case for the two ACFs getImageFromRunsChannels()=2 and getImageFromRunsChannelsAdvised()=1, as you may only want to see the single channel the ACF was calculated for
         *  .
         */
        virtual int getImageFromRunsChannelsAdvised() const=0;

        /*! \brief return a pointer to a double image of size getDataImageWidth() * getDataImageHeight()

            This image can be used to allow the user to select points in the image, or just as an overview. E.g. for
            imFCS-data this could simply be the averaged intensity of every pixel.

            \return  a double image of size getDataImageWidth() * getDataImageHeight(), or NULL if no image is available
         */
        virtual double* getImageFromRunsPreview(int channel=0) const=0;
        /** \brief convert a pixel coordinate to a run index */
        virtual int xyToRun(int x, int y) const=0;
        /** \brief convert a run to a pixel x-coordinate */
        virtual int runToX(int run) const=0;
        /** \brief convert a run to a pixel y-coordinate */
        virtual int runToY(int run) const=0;
        /** \brief convert a pixel coordinate to an array index (in correlations and sigmas) */
        virtual int xyToIndex(int x, int y) const=0;

};

Q_DECLARE_INTERFACE( QFRDRImageToRunInterface,
                     "www.dkfz.de.b040.quickfit3.fcsplugin.QFRDRImageToRunInterface/1.0")

#endif // QFRDRIMAGETORUNINTERFACE_H
