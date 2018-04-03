package com.stomhong.easyffmpeg;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * 播放器页面 ANativeWindow渲染
 */
public class VideoANativeWindowActivity extends AppCompatActivity implements SurfaceHolder.Callback{

    private static final String TAG = VideoANativeWindowActivity.class.getSimpleName();

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
