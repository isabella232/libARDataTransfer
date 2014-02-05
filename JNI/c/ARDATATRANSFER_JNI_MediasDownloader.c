/**
 * @file ARDATATRANSFER_Manager.c
 * @brief libARDataTransfer JNI_MediasDownloader c file.
 * @date 19/12/2013
 * @author david.flattin.ext@parrot.com
 **/

#ifdef NDEBUG
/* Android ndk-build NDK_DEBUG=0 */
#else
/* Android ndk-build NDK_DEBUG=1 */
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

#include "ARDATATRANSFER_JNI.h"

#define ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG       "JNI"

jmethodID methodId_List_add = NULL;

jclass classMDMedia = NULL;
jmethodID methodId_MDMedia_init = NULL;
jmethodID methodId_MDMedia_getProductValue = NULL;
jmethodID methodId_MDMedia_getName = NULL;
jmethodID methodId_MDMedia_getFileName = NULL;
jmethodID methodId_MDMedia_getDate = NULL;
jmethodID methodId_MDMedia_getSize = NULL;
jmethodID methodId_MDMedia_getThumbnail = NULL;

jmethodID methodId_MDListener_didMediaProgress = NULL;
jmethodID methodId_MDListener_didMediaComplete = NULL;
jmethodID methodId_MDListener_didMediaAvailable = NULL;

JNIEXPORT jboolean JNICALL Java_com_parrot_arsdk_ardatatransfer_ARDataTransferMediasDownloader_nativeStaticInit(JNIEnv *env, jclass jClass)
{
    jboolean jret = JNI_FALSE;
    int error = JNI_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "");

    if (env == NULL)
    {
        error = JNI_FAILED;
    }

    if (error == JNI_OK)
    {
        error = ARDATATRANSFER_JNI_MediasDownloader_NewListenersJNI(env);
    }

    if (error == JNI_OK)
    {
        error = ARDATATRANSFER_JNI_MediasDownloader_NewListJNI(env);
    }

    if (error == JNI_OK)
    {
        error = ARDATATRANSFER_JNI_MediasDownloader_NewMediaJNI(env);
    }

    if (error == JNI_OK)
    {
        jret = JNI_TRUE;
    }

    return jret;
}

JNIEXPORT jint JNICALL Java_com_parrot_arsdk_ardatatransfer_ARDataTransferMediasDownloader_nativeNew(JNIEnv *env, jobject jThis, jlong jManager, jstring jDeviceIP, jint jPort, jstring jLocalDirectory)
{
    ARDATATRANSFER_Manager_t *nativeManager = (ARDATATRANSFER_Manager_t*)(intptr_t)jManager;
    const char *nativeDeviceIP = (*env)->GetStringUTFChars(env, jDeviceIP, 0);
    const char *nativeLocalDirectory = (*env)->GetStringUTFChars(env, jLocalDirectory, 0);
    int nativePort = jPort;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    int error = JNI_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "%s %d %s", nativeDeviceIP ? nativeDeviceIP : "null", (int)jPort, nativeLocalDirectory ? nativeLocalDirectory : "null");

    result = ARDATATRANSFER_MediasDownloader_New(nativeManager, nativeDeviceIP, nativePort, nativeLocalDirectory);

    if (error == JNI_OK)
    {
        error = ARDATATRANSFER_JNI_MediasDownloader_NewListenersJNI(env);
    }

    if (error == JNI_OK)
    {
        error = ARDATATRANSFER_JNI_MediasDownloader_NewListJNI(env);
    }

    if (error == JNI_OK)
    {
        error = ARDATATRANSFER_JNI_MediasDownloader_NewMediaJNI(env);
    }

    if (error != JNI_OK)
    {
        result = ARDATATRANSFER_ERROR_SYSTEM;
    }

    //cleanup
    if (nativeDeviceIP != NULL)
    {
        (*env)->ReleaseStringUTFChars(env, jDeviceIP, nativeDeviceIP);
    }

    if (nativeLocalDirectory != NULL)
    {
        (*env)->ReleaseStringUTFChars(env, jLocalDirectory, nativeLocalDirectory);
    }

    return result;
}

