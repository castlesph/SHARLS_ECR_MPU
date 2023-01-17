package com.persistent.app;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.util.Log;

public class BootUpReceiver extends BroadcastReceiver {
    private static final String TAG = "BootUpReceiver";

    public void onReceive(Context context, Intent intent) {
        Log.i(TAG, "onReceive: ");
        if (intent.getAction().equals("android.intent.action.BOOT_COMPLETED")) {
            Log.i(TAG, "onReceive: BOOT_COMPLETED");
            //Toast.makeText(context, "ECR BOOT_COMPLETED", Toast.LENGTH_LONG).show();
        }
        Intent serviceIntent = new Intent(context, EcrService.class);
        serviceIntent.putExtra("ECR_INTENT", "ECR_START");
        context.startService(serviceIntent);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
//            context.startForegroundService(serviceIntent);
        }
    }
}
