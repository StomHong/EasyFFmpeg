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
#include <unistd.h>
#include <libavutil/pixfmt.h>
#include <libyuv.h>
#include <libyuv/convert_from.h>
#include <libavutil/imgutils.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2platform.h>
#include <GLES3/gl3.h>
#include <GLES/egl.h>
#include <GLES/gl.h>
#include <GLES/glplatform.h>




#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, "(>_<)", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "(^_^)", format, ##__VA_ARGS__)


/**
 * 一般来说，媒体播放有这几个步骤：解协议-->解封装-->解码-->视音频同步
 *  目前只是需要：解封装-->解码-->播放视频
 *
 *  解码播放
 * @param env       JNI接口指针
 * @param instance  this指针
 * @param _url      视频地址
 */

JNIEXPORT  void JNICALL Java_com_stomhong_easyffmpeg_VideoOpenGLActivity_setDataResource
        (JNIEnv *env, jobject instance, jstring _url) {

    const char *url = "/storage/emulated/0/sintel.mp4";

//    const char *url = (*env)->GetStringUTFChars(env, _url, 0);
    jclass renderCls = (*env)->GetObjectClass(env, instance);
    jmethodID updateDataId = (*env)->GetMethodID(env, renderCls, "nativeCallback", "([BII)V");

    //注册所有的解码器
    av_register_all();

    AVFormatContext *pFormatCtx = NULL;
    AVCodec *pCodec = NULL;


    //打开输入流并读取数据头 也可以使用avformat_alloc_context()
    int ret = avformat_open_input(&pFormatCtx, url, NULL, NULL);
    if (ret < 0) {
        char buf[1024] = {0};
        av_strerror(ret, buf, sizeof(buf));
        LOGE("open %s failed: %s", url, buf);
        return;
    }

    //获取视频总时长(second)
    int64_t totalSec = pFormatCtx->duration / AV_TIME_BASE;
    LOGE("totalSec %lld", totalSec);

    //获取视频流
    int videoStream = 0;
    videoStream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &pCodec, 0);
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

    //初始化AVPacket
    AVPacket *pPacket = av_packet_alloc();
    //初始化AVFrame
    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pFrameRGBA = av_frame_alloc();

    unsigned int index = 0;
    //循环读取数据帧
    while (av_read_frame(pFormatCtx, pPacket) >= 0) {
        //如果是视频数据包
        if (pPacket->stream_index == videoStream) {
            int pkt_ret = avcodec_send_packet(pCodecCtx, pPacket);
            if (pkt_ret != 0) {
                av_packet_unref(pPacket);
                continue;
            }
//            int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB565, pCodecCtx->width,
//                                                    pCodecCtx->height, 1);
//            uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
//            av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, buffer, AV_PIX_FMT_RGB565,
//                                 pCodecCtx->width, pCodecCtx->height, 1);
//
//            // 由于解码出来的帧格式不是RGBA的,在渲染之前需要进行格式转换
//            struct SwsContext *sws_ctx = sws_getContext(pCodecCtx->width,
//                                                        pCodecCtx->height,
//                                                        pCodecCtx->pix_fmt,
//                                                        pCodecCtx->width,
//                                                        pCodecCtx->height,
//                                                        AV_PIX_FMT_RGB565,
//                                                        SWS_BILINEAR,
//                                                        NULL,
//                                                        NULL,
//                                                        NULL);

            while (avcodec_receive_frame(pCodecCtx, pFrame) >= 0) {
                LOGE("读取视频帧:%d", index++);
                // 格式转换
//                sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data,
//                          pFrame->linesize, 0, pCodecCtx->height,
//                          pFrameRGBA->data, pFrameRGBA->linesize);

//                glClear(GL_COLOR_BUFFER_BIT);
//                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pCodecCtx->width, pCodecCtx->height, GL_RGB, GL_UNSIGNED_BYTE, pFrameRGBA->data[0]);
//                glDrawTexiOES(0, 0, 0, videoWidth, videoHeight);


                (*env)->CallVoidMethod(env, instance, updateDataId);

                usleep(1000 * 10);

            }


        }
    }
    //释放资源
    av_frame_free(&pFrameRGBA);
    av_frame_free(&pFrame);
    av_packet_unref(pPacket);

    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);
//    (*env)->ReleaseStringUTFChars(env, _url, url);
}