JNIEXPORT jint JNICALL Java_com_parrot_arsdk_ardatatransfer_ARDataTransferMediasDownloader_nativeDelete(JNIEnv *env, jobject jThis, jlong jManager)
{
    ARDATATRANSFER_Manager_t *nativeManager = (ARDATATRANSFER_Manager_t*)(intptr_t)jManager;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "");

    result = ARDATATRANSFER_MediasDownloader_Delete(nativeManager);

    return result;
}

JNIEXPORT jint JNICALL Java_com_parrot_arsdk_ardatatransfer_ARDataTransferMediasDownloader_nativeGetAvailableMediasSync(JNIEnv *env, jobject jThis, jlong jManager, jobject jMediaList, jboolean jWithThumbnail)
{
    ARDATATRANSFER_Manager_t *nativeManager = (ARDATATRANSFER_Manager_t*)(intptr_t)jManager;
    ARDATATRANSFER_MediaList_t mediaList;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    int i;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "");

    memset(&mediaList, 0, sizeof(ARDATATRANSFER_MediaList_t));

    result = ARDATATRANSFER_MediasDownloader_GetAvailableMediasSync(nativeManager, &mediaList, (jWithThumbnail == JNI_TRUE) ? 1 : 0);

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "return %d, count %d", result, mediaList.count);

    if ((ARDATATRANSFER_OK == result) && (mediaList.medias != NULL))
    {
        for (i=0; i<mediaList.count; i++)
        {
            ARDATATRANSFER_Media_t *media = mediaList.medias[i];

            ARDATATRANSFER_JNI_MediasDownloader_AddMedia(env, jMediaList, media);
        }
    }

    ARDATATRANSFER_MediasDownloader_FreeMediaList(&mediaList);

    return result;
}

JNIEXPORT jint JNICALL Java_com_parrot_arsdk_ardatatransfer_ARDataTransferMediasDownloader_nativeGetAvailableMediasAsync(JNIEnv *env, jobject jThis, jlong jManager, jobject jAvailableMediaListener, jobject jAvailableMediaArg)
{
    ARDATATRANSFER_Manager_t *nativeManager = (ARDATATRANSFER_Manager_t*)(intptr_t)jManager;
    ARDATATRANSFER_JNI_MediasDownloaderCallbacks_t *callbacks = NULL;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    int error = JNI_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "");

    callbacks = (ARDATATRANSFER_JNI_MediasDownloaderCallbacks_t*)calloc(1, sizeof(ARDATATRANSFER_JNI_MediasDownloaderCallbacks_t));

    if (callbacks == NULL)
    {
        error = JNI_FAILED;
    }
    else
    {
        if (jAvailableMediaListener != NULL)
        {
            callbacks->jAvailableMediaListener = (*env)->NewGlobalRef(env, jAvailableMediaListener);
        }

        if (jAvailableMediaArg != NULL)
        {
            callbacks->jAvailableMediaArg = (*env)->NewGlobalRef(env, jAvailableMediaArg);
        }
    }

    result = ARDATATRANSFER_MediasDownloader_GetAvailableMediasAsync(nativeManager, ARDATATRANSFER_JNI_MediasDownloader_AvailableMediaCallback, callbacks);

    if (error != JNI_OK)
    {
        ARDATATRANSFER_JNI_MediasDownloader_FreeMediasDownloaderCallbacks(env, &callbacks);

        result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
    }

    return result;
}

JNIEXPORT jint JNICALL Java_com_parrot_arsdk_ardatatransfer_ARDataTransferMediasDownloader_nativeCancelGetAvailableMedias(JNIEnv *env, jobject jThis, jlong jManager)
{
    ARDATATRANSFER_Manager_t *nativeManager = (ARDATATRANSFER_Manager_t*)(intptr_t)jManager;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;

    result = ARDATATRANSFER_MediasDownloader_CancelGetAvailableMedias(nativeManager);

    return result;
}

