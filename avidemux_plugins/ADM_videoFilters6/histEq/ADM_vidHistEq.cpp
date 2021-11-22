/***************************************************************************
                          Histogram equalization filter 
    Copyright 2021 szlldm
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#define _USE_MATH_DEFINES // some compilers do not export M_PI etc.. if GNU_SOURCE or that is defined, let's do that
#include <cmath>
#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_coreVideoFilterInternal.h"
#include "DIA_factory.h"
#include "histEq.h"
#include "histEq_desc.cpp"
#include "ADM_vidHistEq.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern uint8_t DIA_getHistEq(histEq *param, ADM_coreVideoFilter *in);


// Add the hook to make it valid plugin
//DECLARE_VIDEO_FILTER(   ADMVideoHistEq,   // Class
DECLARE_VIDEO_FILTER_PARTIALIZABLE(   ADMVideoHistEq,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_COLORS,            // Category
                                      "histEq",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("histEq","Histogram equalization"),            // Display name
                                      QT_TRANSLATE_NOOP("histEq","Contrast enhancement.") // Description
                                  );
/**
    \fn HistEqProcess_Sqrti
*/
float ADMVideoHistEq::HistEqProcess_Sqrti( int n )
{
    int p = 0;
    int q = 1;
    int r = n;
    int h = 0;

    while( q <= n )
        q = q << 2;

    while( q != 1 )
    {
        q = q >> 2;
        h = p + q;
        p = p >> 1;
        if ( r >= h )
        {
            p = p + q;
            r = r - h;
        }
    }

    return (float)p;
}
/**
    \fn HistEqProcess_C
*/
void ADMVideoHistEq::HistEqProcess_C(ADMImage *img, ADMImage *tmp, float intensity, bool chroma)
{
    if (!img || !tmp) return;
    int width=img->GetWidth(PLANAR_Y); 
    int height=img->GetHeight(PLANAR_Y);
    int stride, istride;
    uint8_t * ptr, * iptr;


    uint32_t hist[256];
    for (int i=0; i<256; i++)
        hist[i]=0;

    istride=img->GetPitch(PLANAR_Y);
    iptr=img->GetWritePtr(PLANAR_Y);
    for(int y=0;y<height;y++)
    {
        ptr = iptr + y*istride;
        for (int x=0;x<width;x++)
        {
            hist[*ptr++]++;
        }
    }
    
    double fhist[256];
    for (int i=0; i<256; i++)
    {
        fhist[i] = hist[i];
        fhist[i] /= width;
        fhist[i] /= height;
    }
    
    uint8_t ylut[256];
    double cum = 0.0;
    double ins;
    if(img->_range != ADM_COL_RANGE_MPEG)
    {
        for (int i=0; i<256; i++)
        {
            cum += fhist[i]*255.0;
            ins = (255-i)/255.0;
            ylut[i] = std::floor(cum*ins*intensity + (1-ins*intensity)*i);
        }
    }
    else
    {
        for (int i=0; i<16; i++)
        {
            fhist[16] += fhist[i];
            fhist[i] = 0.0;
        }
        for (int i=236; i<256; i++)
        {
            fhist[235] += fhist[i];
            fhist[i] = 0.0;
        }
        for (int i=16; i<=235; i++)
        {
            cum += fhist[i]*219.0;
            ins = (235-i)/219.0;
            ylut[i] = (std::floor(cum*ins*intensity + (1-ins*intensity)*(i-16)))+16;
        }
        for (int i=0; i<16; i++)
            ylut[i] = ylut[16];
        for (int i=236; i<256; i++)
            ylut[i] = ylut[235];
    }

    istride=img->GetPitch(PLANAR_Y);
    iptr=img->GetWritePtr(PLANAR_Y);
    for(int y=0;y<height;y++)
    {
        ptr = iptr + y*istride;
        for (int x=0;x<width;x++)
        {
            *ptr = ylut[*ptr];
            ptr++;
        }
    }
	return;

/*    tmp->copyPlane(img,tmp,PLANAR_Y);

    // Y plane
    stride=tmp->GetPitch(PLANAR_Y);
    ptr=tmp->GetWritePtr(PLANAR_Y);
    istride=img->GetPitch(PLANAR_Y);
    iptr=img->GetWritePtr(PLANAR_Y);
    for(int y=0;y<height;y++)
    {
       for (int x=0;x<width;x++)
        {
            iptr[x] = sum;
        }
        iptr+=istride;
    }

    int pixel;
    int color_scale;
    color_scale = (1<<8);
    // UV planes
   for (int p=1; p<3; p++)
    {
        istride=img->GetPitch((ADM_PLANE)p);
        iptr=img->GetWritePtr((ADM_PLANE)p);
        for(int y=0;y<height/2;y++)	// 4:2:0
        {
            for (int x=0;x<width/2;x++)
            {
                pixel = iptr[x];
                pixel -= 128;
                iptr[x] = (uint8_t)(((pixel*color_scale)>>8) + 128);
            }
            iptr+=istride;
        }
    }*/

}

/**
    \fn configure
*/
bool ADMVideoHistEq::configure()
{
    uint8_t r=0;

    r=  DIA_getHistEq(&_param, previousFilter);
    if(r) update();
    return r;
}
/**
    \fn getConfiguration
*/
const char   *ADMVideoHistEq::getConfiguration(void)
{
    static char s[256];
    snprintf(s,255,"Intensity:%.2f%s",_param.intensity, (_param.chroma ? ", Process chroma":""));
    return s;
}
/**
    \fn ctor
*/
ADMVideoHistEq::ADMVideoHistEq(  ADM_coreVideoFilter *in,CONFcouple *couples)  :ADM_coreVideoFilter(in,couples)
{
    if(!couples || !ADM_paramLoad(couples,histEq_param,&_param))
        reset(&_param);
    work=new ADMImageDefault(info.width,info.height);
    update();
}
/**
    \fn reset
*/
void ADMVideoHistEq::reset(histEq *cfg)
{
    cfg->intensity = 0.0;
    cfg->chroma = true;
}
/**
    \fn valueLimit
*/
float ADMVideoHistEq::valueLimit(float val, float min, float max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn valueLimit
*/
int32_t ADMVideoHistEq::valueLimit(int32_t val, int32_t min, int32_t max)
{
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
}
/**
    \fn update
*/
void ADMVideoHistEq::update(void)
{
    _intensity=valueLimit(_param.intensity,0.0,1.0);
    _chroma=_param.chroma;
}
/**
    \fn dtor
*/
ADMVideoHistEq::~ADMVideoHistEq()
{
    if(work) delete work;
    work=NULL;
}
/**
    \fn getCoupledConf
*/
bool ADMVideoHistEq::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, histEq_param,&_param);
}

void ADMVideoHistEq::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, histEq_param, &_param);
}

/**
    
*/

/**
    \fn getNextFrame
    \brief
*/
bool ADMVideoHistEq::getNextFrame(uint32_t *fn,ADMImage *image)
{
    /*
    ADMImage *src;
    src=vidCache->getImage(nextFrame);
    if(!src)
        return false; // EOF
    *fn=nextFrame++;
    image->copyInfo(src);
    image->copyPlane(src,image,PLANAR_Y); // Luma is untouched
    src = image;

    DoFilter(...);

    vidCache->unlockAll();
    */
    if(!previousFilter->getNextFrame(fn,image)) return false;

    HistEqProcess_C(image, work,_intensity, _chroma);

    return 1;
}

