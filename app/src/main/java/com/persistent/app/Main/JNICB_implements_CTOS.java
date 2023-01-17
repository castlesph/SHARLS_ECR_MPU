package com.persistent.app.Main;

import android.annotation.SuppressLint;
import android.content.Context;
import android.util.Log;

public class JNICB_implements_CTOS implements JNICB_Lib.cb_CTOS {
    private MainActivity MainContext;
    private Context mContext;
    public JNICB_implements_CTOS(Context InContext)
    {
        mContext = InContext;
    }

    @SuppressLint("LongLogTag")
    public long moveTaskToBack_java()
    {
        Log.d("moveTaskToBack_java,", "End");

        MainContext.moveTaskToBack(true);
        return 0;
    }

}
