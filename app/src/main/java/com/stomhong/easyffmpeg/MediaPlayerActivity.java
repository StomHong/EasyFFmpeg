package com.stomhong.easyffmpeg;

import android.content.Context;
import android.media.AudioAttributes;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.provider.MediaStore;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

import java.io.IOException;
import java.security.spec.RSAPrivateKeySpec;

public class MediaPlayerActivity extends AppCompatActivity {


    MediaPlayer mediaPlayer;
    private Context context;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_media_player);
        context = this;
        init();
    }

    private void init(){


    }

    @Override
    protected void onResume() {
        super.onResume();
        try {
//            mediaPlayer = new MediaPlayer();
            mediaPlayer = MediaPlayer.create(this,R.raw.baidu_beep);
//            mediaPlayer.setDataSource("/sdcard/fengjing.mp3");
            mediaPlayer.prepare();
            mediaPlayer.setLooping(true);
            mediaPlayer.start();
        } catch (IOException e) {
            e.printStackTrace();
        }

    }

    @Override
    protected void onDestroy() {
        mediaPlayer.release();
        mediaPlayer = null;
        super.onDestroy();
    }
}
