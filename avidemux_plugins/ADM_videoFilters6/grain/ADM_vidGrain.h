/***************************************************************************
                          Grain filter 
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
    \class ADMVideoGrain
*/
class  ADMVideoGrain:public ADM_coreVideoFilter
{

  protected:
    void            update(void);
    grain          _param;
    float           _noise;
  public:
    ADMVideoGrain(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoGrain();

    virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void         setCoupledConf(CONFcouple *couples);
    virtual bool         configure(void) ;                 /// Start graphical user interface

    static  void         GrainProcess_C(ADMImage *img, float noise);
    static  void         reset(grain *cfg);

  private:
    float valueLimit(float val, float min, float max);
};

