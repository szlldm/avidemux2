/***************************************************************************
  \fn ADM_pics
  \brief sequence of image demuxer
 The bmp/bmp2 code is pretty bad, but it's not worth doing something better

    copyright            : (C) 2002/2016 by mean
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

#include "ADM_default.h"

#include <string.h>
#include <math.h>

#include "ADM_Video.h"
#include "fourcc.h"
#include "ADM_pics.h"
#include "prefs.h"

#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

#define MAX_ACCEPTED_OPEN_FILE 0x7FFFF
/* protect against integer overflow */
#define MAX_DIGITS 8

#ifdef __APPLE__
 #define MAX_LEN 1024
#else
 #define MAX_LEN 4096
#endif

/**
 * 
 */
picHeader::picHeader(void)
{
    _nbFiles = 0;
    _duration = 0;
    _totalSize = 0;
    _bmpHeaderOffset=0;
    _tbNum = 1;
    _tbDen = 25;
    _reverseOrder = false;
}
/**
    \fn getTime
*/

uint64_t  picHeader::getTime(uint32_t frameNum)
{
    double f = _tbNum;
    f*=1000000;
    f*=frameNum;
    f /= _tbDen;
    f += 0.49;
    return (uint64_t)f;

}
/**
    \fn getFrameSize
*/
uint8_t  picHeader::getFrameSize(uint32_t frame,uint32_t *size)
{
    if(frame >= _nbFiles)
        return 0;
    *size= _imgSize[frame];
    return 1;
}
/**
    \fn getFrame
*/
uint8_t picHeader::getFrame(uint32_t framenum, ADMCompressedImage *img)
{
    if(framenum >= _nbFiles)
        return 0;
    FILE* fd = openFrameFile(framenum);
    if(!fd)
        return false;
    // skip bmp header
    if(_bmpHeaderOffset)
        fseek(fd,_bmpHeaderOffset,SEEK_SET);
    int l= _imgSize[framenum];
    int n=fread(img->data, l , 1, fd);
    
    int current=ftello(fd);
    fseek(fd,0,SEEK_END);
    int end=ftello(fd);
    aprintf("Current=%d end=%d delta=%d\n",current,end,end-current);
    
    if(n!=1)
    {
        ADM_error("Read incomplete \n");
    }
    
    fclose(fd);

    uint64_t timeP = getTime(framenum);
    img->dataLength = _imgSize[framenum];
    img->demuxerDts=timeP;
    img->demuxerPts=timeP;
    img->flags = AVI_KEY_FRAME;    
    return 1;
}
/**
 * 
 * @return 
 */
uint8_t picHeader::close(void)
{
	_nbFiles = 0;
        _imgSize.clear();
	return 0;
}


/**
 * \fn extractBmpAdditionalInfo
 * @param name
 * @param type
 * @param bpp
 * @param bmpHeaderOffset
 * @return 
 */
static bool extractBmpAdditionalInfo(const char *name,ADM_PICTURE_TYPE type,int &bpp,int &bmpHeaderOffset)
{
    FILE *fd=ADM_fopen(name,"rb");
    if(!fd) return false;
    
    bool r=true;
    uint16_t s16;
    BmpLowLevel low(fd);
    
    switch(type) // this is bad. All the offsets are hardcoded and could be actually different.
    {
        case ADM_PICTURE_BMP2:
            // 0 2 bytes BM
            // 2 4 Bytes file size
            // 6 4 bytes xxxx
            //10  4 bytes header size, = direct offset to data            
            {
                ADM_BITMAPINFOHEADER bmph;
                fseek(fd, 10, SEEK_SET);
                bmpHeaderOffset = low.read32LE();
                low.readBmphLE(bmph);
                if (bmph.biCompression != 0 && bmph.biCompression != 3) 
                {
                    ADM_warning("cannot handle compressed bmp 0x%x <%s>\n",bmph.biCompression,fourCC::tostring(bmph.biCompression));
                    r=false;
                    break;
                }
                bpp = bmph.biBitCount;
                if (bpp > 32)
                {
                    ADM_warning("Invalid bpp = %d\n",bpp);
                    r=false;
                    break;
                }
                if (bpp == 32 && bmph.biCompression == 3)
                { // read channel masks, FIXME: BGR is assumed
                    low.read32LE(); // red
                    low.read32LE(); // green
                    uint32_t bmask=low.read32LE(); // blue
                    uint32_t amask=low.read32LE(); // alpha
                    if((!amask && bmask == 0xff00) || amask == 0xff)
                        bpp=96; // xBGR
                }
                aprintf("Bmp bpp=%d offset: %d (bmp header=%d,%d)\n", bpp, bmpHeaderOffset,sizeof(bmph),bmph.biSize);
            }
            break;
        case ADM_PICTURE_BMP:
	{
	    ADM_BITMAPINFOHEADER bmph;

            if (!fread(&s16, 2, 1, fd))
            {
                ADM_warning("Cannot read bmp file.\n");
                r=false;
                break;
            }
	    if (s16 != 0x4D42) 
            {
		ADM_warning(" incorrect bmp sig.\n");
		r=false;
                break;  
	    }
            low.read32LE( );
            low.read32LE( );
            low.read32LE( );
            low.readBmphLE( bmph);
            if (bmph.biCompression != 0 && bmph.biCompression != 3 ) 
            {
		ADM_warning("cannot handle compressed bmp\n");
                r=false;
                break;
	    }
	    bmpHeaderOffset = bmph.biSize + 14;
            bpp = bmph.biBitCount;
	}
	break;

        default:
            ADM_assert(0);
            break;
    }
    
    fclose(fd);
    return r;
            
}
/**
 * 
 * @param inname
 * @param nbOfDigits
 * @param filePrefix
 */
