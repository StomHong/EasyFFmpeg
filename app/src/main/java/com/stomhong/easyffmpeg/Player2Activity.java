package com.stomhong.easyffmpeg;

import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;

import com.stomhong.easyffmpeg.opengl.MyRenderer;

import java.io.File;

/**
 * 播放器页面 OpenGLES 渲染
 */
public class Player2Activity extends AppCompatActivity{

    private static final String TAG = Player2Activity.class.getSimpleName();

    //加载库文件
    static {
        System.loadLibrary("easyffmpeg");
    }

   private GLSurfaceView mGLSurfaceView;
    MyRenderer mRenderer;
    private Button mPlayBtn;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_player2);

        init();
    }

    private void init() {
        mPlayBtn = findViewById(R.id.btn_play);
        mGLSurfaceView = findViewById(R.id.glView);
        mGLSurfaceView.setEGLContextClientVersion(2);
        mGLSurfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
        mRenderer = new MyRenderer(this,mGLSurfaceView);
        mGLSurfaceView.setRenderer(mRenderer);
        mGLSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        mPlayBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String url = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "fengjing.mp4";
                Log.e(TAG,url);
                setDataResource(url);
            }
        });


    }

    @Override
    protected void onPause() {
        super.onPause();
//        mGLSurfaceView.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
//        mGLSurfaceView.onResume();
    }

    public void nativeCallback(byte[] dataY,int width,int height){
        Log.e("player","update.............");
        mRenderer.update(dataY,width,height);
    }


    public native void setDataResource(String url);

}
