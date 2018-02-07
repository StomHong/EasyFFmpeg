package com.stomhong.easyffmpeg;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

public class AudioActivity extends AppCompatActivity {

    static {
        System.loadLibrary("easyffmpeg");
    }

    Button mPlay;
    Button mStop;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio);
        //创建引擎
        createEngine();

        mPlay = findViewById(R.id.btn_play);
        mStop = findViewById(R.id.btn_stop);
        mPlay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                play();
            }
        });
        mStop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                stop();
            }
        });
    }

    @Override
    protected void onDestroy() {
        destroy();
        super.onDestroy();
    }

    public  native void play();
    public  native int stop();
    public  native void createEngine();
    public native void destroy();
}
