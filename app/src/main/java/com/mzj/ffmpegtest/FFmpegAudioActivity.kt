package com.mzj.ffmpegtest

import android.os.Bundle
import android.widget.Button
import androidx.appcompat.app.AppCompatActivity
import java.io.File

/**
 * 获取音频文件路径 -> 把音频文件路径传递到NDK层 -> NDK层通过FFmpeg打开音频文件 ->
 * FFmpeg获取音频文件的信息 -> FFmpeg通过音频文件信息获得音频流 ->
 * FFmpeg通过音频流获取所需要的解码器的信息 -> FFmpeg通过解码器的信息在FFmpeg中获取解码器 ->
 * 打开解码器 -> Java层创建AudioTrack -> AudioTrack调用Play-> 解码音频获得原生数据（PCM） ->
 * 原生数据（PCM）重采样 -> 将重采样后的数据转换为Java的byte[]（Java的byte[]对应JNI的jbyteArray） ->
 * AudioTrack调用Write写入byte[]
 */
class FFmpegAudioActivity : AppCompatActivity() {
    private var playThread:Thread? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_audio)
        findViewById<Button>(R.id.btn_play).setOnClickListener{
            if(playThread != null){
                playThread!!.interrupt()
                playThread = null
            }
            playThread = Thread{
                val file = File(application.getExternalFilesDir(null),"renjian.mp3")
                FFmpegTest.deCodeAudio(file.absolutePath)
            }
            playThread!!.start()
        }
    }
}