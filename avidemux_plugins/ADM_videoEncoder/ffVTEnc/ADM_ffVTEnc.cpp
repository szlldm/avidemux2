/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_ffVTEnc.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"
#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

extern "C"
{
    #include "libavutil/opt.h"
}

ffvtenc VTEncSettings = VT_ENC_CONF_DEFAULT;

/**
    \fn ADM_ffVTEncoder
*/
ADM_ffVTEncoder::ADM_ffVTEncoder(ADM_coreVideoFilter *src, bool globalHeader) : ADM_coreVideoEncoderFFmpeg(src, NULL, globalHeader)
{
#ifdef H265_ENCODER
    ADM_info("Creating VideoToolbox HW accelerated HEVC encoder...\n");
#else
    ADM_info("Creating VideoToolbox HW accelerated H.264 encoder...\n");
#endif
    frameIncrement=src->getInfo()->frameIncrement;
}

/**
    \fn configureContext
*/
bool ADM_ffVTEncoder::configureContext(void)
{
    switch(VTEncSettings.profile)
    {
#ifdef H265_ENCODER
#define SAY(x,y) case FF_VT_HEVC_PROFILE_##x: av_dict_set(&_options,"profile",y,0); break;
        SAY(MAIN,"main")
        SAY(MAIN10,"main10")
#else
#define SAY(x,y) case FF_VT_H264_PROFILE_##x: av_dict_set(&_options,"profile",y,0); break;
        SAY(BASELINE,"baseline")
        SAY(MAIN,"main")
        SAY(HIGH,"high")
#endif
        default:break;
#undef SAY
    }

    _context->bit_rate=VTEncSettings.bitrate*1000;
    _context->rc_max_rate=VTEncSettings.max_bitrate*1000;
    _context->gop_size=VTEncSettings.gopsize;
    // We have no control about the exact # of consecutive B-frames.
    // Even worse, the HEVC encoder, at least on some Intel Macs,
    // ignores this parameter entirely, outputs B-frames no matter what.
    _context->max_b_frames=VTEncSettings.bframes;
    _context->pix_fmt=AV_PIX_FMT_YUV420P;

    return true;
}

/**
    \fn setup
*/
bool ADM_ffVTEncoder::setup(void)
{
    const char *encName =
#ifdef H265_ENCODER
    "hevc_videotoolbox";
#else
    "h264_videotoolbox";
#endif
    if(false == ADM_coreVideoEncoderFFmpeg::setupByName(encName))
    {
        ADM_warning("Cannot set up %s encoder.\n",encName);
        return false;
    }
    ADM_info("%s encoder set up successfully.\n",encName);
    return true;
}

/**
    \fn getEncoderDelay
*/
uint64_t ADM_ffVTEncoder::getEncoderDelay(void)
{
    uint64_t delay=0;
    if(VTEncSettings.bframes)
        delay = frameIncrement *
#ifdef H265_ENCODER
        3;
#else
        2;
#endif
    return delay;
}

/**
    \fn dtor
*/
ADM_ffVTEncoder::~ADM_ffVTEncoder()
{
    ADM_info("[ffVTEncoder] Destroying\n");
}

/**
    \fn getFourcc
*/
const char *ADM_ffVTEncoder::getFourcc(void)
{
#ifdef H265_ENCODER
    return "HEVC";
#else
    return "H264";
#endif
}

/**
    \fn encode
*/
bool ADM_ffVTEncoder::encode(ADMBitstream *out)
{
    int sz,q;
again:
    sz=0;
    if(false==preEncode()) // Pop out the frames stored in the queue due to B-frames
    {
        sz=encodeWrapper(NULL,out);

        if (sz == 0)
            return false;
        if (sz < 0)
        {
            ADM_info("[ffvtenc] Error %d encoding video\n",sz);
            return false;
        }
        ADM_info("[ffvtenc] Popping delayed bframes (%d)\n",sz);
        goto link;
    }
    q = image->_Qp;

    if(!q) q=2;
    aprintf("[CODEC] Flags = 0x%x, QSCALE=%x, bit_rate=%d, quality=%d qz=%d incoming qz=%d\n",_context->flags,CODEC_FLAG_QSCALE,
                                     _context->bit_rate,  _frame->quality, _frame->quality/ FF_QP2LAMBDA,q);

    _frame->width = image->GetWidth(PLANAR_Y);
    _frame->height = image->GetHeight(PLANAR_Y);
    _frame->format = AV_PIX_FMT_YUV420P;
    sz=encodeWrapper(_frame,out);
    if(sz<0)
    {
        ADM_warning("[ffvtenc] Error %d encoding video\n",sz);
        return false;
    }

    if(sz==0) // no pic, probably pre filling, try again
        goto again;
link:
    return postEncode(out,sz);
}

