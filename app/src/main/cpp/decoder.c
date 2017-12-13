//
// Created by StomHong on 2017/12/5.
//

#include <jni.h>
#include <android/log.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>


#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, "(>_<)", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "(^_^)", format, ##__VA_ARGS__)
/**
 *  解码播放
 * @param env       JNI接口指针
 * @param instance  this指针
 * @param _url      视频地址
 */
JNIEXPORT void JNICALL
Java_com_stomhong_easyffmpeg_PlayerActivity_setDataResource
        (JNIEnv *env, jobject instance, jstring _url) {

    const char *url = (*env)->GetStringUTFChars(env, _url, 0);

    //注册所有的解码器
    av_register_all();


    AVFormatContext *ic = NULL;

    //打开输入流并读取数据头
    int ret = avformat_open_input(&ic, url, NULL, NULL);

    if (ret != 0) {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf));
        LOGE("open %s failed: %s", url, buf);
        return;
    }

    avformat_alloc_context();

    AVCodec *avCodec = NULL;


    int videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, &avCodec, 0);
    if(videoStream < 0){
        LOGE("find videoCodec failed");
        return;
    }

    AVCodecContext *avctx = avcodec_alloc_context3(avCodec);


    int err = avcodec_open2(avctx, avCodec, NULL);
    if (err < 0) {
        char buf[1024] = {0};
        av_strerror(err, buf, sizeof(buf));
        LOGE("open videoCodec failed: %s", buf);
        return;
    }

    AVPacket *avPacket = av_packet_alloc();

   int frame =  avcodec_send_packet(avctx,avPacket);
    if (frame != 0){
        LOGE("open videoCodec failed: %s","");
    }

    av_packet_unref(avPacket);
    //关闭
    avformat_close_input(&ic);
    avformat_free_context(ic);
    (*env)->ReleaseStringUTFChars(env,_url,url);


}