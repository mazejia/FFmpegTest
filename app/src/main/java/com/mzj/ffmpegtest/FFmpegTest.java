package com.mzj.ffmpegtest;

import android.view.Surface;

public class FFmpegTest {
//    public native static String getFFmpegCodecInfo();

    public native static void deCodeVideo(String input, Surface surface);


    static {
        System.loadLibrary("ffmpeg");
        System.loadLibrary("ffmpeg_test");
    }
}
