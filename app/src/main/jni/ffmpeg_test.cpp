//
// Created by mazejia on 2021/4/19.
//

#include "com_mzj_ffmpegtest_FFmpegTest.h"
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <android/log.h>
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"TAG",FORMAT,##__VA_ARGS__);
#define MAX_AUDIO_FRME_SIZE 48000*4

extern "C" {
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/imgutils.h"
    #include "libswscale/swscale.h"
    #include "libswresample/swresample.h"
}

//对应java的audioTrack
jobject audioTrack = NULL;
//对应audioTrack.play()的methodID
jmethodID audioTrack_play_mid;
//对应audioTrack.stop()的methodID
jmethodID audioTrack_stop_mid;
//对应audioTrack.write()的methodID
jmethodID audioTrack_write_mid;
//在java层创建的audioTrack
void createAudioTrackForJava(JNIEnv *,jclass,int32_t,int32_t);
//调用java层的audioTrack.play()
void audioTrackPlayForJava(JNIEnv *);
//调用java层的audioTrack.stop
void audioTrackStopForJava(JNIEnv *);
//调用Java层的audiaoTrack.write()
void audioTrackWriteForJava(JNIEnv *,uint8_t*,int32_t);


extern "C"
JNIEXPORT void JNICALL
Java_com_mzj_ffmpegtest_FFmpegTest_deCodeVideo(JNIEnv *env, jclass jcls, jstring input_jstr,
                                               jobject surface) {

    const char* input_path = env->GetStringUTFChars(input_jstr, NULL);

    //一、注册所有组件
    av_register_all();

    //二、打开输入文件
    //1.初始化AVFormatContext
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    //2.打开输入文件
    if(avformat_open_input(&pFormatCtx,input_path,NULL,NULL) != 0){
        LOGE("打开输入文件失败");
        return;
    }

    //三、获取视频文件信息
    if(avformat_find_stream_info(pFormatCtx,NULL) < 0){
        LOGE("获取视频文件信息失败");
        return;
    }

    //四、查找编解码器
    //AVCodec *avcodec_find_decoder(enum AVCodecID id);
    //1. 获取视频流的索引(下标)位置
    int video_stream_index = -1;//存放视频流的索引位置
    for(int i = 0;i<pFormatCtx->nb_streams;++i){
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_index = i;
            break;
        }
    }

    if(video_stream_index == -1){
        LOGE("没有找到视频流");
        return;
    }

    //2. 获取视频流编解码器上下文（保存了视频或音频解码器的信息）
    AVCodecContext *pCodecCtx = pFormatCtx->streams[video_stream_index]->codec;

    //3. 通过编解码器上下文（存放的编解码信息）存放的编解码器ID获取编解码器
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

    //五、打开编码器
    if(avcodec_open2(pCodecCtx,pCodec,NULL) < 0){
        LOGE("打开编码器失败");
        return;
    }

    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env,surface);
    ANativeWindow_Buffer outBuffer;

    //六、从输入文件读取数据（循环读取），av_read_frame只能读取1帧
    //int av_read_frame(AVFormatContext *s,ACPacket *pkt);
    //1. 初始化AVPacket 存放解码器数据
    AVPacket *pPacket = av_packet_alloc();
    //2. 初始化AVFrame  存放解码后数据
    AVFrame *pFrame = av_frame_alloc();
    //3. 初始化AVFrame 存放转换为RGBA后的数据
    AVFrame  *pFrameRBGA = av_frame_alloc();
    //4. 初始化用于格式转换的SwsContext，由于解码出来的帧格式不是RGBA的，再渲染之前需要进行格式转换
    SwsContext *sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                         pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGBA,
                                         SWS_BILINEAR, NULL,
                                         NULL, NULL);

    int got_picture_ptr;//如果没有帧可以解压缩，则为零，否则为非零。
    int countFrame = 0;
    while(av_read_frame(pFormatCtx,pPacket) == 0){
        //解码视频
        if(pPacket->stream_index == video_stream_index){
            //七、解码一帧数据
            if(avcodec_decode_video2(pCodecCtx,pFrame,&got_picture_ptr,pPacket) < 0){
                LOGE("解码错误");
            }
            if(got_picture_ptr >= 0){
                LOGE("解码第%d帧",++countFrame)

                ANativeWindow_setBuffersGeometry(nativeWindow,pCodecCtx->width,pCodecCtx->height,WINDOW_FORMAT_RGBA_8888);
                ANativeWindow_lock(nativeWindow,&outBuffer,NULL);
                av_image_fill_arrays(pFrameRBGA->data,pFrameRBGA->linesize,(uint8_t*)outBuffer.bits, AV_PIX_FMT_RGBA,
                                     pCodecCtx->width, pCodecCtx->height, 1);
                //格式转换
                sws_scale(sws_ctx,(const uint8_t *const*)pFrame->data,pFrame->linesize,0,
                        pCodecCtx->height,pFrameRBGA->data,pFrameRBGA->linesize);

                //unlock
                ANativeWindow_unlockAndPost(nativeWindow);
            }
        }

        av_packet_unref(pPacket);
    }
    ANativeWindow_release(nativeWindow);
    av_free(pPacket);
    av_free(pFrame);
    av_free(pFrameRBGA);
    sws_freeContext(sws_ctx);
    //八、关闭解编码器
    avcodec_close(pCodecCtx);

    //九、关闭输入文件
    avformat_close_input(&pFormatCtx);

    //释放
    env->ReleaseStringUTFChars(input_jstr,input_path);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_mzj_ffmpegtest_FFmpegTest_deCodeAudio(JNIEnv *env, jclass jcls, jstring input_jstr) {
    //java String -> c char*
    const char* input_path = env->GetStringUTFChars(input_jstr,NULL);

    //一，注册所有组件
    av_register_all();

    //二，打开输入文件
    //1. 初始化AVFormatContext
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    //2. 打开输入文件
    if(avformat_open_input(&pFormatCtx,input_path,NULL,NULL) != 0){
        LOGE("打开输入文件失败");
        return;
    }

    //三、获取音频文件信息
    if(avformat_find_stream_info(pFormatCtx,NULL) < 0){
        LOGE("获取音频文件信息失败");
        return;
    }

    //四、查找编解码器
    //1. 获取音频流的索引位置
    //存放音频流的索引位置
    int audio_stream_index = -1;
    for(int i = 0;i<pFormatCtx->nb_streams;++i){
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
            audio_stream_index = i;
            break;
        }
    }
    if(audio_stream_index == -1){
        LOGE("没有找到音频流");
        return;
    }
    //2.获取音频流的编解码上下文
    AVCodecContext *pCodecCtx = pFormatCtx->streams[audio_stream_index]->codec;
    //3. 通过编解码上下文存放的编解码器ID获取编解码器
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

    //五.打开编码器
    if(avcodec_open2(pCodecCtx,pCodec,NULL) < 0){
        LOGE("打开编码器失败");
        return;
    }
    //六.从输入文件读取数据（循环读取），av_read_frame只能读取1帧
    //1.初始化AVPacket 存放解码前数据
    AVPacket *pPacket = av_packet_alloc();
    //2.初始化AVFrame 存放解码后数据
    AVFrame  *pFrame = av_frame_alloc();
    //3. 初始化用于重采样的SwrContext
    SwrContext *swrCtx = swr_alloc();
    //输入的采样格式
    enum AVSampleFormat in_sample_fmt = pCodecCtx->sample_fmt;
    //输出采样格式16bit PCM
    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    //输入采样率
    int in_sample_rate = pCodecCtx->sample_rate;
    //输出采样率
    int out_sample_rate = 44100;
    //获取输入的声道布局
    uint64_t in_ch_layout = pCodecCtx->channel_layout;
    //输出的声道布局
    uint64_t  out_ch_layout = AV_CH_LAYOUT_STEREO;
    //设置参数到SwrContext
    swr_alloc_set_opts(swrCtx,out_ch_layout,out_sample_fmt,out_sample_rate,
                              in_ch_layout,in_sample_fmt,in_sample_rate,
                              0,NULL);
    //初始化SwrContext
    swr_init(swrCtx);
    //输出的声道个数
    int out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);
    //分配存放16bit 44100 PCM 数据的内存
    uint8_t *out_buffer = (uint8_t *)av_malloc(MAX_AUDIO_FRME_SIZE);
    //4. Java层创建的AudioTrack
    createAudioTrackForJava(env,jcls,out_sample_rate,out_channel_nb);
    //5. 调用Java层的AudioTrack.play()
    audioTrackPlayForJava(env);

    int got_frame = 0;
    int countFrame = 0;
    while(av_read_frame(pFormatCtx,pPacket) == 0){
        if(pPacket->stream_index == audio_stream_index){
            //七. 解码一帧数据
            avcodec_decode_audio4(pCodecCtx,pFrame,&got_frame,pPacket);
            if(got_frame > 0){
                LOGE("解码：%d", ++countFrame);
                //重采样
                swr_convert(swrCtx,&out_buffer,MAX_AUDIO_FRME_SIZE,
                        (const uint8_t **) pFrame->data , pFrame->nb_samples);
                //获取samples(类似于视频的一帧)的大小
                int out_buffer_size = av_samples_get_buffer_size(NULL,out_channel_nb,
                                                                pFrame->nb_samples,out_sample_fmt,1);
                //调用java层的audioTrack.write
                audioTrackWriteForJava(env,out_buffer,out_buffer_size);
            }
        }
        av_packet_unref(pPacket);
    }
    //调用Java层的audioTrack.stop()
    audioTrackStopForJava(env);
    av_free(pPacket);
    av_free(pFrame);
    av_free(out_buffer);
    swr_free(&swrCtx);
    //八.关闭编解码器
    avcodec_close(pCodecCtx);

    //九，关闭输入文件
    avformat_close_input(&pFormatCtx);

    //释放
    env->ReleaseStringUTFChars(input_jstr,input_path);
}

