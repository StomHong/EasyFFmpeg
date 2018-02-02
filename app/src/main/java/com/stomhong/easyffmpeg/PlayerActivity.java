package com.stomhong.easyffmpeg;

import android.opengl.GLSurfaceView;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.QuickContactBadge;

import com.stomhong.easyffmpeg.opengl.MyRenderer;

import java.io.File;

/**
 * 播放器页面 ANativeWindow渲染
 */
public class PlayerActivity extends AppCompatActivity implements SurfaceHolder.Callback{

    private static final String TAG = PlayerActivity.class.getSimpleName();

    //加载库文件
    static {
        System.loadLibrary("easyffmpeg");
    }

   private SurfaceView mGLSurfaceView;
    private SurfaceHolder surfaceHolder;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_player);

        init();
    }

    private void init() {

        mGLSurfaceView = findViewById(R.id.glView);

        surfaceHolder = mGLSurfaceView.getHolder();
        surfaceHolder.addCallback(this);



    }



    public native void setDataResource(Object glSurfaceView);


    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                setDataResource(surfaceHolder.getSurface());
            }
        }).start();

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }
}
