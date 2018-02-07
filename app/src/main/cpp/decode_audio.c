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

// pointer and size of the next player buffer to enqueue, and number of remaining buffers
void *nextBuffer;
static unsigned int nextSize;
static int nextCount;

//ffmpeg部分
AVFormatContext *pFormatCtx = NULL;
AVCodec *pCodec = NULL;
AVCodecContext *pCodecCtx;
AVPacket *pPacket;
AVFrame *pFrame;
SwrContext *swrContext;
int audioStream = 0;

void createBufferQueueAudioPlayer(jint sampleRate, jint bufSize);

void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context);

/**
 * 一般来说，媒体播放有这几个步骤：解协议-->解封装-->解码-->视音频同步
 *  目前只是需要：解封装-->解码-->播放视频
 *  OpenSLES 都是通过接口API来访问的，一般先GetInterface，然后Realize
 *  解码播放
 * @param env       JNI接口指针
 * @param instance  this指针
 * @param _url      视频地址
 */

JNIEXPORT JNICALL void
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

    pFrame = av_frame_alloc();
    pPacket = av_packet_alloc();

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
//    *rate = pCodecCtx->sample_rate;
//    *channel = pCodecCtx->channels;
    createBufferQueueAudioPlayer(pCodecCtx->sample_rate,pCodecCtx->channels);

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

            while (avcodec_receive_frame(pCodecCtx, pFrame) == 0) {
                LOGE("读取音频:%d", index++);

                LOGE("音频声道数:%d 采样率：%d 数据格式: %d", pFrame->channels,pFrame->sample_rate,pFrame->format);

                swr_convert(swrContext, &out_buffer, 44100 * 2, (const uint8_t **) pFrame->data, pFrame->nb_samples);
//                缓冲区的大小
                int size = av_samples_get_buffer_size(NULL, out_channer_nb, pFrame->nb_samples,
                                                      AV_SAMPLE_FMT_S16, 1);
                *pcm = out_buffer;
                *pcm_size = size;

//                usleep(1000 * 40);

            }


        }
    }

}

// create the engine and output mix objects
//创建引擎和混响对象
JNIEXPORT JNICALL void Java_com_stomhong_easyffmpeg_AudioActivity_createEngine(JNIEnv* env, jclass clazz)
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

// create buffer queue audio player
void createBufferQueueAudioPlayer(jint sampleRate, jint bufSize)
{
    SLresult result;
    if (sampleRate >= 0 && bufSize >= 0 ) {
        bqPlayerSampleRate = sampleRate * 1000;
        /*
         * device native buffer size is another factor to minimize audio latency, not used in this
         * sample: we only play one giant buffer here
         */
        bqPlayerBufSize = bufSize;
    }

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_8,
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    /*
     * Enable Fast Audio when possible:  once we set the same rate to be the native, fast audio path
     * will be triggered
     */
    if(bqPlayerSampleRate) {
        format_pcm.samplesPerSec = bqPlayerSampleRate;       //sample rate in mili second
    }
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    /*
     * create audio player:
     *     fast audio does not support when SL_IID_EFFECTSEND is required, skip it
     *     for fast audio case
     */
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_EFFECTSEND,
            /*SL_IID_MUTESOLO,*/};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/ };

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk,
                                                bqPlayerSampleRate? 2 : 3, ids, req);

    (void)result;

    // realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);

    (void)result;

    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);

    (void)result;

    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                             &bqPlayerBufferQueue);

    (void)result;

    // register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, NULL);

    (void)result;

    // get the effect send interface
    bqPlayerEffectSend = NULL;
    if( 0 == bqPlayerSampleRate) {
        result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_EFFECTSEND,
                                                 &bqPlayerEffectSend);
        (void)result;
    }

#if 0   // mute/solo is not supported for sources that are known to be mono, as this is
    // get the mute/solo interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_MUTESOLO, &bqPlayerMuteSolo);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
#endif

    // get the volume interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);
    (void)result;

    // set the player's state to playing
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    (void)result;

    //开始播放
    bqPlayerCallback(bqPlayerBufferQueue,NULL);
}

//size_t nextSize =0;
//void *nextBuffer;
// this callback handler is called every time a buffer finishes playing
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    getPCM(&nextBuffer,&nextSize);
    if(nextBuffer!=NULL&&nextSize!=0) {
        (*bq)->Enqueue(bq, nextBuffer, nextSize);
    }

}


// shut down the native audio system
JNIEXPORT JNICALL void Java_com_stomhong_easyffmpeg_AudioActivity_stop(JNIEnv* env, jclass clazz)
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

JNIEXPORT JNICALL void Java_com_stomhong_easyffmpeg_AudioActivity_destroy(JNIEnv* env, jclass clazz){
    //    //释放资源
    av_frame_free(&pFrame);
    av_packet_unref(pPacket);

    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);
}