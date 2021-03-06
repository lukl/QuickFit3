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

#include "qfimporterimageseries.h"
#include <cstdlib>
#include <cmath>
#include<QDebug>
#include "qftools.h"
QFImporterImageSeries::QFImporterImageSeries() {
    binning=1;
    x0=0;
    x1=0;
    y0=0;
    y1=0;
    crop=false;
    interleavedBinning=false;
    averageBinning=false;
}

uint32_t QFImporterImageSeries::frameWidth() {
    if (binning<1) binning=1;
    int xx0=x0;
    int xx1=x1;
    if (!crop) {
        xx0=0;
        xx1=intFrameWidth()-1;
    }
    if (xx0<0) xx0=0;
    if (xx0>(int64_t)intFrameWidth()-1) xx0=intFrameWidth()-1;
    if (xx1<0) xx1=0;
    if (xx1>(int64_t)intFrameWidth()-1) xx1=intFrameWidth()-1;

    int w=abs(xx1-xx0)+1;
    if (!interleavedBinning) {
        return w/binning;
    } else {
        if (binning>1) return w-(binning-1);
    }
    return w;
}

uint32_t QFImporterImageSeries::frameHeight() {
    if (binning<1) binning=1;
    int yy0=y0;
    int yy1=y1;
    if (!crop) {
        yy0=0;
        yy1=intFrameHeight()-1;
    }
    if (yy0<0) yy0=0;
    if (yy0>(int64_t)intFrameHeight()-1) yy0=intFrameHeight()-1;
    if (yy1<0) yy1=0;
    if (yy1>(int64_t)intFrameHeight()-1) yy1=intFrameHeight()-1;

    int h=abs(yy1-yy0)+1;
    if (!interleavedBinning) {
        return h/binning;
    } else {
        if (binning>1) return h-(binning-1);
    }
    return h;
}

uint32_t QFImporterImageSeries::frameChannels()
{
    return intFrameChannels();
}

bool QFImporterImageSeries::readFrameFloat(float* data, int channel) {
    if (!crop && binning==1) return intReadFrameFloat(data, channel);
    int fw=intFrameWidth();
    int fh=intFrameHeight();
    QVector<float> d(fw*fh);
    QVector<float> d1(fw*fh);
    bool OK=intReadFrameFloat(d.data(), channel);
    if (!OK) {
        return false;
    }

    int xx0=x0;
    int xx1=x1;
    if (!crop) {
        xx0=0;
        xx1=intFrameWidth()-1;
    }
    if (xx0<0) xx0=0;
    if (xx0>(int64_t)intFrameWidth()-1) xx0=intFrameWidth()-1;
    if (xx1<0) xx1=0;
    if (xx1>(int64_t)intFrameWidth()-1) xx1=intFrameWidth()-1;
    if (xx0>xx1) qSwap(xx0, xx1);

    int yy0=y0;
    int yy1=y1;
    if (!crop) {
        yy0=0;
        yy1=intFrameHeight()-1;
    }
    if (yy0<0) yy0=0;
    if (yy0>(int64_t)intFrameHeight()-1) yy0=intFrameHeight()-1;
    if (yy1<0) yy1=0;
    if (yy1>(int64_t)intFrameHeight()-1) yy1=intFrameHeight()-1;
    if (yy0>yy1) qSwap(yy0, yy1);

    uint32_t w=frameWidth();
    uint32_t h=frameHeight();
    int hh=abs(yy1-yy0)+1;
    int ww=abs(xx1-xx0)+1;

    for (register int y=0; y<hh; y++) {
        for (register int x=0; x<ww; x++) {
            d1[y*ww+x]=d[(yy0+y)*fw+xx0+x];
        }
    }

    for (register uint64_t i=0; i<w*h; i++) data[i]=0;
    float binAvg=1.0;
    if (averageBinning) binAvg=binning*binning;
    if (!interleavedBinning) {
        for (register int64_t y=0; y<(int64_t)h*binning; y++) {
            for (register int64_t x=0; x<(int64_t)w*binning; x++) {
                const int idx=(y/binning)*w+(x/binning);
                data[idx]=data[idx]+d1[y*ww+x]/binAvg;
            }
        }
    } else {
        for (register int64_t y=0; y<(int64_t)h; y++) {
            for (register int64_t x=0; x<(int64_t)w; x++) {
                int idx=y*w+x;
                data[idx]=0;
                for (int bx=0; bx<binning; bx++) {
                    for (int by=0; by<binning; by++) {
                        data[idx]=data[idx]+d1[(y+by)*ww+x+bx]/binAvg;
                    }
                }
            }
        }
    }

    return true;
}

