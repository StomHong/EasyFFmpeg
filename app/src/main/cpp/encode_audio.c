//
// Created by StomHong on 2017/12/5.
//

#include <jni.h>
#include <libavformat/avformat.h>

JNIEXPORT void JNICALL Java_com_stomhong_easyffmpeg_RecordActivity_record(JNIEnv *env, jobject instance) {

    //注册所有
    av_register_all();

    AVCodecContext pCodecCtx;

    AVCodec *pCodec;

    pCodec = avcodec_find_encoder(AV_CODEC_ID_H264);



}