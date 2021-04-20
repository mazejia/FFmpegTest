//
// Created by mazejia on 2021/4/19.
//

#include "com_mzj_ffmpegtest_FFmpegTest.h"
extern "C" {
    #include "libavcodec/avcodec.h"
}
extern "C" JNIEXPORT jstring JNICALL
Java_com_mzj_ffmpegtest_FFmpegTest_getFFmpegCodecInfo(JNIEnv *env, jclass clazz) {
    return env->NewStringUTF(avcodec_configuration());
}