bool QFImporterImageSeries::readFrameDouble(double *data, int channel) {
    if (!crop && binning==1) return intReadFrameDouble(data, channel);
    int fw=intFrameWidth();
    int fh=intFrameHeight();
    QVector<double> d(fw*fh);
    QVector<double> d1(fw*fh);
    //double* d=(double*)qfMalloc(fw*fh*sizeof(double));
    //double* d1=(double*)qfMalloc(fw*fh*sizeof(double));
    bool OK=intReadFrameDouble(d.data(), channel);
    if (!OK) {
        return false;
    }

    int xx0=x0;
    int xx1=x1;
    if (!crop) {
        xx0=0;
        xx1=intFrameWidth()-1;
    }
    if (xx0<0) xx0=0;
    if (xx0>(int64_t)intFrameWidth()-1) xx0=intFrameWidth()-1;
    if (xx1<0) xx1=0;
    if (xx1>(int64_t)intFrameWidth()-1) xx1=intFrameWidth()-1;
    if (xx0>xx1) qSwap(xx0, xx1);

    int yy0=y0;
    int yy1=y1;
    if (!crop) {
        yy0=0;
        yy1=intFrameHeight()-1;
    }
    if (yy0<0) yy0=0;
    if (yy0>(int64_t)intFrameHeight()-1) yy0=intFrameHeight()-1;
    if (yy1<0) yy1=0;
    if (yy1>(int64_t)intFrameHeight()-1) yy1=intFrameHeight()-1;
    if (yy0>yy1) qSwap(yy0, yy1);

    uint32_t w=frameWidth();
    uint32_t h=frameHeight();
    int hh=abs(yy1-yy0)+1;
    int ww=abs(xx1-xx0)+1;

    for (register int y=0; y<hh; y++) {
        for (register int x=0; x<ww; x++) {
            d1[y*ww+x]=d[(yy0+y)*fw+xx0+x];
        }
    }

    for (register uint64_t i=0; i<w*h; i++) data[i]=0;
    double binAvg=1.0;
    if (averageBinning) binAvg=binning*binning;

    if (!interleavedBinning) {
        for (register int64_t y=0; y<(int64_t)h*binning; y++) {
            for (register int64_t x=0; x<(int64_t)w*binning; x++) {
                data[(y/binning)*w+(x/binning)]=data[(y/binning)*w+(x/binning)]+d1[y*ww+x]/binAvg;
            }
        }
    } else {
        for (register int64_t y=0; y<(int64_t)h; y++) {
            for (register int64_t x=0; x<(int64_t)w; x++) {
                int64_t idx=y*w+x;
                data[idx]=0;
                for (int bx=0; bx<binning; bx++) {
                    for (int by=0; by<binning; by++) {
                        data[idx]=data[idx]+d1[(y+by)*ww+x+bx]/binAvg;
                    }
                }
            }
        }
    }

   return true;
}

bool QFImporterImageSeries::readFrameUINT16(uint16_t* data, int channel) {
    if (!crop && binning==1) return intReadFrameUINT16(data, channel);
    int fw=intFrameWidth();
    int fh=intFrameHeight();
    QVector<uint16_t> d(fw*fh);
    QVector<uint16_t> d1(fw*fh);
    bool OK=intReadFrameUINT16(d.data(), channel);
    if (!OK) {
        return false;
    }

    int xx0=x0;
    int xx1=x1;
    if (!crop) {
        xx0=0;
        xx1=intFrameWidth()-1;
    }
    if (xx0<0) xx0=0;
    if (xx0>(int64_t)intFrameWidth()-1) xx0=intFrameWidth()-1;
    if (xx1<0) xx1=0;
    if (xx1>(int64_t)intFrameWidth()-1) xx1=intFrameWidth()-1;
    if (xx0>xx1) qSwap(xx0, xx1);

    int yy0=y0;
    int yy1=y1;
    if (!crop) {
        yy0=0;
        yy1=intFrameHeight()-1;
    }
    if (yy0<0) yy0=0;
    if (yy0>(int64_t)intFrameHeight()-1) yy0=intFrameHeight()-1;
    if (yy1<0) yy1=0;
    if (yy1>(int64_t)intFrameHeight()-1) yy1=intFrameHeight()-1;
    if (yy0>yy1) qSwap(yy0, yy1);

    uint32_t w=frameWidth();
    uint32_t h=frameHeight();
    int hh=abs(yy1-yy0)+1;
    int ww=abs(xx1-xx0)+1;

    for (register int y=0; y<hh; y++) {
        for (register int x=0; x<ww; x++) {
            d1[y*ww+x]=d[(yy0+y)*fw+xx0+x];
        }
    }

    for (register uint64_t i=0; i<w*h; i++) data[i]=0;
    uint32_t binAvg=1.0;
    if (averageBinning) binAvg=binning*binning;

    if (!interleavedBinning) {
        for (register int64_t y=0; y<(int64_t)h*binning; y++) {
            for (register int64_t x=0; x<(int64_t)w*binning; x++) {
                data[(y/binning)*w+(x/binning)]=data[(y/binning)*w+(x/binning)]+d1[y*ww+x]/binAvg;
            }
        }
    } else {
        for (register int64_t y=0; y<(int64_t)h; y++) {
            for (register int64_t x=0; x<(int64_t)w; x++) {
                int idx=y*w+x;
                data[idx]=0;
                for (int bx=0; bx<binning; bx++) {
                    for (int by=0; by<binning; by++) {
                        data[idx]=data[idx]+d1[(y+by)*ww+x+bx]/binAvg;
                    }
                }
            }
        }
    }


    return true;
}

QVariant QFImporterImageSeries::getFileProperty(const QString &name, const QVariant &defaultValue) const
{
    return fileinfo.properties.value(name, defaultValue);
    //return defaultValue;
}

void QFImporterImageSeries::setCropping(int x0, int x1, int y0, int y1) {
    this->x0=x0;
    this->x1=x1;
    this->y0=y0;
    this->y1=y1;
    crop=true;
    //qDebug()<<"enabled cropping   x = "<<x0<<"..."<<x1<<"   y = "<<y0<<"..."<<y1;
}

uint32_t QFImporterImageSeries::intFrameChannels()
{
    return 1;
}
