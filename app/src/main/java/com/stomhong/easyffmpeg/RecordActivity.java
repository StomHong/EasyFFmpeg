package com.stomhong.easyffmpeg;

import android.content.Context;
import android.os.Environment;
import android.os.StatFs;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.format.Formatter;
import android.view.SurfaceView;
import android.widget.TextView;

import java.io.File;

public class RecordActivity extends AppCompatActivity {

    private SurfaceView mSurfaceView;
    private TextView textView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_record);

        init();
    }

    private void init() {
        mSurfaceView = findViewById(R.id.surfaceView);
        textView = findViewById(R.id.textView);
    }


}
