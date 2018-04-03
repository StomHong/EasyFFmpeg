package com.stomhong.easyffmpeg;


import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;

public class AudioAudioTrackActivity extends AppCompatActivity {

    static {
        System.loadLibrary("easyffmpeg");
    }
    AudioTrack trackplayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio2);
        init();
    }

    private void init() {
        //根据采样率，采样精度，单双声道来得到frame的大小。
        int bufsize = AudioTrack.getMinBufferSize(80000,
                AudioFormat.CHANNEL_OUT_STEREO,
                AudioFormat.ENCODING_PCM_16BIT);

//注意，按照数字音频的知识，这个算出来的是一秒钟buffer的大小。
//创建AudioTrack

        trackplayer = new AudioTrack(AudioManager.STREAM_MUSIC, 80000,
                AudioFormat.CHANNEL_OUT_STEREO,
                AudioFormat.ENCODING_PCM_16BIT,
                bufsize,
                AudioTrack.MODE_STREAM);
        findViewById(R.id.btn_play).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                start();
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();

    }

    public native void start();

    public void play(byte[] data){
        trackplayer.play() ;//开始
        //往track中写数据
        trackplayer.write(data, 0, data.length) ;
    }


}
