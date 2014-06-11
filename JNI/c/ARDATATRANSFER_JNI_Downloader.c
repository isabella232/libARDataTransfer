/**
 * @file ARDATATRANSFER_JNI_Downloader.c
 * @brief libARDataTransfer JNI_Downloader c file.
 * @date 09/06/2014
 * @author david.flattin.ext@parrot.com
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
#include <libARUtils/ARUTILS_Manager.h>

#include "libARDataTransfer/ARDATATRANSFER_Error.h"
#include "libARDataTransfer/ARDATATRANSFER_Manager.h"
#include "libARDataTransfer/ARDATATRANSFER_DataDownloader.h"
#include "libARDataTransfer/ARDATATRANSFER_MediasDownloader.h"
#include "libARDataTransfer/ARDATATRANSFER_Downloader.h"
#include "libARDataTransfer/ARDATATRANSFER_Uploader.h"

#include "ARDATATRANSFER_JNI.h"

#define ARDATATRANSFER_JNI_DOWNLOADER_TAG       "JNI"

jmethodID methodId_DListener_didDownloadProgress = NULL;
jmethodID methodId_DListener_didDownloadComplete = NULL;


JNIEXPORT jboolean JNICALL Java_com_parrot_arsdk_ardatatransfer_ARDataTransferDownloader_nativeStaticInit(JNIEnv *env, jclass jClass)
{
    jboolean jret = JNI_FALSE;
    int error = JNI_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_DOWNLOADER_TAG, "");

    if (env == NULL)
    {
        error = JNI_FAILED;
    }

    if (error == JNI_OK)
    {
        error = ARDATATRANSFER_JNI_Downloader_NewListenersJNI(env);
    }

    if (error == JNI_OK)
    {
        jret = JNI_TRUE;
    }

    return jret;
}

JNIEXPORT jint JNICALL Java_com_parrot_arsdk_ardatatransfer_ARDataTransferDownloader_nativeNew(JNIEnv *env, jobject jThis, jlong jManager, jlong jFtpManager, jstring jRemotePath, jstring jLocalPath, jobject jProgressListener, jobject jProgressArg, jobject jCompletionListener, jobject jCompletionArg, jint jResume)
{
    ARDATATRANSFER_Manager_t *nativeManager = (ARDATATRANSFER_Manager_t*)(intptr_t)jManager;
    ARUTILS_Manager_t *nativeFtpManager = (ARUTILS_Manager_t *)(intptr_t)jFtpManager;
    ARDATATRANSFER_JNI_DownloaderCallbacks_t *callbacks = NULL;
    const char *nativeRemotePath = (*env)->GetStringUTFChars(env, jRemotePath, 0);
    const char *nativeLocalPath = (*env)->GetStringUTFChars(env, jLocalPath, 0);
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    int error = JNI_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_DOWNLOADER_TAG, "%s, %s", nativeRemotePath ? nativeRemotePath : "null", nativeLocalPath ? nativeLocalPath : "null");

    error = ARDATATRANSFER_JNI_Downloader_NewListenersJNI(env);

    if (error == JNI_OK)
    {
        callbacks = calloc(1, sizeof(ARDATATRANSFER_JNI_DownloaderCallbacks_t));
        if (callbacks == NULL)
        {
            error = JNI_FAILED;
        }
    }

    if ((error == JNI_OK) && (jProgressListener != NULL))
    {
        callbacks->jProgressListener = (*env)->NewGlobalRef(env, jProgressListener);
        if (callbacks->jProgressListener == NULL)
        {
            error = JNI_FAILED;
        }
    }

    if ((error == JNI_OK) && (jProgressArg != NULL))
    {
        callbacks->jProgressArg = (*env)->NewGlobalRef(env, jProgressArg);
        if (callbacks->jProgressArg == NULL)
        {
            error = JNI_FAILED;
        }
    }

    if ((error == JNI_OK) && (jCompletionListener != NULL))
    {
        callbacks->jCompletionListener = (*env)->NewGlobalRef(env, jCompletionListener);
        if (callbacks->jCompletionListener == NULL)
        {
            error = JNI_FAILED;
        }
    }

    if ((error == JNI_OK) && (jCompletionArg != NULL))
    {
        callbacks->jCompletionArg = (*env)->NewGlobalRef(env, jCompletionArg);
        if (callbacks->jCompletionArg == NULL)
        {
            error = JNI_FAILED;
        }
    }

    if (error != JNI_OK)
    {
        result = ARDATATRANSFER_ERROR_ALLOC;
    }

    if (result == ARDATATRANSFER_OK)
    {
        result = ARDATATRANSFER_Downloader_New(nativeManager, nativeFtpManager, nativeRemotePath, nativeLocalPath, ARDATATRANSFER_JNI_Downloader_ProgressCallback, callbacks, ARDATATRANSFER_JNI_Downloader_CompletionCallback, callbacks, jResume);
    }

    //cleanup
    if (error != JNI_OK)
    {
        ARDATATRANSFER_JNI_Downloader_FreeDownloaderCallbacks(env, &callbacks);
    }

    if (nativeRemotePath != NULL)
    {
        (*env)->ReleaseStringUTFChars(env, jRemotePath, nativeRemotePath);
    }

    if (nativeLocalPath != NULL)
    {
        (*env)->ReleaseStringUTFChars(env, jLocalPath, nativeLocalPath);
    }

    return result;
}


JNIEXPORT jint JNICALL Java_com_parrot_arsdk_ardatatransfer_ARDataTransferDownloader_nativeDelete(JNIEnv *env, jobject jThis, jlong jManager)
{
    ARDATATRANSFER_Manager_t *nativeManager = (ARDATATRANSFER_Manager_t*)(intptr_t)jManager;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_DOWNLOADER_TAG, "");

    result = ARDATATRANSFER_Downloader_Delete(nativeManager);

    return result;
}

JNIEXPORT void JNICALL Java_com_parrot_arsdk_ardatatransfer_ARDataTransferDownloader_nativeThreadRun(JNIEnv *env, jobject jThis, jlong jManager)
{
    ARDATATRANSFER_Manager_t *nativeManager = (ARDATATRANSFER_Manager_t*)(intptr_t)jManager;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_DOWNLOADER_TAG, "");

    ARDATATRANSFER_Downloader_ThreadRun(nativeManager);

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_DOWNLOADER_TAG, "exiting");
}

JNIEXPORT jint JNICALL Java_com_parrot_arsdk_ardatatransfer_ARDataTransferDownloader_nativeCancelThread(JNIEnv *env, jobject jThis, jlong jManager)
{
    ARDATATRANSFER_Manager_t *nativeManager = (ARDATATRANSFER_Manager_t*)(intptr_t)jManager;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_DOWNLOADER_TAG, "");

    result = ARDATATRANSFER_Downloader_CancelThread(nativeManager);


    return result;
}

void ARDATATRANSFER_JNI_Downloader_ProgressCallback(void* arg, float percent)
{
    ARDATATRANSFER_JNI_DownloaderCallbacks_t *callbacks = (ARDATATRANSFER_JNI_DownloaderCallbacks_t*)arg;

    if (callbacks != NULL)
    {
        if ((ARDATATRANSFER_JNI_Manager_VM != NULL) && (callbacks->jProgressListener != NULL) && (methodId_DListener_didDownloadProgress != NULL))
        {
            JNIEnv *env = NULL;
            jfloat jPercent = 0;
            jint jResultEnv = 0;
            int error = JNI_OK;

            jResultEnv = (*ARDATATRANSFER_JNI_Manager_VM)->GetEnv(ARDATATRANSFER_JNI_Manager_VM, (void **) &env, JNI_VERSION_1_6);

            if (jResultEnv == JNI_EDETACHED)
            {
                 (*ARDATATRANSFER_JNI_Manager_VM)->AttachCurrentThread(ARDATATRANSFER_JNI_Manager_VM, &env, NULL);
            }

            if (env == NULL)
            {
                error = JNI_FAILED;
            }

            if ((error == JNI_OK) && (methodId_DListener_didDownloadProgress != NULL))
            {
                jPercent = percent;

                (*env)->CallVoidMethod(env, callbacks->jProgressListener, methodId_DListener_didDownloadProgress, callbacks->jProgressArg, jPercent);
            }

            if ((jResultEnv == JNI_EDETACHED) && (env != NULL))
            {
                 (*ARDATATRANSFER_JNI_Manager_VM)->DetachCurrentThread(ARDATATRANSFER_JNI_Manager_VM);
            }
        }
    }
}

void ARDATATRANSFER_JNI_Downloader_CompletionCallback(void* arg, eARDATATRANSFER_ERROR nativeError)
{
ARDATATRANSFER_JNI_DownloaderCallbacks_t *callbacks = (ARDATATRANSFER_JNI_DownloaderCallbacks_t*)arg;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_DOWNLOADER_TAG, "%x, %d", (int)arg, nativeError);

    if (callbacks != NULL)
    {
        if (ARDATATRANSFER_JNI_Manager_VM != NULL)
        {
            JNIEnv *env = NULL;
            jobject jError = NULL;
			jint jResultEnv = 0;

			jResultEnv = (*ARDATATRANSFER_JNI_Manager_VM)->GetEnv(ARDATATRANSFER_JNI_Manager_VM, (void **) &env, JNI_VERSION_1_6);

			if (jResultEnv == JNI_EDETACHED)
			{
				 (*ARDATATRANSFER_JNI_Manager_VM)->AttachCurrentThread(ARDATATRANSFER_JNI_Manager_VM, &env, NULL);
			}

			if (env == NULL)
			{
				//error = JNI_FAILED;
				ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_DOWNLOADER_TAG, "error no env");
			}

			if ((env != NULL) && (callbacks->jCompletionListener != NULL) && (methodId_DListener_didDownloadComplete != NULL))
			{
				int error = JNI_OK;


				if (error == JNI_OK)
				{
					jError = ARDATATRANSFER_JNI_Manager_NewERROR_ENUM(env, nativeError);

					if (jError == NULL)
					{
						error = JNI_FAILED;
						ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_DOWNLOADER_TAG, "error %d, %x", error, jError);
					}
				}

				if ((error == JNI_OK) && (methodId_DListener_didDownloadComplete != NULL))
				{
					 (*env)->CallVoidMethod(env, callbacks->jCompletionListener, methodId_DListener_didDownloadComplete, callbacks->jCompletionArg, jError);
				}
			}

			if (env != NULL)
			{
			    if (jError != NULL)
			    {
			        (*env)->DeleteLocalRef(env, jError);
			    }

				ARDATATRANSFER_JNI_Downloader_FreeDownloaderCallbacks(env, &callbacks);
			}

			if ((jResultEnv == JNI_EDETACHED) && (env != NULL))
			{
				(*ARDATATRANSFER_JNI_Manager_VM)->DetachCurrentThread(ARDATATRANSFER_JNI_Manager_VM);
			}
        }

		if (callbacks != NULL)
		{
        	free(callbacks);
        }
    }
}

void ARDATATRANSFER_JNI_Downloader_FreeDownloaderCallbacks(JNIEnv *env, ARDATATRANSFER_JNI_DownloaderCallbacks_t **callbacksParam)
{
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_DOWNLOADER_TAG, "%x", callbacksParam ? *callbacksParam : 0);

    if (callbacksParam != NULL)
    {
        ARDATATRANSFER_JNI_DownloaderCallbacks_t *callbacks = *callbacksParam;

        if (callbacks != NULL)
        {
            if (env != NULL)
            {
                if (callbacks->jProgressListener != NULL)
                {
                    (*env)->DeleteGlobalRef(env, callbacks->jProgressListener);
                }

                if (callbacks->jProgressArg != NULL)
                {
                    (*env)->DeleteGlobalRef(env, callbacks->jProgressArg);
                }

                if (callbacks->jCompletionListener != NULL)
                {
                    (*env)->DeleteGlobalRef(env, callbacks->jCompletionListener);
                }

                if (callbacks->jCompletionArg != NULL)
                {
                    (*env)->DeleteGlobalRef(env, callbacks->jCompletionArg);
                }
            }

            free(callbacks);
        }

        *callbacksParam = NULL;
    }
}


int ARDATATRANSFER_JNI_Downloader_NewListenersJNI(JNIEnv *env)
{
    jclass classDProgressListener = NULL;
    jclass classDCompletionListener = NULL;
    int error = JNI_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_DOWNLOADER_TAG, "");

    if (env == NULL)
    {
        error = JNI_FAILED;
    }

    if (methodId_DListener_didDownloadProgress == NULL)
    {
        if (error == JNI_OK)
        {
            classDProgressListener = (*env)->FindClass(env, "com/parrot/arsdk/ardatatransfer/ARDataTransferDownloaderProgressListener");

            if (classDProgressListener == NULL)
            {
                ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_DOWNLOADER_TAG, "ARDataTransferDownloaderProgressListener class not found");
                error = JNI_FAILED;
            }
        }

        if (error == JNI_OK)
        {
            methodId_DListener_didDownloadProgress = (*env)->GetMethodID(env, classDProgressListener, "didDownloadProgress", "(Ljava/lang/Object;F)V");

            if (methodId_DListener_didDownloadProgress == NULL)
            {
                ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_DOWNLOADER_TAG, "Listener didDownloadProgress method not found");
                error = JNI_FAILED;
            }
        }
    }

    if (methodId_DListener_didDownloadComplete == NULL)
    {
        if (error == JNI_OK)
        {
            classDCompletionListener = (*env)->FindClass(env, "com/parrot/arsdk/ardatatransfer/ARDataTransferDownloaderCompletionListener");

            if (classDCompletionListener == NULL)
            {
                ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_DOWNLOADER_TAG, "ARDataTransferDownloaderCompletionListener class not found");
                error = JNI_FAILED;
            }
        }

        if (error == JNI_OK)
        {
            methodId_DListener_didDownloadComplete = (*env)->GetMethodID(env, classDCompletionListener, "didDownloadComplete", "(Ljava/lang/Object;Lcom/parrot/arsdk/ardatatransfer/ARDATATRANSFER_ERROR_ENUM;)V");

            if (methodId_DListener_didDownloadComplete == NULL)
            {
                ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_DOWNLOADER_TAG, "Listener didDownloadComplete method not found");
                error = JNI_FAILED;
            }
        }
    }

    return error;
}

void ARDATATRANSFER_JNI_Downloader_FreeListenersJNI(JNIEnv *env)
{
    int error = JNI_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_DOWNLOADER_TAG, "");

    if (env == NULL)
    {
        error = JNI_FAILED;
    }

    if (error == JNI_OK)
    {
        methodId_DListener_didDownloadProgress = NULL;
        methodId_DListener_didDownloadComplete = NULL;
    }
}
