package com.persistent.app;

import android.app.ActivityManager;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.IBinder;
import android.util.Log;


import com.persistent.app.Main.JNICB_implements_CTOS;
import com.persistent.app.Main.JNI_Lib;

import java.util.List;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

public class EcrService extends Service {
    private static final String ACTION = "com.persistent.app.RECEIVER";
    private static final boolean isDynamicBroadcast = false;
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    static {
        System.loadLibrary("ssl");
    }

    static {
        System.loadLibrary("crypto");
    }
    private JNI_Lib jni_api = new JNI_Lib();
    private JNICB_implements_CTOS jni_cb_ctos = new JNICB_implements_CTOS(this);

    private static final String TAG = "EcrService";
    private int mLoopCnt;
    private MsgReceiver                 msgReceiver = null;
    public ScheduledExecutorService mScheduledExecutorService = null;


    @Override
    public IBinder onBind(Intent intent) {
        Log.i(TAG, "onBind: ");
        return null;
    }

    @Override
    public void onCreate() {
        Log.i(TAG, "onCreate: ");
        super.onCreate();
    }

    private boolean isServiceRunning(final String className) {
        ActivityManager activityManager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningServiceInfo> info = activityManager.getRunningServices(Integer.MAX_VALUE);
        if (info == null || info.size() == 0) {
            return false;
        }
        for (ActivityManager.RunningServiceInfo aInfo : info) {
//            Log.i(TAG, "aInfo.service.getClassName(): " + aInfo.service.getClassName());
            if (className.equals(aInfo.service.getClassName())) {
                return true;
            }
        }
        return false;
    }
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i(TAG, "onStartCommand: ");
        int delaySeconds = 5;
        String strIntent = "";

//        StartForeground
        Log.i(TAG, "flags: " + flags + "  startId: " + startId + " intent is null: " + (intent == null));
        if(intent != null)
            strIntent = intent.getStringExtra("ECR_INTENT");
        Log.i(TAG, "getStringExtra: ");
        Log.i(TAG, "Intent from : " + strIntent);

        Log.i(TAG, "scheduleTaskStart1: " + (mScheduledExecutorService == null));
        Log.i(TAG, "delaySeconds: " + delaySeconds);


        //动态注册广播接收器
        if(isDynamicBroadcast) {
            if(msgReceiver == null) {
                Log.i(TAG, "Register ecr broadcast: ");
                msgReceiver = new MsgReceiver();
                IntentFilter intentFilter = new IntentFilter();
//            intentFilter.setPriority(Integer.MAX_VALUE);
                intentFilter.setPriority(IntentFilter.SYSTEM_HIGH_PRIORITY);
                intentFilter.addAction(ACTION);
                registerReceiver(msgReceiver, intentFilter);
            }
        }

