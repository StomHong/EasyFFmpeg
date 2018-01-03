package com.stomhong.easyffmpeg;

import android.opengl.GLSurfaceView;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.QuickContactBadge;

import com.stomhong.easyffmpeg.opengl.MyRenderer;

import java.io.File;

/**
 * 播放器页面
 */
public class PlayerActivity extends AppCompatActivity {

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
        setContentView(R.layout.activity_player);

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
                setDataResource(url);
            }
        });


    }

    @Override
    protected void onPause() {
        super.onPause();
        mGLSurfaceView.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mGLSurfaceView.onResume();
    }

    public void nativeCallback(byte[] data){
        Log.e("player","update.............");
        mRenderer.update(data);
    }

    public void play(){
        Log.e("play","play========================================");

//        Bitmap bitmap = BitmapFactory.decodeByteArray(data,0,data.length);
//        mImageView.setImageBitmap(bitmap);
    }

    public native void setDataResource(String url);



}
