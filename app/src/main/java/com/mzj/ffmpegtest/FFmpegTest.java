package com.mzj.ffmpegtest;

public class FFmpegTest {
    public native static String getFFmpegCodecInfo();

    static {
        System.loadLibrary("ffmpeg");
        System.loadLibrary("ffmpeg_test");
    }
}