        if(mScheduledExecutorService == null) {
            Log.i(TAG, "run mScheduledExecutorService: ");
            mScheduledExecutorService = new ScheduledThreadPoolExecutor(1);
            mScheduledExecutorService.scheduleAtFixedRate(new Runnable() {
                @Override
                public void run() {
                    try {
                        int TxnType = 0;
                        String strTxnType;
                        Log.i(TAG, "*** run ScheduledThreadPoolExecutor: " + mLoopCnt + " max " + Integer.MAX_VALUE + " ***");
                        jni_api.REG_CB_CTOS(jni_cb_ctos);
//                        if ((TxnType = inCTOSS_CheckEcrEvent()) > 0)
                        strTxnType = inCTOSS_CheckEcrEvent();
                        if(strTxnType != null && !strTxnType.isEmpty())
                        {
                            Log.i(TAG, "run ecr txn: do call payment app, " + "  mLoopCnt:" + mLoopCnt + " " + strTxnType);
                            sendCommandToMainApp(strTxnType);
                        }
                        mLoopCnt++;
                        if (mLoopCnt >= Integer.MAX_VALUE)
                            mLoopCnt = 0;
                        //60*5, every 5 minutes reset
                        if(isDynamicBroadcast) {
                            if(mLoopCnt % 300 == 0) {
                                Log.d(TAG, "unregisterReceiver: " + (msgReceiver != null));
                                if(msgReceiver != null) {
                                    Log.i(TAG, "unregisterReceiver: ");
                                    unregisterReceiver(msgReceiver);
                                    msgReceiver = null;

                                    {
                                        Log.i(TAG, "Re-Register ecr broadcast: ");
                                        msgReceiver = new MsgReceiver();
                                        IntentFilter intentFilter = new IntentFilter();

                                        intentFilter.setPriority(IntentFilter.SYSTEM_HIGH_PRIORITY);
                                        intentFilter.addAction(ACTION);
                                        registerReceiver(msgReceiver, intentFilter);
                                    }

                                }
                            }
                        }

                                    /*if(mLoopCnt % 20 == 0) {
                    Calendar calendar = Calendar.getInstance();
                    String transDateTime = String.format("%d%02d%02d%02d%02d", calendar.get(Calendar.YEAR), calendar.get(Calendar.MONTH) + 1, calendar.get(Calendar.DAY_OF_MONTH), calendar.get(Calendar.HOUR_OF_DAY), calendar.get(Calendar.MINUTE));
                    String transTime = String.format("%02d:%02d", calendar.get(Calendar.HOUR_OF_DAY), calendar.get(Calendar.MINUTE));

                    if(isServiceRunning("com.persistent.app.EcrService")) {
                        Log.i(TAG, "service is running: ");
                    }
                    Log.v(TAG, "loop Cnt=" + mLoopCnt + " transTime=" + transTime +  " transDateTime=" + transDateTime);
                    Log.d(TAG, "sendBroadcast to other app: ");
                    sendCommandToMainApp();
                }*/
                    } catch (Exception e) {
                        Log.e(TAG, e.getMessage());
                        e.printStackTrace();
                    } finally {
//                        Log.i(TAG, "finally: ");
                    }
                }
            }, delaySeconds, 1, TimeUnit.SECONDS);
        }


/*        new Thread(new Runnable() {
            @Override
            public void run() {
                while(true) {
                    Calendar calendar = Calendar.getInstance();
                    String transDateTime = String.format("%d%02d%02d%02d%02d", calendar.get(Calendar.YEAR), calendar.get(Calendar.MONTH) + 1, calendar.get(Calendar.DAY_OF_MONTH), calendar.get(Calendar.HOUR_OF_DAY), calendar.get(Calendar.MINUTE));
                    String transTime = String.format("%02d:%02d", calendar.get(Calendar.HOUR_OF_DAY), calendar.get(Calendar.MINUTE));

//                    Log.v(TAG, "loop Cnt=" + mLoopCnt + " transTime=" + transTime + " S=" +  " transDateTime=" + transDateTime);
                    if(mLoopCnt % 400 == 0) {
                        if(isServiceRunning("com.persistent.app.EcrService")) {
                            Log.i(TAG, "service is running: ");
                        }
                        Log.v(TAG, "loop Cnt=" + mLoopCnt + " transTime=" + transTime +  " transDateTime=" + transDateTime);
                        Log.d(TAG, "sendBroadcast to other app: ");
                        sendCommandToMainApp();
                        Log.i(TAG, "sendBroadcast to other app done: ");
                    }
                    mLoopCnt++;
                    if(mLoopCnt > 10000)
                        mLoopCnt = 0;

                    try {
                        Thread.sleep(10);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        }).start();*/

//        return super.onStartCommand(intent, flags, startId);
        //If service is killed while starting, it restarts.
        Log.i(TAG, "exit onStartCommand: ");
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        Log.e(TAG, "onDestroy: ");
        super.onDestroy();
        if(mScheduledExecutorService != null) {
            Log.d(TAG, "shutdown schedule exec: ");
            mScheduledExecutorService.shutdown(); 
        }
        if(msgReceiver != null) {
            Log.i(TAG, "unregisterReceiver: ");
            unregisterReceiver(msgReceiver);
        }
    }

    private void sendCommandToMainApp (String txnType) {
        String Txn = "";
        String PaymentType= "";
        String[] txninfo = txnType.split("\\|");
        int cnt = txninfo.length;
        if(cnt > 0)
            Txn = txninfo[0];
        if(cnt > 1)
            PaymentType = txninfo[1];

        //自定义一个action
        String receiver = EcrCommandDefintion.ECR_BROADCAST_ACTION;
        Intent intent = new Intent(receiver);
		intent.addFlags(0x01000000);
//        if(txnType == 1)
        if(Txn != null && Txn.equals(EcrCommandDefintion.ECR_REQ_CMD_SALE))
        {
            intent.putExtra(EcrCommandDefintion.ECR_REQ_CMD_NAME, Txn);
            intent.putExtra(EcrCommandDefintion.ECR_REQ_PAYMENT_TYPE, PaymentType);
        } else if(Txn != null && Txn.equals(EcrCommandDefintion.ECR_REQ_CMD_VOID)){
            intent.putExtra(EcrCommandDefintion.ECR_REQ_CMD_NAME, Txn);
            intent.putExtra(EcrCommandDefintion.ECR_REQ_PAYMENT_TYPE, PaymentType);
        } else if (Txn != null && Txn.equals(EcrCommandDefintion.ECR_REQ_CMD_REFUND)) {
            intent.putExtra(EcrCommandDefintion.ECR_REQ_CMD_NAME, Txn);
            intent.putExtra(EcrCommandDefintion.ECR_REQ_PAYMENT_TYPE, PaymentType);
        } else if (Txn != null && Txn.equals(EcrCommandDefintion.ECR_REQ_CMD_SETTLE_ALL)){
            intent.putExtra(EcrCommandDefintion.ECR_REQ_CMD_NAME, Txn);
            intent.putExtra(EcrCommandDefintion.ECR_REQ_PAYMENT_TYPE, PaymentType);
        } else if (Txn != null && Txn.equals(EcrCommandDefintion.ECR_REQ_CMD_SETTLE)) {
            intent.putExtra(EcrCommandDefintion.ECR_REQ_CMD_NAME, Txn);
            intent.putExtra(EcrCommandDefintion.ECR_REQ_PAYMENT_TYPE, PaymentType);
        }else if (Txn != null && Txn.equals(EcrCommandDefintion.ECR_REQ_CMD_TIPS)) {
            intent.putExtra(EcrCommandDefintion.ECR_REQ_CMD_NAME, Txn);
            intent.putExtra(EcrCommandDefintion.ECR_REQ_PAYMENT_TYPE, PaymentType);
        }else if (Txn != null && Txn.equals(EcrCommandDefintion.ECR_REQ_CMD_RETRIEVAL)) {
            intent.putExtra(EcrCommandDefintion.ECR_REQ_CMD_NAME, Txn);
            intent.putExtra(EcrCommandDefintion.ECR_REQ_PAYMENT_TYPE, PaymentType);
        }else if (Txn != null && Txn.equals(EcrCommandDefintion.ECR_REQ_CMD_BATCH_TOTAL)) {
            intent.putExtra(EcrCommandDefintion.ECR_REQ_CMD_NAME, Txn);
            intent.putExtra(EcrCommandDefintion.ECR_REQ_PAYMENT_TYPE, PaymentType);
        }else if (Txn != null && Txn.equals(EcrCommandDefintion.ECR_REQ_CMD_REPRINT_SETTLE)) {
            intent.putExtra(EcrCommandDefintion.ECR_REQ_CMD_NAME, Txn);
            intent.putExtra(EcrCommandDefintion.ECR_REQ_PAYMENT_TYPE, PaymentType);
        }else if (Txn != null && Txn.equals(EcrCommandDefintion.ECR_REQ_CMD_PREAUTH)) {
            intent.putExtra(EcrCommandDefintion.ECR_REQ_CMD_NAME, Txn);
            intent.putExtra(EcrCommandDefintion.ECR_REQ_PAYMENT_TYPE, PaymentType);
        }else if (Txn != null && Txn.equals(EcrCommandDefintion.ECR_REQ_CMD_PREAUTH_COMP)) {
            intent.putExtra(EcrCommandDefintion.ECR_REQ_CMD_NAME, Txn);
            intent.putExtra(EcrCommandDefintion.ECR_REQ_PAYMENT_TYPE, PaymentType);
        }else if (Txn != null && Txn.equals(EcrCommandDefintion.ECR_REQ_CMD_REPRINTLAST)) {
            intent.putExtra(EcrCommandDefintion.ECR_REQ_CMD_NAME, Txn);
            intent.putExtra(EcrCommandDefintion.ECR_REQ_PAYMENT_TYPE, PaymentType);
        }else if (Txn != null && Txn.equals(EcrCommandDefintion.ECR_REQ_CMD_REPRINTANY)) {
            intent.putExtra(EcrCommandDefintion.ECR_REQ_CMD_NAME, Txn);
            intent.putExtra(EcrCommandDefintion.ECR_REQ_PAYMENT_TYPE, PaymentType);
        }else if (Txn != null && Txn.equals(EcrCommandDefintion.ECR_REQ_CMD_INSTALLMENT)) {
            intent.putExtra(EcrCommandDefintion.ECR_REQ_CMD_NAME, Txn);
            intent.putExtra(EcrCommandDefintion.ECR_REQ_PAYMENT_TYPE, PaymentType);
        }
		else if (Txn != null && Txn.equals(EcrCommandDefintion.ECR_REQ_CMD_ECHOTEST)) 
		{
            intent.putExtra(EcrCommandDefintion.ECR_REQ_CMD_NAME, Txn);
            intent.putExtra(EcrCommandDefintion.ECR_REQ_PAYMENT_TYPE, PaymentType);
        }
		else if (Txn != null && Txn.equals(EcrCommandDefintion.ECR_REQ_CMD_GET_TOTAL)) 
		{
            intent.putExtra(EcrCommandDefintion.ECR_REQ_CMD_NAME, Txn);
            intent.putExtra(EcrCommandDefintion.ECR_REQ_PAYMENT_TYPE, PaymentType);
        }
		else if (Txn != null && Txn.equals(EcrCommandDefintion.ECR_REQ_CMD_GET_LAST_TOTAL)) 
		{
            intent.putExtra(EcrCommandDefintion.ECR_REQ_CMD_NAME, Txn);
            intent.putExtra(EcrCommandDefintion.ECR_REQ_PAYMENT_TYPE, PaymentType);
        }
		else if (Txn != null && Txn.equals(EcrCommandDefintion.ECR_REQ_CMD_BARCODE)) 
		{
            intent.putExtra(EcrCommandDefintion.ECR_REQ_CMD_NAME, Txn);
            intent.putExtra(EcrCommandDefintion.ECR_REQ_PAYMENT_TYPE, PaymentType);
        }
		else if (Txn != null && Txn.equals(EcrCommandDefintion.ECR_REQ_CMD_PRINTING)) 
		{
            intent.putExtra(EcrCommandDefintion.ECR_REQ_CMD_NAME, Txn);
            intent.putExtra(EcrCommandDefintion.ECR_REQ_PAYMENT_TYPE, PaymentType);
        }
		else if (Txn != null && Txn.equals(EcrCommandDefintion.ECR_REQ_CMD_READCARD)) 
		{
            intent.putExtra(EcrCommandDefintion.ECR_REQ_CMD_NAME, Txn);
            intent.putExtra(EcrCommandDefintion.ECR_REQ_PAYMENT_TYPE, PaymentType);
        }
        else if (Txn != null && Txn.equals(EcrCommandDefintion.ECR_REQ_CMD_VOID_PREAUTH)) 
		{
            intent.putExtra(EcrCommandDefintion.ECR_REQ_CMD_NAME, Txn);
            intent.putExtra(EcrCommandDefintion.ECR_REQ_PAYMENT_TYPE, PaymentType);
        }

        sendBroadcast(intent);
    }

    class MsgReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            Log.i(TAG, "onReceive: to send ecr resp");
            if (intent.hasExtra("ECR_RESP")) {
                Log.i(TAG, "ecr resp msg from payment app: ");
                inCTOSS_SendEcrResponse();

            }
        }
    }

    public native String inCTOSS_CheckEcrEvent();
    public native int inCTOSS_SendEcrResponse();

}

