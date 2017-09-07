package com.stomhong.easyffmpeg;

import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

/**
 *
 */
public class MainActivity extends AppCompatActivity {

    //加载库文件
    static {
        System.loadLibrary("avutil-54");
        System.loadLibrary("swresample-1");
        System.loadLibrary("avcodec-56");
        System.loadLibrary("avformat-56");
        System.loadLibrary("swscale-3");
        System.loadLibrary("postproc-53");
        System.loadLibrary("avfilter-5");
        System.loadLibrary("avdevice-56");
        System.loadLibrary("simplest_ffmpeg_streamer");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);





        Button startButton = (Button) this.findViewById(R.id.button_start);
        final EditText urlEdittext_input = (EditText) this.findViewById(R.id.input_url);
        final EditText urlEdittext_output = (EditText) this.findViewById(R.id.output_url);

        startButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View arg0) {

                String folderurl = Environment.getExternalStorageDirectory().getPath();

                String urltext_input = urlEdittext_input.getText().toString();
                String inputurl = folderurl + "/" + urltext_input;

//		        String outputurl=urlEdittext_output.getText().toString();
                String outputurl = "";

                Log.e("inputurl", inputurl);
                Log.e("outputurl", outputurl);
                String info = "";

                stream(inputurl, outputurl);

                Log.e("Info", info);
            }
        });
    }

    /**
     * 推流
     * @param inputUrl 输入文件url
     * @param outputUrl 推流url
     * @return
     */
    public native int stream(String inputUrl, String outputUrl);

}
