package com.persistent.app;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class EcrCommandReceiver extends BroadcastReceiver {
    private static final String TAG = "EcrCommandReceiver";
    private static boolean isSetInstance = false;
    static {
        System.loadLibrary("crypto");
    }

    static {
        System.loadLibrary("ssl");
    }

    static {
        System.loadLibrary("native-lib");
    }




    @Override
    public void onReceive(Context context, Intent intent) {
        Log.i(TAG, "onReceive: to send ecr resp");
        if(!isSetInstance) {
//            vdEcrSetupInstance();
            isSetInstance = true;
        }
        if (intent.hasExtra("ECR_RESP")) {
            Log.i(TAG, "ecr resp msg from payment app: ");
            inCTOSS_SendEcrResponse();

        }
    }

    public native int inCTOSS_SendEcrResponse();
    public native int vdEcrSetupInstance();
}
