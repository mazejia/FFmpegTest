package com.mzj.ffmpegtest

import android.graphics.PixelFormat
import android.os.Bundle
import android.view.SurfaceView
import android.widget.Button
import androidx.appcompat.app.AppCompatActivity
import java.io.File

class MainActivity : AppCompatActivity() {
    private lateinit var surfaceView: SurfaceView
    private var playThread: Thread? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        surfaceView = findViewById(R.id.surfaceview)

        val holder = surfaceView.holder
        holder.setFormat(PixelFormat.RGBA_8888)

        findViewById<Button>(R.id.btn_play).setOnClickListener {
            if (playThread != null) {
                playThread!!.interrupt();
                playThread = null;
            }
            playThread = Thread {

                val input =
                    application.getExternalFilesDir(null)
                var file = File(input,"ffmpegtest.mp4")

                var surface = surfaceView.holder.surface
                FFmpegTest.deCodeVideo(file.absolutePath, surface);
            };
            playThread!!.start();
        }
    }
}