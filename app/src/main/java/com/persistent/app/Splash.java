package com.persistent.app;


import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;

import com.persistent.app.Main.MainActivity;
import com.persistent.app.model.DemoAppActivity;


public class Splash extends DemoAppActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO 自动生成的方法存根
        super.onCreate(savedInstanceState);

        new Handler().postDelayed(new Runnable() {
            public void run() {
                Intent i = new Intent(Splash.this, MainActivity.class);
                startActivity(i);
                finish();
//                new Thread(new Runnable() {
//                    @Override
//                    public void run() {
//                        try {
//                            Thread.sleep(1000);
//                            finish();
//                        } catch (Exception e) {
//                            e.printStackTrace();
//                        }
//                    }
//                }).start();
            }
        }, 2000);   //5秒
    }


}