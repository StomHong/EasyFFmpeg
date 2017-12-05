//
// Created by StomHong on 2017/12/5.
//

#include <jni.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, "(>_<)", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "(^_^)", format, ##__VA_ARGS__)

JNIEXPORT void JNICALL
Java_com_stomhong_easyffmpeg_PlayerActivity_setDataResource
        (JNIEnv *env, jobject instance, jstring _url)
{
    const char *url = (*env)->GetStringUTFChars(env,_url,0);
    //注册所有的解码器
    av_register_all();



}