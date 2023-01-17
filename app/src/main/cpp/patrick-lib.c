//
// Created by patrik on 6/3/2018.
//

#include "patrick-lib.h"
#include "sqlite3.h"
#include "Debug/debug.h"
#include "Includes/myFileFunc.h"
#include "Includes/DatabaseFunc.h"
#include <ctosapi.h>
#include <vegaapi.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <android_jni_log.h>
//#include "./Main/appmain.h"

sqlite3 * db;
sqlite3_stmt *stmt;

static JavaVM *g_JavaVM;
jobject g_callback_obj_ctos;
jmethodID g_callback_moveTaskToBack;
JavaVM *g_vm;
extern BOOL isEcrProcessing;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    JNIEnv *env;
    g_vm = vm;
    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR; // JNI version not supported.
    }

    return JNI_VERSION_1_6;
}

JNIEXPORT jint JNICALL
Java_com_persistent_app_Main_MainActivity_inCTOSS_1SubAPMain(JNIEnv *env,
																			   jobject instance) {
	int inRetVal = 0;

	inTCTRead(1);
	strTCT.byRS232DebugPort = 8;

	inRetVal = main(0, NULL);

    return 0;
}

/*
JNIEXPORT jint JNICALL
Java_com_persistent_app_Main_MultiAP_1SubAP_inCTOSS_1SubAPMain(JNIEnv *env,
																				 jobject instance) {

	int inRetVal = 0;

	inTCTRead(1);
	strTCT.byRS232DebugPort = 8;

	inRetVal = main(0, NULL);

}
*/

JNIEXPORT jlong JNICALL
Java_com_persistent_app_Main_JNI_1Lib_REG_1CB_1CTOS(
        JNIEnv *env, jobject obj, jobject instance)
{
    int Rtn = 0;

//    vdDebug_LogPrintf("GetJavaVM");
    (*env)->GetJavaVM(env, &g_JavaVM);
//    vdDebug_LogPrintf("NewGlobalRef");
    if(g_callback_obj_ctos != NULL)
        (*env)->DeleteGlobalRef(env, g_callback_obj_ctos);

    g_callback_obj_ctos = (*env)->NewGlobalRef(env,instance);
//    vdDebug_LogPrintf("GetObjectClass");
    jclass clz = (*env)->GetObjectClass(env,g_callback_obj_ctos);

    if(clz == NULL)
    {
        //failed to find class
    }

//    vdDebug_LogPrintf("GetMethodID");
    g_callback_moveTaskToBack = (*env)->GetMethodID(env, clz, "moveTaskToBack_java", "()J");
//    vdDebug_LogPrintf("GetMethodID END");

//    vdDebug_LogPrintf("JNI_1Lib_REG_1CB_1CTOS");
    return Rtn;
}

LONG moveTaskToBack_java(void)
{
    JNIEnv *tenv;
    LOGD("---JNI AP CB--- moveTaskToBack_java\n");
    if((*g_JavaVM)->AttachCurrentThread(g_JavaVM, &tenv, NULL) != JNI_OK)
    {
        // attachCurrentThread() failed.
    }
    else {
        (*tenv)->CallLongMethod(tenv, g_callback_obj_ctos, g_callback_moveTaskToBack);
    }
    return 0;
}


//From Ecr service(EcrService), it will keep calling this function to check ecr event
JNIEXPORT jstring JNICALL
Java_com_persistent_app_EcrService_inCTOSS_1CheckEcrEvent(JNIEnv *env,jobject instance) {
    int inRetVal = 0;

    inTCTRead(1);
//    char * str = (*env)->GetStringUTFChars( env, instance, NULL );
//    char *str = env->GetStringUTFChars(PaymentType, 0);
    // Step 1: Convert the JNI String (jstring) into C-String (char*)
//    const char *inCStr = (*env)->GetStringUTFChars(env, inJNIStr, NULL);

    // Step 2: Perform its intended operations
//    (*env)->ReleaseStringUTFChars(env, inJNIStr, inCStr);  // release resources


// Step 3: Convert the C-string (char*) into JNI String (jstring) and return
    char outCStr[128];
    char szPaymentType[128];
//    (*env)->NewStringUTF(env, outCStr);
//    vdDebug_LogPrintf("check ecr, isEcrProcessing=%d，PaymentType=[%s]", isEcrProcessing, inCStr);
    memset(szPaymentType, 0, sizeof(szPaymentType));
    memset(outCStr, 0, sizeof(outCStr));
    inRetVal = inCTOSS_CheckECREventEx(szPaymentType);
    vdDebug_LogPrintf("inCTOSS_1CheckEcrEvent inRetVal=%d,szPaymentType=[%s]", inRetVal, szPaymentType);
	
    if(inRetVal > 0)
    {
        isEcrProcessing = TRUE;
        return (*env)->NewStringUTF(env, szPaymentType);
//        strcpy(inCStr, "VM");
    } else
    {
        return NULL;
    }

}

JNIEXPORT jint JNICALL
Java_com_persistent_app_EcrCommandReceiver_inCTOSS_1SendEcrResponse(JNIEnv *env,
                                                                          jobject instance) {
    int inRetVal = 0;
    int usResult;
    vdDebug_LogPrintf("send ecr resp, read db first");
	usResult = inCTOSS_MultiAPGetData();

    vdDebug_LogPrintf("Resp PAN:%s",srTransRec.szPAN);
    vdDebug_LogPrintf("Resp DE39:%s",srTransRec.szRespCode);
    vdDebug_LogPrintf("Resp RREF:%s",srTransRec.szRRN);
    vdDebug_LogPrintf("Resp ExpDate:[%02X%02X]",srTransRec.szExpireDate[0], srTransRec.szExpireDate[1]);
    vdDebug_LogPrintf("Resp MID:%s",srTransRec.szTID);
    vdDebug_LogPrintf("Resp TID:%s",srTransRec.szMID);
    vdDebug_LogPrintf("Inv Num:[%02X%02X%02X]",srTransRec.szInvoiceNo[0], srTransRec.szInvoiceNo[1], srTransRec.szInvoiceNo[2]);

    //inECRFillResponse();
    //inECRSendResponse();
    vdDebug_LogPrintf("inCTOSS_1SendEcrResponse inRetVal=%d", inRetVal);

	inECRSendAnalyse();
    return inRetVal;
}

