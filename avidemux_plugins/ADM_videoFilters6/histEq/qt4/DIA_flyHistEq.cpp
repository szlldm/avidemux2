/**/
/***************************************************************************
                          DIA_HistEq
                             -------------------

    begin                : 08 Apr 2005
    copyright            : (C) 2004/7 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "DIA_flyDialogQt4.h"
#include "ADM_default.h"
#include "ADM_image.h"
#include "DIA_flyHistEq.h"

#include "ADM_vidHistEq.h"

/************* COMMON PART *********************/
/**
 * 
 * @param parent
 * @param width
 * @param height
 * @param in
 * @param canvas
 * @param slider
 */
flyHistEq::flyHistEq (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider) : ADM_flyDialogYuv(parent, width, height, in, canvas, slider, RESIZE_AUTO)
{
     work=  new ADMImageDefault(_w,_h);
}
/**
    \fn dtor
*/
flyHistEq::~flyHistEq()
{
    delete work;
    work=NULL;
}
/**
    \fn update
*/
uint8_t  flyHistEq::update(void)
{
    return 1;
}
/**
    \fn processYuv
*/
uint8_t   flyHistEq::processYuv(ADMImage *in,ADMImage *out )
{
    uint8_t *src,*dst;
    uint32_t stride;
    out->duplicate(in);

    // Do it!
    ADMVideoHistEq::HistEqProcess_C(out, work, param.intensity, param.chroma);
    return 1;
}