static void splitImageSequence(std::string inname,int &nbOfDigits, int &first,std::string &filePrefix)
{   
    std::string workName=inname;   
    int num=0;
    first=0;
    int radix=1;
    while( workName.size() )
    {
        const char c=workName[workName.size()-1];
        if((c<'0') || (c>'9'))
            break;       
        if(num >= MAX_DIGITS) // else definitely false positive
            break;
        num++;
        first=first+radix*+(c-'0');
        radix*=10;
        workName.resize(workName.size()-1);
    }
    nbOfDigits=num;
    filePrefix=workName;
}
/**
 * \fn setTimeBaseFromIdx
 */
static void setTimeBaseFromIdx(uint32_t idx, uint32_t &num, uint32_t &den)
{
    typedef struct { uint32_t n; uint32_t d; } tbase_t;
    const tbase_t standardFps[] = {
        {1001,24000},
        {1000,25000},
        {1001,30000},
        {1000,30000},
        {1000,50000},
        {1001,60000},
        {1000,60000}
    };
    if(idx >= sizeof(standardFps)/sizeof(tbase_t))
        return;
    num = standardFps[idx].n;
    den = standardFps[idx].d;
}

/**
 * \fn open
 * @param inname
 * @return 
 */
uint8_t picHeader::open(const char *inname)
{
    if(strlen(inname)>=MAX_LEN)
    {
        ADM_warning("Path too long, aborting.\n");
        return 0;
    }

    FILE *fd;    
    int bpp = 0;
    
    // 1- identity the image type    
    _type=ADM_identifyImageFile(inname,&_w,&_h);
    if(_type==ADM_PICTURE_UNKNOWN)
    {
        ADM_warning("\n Cannot open that file!\n");
	return 0;
    }

    if(!prefs->get(PICTURES_LOAD_IN_REVERSE_ORDER,&_reverseOrder))
        _reverseOrder = false;
    uint32_t fpsIdx = 1; // 25 fps
    if(!prefs->get(PICTURES_FPS_TYPE,&fpsIdx))
        fpsIdx = 1;

    setTimeBaseFromIdx(fpsIdx,_tbNum,_tbDen);

    // Then spit the name in name and extension
    int nbOfDigits;
    std::string name,extension;
    std::string prefix;
    ADM_PathSplit(inname,name,extension);
    splitImageSequence(name,nbOfDigits,_first,prefix);
    uint32_t maxCount = 1;
    for(int i=0; i < nbOfDigits; i++)
    {
        maxCount *= 10;
    }
    maxCount--;
    if(!nbOfDigits) // no digit at all
    {
        _nbFiles=1;
        _filePrefix=prefix+std::string(".")+extension;
    }
    else
    {
        char realstring[MAX_LEN];
        snprintf(realstring, MAX_LEN, "%s%%0%" PRIu32"d.%s", prefix.c_str(), nbOfDigits, extension.c_str());
        _filePrefix = realstring;
        _nbFiles = 0;
        for (uint32_t i = 0; i < MAX_ACCEPTED_OPEN_FILE; i++)
        {
            if(i + _first > maxCount)
                break;
            snprintf(realstring, MAX_LEN, _filePrefix.c_str(), i + _first);
            ADM_info(" %" PRIu32" : %s\n", i, realstring);
            if(!i && strcmp(realstring,inname))
            {
                printf("[picHeader::open] Derived filename wrong, should be \"%s\"\n", inname);
                return 0;
            }

            fd = ADM_fopen(realstring, "rb");
            if (fd == NULL)
                break;
            fclose(fd);
            _nbFiles++;
        }
        if(!_nbFiles)
        {
            ADM_warning("Cannot read \"%s\"\n", realstring);
            return 0;
        }
    }
    if(_type==ADM_PICTURE_BMP || _type==ADM_PICTURE_BMP2)
    {
         // get bpp and offset
        if(!extractBmpAdditionalInfo(inname,_type,bpp,_bmpHeaderOffset))
        {
            ADM_warning("Could not get BMP/BMP2 info\n");
            return false;
        }
                
    }
    
    ADM_info("Found %" PRIu32" images\n", _nbFiles);
    //_________________________________
    // now open them and assign imgSize
    //__________________________________
    for (uint32_t i = 0; i < _nbFiles; i++)
    {
        fd = openFrameFile(i);
        ADM_assert(fd);
        fseek(fd, 0, SEEK_END);
        int sz = ftell(fd)-_bmpHeaderOffset;
        aprintf("Size %d, actual = 24 %d 32=%d offset=%d\n",sz,_w*_h*3,_w*_h*4,_bmpHeaderOffset);
        _imgSize.push_back(sz);
        _totalSize += sz;
        fclose(fd);
    }
//_______________________________________
//              Now build header info
//_______________________________________
    _isaudiopresent = 0;	// Remove audio ATM
    _isvideopresent = 1;	// Remove audio ATM

#define CLR(x)              memset(& x,0,sizeof(  x));

    CLR(_videostream);
    CLR(_mainaviheader);

    _videostream.dwScale = _tbNum;
    _videostream.dwRate = _tbDen;
    _mainaviheader.dwMicroSecPerFrame = 0; // constant fps
    _videostream.fccType = fourCC::get((uint8_t *) "vids");

    if (bpp)
        _video_bih.biBitCount = bpp;
    else
        _video_bih.biBitCount = 24;

    _videostream.dwLength = _mainaviheader.dwTotalFrames = _nbFiles;
    _videostream.dwInitialFrames = 0;
    _videostream.dwStart = 0;
    _video_bih.biWidth = _mainaviheader.dwWidth = _w;
    _video_bih.biHeight = _mainaviheader.dwHeight = _h;

    _duration = getTime(_nbFiles);

    switch(_type)
    {
#define SET_FCC(x,y) _video_bih.biCompression = _videostream.fccHandler =  fourCC::get((uint8_t *) x);ADM_info("Image type=%s\n",y);break;
            case ADM_PICTURE_JPG : SET_FCC("MJPG","JPG")
	    case ADM_PICTURE_BMP : SET_FCC("DIB ","BMP")
	    case ADM_PICTURE_BMP2: SET_FCC("DIB ","BMP2")
	    case ADM_PICTURE_PNG : SET_FCC("PNG ","PNG")
            default:
                ADM_assert(0);
    }
    return 1;
}
/**
 * 
 * @param frame
 * @param flags
 * @return 
 */
