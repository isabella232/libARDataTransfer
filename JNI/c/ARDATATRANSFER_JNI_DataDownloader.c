/**
 * @file ARDATATRANSFER_Manager.c
 * @brief libARDataTransfer JNI_DataDownloader c file.
 **/

#ifdef NDEBUG
/* Android ndk-build NDK_DEBUG=0*/
#else
/* Android ndk-build NDK_DEBUG=1*/
#ifndef DEBUG
#define DEBUG
#endif
#endif

#include <jni.h>
#include <inttypes.h>
#include <stdlib.h>

#include <libARSAL/ARSAL_Sem.h>
#include <libARSAL/ARSAL_Print.h>
#include <libARUtils/ARUTILS_Error.h>

#include "libARDataTransfer/ARDATATRANSFER_Error.h"
#include "libARDataTransfer/ARDATATRANSFER_Manager.h"
#include "libARDataTransfer/ARDATATRANSFER_DataDownloader.h"
#include "libARDataTransfer/ARDATATRANSFER_MediasDownloader.h"

#define ARDATATRANSFER_JNI_DATADOWNLOADER_TAG       "JNI"

JNIEXPORT jint JNICALL Java_com_parrot_arsdk_ardatatransfer_ARDataTransferDataDownloader_nativeNew(JNIEnv *env, jobject jThis, jlong jManager, jlong jUtilsManager, jstring jRemoteDirectory, jstring jLocalDirectory)
{
    ARDATATRANSFER_Manager_t *nativeManager = (ARDATATRANSFER_Manager_t*)(intptr_t)jManager;
    ARUTILS_Manager_t *nativeUtilsManager = (ARUTILS_Manager_t*)(intptr_t)jUtilsManager;
    const char *nativeRemoteDirectory = (*env)->GetStringUTFChars(env, jRemoteDirectory, 0);
    const char *nativeLocalDirectory = (*env)->GetStringUTFChars(env, jLocalDirectory, 0);
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_DATADOWNLOADER_TAG, "%s, %s", nativeRemoteDirectory ? nativeRemoteDirectory : "null", nativeLocalDirectory ? nativeLocalDirectory : "null");

    result = ARDATATRANSFER_DataDownloader_New(nativeManager, nativeUtilsManager, nativeRemoteDirectory, nativeLocalDirectory);

    //cleanup
    if (nativeRemoteDirectory != NULL)
    {
        (*env)->ReleaseStringUTFChars(env, jRemoteDirectory, nativeRemoteDirectory);
    }

    if (nativeLocalDirectory != NULL)
    {
        (*env)->ReleaseStringUTFChars(env, jLocalDirectory, nativeLocalDirectory);
    }

    return result;
}

JNIEXPORT jint JNICALL Java_com_parrot_arsdk_ardatatransfer_ARDataTransferDataDownloader_nativeDelete(JNIEnv *env, jobject jThis, jlong jManager)
{
    ARDATATRANSFER_Manager_t *nativeManager = (ARDATATRANSFER_Manager_t*)(intptr_t)jManager;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_DATADOWNLOADER_TAG, "");

    result = ARDATATRANSFER_DataDownloader_Delete(nativeManager);

    return result;
}

JNIEXPORT void JNICALL Java_com_parrot_arsdk_ardatatransfer_ARDataTransferDataDownloader_nativeThreadRun(JNIEnv *env, jobject jThis, jlong jManager)
{
    ARDATATRANSFER_Manager_t *nativeManager = (ARDATATRANSFER_Manager_t*)(intptr_t)jManager;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_DATADOWNLOADER_TAG, "");

    ARDATATRANSFER_DataDownloader_ThreadRun(nativeManager);

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_DATADOWNLOADER_TAG, "exit");
}

JNIEXPORT jint JNICALL Java_com_parrot_arsdk_ardatatransfer_ARDataTransferDataDownloader_nativeCancelThread(JNIEnv *env, jobject jThis, jlong jManager)
{
    ARDATATRANSFER_Manager_t *nativeManager = (ARDATATRANSFER_Manager_t*)(intptr_t)jManager;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_DATADOWNLOADER_TAG, "");

    result = ARDATATRANSFER_DataDownloader_CancelThread(nativeManager);

    return result;
}



