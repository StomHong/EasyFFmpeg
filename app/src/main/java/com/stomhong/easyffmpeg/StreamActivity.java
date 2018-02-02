package com.stomhong.easyffmpeg;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

public class StreamActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_stream);
    }

    public native void stream(String inputUrl,String outputUrl);
}
