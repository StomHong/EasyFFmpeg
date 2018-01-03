//
// Created by StomHong on 2017/12/5.
//

#include <jni.h>
#include <android/log.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <android/native_window_jni.h>
#include <libavutil/time.h>


#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, "(>_<)", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "(^_^)", format, ##__VA_ARGS__)

JNIEXPORT void JNICALL
Java_com_stomhong_easyffmpeg_PlayerCore_start(JNIEnv *env, jobject instance);

/**
 * 一般来说，媒体播放有这几个步骤：解协议-->解封装-->解码-->视音频同步
 *  目前只是需要：解封装-->解码
 *
 *  解码播放
 * @param env       JNI接口指针
 * @param instance  this指针
 * @param _url      视频地址
 */
JNIEXPORT void JNICALL
Java_com_stomhong_easyffmpeg_PlayerActivity_setDataResource
        (JNIEnv *env, jobject instance, jstring _url) {

    const char *url = (*env)->GetStringUTFChars(env, _url, 0);
    jclass renderCls = (*env)->GetObjectClass(env, instance);
    jmethodID updateDataId = (*env)->GetMethodID(env, renderCls, "nativeCallback", "([B)V");

    //注册所有的解码器/编码器,这里只用到了解码器
    av_register_all();

    AVFormatContext *pFormatCtx = NULL;

    //打开输入流并读取数据头 也可以使用avformat_alloc_context()
    int ret = avformat_open_input(&pFormatCtx, url, NULL, NULL);
    if (ret < 0) {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf));
        LOGE("open %s failed: %s", url, buf);
        return;
    }

    AVCodec *pCodec = NULL;
    //获取视频流
    int videoStream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &pCodec, 0);
    if (videoStream < 0) {
        LOGE("find videoStream failed");
        return;
    }

    //初始化解码器上下文
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoStream]->codecpar);
    int err = avcodec_open2(pCodecCtx, pCodec, NULL);
    if (err < 0) {
        char buf[1024] = {0};
        av_strerror(err, buf, sizeof(buf));
        LOGE("open videoCodec failed: %s", buf);
        return;
    }

    AVPacket *pPacket = av_packet_alloc();

    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pFrameRGB = av_frame_alloc();


    struct SwsContext *pSwsCtx = sws_alloc_context();
    unsigned int index = 0;

    //循环读取视频帧
    while (av_read_frame(pFormatCtx, pPacket) >= 0) {
        //如果是视频数据包
        if (pPacket->stream_index == videoStream) {
            ret = avcodec_send_packet(pCodecCtx, pPacket);
            if (ret != 0) {
                av_packet_unref(pPacket);
                LOGE("读取视频帧出错:%d", index++);
                continue;
            }
            while (avcodec_receive_frame(pCodecCtx, pFrame) == 0) {
                LOGE("读取视频帧:%d", index++);
                LOGE("读取视频帧:%s", pFrame->data[index]);
                unsigned int lenth =  pFrame->data[0] + pCodecCtx->width;

                jbyteArray vArr = (*env)->NewByteArray(env, lenth);
                (*env)->SetByteArrayRegion(env, vArr, 0, lenth,
                                           (jbyte *) pFrame->data[index]);

                (*env)->CallVoidMethod(env, instance, updateDataId, vArr);

                (*env)->DeleteLocalRef(env, vArr);
                free(vArr);

                av_usleep(1000 * 4);

            }
            //yuv转换rgb
//            sws_scale(pSwsCtx,
//                      (uint8_t const * const *) pFrame->data,
//                      pFrame->linesize, 0,
//                      pCodecCtx->height,
//                      pFrameRGB->data,
//                      pFrameRGB->linesize);


        }
    }
    //释放资源
    av_frame_free(&pFrameRGB);
    av_frame_free(&pFrame);
    av_packet_unref(pPacket);

    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);
    (*env)->ReleaseStringUTFChars(env, _url, url);

}

//JNIEXPORT void JNICALL
//Java_com_stomhong_easyffmpeg_PlayerCore_start(JNIEnv *env, jobject instance) {

// TODO
//}