void createAudioTrackForJava(JNIEnv *env,jclass ffmpegTest_clas,
        int32_t out_sample_rate,int32_t out_channel_nb){
    //AudioTrack对象
    jmethodID createAudioTrack_mid = env->GetStaticMethodID(ffmpegTest_clas,"createAudioTrack"
                                                            ,"(II)Landroid/media/AudioTrack;");
    //调用Java层的createAudioTrack
    audioTrack = env->CallStaticObjectMethod(ffmpegTest_clas,createAudioTrack_mid,
                                            out_sample_rate,out_channel_nb);
    //获得AudioTrack 的class
    jclass audio_track_class = env->GetObjectClass(audioTrack);
    //AudioTrack.play
    audioTrack_play_mid = env->GetMethodID(audio_track_class,"play","()V");
    //AudioTrack.stop
    audioTrack_stop_mid = env->GetMethodID(audio_track_class,"stop","()V");
    //AudioTrack.write
    audioTrack_write_mid = env->GetMethodID(audio_track_class,"write","([BII)I");
}

//调用Java层的audioTrack.play
void audioTrackPlayForJava(JNIEnv *env){
    if(audioTrack != NULL){
        env->CallVoidMethod(audioTrack,audioTrack_play_mid);
    }
}

//调用Java层的audioTrack.stop
void audioTrackStopForJava(JNIEnv *env){
    if(audioTrack != NULL){
        env->CallVoidMethod(audioTrack,audioTrack_stop_mid);
    }
}

//调用Java层的audioTrack.write
void audioTrackWriteForJava(JNIEnv *env,uint8_t* out_buffer,int32_t out_buffer_size){
    if(audioTrack != NULL){
        //out_buffer缓冲区数据 -> Java的byte[]
        jbyteArray audio_sample_array = env->NewByteArray(out_buffer_size);
        jbyte *sample_bytep = env->GetByteArrayElements(audio_sample_array,NULL);
        //out_buffer的数据复制到sampe_bytep
        memcpy(sample_bytep,out_buffer,out_buffer_size);
        env->ReleaseByteArrayElements(audio_sample_array,sample_bytep,0);
        //调用audioTrack.write
        env->CallIntMethod(audioTrack,audioTrack_write_mid,
                            audio_sample_array,0,out_buffer_size);
        //释放局部引用
        env->DeleteLocalRef(audio_sample_array);
    }
}






