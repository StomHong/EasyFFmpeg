package com.stomhong.easyffmpeg.opengl;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

/**
 * Created by StomHong on 2017/12/26.
 */

public class GLView extends GLSurfaceView {

    GLFrameRender glFrameRender;

    public GLView(Context context) {
        super(context);
    }

    public GLView(Context context, AttributeSet attrs) {
        super(context, attrs);
        try {
            //需要指定version
            setEGLContextClientVersion(3);

            glFrameRender = new GLFrameRender();
            //在构建的GLSurfaceView 构造中就绑定一个renderer
            setRenderer(glFrameRender);
            //设置上面绑定renderer的绘图模式
            setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        } catch (Exception E) {
            E.printStackTrace();
        }
    }


}