JNIEXPORT jint JNICALL Java_com_parrot_arsdk_ardatatransfer_ARDataTransferMediasDownloader_nativeDeleteMedia(JNIEnv *env, jobject jThis, jlong jManager, jobject jMedia)
{
    ARDATATRANSFER_Manager_t *nativeManager = (ARDATATRANSFER_Manager_t*)(intptr_t)jManager;
    ARDATATRANSFER_Media_t nativeMedia;
    int error = JNI_OK;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "");

    error = ARDATATRANSFER_JNI_MediasDownloader_GetMedia(env, jMedia, &nativeMedia);

    if (error != JNI_OK)
    {
        result = ARDATATRANSFER_ERROR_SYSTEM;
    }

    if (result == ARDATATRANSFER_OK)
    {
        result = ARDATATRANSFER_MediasDownloader_DeleteMedia(nativeManager, &nativeMedia);
    }

    return result;
}

void ARDATATRANSFER_JNI_MediasDownloader_FreeMediasDownloaderCallbacks(JNIEnv *env, ARDATATRANSFER_JNI_MediasDownloaderCallbacks_t **callbacksParam)
{
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "%x", callbacksParam ? *callbacksParam : 0);

    if (callbacksParam != NULL)
    {
        ARDATATRANSFER_JNI_MediasDownloaderCallbacks_t *callbacks = *callbacksParam;

        if (callbacks != NULL)
        {
            if (env != NULL)
            {
                if (callbacks->jMedia != NULL)
                {
                    (*env)->DeleteGlobalRef(env, callbacks->jMedia);
                }

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

JNIEXPORT jint JNICALL Java_com_parrot_arsdk_ardatatransfer_ARDataTransferMediasDownloader_nativeAddMediaToQueue(JNIEnv *env, jobject jThis, jlong jManager, jobject jMedia, jobject jProgressListener, jobject jProgressArg, jobject jCompletionListener, jobject jCompletionArg)
{
    ARDATATRANSFER_Manager_t *nativeManager = (ARDATATRANSFER_Manager_t*)(intptr_t)jManager;
    ARDATATRANSFER_JNI_MediasDownloaderCallbacks_t *callbacks = NULL;
    ARDATATRANSFER_Media_t nativeMedia;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    int error = JNI_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "%x, %x", (int)nativeManager, (int)jMedia);

    callbacks = (ARDATATRANSFER_JNI_MediasDownloaderCallbacks_t*)calloc(1, sizeof(ARDATATRANSFER_JNI_MediasDownloaderCallbacks_t));

    if (callbacks == NULL)
    {
        error = JNI_FAILED;
    }
    else
    {
        if (jMedia != NULL)
        {
            callbacks->jMedia = (*env)->NewGlobalRef(env, jMedia);
        }

        if (jProgressListener != NULL)
        {
            callbacks->jProgressListener = (*env)->NewGlobalRef(env, jProgressListener);
        }

        if (jProgressArg != NULL)
        {
            callbacks->jProgressArg = (*env)->NewGlobalRef(env, jProgressArg);
        }

        if (jCompletionListener != NULL)
        {
            callbacks->jCompletionListener = (*env)->NewGlobalRef(env, jCompletionListener);
        }

        if (jCompletionArg != NULL)
        {
            callbacks->jCompletionArg = (*env)->NewGlobalRef(env, jCompletionArg);
        }
    }

    if (error == JNI_OK)
    {
        memset(&nativeMedia, 0, sizeof(ARDATATRANSFER_Media_t));

        error = ARDATATRANSFER_JNI_MediasDownloader_GetMedia(env, jMedia, &nativeMedia);
    }

    if (error == JNI_OK)
    {
        result = ARDATATRANSFER_MediasDownloader_AddMediaToQueue(nativeManager, &nativeMedia, ARDATATRANSFER_JNI_MediasDownloader_ProgressCallback, callbacks, ARDATATRANSFER_JNI_MediasDownloader_CompletionCallback, callbacks);
    }

    if (error != JNI_OK)
    {
        ARDATATRANSFER_JNI_MediasDownloader_FreeMediasDownloaderCallbacks(env, &callbacks);

        result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
    }

    return result;
}

JNIEXPORT void JNICALL Java_com_parrot_arsdk_ardatatransfer_ARDataTransferMediasDownloader_nativeQueueThreadRun(JNIEnv *env, jobject jThis, jlong jManager)
{
    ARDATATRANSFER_Manager_t *nativeManager = (ARDATATRANSFER_Manager_t*)(intptr_t)jManager;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "");

    ARDATATRANSFER_MediasDownloader_QueueThreadRun(nativeManager);

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "exit");
}

