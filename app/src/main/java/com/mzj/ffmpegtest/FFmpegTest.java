package com.mzj.ffmpegtest;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.view.Surface;

/**
 * ffmpegTest 测试工具类
 */
public class FFmpegTest {

    static {
        System.loadLibrary("ffmpeg");
        System.loadLibrary("ffmpeg_test");
    }

    public native static void deCodeVideo(String input, Surface surface);

    public native static void deCodeAudio(String input);

    /**
     * 创建AuidoTrack
     * @param sampleRateInHz 采样率，单位Hz
     * @param nb_channels 声道个数
     * @return AudioTrack
     */
    public static AudioTrack createAudioTrack(int sampleRateInHz,int nb_channels){
        int channelConfig;
        if(nb_channels == 1){
            channelConfig = AudioFormat.CHANNEL_OUT_MONO;
        } else if(nb_channels == 2){
            channelConfig = AudioFormat.CHANNEL_OUT_STEREO;
        } else {
            channelConfig = AudioFormat.CHANNEL_OUT_MONO;
        }
        int buffersize = AudioTrack.getMinBufferSize(sampleRateInHz,channelConfig,AudioFormat.ENCODING_PCM_16BIT);
        AudioTrack audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC,sampleRateInHz,channelConfig,
                AudioFormat.ENCODING_PCM_16BIT,buffersize,AudioTrack.MODE_STREAM);
        return audioTrack;
    }
}
