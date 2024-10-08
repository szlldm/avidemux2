/***************************************************************************
    \file   ADM_coreDemuxerMpegTemplate.cpp
    \brief  Common part for mpeg demuxer, template like but using define
    \author (C) 2010 by mean  : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MY_CLASS
    #error  MY_CLASS not defined
#endif
#include "ADM_vidMisc.h"

/**
        \fn getVideoDuration
        \brief Returns duration of video in us
*/
uint64_t MY_CLASS::getVideoDuration(void)
{
    if(videoDuration != ADM_NO_PTS)
        return videoDuration;
    int lastFrame=ListOfFrames.size();
    if(!lastFrame) return 0;
    lastFrame--;
    int maxLookup=100;
    if(maxLookup>lastFrame) maxLookup=lastFrame;
    int start=lastFrame-maxLookup;

    uint64_t maxPts=0,maxDts=0;
    int      maxPtsIndex=-1,maxDtsIndex=-1;

    // Search for higher PTS in the last N frames
    // and note down its position..
    for(int i=start;i<=lastFrame;i++)
    {
            uint64_t p=ListOfFrames[i]->pts;
            if(p==ADM_NO_PTS) 
                continue;
            if(p>maxPts) 
            {
                maxPts=p;
                maxPtsIndex=i;
            }
    }
//#define VERBOSE_DURATION
#ifdef VERBOSE_DURATION
    ADM_info("Found maxPts =%s, %d frames from the end\n",ADM_us2plain(maxPts),lastFrame-maxPtsIndex);
#endif
    for(int i=lastFrame;i>=start;i--)
    {
            uint64_t p=ListOfFrames[i]->dts;
            if(p==ADM_NO_PTS) 
                continue;
            maxDtsIndex=i;
            maxDts=p;
            break;
    }
#ifdef VERBOSE_DURATION
    ADM_info("Found maxDts =%s, %d frames from the end\n",ADM_us2plain(maxDts),lastFrame-maxDtsIndex);
#endif
    // Case 1: No Pts
    bool usePts=true;
    if(maxPtsIndex==-1)
    {
        usePts=false;
    }
    uint64_t refTime;
    double refDistance;
    if(usePts==true)
    {
#ifdef VERBOSE_DURATION
        ADM_info("Using PTS..\n");
#endif
        refTime=maxPts;
        refDistance=lastFrame-maxPtsIndex;
    }else   
    {
#ifdef VERBOSE_DURATION
        ADM_info("Using DTS..\n");
#endif
        refTime=maxDts;
        refDistance=lastFrame-maxDtsIndex;    
    }
    double f,g;
    f=1000*1000*1000;
    f/=_videostream.dwRate; 
    g=refTime;
    f=f*refDistance;
    g+=f;
    videoDuration = (uint64_t)g;
    videoDuration += frameToUs(1);
#ifdef VERBOSE_DURATION
    ADM_info("Using duration of %s\n",ADM_us2plain(duration));
#endif
    return videoDuration;
}

/**
    \fn getAudioInfo
    \brief returns wav header for stream i (=0)
*/
WAVHeader *MY_CLASS::getAudioInfo(uint32_t i )
{
        if(!listOfAudioTracks.size()) return NULL;
      ADM_assert(i<listOfAudioTracks.size());
      return listOfAudioTracks[i]->stream->getInfo();
      
}

/**
   \fn getAudioStream
*/

uint8_t   MY_CLASS::getAudioStream(uint32_t i,ADM_audioStream  **audio)
{
    if(!listOfAudioTracks.size())
    {
            *audio=NULL;
            return true;
    }
  ADM_assert(i<listOfAudioTracks.size());
  *audio=listOfAudioTracks[i]->stream;
  return true; 
}

/**
    \fn getNbAudioStreams

*/
uint8_t   MY_CLASS::getNbAudioStreams(void)
{
 
  return listOfAudioTracks.size(); 
}

/**
    \fn setFlag
    \brief Returns timestamp in us of frame "frame" (PTS)
*/

uint8_t  MY_CLASS::setFlag(uint32_t frame,uint32_t flags)
{
    if(frame>=ListOfFrames.size())
        return 0;

    uint32_t f = 2;
    switch(flags & AVI_FRAME_TYPE_MASK)
    {
        case AVI_KEY_FRAME:
            f = 1;
            if (flags & AVI_IDR_FRAME)
                f = 4;
            break;
        case AVI_B_FRAME:
            f = 3;
            break;
        default: break;
    }
    ListOfFrames[frame]->type = f;
    ListOfFrames[frame]->pictureType = flags & AVI_STRUCTURE_TYPE_MASK;
    return 1;
}
/**
    \fn getFlags
    \brief Returns timestamp in us of frame "frame" (PTS)
*/

uint32_t MY_CLASS::getFlags(uint32_t frame,uint32_t *flags)
{
    if(frame>=ListOfFrames.size())
        return 0;

    switch(ListOfFrames[frame]->type)
    {
        case 1: *flags=AVI_KEY_FRAME;break;
        case 3: *flags=AVI_B_FRAME;break;
        case 4:
            *flags = (AVI_KEY_FRAME | AVI_IDR_FRAME);
            break;
        case 2:
        default:
            *flags = 0;
            break;
    }
    *flags += ListOfFrames[frame]->pictureType;
    return 1;
}

/**
    \fn getTime
    \brief Returns timestamp in us of frame "frame" (PTS)
*/
uint64_t MY_CLASS::getTime(uint32_t frame)
{
   if(frame>=ListOfFrames.size()) return 0;
    uint64_t pts=ListOfFrames[frame]->pts;
    return pts;
}

/**
    \fn timeConvert
    \brief FIXME
*/
uint64_t MY_CLASS::timeConvert(uint64_t x)
{
    if(x==ADM_NO_PTS) return ADM_NO_PTS;
    double f=x-ListOfFrames[0]->dts;
    f*=1000.;
    f/=90.;
    f+=0.49;
    return (uint64_t)f;
}


/**
      \fn getFrameSize
      \brief return the size of frame frame
*/
uint8_t MY_CLASS::getFrameSize (uint32_t frame, uint32_t * size)
{
    if(frame>=ListOfFrames.size()) return 0;
    *size=ListOfFrames[frame]->len;
    return true;
}

/**
    \fn getPtsDts
*/
bool    MY_CLASS::getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts)
{
    if(frame>=ListOfFrames.size()) return false;
    dmxFrame *pk=ListOfFrames[frame];

    *dts=pk->dts;
    *pts=pk->pts;
    return true;
}
/**
        \fn setPtsDts
*/
bool    MY_CLASS::setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts)
{
    if(frame>=ListOfFrames.size()) return false;
    videoDuration = ADM_NO_PTS; // invalidate
    dmxFrame *pk=ListOfFrames[frame];

    pk->dts=dts;
    pk->pts=pts;
    return true;


}

// EOF