JNIEXPORT jint JNICALL Java_com_parrot_arsdk_ardatatransfer_ARDataTransferMediasDownloader_nativeCancelQueueThread(JNIEnv *env, jobject jThis, jlong jManager)
{
    ARDATATRANSFER_Manager_t *nativeManager = (ARDATATRANSFER_Manager_t*)(intptr_t)jManager;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "");

    result = ARDATATRANSFER_MediasDownloader_CancelQueueThread(nativeManager);

    return result;
}

void ARDATATRANSFER_JNI_MediasDownloader_ProgressCallback(void* arg, ARDATATRANSFER_Media_t *media, uint8_t percent)
{
    ARDATATRANSFER_JNI_MediasDownloaderCallbacks_t *callbacks = (ARDATATRANSFER_JNI_MediasDownloaderCallbacks_t*)arg;

    if (callbacks != NULL)
    {
        if ((ARDATATRANSFER_JNI_Manager_VM != NULL) && (callbacks->jProgressListener != NULL) && (methodId_MDListener_didMediaProgress != NULL))
        {
            JNIEnv *env = NULL;
            jint jPercent = 0;
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

            if ((error == JNI_OK) && (methodId_MDListener_didMediaProgress != NULL))
            {
                jPercent = percent;

                (*env)->CallVoidMethod(env, callbacks->jProgressListener, methodId_MDListener_didMediaProgress, callbacks->jProgressArg, callbacks->jMedia, jPercent);
            }

            if ((jResultEnv == JNI_EDETACHED) && (env != NULL))
            {
                 (*ARDATATRANSFER_JNI_Manager_VM)->DetachCurrentThread(ARDATATRANSFER_JNI_Manager_VM);
            }
        }
    }
}

void ARDATATRANSFER_JNI_MediasDownloader_CompletionCallback(void* arg, ARDATATRANSFER_Media_t *media, eARDATATRANSFER_ERROR nativeError)
{
    ARDATATRANSFER_JNI_MediasDownloaderCallbacks_t *callbacks = (ARDATATRANSFER_JNI_MediasDownloaderCallbacks_t*)arg;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "%x, %s, %d", (int)arg, media ? media->name : "null", nativeError);

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
				ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "error no env");
			}

			if ((env != NULL) && (callbacks->jCompletionListener != NULL) && (methodId_MDListener_didMediaComplete != NULL))
			{
				int error = JNI_OK;


				if (error == JNI_OK)
				{
					jError = ARDATATRANSFER_JNI_Manager_NewERROR_ENUM(env, nativeError);

					if (jError == NULL)
					{
						error = JNI_FAILED;
						ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "error %d, %x", error, jError);
					}
				}

				if ((error == JNI_OK) && (methodId_MDListener_didMediaComplete != NULL))
				{
					 (*env)->CallVoidMethod(env, callbacks->jCompletionListener, methodId_MDListener_didMediaComplete, callbacks->jCompletionArg, callbacks->jMedia, jError);
				}
			}

			if (env != NULL)
			{
			    if (jError != NULL)
			    {
			        (*env)->DeleteLocalRef(env, jError);
			    }

				ARDATATRANSFER_JNI_MediasDownloader_FreeMediasDownloaderCallbacks(env, &callbacks);
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

void ARDATATRANSFER_JNI_MediasDownloader_AvailableMediaCallback(void* arg, ARDATATRANSFER_Media_t *media)
{
    ARDATATRANSFER_JNI_MediasDownloaderCallbacks_t *callbacks = (ARDATATRANSFER_JNI_MediasDownloaderCallbacks_t*)arg;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "%x, %s", (int)arg, media ? media->name : "null");

    if (callbacks != NULL)
    {
        if (ARDATATRANSFER_JNI_Manager_VM != NULL)
        {
            JNIEnv *env = NULL;
			jint jResultEnv = 0;
			jobject jMedia = NULL;
			int error = JNI_OK;

			jResultEnv = (*ARDATATRANSFER_JNI_Manager_VM)->GetEnv(ARDATATRANSFER_JNI_Manager_VM, (void **) &env, JNI_VERSION_1_6);

			if (jResultEnv == JNI_EDETACHED)
			{
				 (*ARDATATRANSFER_JNI_Manager_VM)->AttachCurrentThread(ARDATATRANSFER_JNI_Manager_VM, &env, NULL);
			}

			if (env == NULL)
			{
				error = JNI_FAILED;
				ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "error no env");
			}

			if (error == JNI_OK)
			{
			    jMedia = ARDATATRANSFER_JNI_MediasDownloader_NewMedia(env, media);

			    if (jMedia == NULL)
			    {
                    error = JNI_FAILED;
			    }
			}

			if ((error == JNI_OK) && (env != NULL) && (callbacks->jAvailableMediaListener != NULL) && (methodId_MDListener_didMediaAvailable != NULL))
			{
				(*env)->CallVoidMethod(env, callbacks->jAvailableMediaListener, methodId_MDListener_didMediaAvailable, callbacks->jAvailableMediaArg, jMedia);
			}

	        if ((env != NULL) && (jMedia != NULL))
            {
                (*env)->DeleteLocalRef(env, jMedia);
            }

			if ((jResultEnv == JNI_EDETACHED) && (env != NULL))
			{
				(*ARDATATRANSFER_JNI_Manager_VM)->DetachCurrentThread(ARDATATRANSFER_JNI_Manager_VM);
			}
        }
    }
}

