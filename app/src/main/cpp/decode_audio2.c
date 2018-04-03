//
// Created by StomHong on 2018/3/30.
//

/**
 * 一般来说，媒体播放有这几个步骤：解协议-->解封装-->解码-->视音频同步
 *  目前只是需要：解封装-->解码-->播放视频
 *  OpenSLES 都是通过接口API来访问的，一般先GetInterface，然后Realize
 *  解码播放
 * @param env       JNI接口指针
 * @param instance  this指针
 * @param _url      视频地址
 */

#include <jni.h>
#include <android/log.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>
#include <unistd.h>


#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, "(>_<)", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "(^_^)", format, ##__VA_ARGS__)



JNIEXPORT  void JNICALL
Java_com_stomhong_easyffmpeg_AudioAudioTrackActivity_start
        (JNIEnv *env, jobject instance) {

    char *url = "/storage/emulated/0/sintel.mp4";

    AVFormatContext *pFormatCtx = NULL;
    AVCodecContext *pCodecCtx = NULL;
    AVCodec *pCodec = NULL;
    int audioStream = -1;
    int videoStream = -1;
    AVFrame *pFrame = NULL;
    AVPacket *pPacket = NULL;
    SwrContext *swrContext = NULL;
    uint8_t *out_buffer;
    int out_channer_nb;

// pointer and size of the next player buffer to enqueue, and number of remaining buffers
    void *nextBuffer;
    unsigned int nextSize;
    static int nextCount;

    int rate;

    int channel;

    uint8_t **pcm;

    int *pcm_size;

    //1.获得实例对应的class类
    jclass jcls = (*env)->GetObjectClass(env,instance);

    //2.通过class类找到对应的method id
    //name 为java类中变量名，Ljava/lang/String; 为变量的类型String
    jmethodID jmid = (*env)->GetMethodID(env,jcls,"play","([B)V");

    //注册所有的解码器
    av_register_all();

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
    audioStream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &pCodec, 0);
    if (audioStream < 0) {
        LOGE("find audioStream failed");
        return;
    }

    //初始化解码器上下文
    pCodecCtx = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[audioStream]->codecpar);
    int err = avcodec_open2(pCodecCtx, pCodec, NULL);
    if (err < 0) {
        char buf[1024] = {0};
        av_strerror(err, buf, sizeof(buf));
        LOGE("open videoCodec failed: %s", buf);
        return;
    }

    pFrame = av_frame_alloc();
    pPacket = av_packet_alloc();

    //    mp3  里面所包含的编码格式   转换成  pcm   SwcContext
    swrContext = swr_alloc();

    out_buffer = (uint8_t *) av_malloc(44100 * 2);
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
//    输出采样位数  16位
    enum AVSampleFormat out_formart = AV_SAMPLE_FMT_S16;
//输出的采样率必须与输入相同
    int out_sample_rate = pCodecCtx->sample_rate;


    swr_alloc_set_opts(swrContext, out_ch_layout, out_formart, out_sample_rate,
                       pCodecCtx->channel_layout, pCodecCtx->sample_fmt, pCodecCtx->sample_rate, 0,
                       NULL);

    swr_init(swrContext);
//    获取通道数  2
    out_channer_nb = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    rate = pCodecCtx->sample_rate;
    channel = pCodecCtx->channels;
    unsigned int index = 0;
    //循环读取数据帧
    while (av_read_frame(pFormatCtx, pPacket) == 0) {
        if (pPacket->stream_index == audioStream) {
            int pkt_ret = avcodec_send_packet(pCodecCtx, pPacket);
            if (pkt_ret != 0) {
                av_frame_free(&pFrame);
                av_packet_unref(pPacket);

                avformat_close_input(&pFormatCtx);
                avformat_free_context(pFormatCtx);
                return;
            }

            if (avcodec_receive_frame(pCodecCtx, pFrame)==0) {
                LOGE("读取音频:%d", index++);

                LOGE("音频声道数:%d 采样率：%d 数据格式: %d", pFrame->channels, pFrame->sample_rate,
                     pFrame->format);

                swr_convert(swrContext, &out_buffer, 44100 * 2, (const uint8_t **) pFrame->data,
                            pFrame->nb_samples);
//                缓冲区的大小
                int size = av_samples_get_buffer_size(NULL, out_channer_nb, pFrame->nb_samples,
                                                      AV_SAMPLE_FMT_S16, 1);
                //创建JbyteArray对象，并设置JArray对象的值
                jbyteArray jbArr = (*env)->NewByteArray(env, size);
                (*env)->SetByteArrayRegion(env, jbArr, 0, size, (jbyte *) out_buffer);
                //3.通过obj获得对应的method
                (*env)->CallVoidMethod(env,instance,jmid,jbArr);
                usleep(8000);

            }
        }
    }

}
