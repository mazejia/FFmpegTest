package com.mzj.ffmpegtest

import android.content.Intent
import android.os.Bundle
import android.widget.Button
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        findViewById<Button>(R.id.btn_video).setOnClickListener {
            val intent = Intent(this,FFmpegVideoActivity::class.java)
            startActivity(intent)
        }

        findViewById<Button>(R.id.btn_audio).setOnClickListener {
            startActivity(Intent(this,FFmpegAudioActivity::class.java))
        }
    }
}