int ARDATATRANSFER_JNI_MediasDownloader_NewMediaJNI(JNIEnv *env)
{
    jclass locClassMDMedia = NULL;
    int error = JNI_OK;

    if (classMDMedia == NULL)
    {
        ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "");

        if (env == NULL)
        {
            error = JNI_FAILED;
        }

        if (error == JNI_OK)
        {
            locClassMDMedia = (*env)->FindClass(env, "com/parrot/arsdk/ardatatransfer/ARDataTransferMedia");

            if (locClassMDMedia == NULL)
            {
                ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "ARDataTransferMedia class not found");
                error = JNI_FAILED;
            }
        }

        if (error == JNI_OK)
        {
            classMDMedia = (*env)->NewGlobalRef(env, locClassMDMedia);

            if (classMDMedia == NULL)
            {
                ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "ARDataTransferMedia global ref failed");
                error = JNI_FAILED;
            }
        }

        if (error == JNI_OK)
        {
            methodId_MDMedia_init = (*env)->GetMethodID(env, classMDMedia, "<init>", "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;F[B)V");

            if (methodId_MDMedia_init == NULL)
            {
                ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "Media <init> method not found");
                error = JNI_FAILED;
            }
        }

        if (error == JNI_OK)
        {
            methodId_MDMedia_getProductValue = (*env)->GetMethodID(env, classMDMedia, "getProductValue", "()I");

            if (methodId_MDMedia_getProductValue == NULL)
            {
                ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "Media getProductValue method not found");
                error = JNI_FAILED;
            }
        }

        if (error == JNI_OK)
        {
            methodId_MDMedia_getName = (*env)->GetMethodID(env, classMDMedia, "getName", "()Ljava/lang/String;");

            if (methodId_MDMedia_getName == NULL)
            {
                ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "Media getName method not found");
                error = JNI_FAILED;
            }
        }

        if (error == JNI_OK)
        {
            methodId_MDMedia_getFileName = (*env)->GetMethodID(env, classMDMedia, "getFileName", "()Ljava/lang/String;");

            if (methodId_MDMedia_getFileName == NULL)
            {
                ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "Media getFileName method not found");
                error = JNI_FAILED;
            }
        }

        if (error == JNI_OK)
        {
            methodId_MDMedia_getDate = (*env)->GetMethodID(env, classMDMedia, "getDate", "()Ljava/lang/String;");

            if (methodId_MDMedia_getDate == NULL)
            {
                ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "Media getDate method not found");
                error = JNI_FAILED;
            }
        }

        if (error == JNI_OK)
        {
            methodId_MDMedia_getSize = (*env)->GetMethodID(env, classMDMedia, "getSize", "()F");

            if (methodId_MDMedia_getSize == NULL)
            {
                ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "Media getSize method not found");
                error = JNI_FAILED;
            }
        }

        if (error == JNI_OK)
        {
            methodId_MDMedia_getThumbnail = (*env)->GetMethodID(env, classMDMedia, "getThumbnail", "()[B");

            if (methodId_MDMedia_getThumbnail == NULL)
            {
                ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "Media getThumbnail method not found");
                error = JNI_FAILED;
            }
        }
    }

    return error;
}

