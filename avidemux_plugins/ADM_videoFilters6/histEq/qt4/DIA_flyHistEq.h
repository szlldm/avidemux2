/***************************************************************************
  \fn     DIA_flyHistEq
  \author (C) 2004/2012 by mean/fixounet@free.fr
  \brief  Ui for histEq
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef FLY_HISTEQ_H
#define FLY_HISTEQ_H
#include "histEq.h"
/**
    \class flyHistEq
*/
class flyHistEq : public ADM_flyDialogYuv
{

  public:
    histEq  param;
  protected:
     ADMImage    *work;
  public:
    uint8_t    processYuv(ADMImage* in, ADMImage *out);
    uint8_t    download(void);
    uint8_t    upload(void);
    uint8_t    update(void);
    void       setTabOrder(void);
               flyHistEq (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider);
    virtual    ~flyHistEq() ;
};
#endif
