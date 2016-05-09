/*
    Copyright (c) 2016 Jan Buchholz (<jan.buchholz@dkfz.de>), German Cancer Research Center (DKFZ)



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

#ifndef QFIMAGEREADERZITI_H
#define QFIMAGEREADERZITI_H

#include <stdint.h>
#include <QString>
#include <QMessageBox>
#include "qfpluginimporters.h"
#include "qfimporter.h"
#include "qfimporterimageseries.h"
#include "qftools.h"
#include "qfdlgcsvparameters.h"
#include "programoptions.h"

#include <iostream>     // std::cout
#include <fstream>      // std::ifstream


/*! \brief QFImporter class for SPAD array by ZITI files.
    \ingroup qf3importerplugins_importers_basicimages

*/

class QFImageReaderZITI : public QFImporterImageSeries
{
public:
    inline QFImageReaderZITI() {
        this->ifs=NULL;
        this->width=88;
        this->channels=1;
        this->height=88;
        this->frame=-1;
        this->frames=0;
        this->frameData=NULL;
    }

    inline virtual ~QFImageReaderZITI() {

    }

    /*! \copydoc QFFitFunction::filter()   */
    inline virtual QString filter() const  {
        return formatName()+QObject::tr(" (*.dat *.bin)");
    }
    /*! \copydoc QFFitFunction::formatName()   */
    inline virtual QString formatName() const  {
        return QObject::tr("ZITI SPAD binary files");
    }

    /** \brief open the given image sequence file
     *  \param filename name of the image sequence file
     *  \return \c true on success
     */
    inline virtual bool open(QString filename) {
        bool ok=false;
        close();
        this->ifs= new std::ifstream(filename.toLatin1(),std::fstream::in | std::fstream::binary);
        if (ifs->is_open()) {
            this->frameData = new uint8_t[FRAMESIZE];
            uint8_t *header = new uint8_t[HEADERSIZE];
            ifs->seekg(0, ifs->end);
            unsigned long length = ifs->tellg();
            ifs->seekg(0, ifs->beg);
            ifs->read(reinterpret_cast<char*>(header),HEADERSIZE);
            length -= HEADERSIZE;
            frames = length/FRAMESIZE;
            if(length % FRAMESIZE != 0){

                std::cout << "Some bytes are missing. " << (length % FRAMESIZE) << std::endl;
            }
            //read first frame
            nextFrame();
            this->frame=0;
            ok=true;
        }
        return ok;
    }

    /** \brief close the currently opened image file */
    inline virtual void close() {
        if(this->ifs!=NULL){
           this->ifs->close();
        }
        if(this->frameData!=NULL){
           delete[] this->frameData;
        }
        this->ifs=NULL;
        this->frame=-1;
        this->frames=0;
    }

    /** \brief move the reading pointer back to the first frame */
    inline virtual void reset() {
        if(ifs!=NULL){
            ifs->seekg(HEADERSIZE, ifs->beg);
        }
    }

    /** \brief return the number of frames in the files.*/
    inline virtual uint32_t countFrames() {
        return frames;
    }

    /** \brief move on to the next frame in the file. return \c false if no further image exists */
    inline virtual bool nextFrame() {
        bool ok=false;
        if (ifs->is_open()) {
            ifs->read(reinterpret_cast<char*>(frameData),FRAMESIZE);
           this->frameTimestamp = (this->frameData[3] << 24) | (this->frameData[2] << 16) | (this->frameData[1] << 8) | this->frameData[0];
            //std::cout << "Timestamp: " << this->frameTimestamp << std::endl;
            ok=true;
            frame++;
        }
        return ok;
    }

protected:
    /** \brief return the width of the frames (valid after open() returned \c true */
    inline virtual uint32_t intFrameWidth() {
        return width;
    }

    /** \brief return the height of the frames (valid after open() returned \c true */
    inline virtual uint32_t intFrameHeight() {
        return height;
    }

    inline virtual uint32_t intRawDataFormat() {return QF_RDF_PACKED1B;}

    inline virtual uint32_t intFrameChannels() {
        return channels;
    }

    /** \brief read a new frame into the given array of floating point numbers */
    inline virtual bool intReadFrameFloat(float* data, int channel=0) {
        if (this->frames==0) return false;
        float *dst=data;
        uint8_t *src=&frameData[4];
        for (uint64_t i=0; i<(width*height)/8; i++) {
            uint8_t byte=*src;
            for(unsigned int j=0;j<8;j++){
                *dst=byte & 0x1;
                dst++;
                byte >>= 1;
            }
            src++;
        }
        return true;
    }

    /** \brief read a new frame into the given array of integers */
    inline virtual bool intReadFrameUINT16(uint16_t* data, int channel=0) {
        if (this->frames==0) return false;
        uint16_t *dst=data;
        uint8_t *src=&frameData[4];
        for (uint64_t i=0; i<(width*height)/8; i++) {
            uint8_t byte=*src;
            for(unsigned int j=0;j<8;j++){
                *dst=byte & 0x1;
                dst++;
                byte >>= 1;
            }
            src++;
        }
        return true;
    }
    inline virtual bool intReadFrameDouble(double* data, int channel=0) {
        if (this->frames==0) return false;
        double *dst=data;
        uint8_t *src=&frameData[4];
        for (uint64_t i=0; i<(width*height)/8; i++) {
            uint8_t byte=*src;
            for(unsigned int j=0;j<8;j++){
                *dst=byte & 0x1;
                dst++;
                byte >>= 1;
            }
            src++;
        }
        return true;
    }

    const int FRAMESIZE  = 968+4;
    const int HEADERSIZE =  76;


    std::ifstream *ifs;

    uint32_t width;
    uint32_t height;
    uint32_t channels;
    uint32_t frames;
    uint32_t frame;
    uint8_t *frameData;
    uint32_t frameTimestamp;
};

#endif // QFIMAGEREADERZITI_H