void ARDATATRANSFER_JNI_MediasDownloader_FreeMediaJNI(JNIEnv *env)
{
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "");

    if (env != NULL)
    {
        if (classMDMedia != NULL)
        {
            (*env)->DeleteGlobalRef(env, classMDMedia);
            classMDMedia = NULL;
        }

        methodId_MDMedia_init = NULL;
        methodId_MDMedia_getProductValue = NULL;
        methodId_MDMedia_getName = NULL;
        methodId_MDMedia_getFileName = NULL;
        methodId_MDMedia_getDate = NULL;
        methodId_MDMedia_getSize = NULL;
        methodId_MDMedia_getThumbnail = NULL;
    }
}

int ARDATATRANSFER_JNI_MediasDownloader_GetMedia(JNIEnv *env, jobject jMedia, ARDATATRANSFER_Media_t *media)
{
    jint jProduct = 0;
    jstring jName = NULL;
    jstring jFileName = NULL;
    jstring jDate = NULL;
    jfloat jSize = 0.f;
    const char *nativeName = NULL;
    const char *nativeFileName = NULL;
    const char *nativeDate = NULL;
    int error = JNI_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "");

    if ((env == NULL) || (jMedia == NULL) || (media == NULL))
    {
        ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "wong parameters");
        error = JNI_FAILED;
    }

    if (error == JNI_OK)
    {
        if ((classMDMedia == NULL) ||
            (methodId_MDMedia_getProductValue == NULL) ||
            (methodId_MDMedia_getName == NULL) ||
            (methodId_MDMedia_getFileName == NULL) ||
            (methodId_MDMedia_getDate == NULL) ||
            (methodId_MDMedia_getSize == NULL))
        {
            ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "wrong JNI parameters");
            error = JNI_FAILED;
        }
    }

    if (error == JNI_OK)
    {
        jProduct = (*env)->CallIntMethod(env, jMedia, methodId_MDMedia_getProductValue);
        jName = (*env)->CallObjectMethod(env, jMedia, methodId_MDMedia_getName);
        jFileName = (*env)->CallObjectMethod(env, jMedia, methodId_MDMedia_getFileName);
        jDate = (*env)->CallObjectMethod(env, jMedia, methodId_MDMedia_getDate);
        jSize = (*env)->CallFloatMethod(env, jMedia, methodId_MDMedia_getSize);
    }

    if ((error == JNI_OK) && (jName != NULL))
    {
        nativeName = (*env)->GetStringUTFChars(env, jName, 0);
    }

    if ((error == JNI_OK) && (jDate != NULL))
    {
        nativeDate = (*env)->GetStringUTFChars(env, jDate, 0);
    }

    if (error == JNI_OK)
    {
        media->product = jProduct;
        strcpy(media->name, nativeName ? nativeName : "");
        strcpy(media->fileName, nativeFileName ? nativeFileName : "");
        strcpy(media->date, nativeDate ? nativeDate : "");
        media->size = (double)jSize;
    }

    //cleanup
    if (nativeName != NULL)
    {
        (*env)->ReleaseStringUTFChars(env, jName, nativeName);
    }

    if (nativeFileName != NULL)
    {
        (*env)->ReleaseStringUTFChars(env, jName, nativeFileName);
    }

    if (nativeDate != NULL)
    {
        (*env)->ReleaseStringUTFChars(env, jDate, nativeDate);
    }

    return error;
}

