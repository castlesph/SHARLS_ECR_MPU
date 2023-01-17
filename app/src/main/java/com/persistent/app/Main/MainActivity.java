
package com.persistent.app.Main;

import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.util.Log;

import com.persistent.app.EcrService;
import com.persistent.app.model.DemoAppActivity;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Method;
import java.util.List;

public class MainActivity extends DemoAppActivity{
    private static final String TAG = "MainActivity";

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

    public int chmod(File path, int mode) throws Exception {
        Class fileUtils = Class.forName("android.os.FileUtils");
        Method setPermissions =
                fileUtils.getMethod("setPermissions", String.class, int.class, int.class, int.class);
        return (Integer) setPermissions.invoke(null, path.getAbsolutePath(), mode, -1, -1);
    }

    private void copyFile(String filename) {
        AssetManager assetManager = this.getAssets();

        InputStream in = null;
        OutputStream out = null;
        String newFileName = null;

        String strFilename = filename.substring(filename.lastIndexOf("/") + 1);
        Log.i("Castles", "strFilename() " + strFilename);

        try {
            Log.i("Castles", "copyFile() " + filename);

            if (filename.toLowerCase().startsWith("21"))
            {
                newFileName = "/data/data/pub/" + strFilename;
            }
            else if (filename.toLowerCase().startsWith("55"))
            {
                newFileName = "/data/data/pub/" + strFilename;
            }
            else if (filename.toLowerCase().startsWith("56"))
            {
                newFileName = "/data/data/pub/" + strFilename;
            }
            else
            {
                newFileName = "/data/data/" + this.getPackageName() + "/" + strFilename;
            }

            Log.i("Castles", "copyFile() Path " + newFileName);

            File dbFile = new File(newFileName);
            if (!dbFile.exists())
            {
                Log.i("Castles", "copyFile() !exists");
                in = assetManager.open(filename);
                out = new FileOutputStream(newFileName);

                byte[] buffer = new byte[1024];
                int read;
                while ((read = in.read(buffer)) != -1) {
                    out.write(buffer, 0, read);
                }
                in.close();
                in = null;
                out.flush();
                out.close();
                out = null;
                chmod(new File(newFileName), 0777);
            }
        } catch (Exception e) {
            Log.e("Castles", "Exception in copyFile() of " + newFileName + e.toString());
        }
    }

    public void copyFileOrDir(String path) {
        AssetManager assetManager = this.getAssets();
        String assets[] = null;
        try {
            Log.i("CastlesCastles", "copyFileOrDir() " + path);
            assets = assetManager.list(path);
            if (assets.length == 0) {
                copyFile(path);
            } else {
                for (int i = 0; i < assets.length; ++i) {
                    String p;
                    if (path.equals(""))
                        p = "";
                    else
                        p = path + "/";

                    if (!path.startsWith("images") && !path.startsWith("sounds") && !path.startsWith("webkit"))
                        copyFileOrDir(p + assets[i]);
                }
            }
        } catch (IOException ex) {
            Log.e("Castles", "I/O Exception", ex);
        }
    }
    /**
     * is Service Running
     */
    private boolean isServiceRunning(final String className) {
        ActivityManager activityManager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningServiceInfo> info = activityManager.getRunningServices(Integer.MAX_VALUE);
        if (info == null || info.size() == 0) {
            return false;
        }
        for (ActivityManager.RunningServiceInfo aInfo : info) {
//            Log.i(TAG, "aInfo.service.getClassName(): " + aInfo.service.getClassName());
            if (className.equals(aInfo.service.getClassName())) {
                Log.i(TAG, "ServiceRunning: " + className);
                return true;
            }
        }
        return false;
    }


    public void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "onCreate: ");
        super.onCreate(savedInstanceState);

        overridePendingTransition(0, 0);

        SharedPreferences wmbPreference = PreferenceManager.getDefaultSharedPreferences(this);

        boolean isFirstRun = wmbPreference.getBoolean("FIRSTRUN", true);

        if (isFirstRun)
        {
            // copy file or Dir
            copyFileOrDir("");

            SharedPreferences.Editor editor = wmbPreference.edit();
            editor.putBoolean("FIRSTRUN", false);
            editor.commit();
        }else{
            //what you do everytime goes here
        }

		Log.d("inCTOSS_SubAPMain", "Begin");
        jni_api.REG_CB_CTOS(jni_cb_ctos);

        //Start: Add Ecr Service
        Log.i(TAG, "start ecr service: ");
        if (isServiceRunning("com.persistent.app.EcrService")){
            Log.i(TAG, "Ecr service already running...");
        } else {
//            Intent intent = new Intent(this, EcrService.class);
            Intent serviceIntent = new Intent(this, EcrService.class);
            serviceIntent.putExtra("ECR_INTENT", "ECR_START");
            startService(serviceIntent);
//            startForegroundService(serviceIntent);
            Log.i(TAG, "start ecr service done: ");
        }
        //end
        Log.i(TAG, "Call inCTOSS_SubAPMain: ");
        //inside inCTOSS_SubAPMain got while loop, wait ipc oommand, inMultiAP_GetMainroutine


        inCTOSS_SubAPMain();
        Log.i(TAG, "Call inCTOSS_SubAPMain done: ");
        finish();
        //Intent intent = new Intent();
        //Bundle bundle = new Bundle();
        //intent.setClass(MainActivity.this, MultiAP_SubAP.class);
        //intent.putExtras(bundle);
        //startActivity(intent);
        //finish();
        };


    @Override
    protected void onRestart() {
        Log.d(TAG, "onRestart()");
        super.onRestart();
    }

    @Override
    protected void onStart() {
        Log.d(TAG, "onStart()");
        super.onStart();
    }

    @Override
    protected void onResume() {
        Log.d(TAG, "onResume()");
        super.onResume();
    }

    @Override
    protected void onPause() {
        Log.d(TAG, "onPause()");
        super.onPause();
    }

    @Override
    protected void onStop() {
        Log.d(TAG, "onStop()");
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy()");
        super.onDestroy();
    }
    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native int inCTOSS_SubAPMain();

};
