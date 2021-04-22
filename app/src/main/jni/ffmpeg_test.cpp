//
// Created by mazejia on 2021/4/19.
//

#include "com_mzj_ffmpegtest_FFmpegTest.h"
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <android/log.h>
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"TAG",FORMAT,##__VA_ARGS__);

extern "C" {
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/imgutils.h"
    #include "libswscale/swscale.h"
}
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