jobject ARDATATRANSFER_JNI_MediasDownloader_NewMedia(JNIEnv *env, ARDATATRANSFER_Media_t *media)
{
    jobject jMedia = NULL;
    jstring jName = NULL;
    jstring jFileName = NULL;
    jstring jDate = NULL;
    jbyteArray jThumbnail = NULL;
    int error = JNI_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "%s", (media->name != NULL) ? media->name : "null");

    if ((env == NULL) || (media == NULL))
    {
        error = JNI_FAILED;
    }

    if (error == JNI_OK)
    {
        if ((classMDMedia == NULL) || (methodId_MDMedia_init == NULL))
        {
            error = JNI_FAILED;
        }
    }

    if ((error == JNI_OK) && (media->name))
    {
        jName = (*env)->NewStringUTF(env, media->name);

        if (jName == NULL)
        {
            error = JNI_FAILED;
        }
    }

    if ((error == JNI_OK) && (media->fileName))
    {
        jFileName = (*env)->NewStringUTF(env, media->fileName);

        if (jFileName == NULL)
        {
            error = JNI_FAILED;
        }
    }

    if ((error == JNI_OK) && (media->date != NULL))
    {
        jDate = (*env)->NewStringUTF(env, media->date);

        if (jDate == NULL)
        {
            error = JNI_FAILED;
        }
    }

    if (error == JNI_OK)
    {
        jThumbnail = (*env)->NewByteArray(env, media->thumbnailSize);

        if (jThumbnail == NULL)
        {
            error = JNI_FAILED;
        }
    }

    if (error == JNI_OK)
    {
        (*env)->SetByteArrayRegion(env, jThumbnail, 0, media->thumbnailSize, (jbyte*)media->thumbnail);
    }

    if (error == JNI_OK)
    {
        jMedia = (*env)->NewObject(env, classMDMedia, methodId_MDMedia_init, (jint)media->product, jName, jFileName, jDate, (jfloat)media->size, jThumbnail);
    }

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "return jMedia %d", (int)jMedia);

    return jMedia;
}

jboolean ARDATATRANSFER_JNI_MediasDownloader_AddMedia(JNIEnv *env, jobject jMediaList, ARDATATRANSFER_Media_t *media)
{
    jobject jMedia = NULL;
    jboolean jret = JNI_FALSE;
    int error = JNI_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "%s", media ? media->name : "null");

    if ((env == NULL) || (jMediaList == NULL) || (media == NULL))
    {
        error = JNI_FAILED;
    }

    if (error == JNI_OK)
    {
        if (/*(classList == NULL) ||*/ (methodId_List_add == NULL))
        {
            error = JNI_FAILED;
        }
    }

    if (error == JNI_OK)
    {
        error = ARDATATRANSFER_JNI_MediasDownloader_NewMediaJNI(env);
    }

    if (error == JNI_OK)
    {
        jMedia = ARDATATRANSFER_JNI_MediasDownloader_NewMedia(env, media);

        if (jMedia == NULL)
        {
            error = JNI_FAILED;
        }
    }

    if (error == JNI_OK)
    {
        jret = (*env)->CallBooleanMethod(env, jMediaList, methodId_List_add, jMedia);
    }

    return jret;
}

int ARDATATRANSFER_JNI_MediasDownloader_NewListJNI(JNIEnv *env)
{
    jclass classList = NULL;
    int error = JNI_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "");

    if (env == NULL)
    {
        error = JNI_FAILED;
    }

    if (methodId_List_add == NULL)
    {
        if (error == JNI_OK)
        {
            classList = (*env)->FindClass(env, "java/util/List");

            if (classList == NULL)
            {
                ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "List class not found");
                error = JNI_FAILED;
            }
        }

        if (error == JNI_OK)
        {
            methodId_List_add = (*env)->GetMethodID(env, classList, "add", "(Ljava/lang/Object;)Z");

            if (methodId_List_add == NULL)
            {
                ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "List add method not found");
                error = JNI_FAILED;
            }
        }
    }

    return error;
}