uint8_t picHeader::setFlag(uint32_t frame, uint32_t flags)
{
    UNUSED_ARG(frame);
    UNUSED_ARG(flags);
    return 0;
}
/**
 * 
 * @param frame
 * @param flags
 * @return 
 */
uint32_t picHeader::getFlags(uint32_t frame, uint32_t * flags)
{
    UNUSED_ARG(frame);
    *flags = AVI_KEY_FRAME;
    return 1;
}
/**
 * 
 * @param frameNum
 * @return 
 */
FILE* picHeader::openFrameFile(uint32_t frameNum)
{
    if(frameNum >= _nbFiles)
        return NULL;
    if(_reverseOrder)
        frameNum = _nbFiles - frameNum - 1;
    char filename[MAX_LEN];
    sprintf(filename, _filePrefix.c_str(), frameNum + _first);
    return ADM_fopen(filename, "rb");
}
/**
 * 
 * @param frame
 * @param pts
 * @param dts
 * @return 
 */
bool       picHeader::getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts)
{
    if(frame >= _nbFiles)
        return false;
    uint64_t timeP = getTime(frame);
    *pts=timeP;
    *dts=timeP;
    return true;
}
/**
 * 
 * @param frame
 * @param pts
 * @param dts
 * @return 
 */
bool       picHeader::setPtsDts(uint32_t frame,uint64_t pts,uint64_t dts)
{
    return false;
}
// EOF