/**
    \fn isDualPass

*/
bool ADM_ffVTEncoder::isDualPass(void)
{
    return false;
}

/**
    \fn ffVTEncConfigure
    \brief UI configuration for the h264_videotoolbox encoder
*/

bool ffVTEncConfigure(void)
{
    ffvtenc *conf=&VTEncSettings;

    diaMenuEntry vtProfile[]={
#ifdef H265_ENCODER
        {FF_VT_HEVC_PROFILE_MAIN,QT_TRANSLATE_NOOP("ffvtenc","Main"),NULL},
        {FF_VT_HEVC_PROFILE_MAIN10,QT_TRANSLATE_NOOP("ffvtenc","Main10"),NULL}
#else
        {FF_VT_H264_PROFILE_BASELINE,QT_TRANSLATE_NOOP("ffvtenc","Baseline"),NULL},
        {FF_VT_H264_PROFILE_MAIN,QT_TRANSLATE_NOOP("ffvtenc","Main"),NULL},
        {FF_VT_H264_PROFILE_HIGH,QT_TRANSLATE_NOOP("ffvtenc","High"),NULL}
#endif
};

#define PX(x) &(conf->x)

    diaElemMenu profile(PX(profile),QT_TRANSLATE_NOOP("ffvtenc","Profile:"),sizeof(vtProfile)/sizeof(diaMenuEntry),vtProfile);
    diaElemUInteger gopSize(PX(gopsize),QT_TRANSLATE_NOOP("ffvtenc","GOP Size:"),1,250);

    bool enableBf = conf->bframes;
    diaElemToggle allowBframes(&enableBf,QT_TRANSLATE_NOOP("ffvtenc","Allow B-frames"));
#ifdef H265_ENCODER
    diaElemReadOnlyText bfWarning(NULL,QT_TRANSLATE_NOOP("ffvtenc","On some Macs, disabling B-frames is not possible"));
#endif
    diaElemUInteger bitrate(PX(bitrate), QT_TRANSLATE_NOOP("ffvtenc","Bitrate (kbps):"),1,50000);
    diaElemUInteger maxBitrate(PX(max_bitrate), QT_TRANSLATE_NOOP("ffvtenc","Max Bitrate (kbps):"),1,50000);
    diaElemFrame rateControl(QT_TRANSLATE_NOOP("ffvtenc","Rate Control"));
    diaElemFrame frameControl(QT_TRANSLATE_NOOP("ffvtenc","Frame Control"));

    rateControl.swallow(&bitrate);
    rateControl.swallow(&maxBitrate);
    frameControl.swallow(&gopSize);
    frameControl.swallow(&allowBframes);

#ifdef H265_ENCODER
    frameControl.swallow(&bfWarning);
#else
    profile.link(&vtProfile[1],1,&allowBframes);
    profile.link(&vtProfile[2],1,&allowBframes);
#endif
    diaElem *diamode[] = {&profile,&rateControl,&frameControl};

    if(diaFactoryRun(
#ifdef H265_ENCODER
        QT_TRANSLATE_NOOP("ffvtenc","VideoToolbox HEVC Encoder Configuration"),
#else
        QT_TRANSLATE_NOOP("ffvtenc","VideoToolbox H.264 Encoder Configuration"),
#endif
        3,diamode))
    {
        conf->bframes = enableBf ? 1 : 0;
#ifndef H265_ENCODER
        if(conf->profile==FF_VT_H264_PROFILE_BASELINE)
            conf->bframes=0;
#endif
        return true;
    }
    return false;
}
// EOF