void ARDATATRANSFER_JNI_MediasDownloader_FreeListJNI(JNIEnv *env)
{
    int error = JNI_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "");

    if (env == NULL)
    {
        error = JNI_FAILED;
    }

    if (error == JNI_OK)
    {
        methodId_List_add = NULL;
    }
}

int ARDATATRANSFER_JNI_MediasDownloader_NewListenersJNI(JNIEnv *env)
{
    jclass classMDProgressListener = NULL;
    jclass classMDCompletionListener = NULL;
    jclass classMDAvailableListener = NULL;
    int error = JNI_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "");

    if (env == NULL)
    {
        error = JNI_FAILED;
    }

    if (methodId_MDListener_didMediaProgress == NULL)
    {
        if (error == JNI_OK)
        {
            classMDProgressListener = (*env)->FindClass(env, "com/parrot/arsdk/ardatatransfer/ARDataTransferMediasDownloaderProgressListener");

            if (classMDProgressListener == NULL)
            {
                ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "ARDataTransferMediasDownloaderProgressListener class not found");
                error = JNI_FAILED;
            }
        }

        if (error == JNI_OK)
        {
            methodId_MDListener_didMediaProgress = (*env)->GetMethodID(env, classMDProgressListener, "didMediaProgress", "(Ljava/lang/Object;Lcom/parrot/arsdk/ardatatransfer/ARDataTransferMedia;I)V");

            if (methodId_MDListener_didMediaProgress == NULL)
            {
                ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "Listener didProgress method not found");
                error = JNI_FAILED;
            }
        }
    }

    if (methodId_MDListener_didMediaComplete == NULL)
    {
        if (error == JNI_OK)
        {
            classMDCompletionListener = (*env)->FindClass(env, "com/parrot/arsdk/ardatatransfer/ARDataTransferMediasDownloaderCompletionListener");

            if (classMDCompletionListener == NULL)
            {
                ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "ARDataTransferMediasDownloaderCompletionListener class not found");
                error = JNI_FAILED;
            }
        }

        if (error == JNI_OK)
        {
            methodId_MDListener_didMediaComplete = (*env)->GetMethodID(env, classMDCompletionListener, "didMediaComplete", "(Ljava/lang/Object;Lcom/parrot/arsdk/ardatatransfer/ARDataTransferMedia;Lcom/parrot/arsdk/ardatatransfer/ARDATATRANSFER_ERROR_ENUM;)V");

            if (methodId_MDListener_didMediaComplete == NULL)
            {
                ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "Listener didComplete method not found");
                error = JNI_FAILED;
            }
        }
    }

    if (methodId_MDListener_didMediaAvailable == NULL)
    {
        if (error == JNI_OK)
        {
            classMDAvailableListener = (*env)->FindClass(env, "com/parrot/arsdk/ardatatransfer/ARDataTransferMediasDownloaderAvailableMediaListener");

            if (classMDAvailableListener == NULL)
            {
                ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "ARDataTransferMediasDownloaderAvailableMediaListener class not found");
                error = JNI_FAILED;
            }
        }

        if (error == JNI_OK)
        {
            methodId_MDListener_didMediaAvailable = (*env)->GetMethodID(env, classMDAvailableListener, "didMediaAvailable", "(Ljava/lang/Object;Lcom/parrot/arsdk/ardatatransfer/ARDataTransferMedia;)V");

            if (methodId_MDListener_didMediaAvailable == NULL)
            {
                ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "Listener didMediaAvailable method not found");
                error = JNI_FAILED;
            }
        }
    }

    return error;
}

void ARDATATRANSFER_JNI_MediasDownloader_FreeListenersJNI(JNIEnv *env)
{
    int error = JNI_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_JNI_MEDIADOWNLOADER_TAG, "");

    if (env == NULL)
    {
        error = JNI_FAILED;
    }

    if (error == JNI_OK)
    {
        methodId_MDListener_didMediaProgress = NULL;
        methodId_MDListener_didMediaComplete = NULL;
        methodId_MDListener_didMediaAvailable = NULL;
    }
}

