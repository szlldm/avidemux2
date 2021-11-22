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
#pragma once

/**
    \class ADMVideoHistEq
*/
class  ADMVideoHistEq:public ADM_coreVideoFilter
{

  protected:
    void            update(void);
    histEq     _param;
    float           _intensity;
    bool            _chroma;
    ADMImage        *work;
  public:
    ADMVideoHistEq(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoHistEq();

    virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void         setCoupledConf(CONFcouple *couples);
    virtual bool         configure(void) ;                 /// Start graphical user interface

    static  void         HistEqProcess_C(ADMImage *img, ADMImage *tmp, float intensity, bool chroma);
    static  void         reset(histEq *cfg);

  private:
    static  float         HistEqProcess_Sqrti( int n );
    float   valueLimit(float val, float min, float max);
    int32_t valueLimit(int32_t val, int32_t min, int32_t max);
};


