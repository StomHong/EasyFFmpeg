//
// Created by StomHong on 2018/1/31.
//

#include <jni.h>
#include <android/log.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <libavutil/time.h>
#include <unistd.h>
#include <libyuv.h>
#include <libyuv/convert_from.h>
#include <libavutil/imgutils.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>


#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, "(>_<)", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "(^_^)", format, ##__VA_ARGS__)

// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;

// output mix interfaces
static SLObjectItf outputMixObject = NULL;
static SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;

// aux effect on the output mix, used by the buffer queue player
static const SLEnvironmentalReverbSettings reverbSettings =
        SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

// buffer queue player interfaces
static SLObjectItf bqPlayerObject = NULL;
static SLPlayItf bqPlayerPlay;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
static SLEffectSendItf bqPlayerEffectSend;
static SLMuteSoloItf bqPlayerMuteSolo;
static SLVolumeItf bqPlayerVolume;
static SLmilliHertz bqPlayerSampleRate = 0;
static jint   bqPlayerBufSize = 0;
static short *resampleBuf = NULL;
uint8_t *out_buffer;
int out_channer_nb;

//ffmpeg部分
AVFormatContext *pFormatCtx = NULL;
AVCodec *pCodec = NULL;
AVCodecContext *pCodecCtx;
AVPacket *pPacket;
AVFrame *pFrame;
SwrContext *swrContext;
int audioStream = 0;

/**
 * 一般来说，媒体播放有这几个步骤：解协议-->解封装-->解码-->视音频同步
 *  目前只是需要：解封装-->解码-->播放视频
 *  OpenSLES 都是通过接口API来访问的，一般先GetInterface，然后Realize
 *  解码播放
 * @param env       JNI接口指针
 * @param instance  this指针
 * @param _url      视频地址
 */

JNIEXPORT  void JNICALL
Java_com_stomhong_easyffmpeg_AudioActivity_play
        (JNIEnv *env, jobject instance) {

    char *url = "/storage/emulated/0/sintel.mp4";

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

    //初始化AVPacket
    AVPacket *pPacket = av_packet_alloc();
    //初始化AVFrame
    AVFrame *pFrame = av_frame_alloc();

    //    mp3  里面所包含的编码格式   转换成  pcm   SwcContext
    swrContext = swr_alloc();

//    av_samples_get_buffer_size();
//    av_samples_fill_arrays()

    out_buffer = (uint8_t *) av_malloc(44100 * 2);
    uint64_t  out_ch_layout=AV_CH_LAYOUT_STEREO;
//    输出采样位数  16位
    enum AVSampleFormat out_formart=AV_SAMPLE_FMT_S16;
//输出的采样率必须与输入相同
    int out_sample_rate = pCodecCtx->sample_rate;


    swr_alloc_set_opts(swrContext, out_ch_layout, out_formart, out_sample_rate,
                       pCodecCtx->channel_layout, pCodecCtx->sample_fmt, pCodecCtx->sample_rate, 0,
                       NULL);

    swr_init(swrContext);
//    获取通道数  2
    out_channer_nb = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    *rate = pCodecCtx->sample_rate;
    *channel = pCodecCtx->channels;


    //释放资源
    av_frame_free(&pFrame);
    av_packet_unref(pPacket);

    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);
}

int getPCM(void **pcm,size_t *pcm_size){

    unsigned int index = 0;
    //循环读取数据帧
    while (av_read_frame(pFormatCtx, pPacket) == 0) {
        //如果是视频数据包
        if (pPacket->stream_index == audioStream) {
            int pkt_ret = avcodec_send_packet(pCodecCtx, pPacket);
            if (pkt_ret != 0) {
                av_packet_unref(pPacket);
                continue;
            }
//            swr_init()
//            swr_convert()
//            swr_get_out_samples()
//            av_get_bytes_per_sample();

            while (avcodec_receive_frame(pCodecCtx, pFrame) == 0) {
                LOGE("读取音频:%d", index++);

                LOGE("音频声道数:%d 采样率：%d 数据格式: %d", pFrame->channels,pFrame->sample_rate,pFrame->format);

                swr_convert(swrContext, &out_buffer, 44100 * 2, (const uint8_t **) pFrame->data, pFrame->nb_samples);
//                缓冲区的大小
                int size = av_samples_get_buffer_size(NULL, out_channer_nb, pFrame->nb_samples,
                                                      AV_SAMPLE_FMT_S16, 1);
                *pcm = out_buffer;
                *pcm_size = size;
                break;

                usleep(1000 * 10);

            }


        }
    }

}

// create the engine and output mix objects
//创建引擎和混响对象
void Java_com_stomhong_easyffmpeg_AudioActivity_createEngine(JNIEnv* env, jclass clazz)
{
    SLresult result;

    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    (void)result;

    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    (void)result;

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    (void)result;

    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
    (void)result;

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    (void)result;

    // get the environmental reverb interface
    // this could fail if the environmental reverb effect is not available,
    // either because the feature is not present, excessive CPU load, or
    // the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        (void)result;
    }
    // ignore unsuccessful result codes for environmental reverb, as it is optional for this example
    LOGE("执行到这");
}

void * pcmBufferCallBack(SLAndroidBufferQueueItf bf, void * context)
{
    //assert(NULL == context);
    getPCM(&buffer);
    // for streaming playback, replace this test by logic to find and fill the next buffer
    if (NULL != buffer) {
        SLresult result;
        // enqueue another buffer
        result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, buffer, 44100 * 2 * 2);
        // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
        // which for this code example would indicate a programming error
    }
}


// shut down the native audio system
void Java_com_stomhong_easyffmpeg_AudioActivity_stop(JNIEnv* env, jclass clazz)
{

    // destroy buffer queue audio player object, and invalidate all associated interfaces
    if (bqPlayerObject != NULL) {
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = NULL;
        bqPlayerPlay = NULL;
        bqPlayerBufferQueue = NULL;
        bqPlayerEffectSend = NULL;
        bqPlayerMuteSolo = NULL;
        bqPlayerVolume = NULL;
    }

//    // destroy file descriptor audio player object, and invalidate all associated interfaces
//    if (fdPlayerObject != NULL) {
//        (*fdPlayerObject)->Destroy(fdPlayerObject);
//        fdPlayerObject = NULL;
//        fdPlayerPlay = NULL;
//        fdPlayerSeek = NULL;
//        fdPlayerMuteSolo = NULL;
//        fdPlayerVolume = NULL;
//    }
//
//    // destroy URI audio player object, and invalidate all associated interfaces
//    if (uriPlayerObject != NULL) {
//        (*uriPlayerObject)->Destroy(uriPlayerObject);
//        uriPlayerObject = NULL;
//        uriPlayerPlay = NULL;
//        uriPlayerSeek = NULL;
//        uriPlayerMuteSolo = NULL;
//        uriPlayerVolume = NULL;
//    }

    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }

    // destroy engine object, and invalidate all associated interfaces
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }

//    pthread_mutex_destroy(&audioEngineLock);
}