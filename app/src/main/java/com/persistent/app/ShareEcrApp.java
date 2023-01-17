package com.persistent.app;

import android.app.Application;
import android.content.Intent;
import android.util.Log;

public class ShareEcrApp extends Application {
    private static final String TAG = "ShareEcrApp";

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG, "onCreate: ");
        //Check Memory Leak
/*        if (LeakCanary.isInAnalyzerProcess(this)) {
            //此过程专用于LeakCanary进行堆分析。在此过程中不应初始化应用程序。
            Log.i(TAG, "isInAnalyzerProcess: ");
            return;
        }
        LeakCanary.install(this);*/

        if(true) {
            Log.i(TAG, "onCreate:May try to disable this,");
            Intent serviceIntent = new Intent(this, EcrService.class);
            serviceIntent.putExtra("ECR_INTENT", "ECR_START");
            startService(serviceIntent);
//            startForegroundService(serviceIntent);
            Log.i(TAG, "start ecr service done: ");
        }

        EcrCommandReceiver ecrCommandReceiver = new EcrCommandReceiver();
    }
}

