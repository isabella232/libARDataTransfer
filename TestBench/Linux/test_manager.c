/*
    Copyright (C) 2014 Parrot SA

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the 
      distribution.
    * Neither the name of Parrot nor the names
      of its contributors may be used to endorse or promote products
      derived from this software without specific prior written
      permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
    OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
    OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/
/**
 * @file test_manager.c
 * @brief libARDataTransfer test manager c file.
 */

#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include <libARSAL/ARSAL_Print.h>
#include <libARSAL/ARSAL_Sem.h>
#include <libARSAL/ARSAL_Thread.h>
#include <libARUtils/ARUTILS_Error.h>
#include <libARUtils/ARUTILS_Ftp.h>
#include <libARUtils/ARUTILS_FileSystem.h>

#include <libARDataTransfer/ARDataTransfer.h>


#define TAG             "test_manager"
#define RUN_TIMEOUT     10
#define CANCEL_TIMEOUT  3
//#define DEVICE_IP       "172.20.5.16"  /* PC */
//#define DEVICE_IP       "192.168.1.1" /* ARDrone2 */
#define DEVICE_IP       "192.168.42.1" /* ARDrone3 */
#define DEVICE_PORT     21

typedef void (*test_manager_timer_tick_callback)(void *arg);

typedef struct
{
    ARSAL_Sem_t sem;
    int seconds;
    test_manager_timer_tick_callback tick_callback;
    void *tick_callback_arg;


} test_manager_timer_t;

typedef struct
{
    ARDATATRANSFER_Manager_t *manager;
    ARSAL_Sem_t sem;
    const char *tmp;
    
} test_manager_thread_t;

ARSAL_Sem_t semRunning;
ARDATATRANSFER_Manager_t *managerRunning = NULL;


void * test_manager_timer_thread_run(void *arg)
{
    test_manager_timer_t *timerdata = (test_manager_timer_t*)arg;
    struct timespec tm;

    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "enter");

    tm.tv_sec = timerdata->seconds;
    tm.tv_nsec = 0;

    ARSAL_Sem_Timedwait(&timerdata->sem, &tm);

    if (timerdata->tick_callback != NULL)
    {
        timerdata->tick_callback(timerdata->tick_callback_arg);
    }

    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "timer exit");
    return NULL;
}

void test_manager_timer_wait(int seconds)
{
    test_manager_timer_t timerdata;
    ARSAL_Thread_t threadTimer;
    void *resultThread;
    int resultSys;

    ARSAL_Sem_Init(&timerdata.sem, 0, 0);
    timerdata.seconds = seconds;
    timerdata.tick_callback = NULL;
    timerdata.tick_callback_arg = NULL;

    resultSys = ARSAL_Thread_Create(&threadTimer, test_manager_timer_thread_run, &timerdata);
    resultSys = ARSAL_Thread_Join(threadTimer, &resultThread);

    ARSAL_Sem_Destroy(&timerdata.sem);
}

void test_manager_assert(int check)
{
    if (check == 0)
    {
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "exit with ASSERT FAILED !!!!!!!!!!!");
        exit(0);
    }
}

void test_manager_medias_downloader_progress_callback(void* arg, ARDATATRANSFER_Media_t *media, float percent)
{
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %02f%%", media->name, percent);
}

void test_manager_medias_downloader_completion_callback(void* arg, ARDATATRANSFER_Media_t *media, eARDATATRANSFER_ERROR error)
{
    const char *tmp = (const char *)arg;
    eARUTILS_ERROR errorFile = ARUTILS_OK;
    char localFile[ARUTILS_FTP_MAX_PATH_SIZE];
    
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s %s: %d, error %d", ARDISCOVERY_getProductName(media->product), media->name, (int)media->size, (int)error);
    
    if (tmp != NULL)
    {
        strcpy(localFile, media->filePath);
        
        errorFile = ARUTILS_FileSystem_RemoveFile(localFile);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_FileSystem_RemoveFile", errorFile);
    }
}

void test_manager_data_downloader_completion_callback(void* arg, const char *fileName, eARDATATRANSFER_ERROR error)
{
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s, error %d", fileName, error);
}

void test_manager_data_downloader(const char *tmp)
{
    ARDATATRANSFER_Manager_t *manager;
    ARUTILS_Manager_t *ftpListManager;
    ARUTILS_Manager_t *ftpDataManager;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    eARUTILS_ERROR resultUtils = ARUTILS_OK;
    ARSAL_Thread_t threadDataDownloader;
    int resultSys;
    void *resultThread = NULL;
    long filesNumber = 0;

    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "**************************************************************");
    
    ftpListManager = ARUTILS_Manager_New(&resultUtils);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_New", resultUtils);
    
    resultUtils = ARUTILS_Manager_InitWifiFtp(ftpListManager, DEVICE_IP, DEVICE_PORT, ARUTILS_FTP_ANONYMOUS, "");
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_InitWifiFtp", resultUtils);

    ftpDataManager = ARUTILS_Manager_New(&resultUtils);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_New", resultUtils);
    
    resultUtils = ARUTILS_Manager_InitWifiFtp(ftpDataManager, DEVICE_IP, DEVICE_PORT, ARUTILS_FTP_ANONYMOUS, "");
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_InitWifiFtp", resultUtils);
    
    resultUtils = ARUTILS_Manager_Ftp_Connection_Disconnect(ftpDataManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_Disconnect", resultUtils);
    
    resultUtils = ARUTILS_Manager_Ftp_Connection_Reconnect(ftpDataManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_Ftp_Connection_Reconnect", resultUtils);
    
    manager = ARDATATRANSFER_Manager_New(&result);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_Manager_New", result);

    result = ARDATATRANSFER_DataDownloader_New(manager, ftpListManager, ftpDataManager, "internal_000", tmp, test_manager_data_downloader_completion_callback, "arg");
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_DataDownloader_New", result);
    
    result = ARDATATRANSFER_DataDownloader_GetAvailableFiles(manager, &filesNumber);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d, %d", "ARDATATRANSFER_DataDownloader_GetAvailableFiles", result, filesNumber);

    resultSys = ARSAL_Thread_Create(&threadDataDownloader, ARDATATRANSFER_DataDownloader_ThreadRun, manager);

    //result = ARDATATRANSFER_DataDownloader_CancelThread(manager);
    //ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_DataDownloader_CancelThread", result);

    resultSys = ARSAL_Thread_Join(threadDataDownloader, &resultThread);

    result = ARDATATRANSFER_DataDownloader_Delete(manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_DataDownloader_Delete", result);
    
    ARDATATRANSFER_Manager_Delete(&manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_Manager_Delete", result);
    
    ARUTILS_Manager_CloseWifiFtp(ftpListManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_CloseWifiFtp");
    
    ARUTILS_Manager_Delete(&ftpListManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_Delete");

    ARUTILS_Manager_CloseWifiFtp(ftpDataManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_CloseWifiFtp");
    
    ARUTILS_Manager_Delete(&ftpDataManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_Delete");
}

void test_manager_medias_downloader_available_media_callback(void* arg, ARDATATRANSFER_Media_t *media, int index)
{
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%d, %s", index, media->name);
}

void test_manager_medias_downloader(const char *tmp)
{
    ARDATATRANSFER_Manager_t *manager;
    ARUTILS_Manager_t *ftpListManager;
    ARUTILS_Manager_t *ftpQueueManager;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    eARUTILS_ERROR resultUtils = ARUTILS_OK;
    ARSAL_Thread_t threadMediasDownloader;
    int resultSys;
    void *resultThread = NULL;
    int count;
    int i;

    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "**************************************************************");
    
    ftpListManager = ARUTILS_Manager_New(&resultUtils);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_New", resultUtils);
    
    resultUtils = ARUTILS_Manager_InitWifiFtp(ftpListManager, DEVICE_IP, DEVICE_PORT, ARUTILS_FTP_ANONYMOUS, "");
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_InitWifiFtp", resultUtils);
    
    ftpQueueManager = ARUTILS_Manager_New(&resultUtils);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_New", resultUtils);
    
    resultUtils = ARUTILS_Manager_InitWifiFtp(ftpQueueManager, DEVICE_IP, DEVICE_PORT, ARUTILS_FTP_ANONYMOUS, "");
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_InitWifiFtp", resultUtils);

    manager = ARDATATRANSFER_Manager_New(&result);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_Manager_New", result);

    result = ARDATATRANSFER_MediasDownloader_New(manager, ftpListManager, ftpQueueManager, "", tmp);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_New", result);

    count = ARDATATRANSFER_MediasDownloader_GetAvailableMediasSync(manager, 1, &result);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_GetAvailableMediasSync", result);

    /*for (i=0; i<count; i++)
    {
        ARDATATRANSFER_Media_t *media = ARDATATRANSFER_MediasDownloader_GetAvailableMediaAtIndex(manager, i, &result);

        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "media, name: %s, date: %s, size: %0.f, thumbnail: %d\n", media->name, media->date, media->size, media->thumbnailSize);

        result = ARDATATRANSFER_MediasDownloader_AddMediaToQueue(manager, media, test_manager_medias_downloader_progress_callback, NULL, test_manager_medias_downloader_completion_callback, (void *)tmp);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_AddMediaToQueue", result);
        
        result = ARDATATRANSFER_MediasDownloader_DeleteMedia(manager, media);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_DeleteMedia", result);
    }*/

    resultSys = ARSAL_Thread_Create(&threadMediasDownloader, ARDATATRANSFER_MediasDownloader_QueueThreadRun, manager);

    count = ARDATATRANSFER_MediasDownloader_GetAvailableMediasSync(manager, 0, &result);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_GetAvailableMediasSync", result);
    
    ARDATATRANSFER_MediasDownloader_GetAvailableMediasAsync(manager, test_manager_medias_downloader_available_media_callback, NULL);

    for (i=0; i<count; i++)
    {
        ARDATATRANSFER_Media_t *media = ARDATATRANSFER_MediasDownloader_GetAvailableMediaAtIndex(manager, i, &result);

        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "media, name: %s, date: %s, size: %0.f, thumbnail: %d\n", media->name, media->date, media->size, media->thumbnailSize);

        result = ARDATATRANSFER_MediasDownloader_AddMediaToQueue(manager, media, test_manager_medias_downloader_progress_callback, NULL, test_manager_medias_downloader_completion_callback, (void *)tmp);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_AddMediaToQueue", result);
    }

    //result = ARDATATRANSFER_MediasDownloader_CancelQueueThread(manager);
    //ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_CancelQueueThread", result);

    resultSys = ARSAL_Thread_Join(threadMediasDownloader, &resultThread);
    
    result = ARDATATRANSFER_MediasDownloader_Delete(manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_Delete", result);

    ARDATATRANSFER_Manager_Delete(&manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_Manager_Delete", result);
    
    ARUTILS_Manager_CloseWifiFtp(ftpListManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_CloseWifiFtp");
    
    ARUTILS_Manager_Delete(&ftpListManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_Delete");
    
    ARUTILS_Manager_CloseWifiFtp(ftpQueueManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_CloseWifiFtp");
    
    ARUTILS_Manager_Delete(&ftpQueueManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_Delete");
}

void test_manager_checking_parameters(const char *tmp)
{
    ARDATATRANSFER_Manager_t *manager;
    ARUTILS_Manager_t *ftpListManager;
    ARUTILS_Manager_t *ftpDataManager;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    eARUTILS_ERROR resultUtils = ARUTILS_OK;
    ARDATATRANSFER_Media_t media;
    ARSAL_Thread_t threadMediasDownloader;
    ARSAL_Thread_t threadDataDownloader;
    int resultSys;
    int count;
    void *resultThread = NULL;

    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "**************************************************************");

    memset(&media, 0, sizeof(ARDATATRANSFER_Media_t));

    //Without Manager
    ARDATATRANSFER_Manager_Delete(&manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARDATATRANSFER_Manager_Delete");

    result = ARDATATRANSFER_DataDownloader_New(manager, NULL, NULL, "", tmp, test_manager_data_downloader_completion_callback, "arg");
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_DataDownloader_New", result);
    test_manager_assert(result == ARDATATRANSFER_ERROR_BAD_PARAMETER);
    
    result = ARDATATRANSFER_DataDownloader_Delete(manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_DataDownloader_Delete", result);
    test_manager_assert(result == ARDATATRANSFER_ERROR_BAD_PARAMETER);

    result = ARDATATRANSFER_DataDownloader_CancelThread(manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_DataDownloader_CancelThread", result);
    test_manager_assert(result == ARDATATRANSFER_ERROR_BAD_PARAMETER);

    resultSys = ARSAL_Thread_Create(&threadDataDownloader, ARDATATRANSFER_DataDownloader_ThreadRun, manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Thread_Create - ARDATATRANSFER_DataDownloader_ThreadRun", resultSys);
    test_manager_assert(resultSys == 0);
    resultSys = ARSAL_Thread_Join(threadDataDownloader, &resultThread);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Thread_Join - ARDATATRANSFER_DataDownloader_ThreadRun", resultSys);
    test_manager_assert(resultSys == 0);
    
/*    ftpManager = ARUTILS_Manager_New(&resultUtils);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_New", resultUtils);

    result = ARDATATRANSFER_MediasDownloader_New(manager, DEVICE_IP, DEVICE_PORT, "", tmp);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_New", result);
    test_manager_assert(result == ARDATATRANSFER_ERROR_BAD_PARAMETER);*/
    
    result = ARDATATRANSFER_MediasDownloader_Delete(manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_Delete", result);
    test_manager_assert(result == ARDATATRANSFER_ERROR_BAD_PARAMETER);

    count = ARDATATRANSFER_MediasDownloader_GetAvailableMediasSync(manager, 1, &result);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_GetAvailableMediasSync", result);
    test_manager_assert(result == ARDATATRANSFER_ERROR_BAD_PARAMETER);

    result = ARDATATRANSFER_MediasDownloader_AddMediaToQueue(manager, NULL, NULL, NULL, NULL, NULL);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_AddMediaToQueue", result);
    test_manager_assert(result == ARDATATRANSFER_ERROR_BAD_PARAMETER);

    result = ARDATATRANSFER_MediasDownloader_CancelQueueThread(manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_CancelQueueThread", result);
    test_manager_assert(result == ARDATATRANSFER_ERROR_BAD_PARAMETER);

    resultSys = ARSAL_Thread_Create(&threadMediasDownloader, ARDATATRANSFER_MediasDownloader_QueueThreadRun, manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Thread_Create - ARDATATRANSFER_MediasDownloader_QueueThreadRun", resultSys);
    test_manager_assert(resultSys == 0);
    resultSys = ARSAL_Thread_Join(threadMediasDownloader, &resultThread);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Thread_Join - ARDATATRANSFER_MediasDownloader_QueueThreadRun", resultSys);
    test_manager_assert(resultSys == 0);

    //Without DataManager
    manager = ARDATATRANSFER_Manager_New(&result);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_Manager_New", result);
    test_manager_assert(result == ARDATATRANSFER_OK);

    result = ARDATATRANSFER_DataDownloader_CancelThread(manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_DataDownloader_CancelThread", result);
    test_manager_assert(result == ARDATATRANSFER_ERROR_NOT_INITIALIZED);

    resultSys = ARSAL_Thread_Create(&threadDataDownloader, ARDATATRANSFER_DataDownloader_ThreadRun, manager);
    resultSys = ARSAL_Thread_Join(threadDataDownloader, &resultThread);

    //Without MediasMananger
    count = ARDATATRANSFER_MediasDownloader_GetAvailableMediasSync(manager, 1, &result);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_GetAvailableMediasSync", result);
    test_manager_assert(result == ARDATATRANSFER_ERROR_NOT_INITIALIZED);

    result = ARDATATRANSFER_MediasDownloader_AddMediaToQueue(manager, NULL, NULL, NULL, NULL, NULL);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_AddMediaToQueue", result);
    test_manager_assert(result == ARDATATRANSFER_ERROR_BAD_PARAMETER);

    result = ARDATATRANSFER_MediasDownloader_AddMediaToQueue(manager, &media, NULL, NULL, NULL, NULL);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_AddMediaToQueue", result);
    test_manager_assert(result == ARDATATRANSFER_ERROR_NOT_INITIALIZED);

    result = ARDATATRANSFER_MediasDownloader_CancelQueueThread(manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_CancelQueueThread", result);
    test_manager_assert(result == ARDATATRANSFER_ERROR_NOT_INITIALIZED);

    resultSys = ARSAL_Thread_Create(&threadMediasDownloader, ARDATATRANSFER_MediasDownloader_QueueThreadRun, manager);
    resultSys = ARSAL_Thread_Join(threadMediasDownloader, &resultThread);

    //Already Initialized
    ftpListManager = ARUTILS_Manager_New(&resultUtils);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_New", resultUtils);
    
    resultUtils = ARUTILS_Manager_InitWifiFtp(ftpListManager, DEVICE_IP, DEVICE_PORT, ARUTILS_FTP_ANONYMOUS, "");
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_InitWifiFtp", resultUtils);
    
    ftpDataManager = ARUTILS_Manager_New(&resultUtils);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_New", resultUtils);
    
    resultUtils = ARUTILS_Manager_InitWifiFtp(ftpDataManager, DEVICE_IP, DEVICE_PORT, ARUTILS_FTP_ANONYMOUS, "");
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_InitWifiFtp", resultUtils);
    
    result = ARDATATRANSFER_DataDownloader_New(manager, ftpListManager, ftpDataManager, "", tmp, test_manager_data_downloader_completion_callback, "arg");
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_DataDownloader_New", result);
    test_manager_assert(result == ARDATATRANSFER_OK);

    result = ARDATATRANSFER_DataDownloader_New(manager, ftpListManager, ftpDataManager, "", tmp, test_manager_data_downloader_completion_callback, "arg");
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_DataDownloader_New", result);
    test_manager_assert(result == ARDATATRANSFER_ERROR_ALREADY_INITIALIZED);

/*    result = ARDATATRANSFER_MediasDownloader_New(manager, DEVICE_IP, DEVICE_PORT, "", tmp);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_New", result);
    test_manager_assert(result == ARDATATRANSFER_OK);

    result = ARDATATRANSFER_MediasDownloader_New(manager, DEVICE_IP, DEVICE_PORT, "", tmp);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_New", result);
    test_manager_assert(result == ARDATATRANSFER_ERROR_ALREADY_INITIALIZED);*/

    ARDATATRANSFER_Manager_Delete(&manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARDATATRANSFER_Manager_Delete");
    
    ARUTILS_Manager_CloseWifiFtp(ftpListManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_CloseWifiFtp");
    
    ARUTILS_Manager_Delete(&ftpListManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_Delete");
    
    ARUTILS_Manager_CloseWifiFtp(ftpDataManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_CloseWifiFtp");
    
    ARUTILS_Manager_Delete(&ftpDataManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_Delete");
}

void test_manager_checking_run_cancel(const char *tmp)
{
    ARDATATRANSFER_Manager_t *manager;
    ARUTILS_Manager_t *ftpListManager;
    ARUTILS_Manager_t *ftpDataManager;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    eARUTILS_ERROR resultUtils = ARUTILS_OK;
    ARSAL_Thread_t threadDataDownloader;
    ARSAL_Thread_t threadMediasDownloader;
    int resultSys;
    int count;
    void *resultThread = NULL;
    int i;

    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "**************************************************************");
    
    ftpListManager = ARUTILS_Manager_New(&resultUtils);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_New", resultUtils);
    
    resultUtils = ARUTILS_Manager_InitWifiFtp(ftpListManager, DEVICE_IP, DEVICE_PORT, ARUTILS_FTP_ANONYMOUS, "");
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_InitWifiFtp", resultUtils);
    
    ftpDataManager = ARUTILS_Manager_New(&resultUtils);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_New", resultUtils);
    
    resultUtils = ARUTILS_Manager_InitWifiFtp(ftpDataManager, DEVICE_IP, DEVICE_PORT, ARUTILS_FTP_ANONYMOUS, "");
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_InitWifiFtp", resultUtils);

    manager = ARDATATRANSFER_Manager_New(&result);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_Manager_New", result);
    test_manager_assert(result == ARDATATRANSFER_OK);

    //Data Downloader
    result = ARDATATRANSFER_DataDownloader_New(manager, ftpListManager, ftpDataManager, "", tmp, test_manager_data_downloader_completion_callback, "arg");
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_DataDownloader_New", result);
    test_manager_assert(result == ARDATATRANSFER_OK);

    resultSys = ARSAL_Thread_Create(&threadDataDownloader, ARDATATRANSFER_DataDownloader_ThreadRun, manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Thread_Create - ARDATATRANSFER_DataDownloader_ThreadRun", resultSys);
    test_manager_assert(resultSys == 0);

    test_manager_timer_wait(CANCEL_TIMEOUT);

    result = ARDATATRANSFER_DataDownloader_CancelThread(manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_DataDownloader_CancelThread", result);
    test_manager_assert(result == ARDATATRANSFER_OK);

    resultSys = ARSAL_Thread_Join(threadDataDownloader, &resultThread);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Thread_Join - ARDATATRANSFER_DataDownloader_ThreadRun", resultSys);
    test_manager_assert(resultSys == 0);

    //Media Downloader
    /*result = ARDATATRANSFER_MediasDownloader_New(manager, DEVICE_IP, DEVICE_PORT, "", tmp);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_New", result);
    test_manager_assert(result == ARDATATRANSFER_OK);*/

    count = ARDATATRANSFER_MediasDownloader_GetAvailableMediasSync(manager, 1, &result);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_GetAvailableMediasSync", result);
    test_manager_assert(result == ARDATATRANSFER_OK);

    for (i=0; i<count; i++)
    {
        ARDATATRANSFER_Media_t *media = ARDATATRANSFER_MediasDownloader_GetAvailableMediaAtIndex(manager, i, &result);

        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "media, name: %s, date: %s, size: %0.f, thumbnail: %d\n", media->name, media->date, media->size, media->thumbnailSize);

        result = ARDATATRANSFER_MediasDownloader_AddMediaToQueue(manager, media, test_manager_medias_downloader_progress_callback, NULL, test_manager_medias_downloader_completion_callback, NULL);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_AddMediaToQueue", result);
        test_manager_assert(result == ARDATATRANSFER_OK);
    }

    resultSys = ARSAL_Thread_Create(&threadMediasDownloader, ARDATATRANSFER_MediasDownloader_QueueThreadRun, manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Thread_Create - ARDATATRANSFER_MediasDownloader_QueueThreadRun", resultSys);
    test_manager_assert(resultSys == 0);

    count = ARDATATRANSFER_MediasDownloader_GetAvailableMediasSync(manager, 1, &result);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_GetAvailableMediasSync", result);
    test_manager_assert(result == ARDATATRANSFER_OK);

    for (i=0; i<count; i++)
    {
        ARDATATRANSFER_Media_t *media = ARDATATRANSFER_MediasDownloader_GetAvailableMediaAtIndex(manager, i, &result);

        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "media, name: %s, date: %s, size: %0.f, thumbnail: %d\n", media->name, media->date, media->size, media->thumbnailSize);

        result = ARDATATRANSFER_MediasDownloader_AddMediaToQueue(manager, media, test_manager_medias_downloader_progress_callback, NULL, test_manager_medias_downloader_completion_callback, NULL);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_AddMediaToQueue", result);
        test_manager_assert(result == ARDATATRANSFER_OK);
    }

    test_manager_timer_wait(CANCEL_TIMEOUT);

    result = ARDATATRANSFER_MediasDownloader_CancelQueueThread(manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_CancelQueueThread", result);
    test_manager_assert(result == ARDATATRANSFER_OK);

    resultSys = ARSAL_Thread_Join(threadMediasDownloader, &resultThread);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Thread_Join - ARDATATRANSFER_MediasDownloader_QueueThreadRun", resultSys);
    test_manager_assert(resultSys == 0);
    
    result = ARDATATRANSFER_DataDownloader_Delete(manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_DataDownloader_Delete", result);
    test_manager_assert(result == ARDATATRANSFER_OK);
    
    result = ARDATATRANSFER_MediasDownloader_Delete(manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_Delete", result);
    test_manager_assert(result == ARDATATRANSFER_OK);

    ARDATATRANSFER_Manager_Delete(&manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARDATATRANSFER_Manager_Delete");
    
    ARUTILS_Manager_CloseWifiFtp(ftpListManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_CloseWifiFtp");
    
    ARUTILS_Manager_Delete(&ftpListManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_Delete");
    
    ARUTILS_Manager_CloseWifiFtp(ftpDataManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_CloseWifiFtp");
    
    ARUTILS_Manager_Delete(&ftpDataManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_Delete");
}

void test_manager_checking_running(const char *tmp)
{
    ARSAL_Thread_t threadMediasDownloader;
    ARSAL_Thread_t threadDataDownloader;
    ARUTILS_Manager_t *ftpListManager;
    ARUTILS_Manager_t *ftpDataManager;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    eARUTILS_ERROR resultUtils = ARUTILS_OK;
    int resultSys;
    int count;
    void *resultThread = NULL;

    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "**************************************************************");

    ARSAL_Sem_Init(&semRunning, 0, 0);
    
    ftpListManager = ARUTILS_Manager_New(&resultUtils);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_New", resultUtils);
    
    resultUtils = ARUTILS_Manager_InitWifiFtp(ftpListManager, DEVICE_IP, DEVICE_PORT, ARUTILS_FTP_ANONYMOUS, "");
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_InitWifiFtp", resultUtils);
    
    ftpDataManager = ARUTILS_Manager_New(&resultUtils);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_New", resultUtils);
    
    resultUtils = ARUTILS_Manager_InitWifiFtp(ftpDataManager, DEVICE_IP, DEVICE_PORT, ARUTILS_FTP_ANONYMOUS, "");
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_InitWifiFtp", resultUtils);

    managerRunning = ARDATATRANSFER_Manager_New(&result);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_Manager_New", result);
    test_manager_assert(result == ARDATATRANSFER_OK);

    //Data
    result = ARDATATRANSFER_DataDownloader_New(managerRunning, ftpListManager, ftpDataManager, "", tmp, test_manager_data_downloader_completion_callback, "arg");
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_DataDownloader_New", result);
    test_manager_assert(result == ARDATATRANSFER_OK);

    resultSys = ARSAL_Thread_Create(&threadDataDownloader, ARDATATRANSFER_DataDownloader_ThreadRun, managerRunning);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Thread_Create - ARDATATRANSFER_DataDownloader_ThreadRun", resultSys);
    test_manager_assert(resultSys == 0);

    //Medias
    /*result = ARDATATRANSFER_MediasDownloader_New(managerRunning, DEVICE_IP, DEVICE_PORT, "", tmp);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_New", result);
    test_manager_assert(result == ARDATATRANSFER_OK);*/

    resultSys = ARSAL_Thread_Create(&threadMediasDownloader, ARDATATRANSFER_MediasDownloader_QueueThreadRun, managerRunning);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Thread_Create - ARDATATRANSFER_MediasDownloader_QueueThreadRun", resultSys);
    test_manager_assert(resultSys == 0);

    do
    {
        struct timespec tm;
        int i;

        count = ARDATATRANSFER_MediasDownloader_GetAvailableMediasSync(managerRunning, 1, &result);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_GetAvailableMediasSync", result);
        test_manager_assert(result == ARDATATRANSFER_OK);

        for (i=0; i<count; i++)
        {
            ARDATATRANSFER_Media_t *media = ARDATATRANSFER_MediasDownloader_GetAvailableMediaAtIndex(managerRunning, i, &result);

            ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "media, name: %s, date: %s, size: %0.f, thumbnail: %d\n", media->name, media->date, media->size, media->thumbnailSize);

            result = ARDATATRANSFER_MediasDownloader_AddMediaToQueue(managerRunning, media, test_manager_medias_downloader_progress_callback, NULL, test_manager_medias_downloader_completion_callback, NULL);
            ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_AddMediaToQueue", result);
            test_manager_assert(result == ARDATATRANSFER_OK);
        }

        tm.tv_sec = RUN_TIMEOUT;
        tm.tv_nsec = 0;
        resultSys = ARSAL_Sem_Timedwait(&semRunning, &tm);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Sem_Timedwait", resultSys);
    }
    while (resultSys != 0);

    //Waiting exit
    resultSys = ARSAL_Thread_Join(threadDataDownloader, &resultThread);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Thread_Join - ARDATATRANSFER_DataDownloader_ThreadRun", resultSys);
    test_manager_assert(resultSys == 0);
    
    resultSys = ARSAL_Thread_Join(threadMediasDownloader, &resultThread);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Thread_Join - ARDATATRANSFER_MediasDownloader_QueueThreadRun", resultSys);
    test_manager_assert(resultSys == 0);
    
    result = ARDATATRANSFER_DataDownloader_Delete(managerRunning);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_DataDownloader_Delete", result);
    test_manager_assert(result == ARDATATRANSFER_OK);
    
    result = ARDATATRANSFER_MediasDownloader_Delete(managerRunning);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_Delete", result);
    test_manager_assert(result == ARDATATRANSFER_OK);

    ARDATATRANSFER_Manager_Delete(&managerRunning);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARDATATRANSFER_Manager_Delete");
    
    ARUTILS_Manager_CloseWifiFtp(ftpListManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_CloseWifiFtp");
    
    ARUTILS_Manager_Delete(&ftpListManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_Delete");
    
    ARUTILS_Manager_CloseWifiFtp(ftpDataManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_CloseWifiFtp");
    
    ARUTILS_Manager_Delete(&ftpDataManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_Delete");

    ARSAL_Sem_Destroy(&semRunning);
}

void * test_manager_checking_thread_medias_list(void *arg)
{
    test_manager_thread_t *thread = (test_manager_thread_t*)arg;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    int resultSys = 0;
    int count;
    unsigned int timeout;
    struct timespec tm;
    int i;
    
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "");
    srand((unsigned)time(NULL));
    
    do
    {
        count = ARDATATRANSFER_MediasDownloader_GetAvailableMediasSync(thread->manager, 1, &result);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_GetAvailableMediasSync", result);
        test_manager_assert(result == ARDATATRANSFER_OK);
        
        for (i=0; i<count; i++)
        {
            ARDATATRANSFER_Media_t *media = ARDATATRANSFER_MediasDownloader_GetAvailableMediaAtIndex(thread->manager, i, &result);
            
            ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "media, name: %s, date: %s, size: %0.f, thumbnail: %d\n", media->name, media->date, media->size, media->thumbnailSize);
            
            result = ARDATATRANSFER_MediasDownloader_AddMediaToQueue(thread->manager, media, test_manager_medias_downloader_progress_callback, NULL, test_manager_medias_downloader_completion_callback, (void *)thread->tmp);
            ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_AddMediaToQueue", result);
            test_manager_assert(result == ARDATATRANSFER_OK);
        }
        
        timeout = ((unsigned int)rand()) & 0x0F;
        //tm.tv_sec = RUN_TIMEOUT;
        tm.tv_sec = timeout;
        tm.tv_nsec = 0;
        resultSys = ARSAL_Sem_Timedwait(&thread->sem, &tm);
        
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Sem_Timedwait", resultSys);
    }
    while (resultSys != 0);
    
    return NULL;
}

void test_manager_checking_running_async(const char *tmp)
{
    test_manager_thread_t threadList;
    ARDATATRANSFER_Manager_t *manager = NULL;
    ARSAL_Thread_t threadMediasDownloader;
    ARSAL_Thread_t threadMediasList;
    //ARSAL_Thread_t threadDataDownloader;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    int resultSys;
    //void *resultThreadData = NULL;
    void *resultThreadMediasDownloader = NULL;
    void *resultThreadMediasList = NULL;
    unsigned int timeout;
    
    srand((unsigned)time(NULL));
    ARSAL_Sem_Init(&semRunning, 0, 0);
    
    manager = ARDATATRANSFER_Manager_New(&result);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_Manager_New", result);
    test_manager_assert(result == ARDATATRANSFER_OK);
    
    do
    {
        threadList.manager = manager;
        threadList.tmp = tmp;
        
        resultSys = ARSAL_Sem_Init(&threadList.sem, 0, 0);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Sem_Init", resultSys);
        test_manager_assert(resultSys == 0);
        
        /*result = ARDATATRANSFER_MediasDownloader_New(manager, DEVICE_IP, DEVICE_PORT, "", tmp);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_New", result);
        test_manager_assert(result == ARDATATRANSFER_OK);*/
        
        resultSys = ARSAL_Thread_Create(&threadMediasDownloader, ARDATATRANSFER_MediasDownloader_QueueThreadRun, manager);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Thread_Create - ARDATATRANSFER_MediasDownloader_QueueThreadRun", resultSys);
        test_manager_assert(resultSys == 0);
        
        resultSys = ARSAL_Thread_Create(&threadMediasList, test_manager_checking_thread_medias_list, &threadList);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Thread_Create - test_manager_checking_thread_medias_list", resultSys);
        test_manager_assert(resultSys == 0);
        
        // signal end
        //timeout = ((unsigned int)rand()) & 0x3F;
        timeout = ((unsigned int)rand()) & 0x1F;
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %ds", "test_manager_timer_wait", timeout);
        test_manager_timer_wait(timeout);
        
        ARSAL_Sem_Post(&threadList.sem);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s:", "ARSAL_Sem_Post");
        
        result = ARDATATRANSFER_MediasDownloader_CancelQueueThread(manager);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_CancelQueueThread", result);
    
        // waiting exiting
        resultSys = ARSAL_Thread_Join(threadMediasDownloader, &resultThreadMediasDownloader);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Thread_Join - ARDATATRANSFER_MediasDownloader_QueueThreadRun", resultSys);
        test_manager_assert(resultSys == 0);
        
        resultSys = ARSAL_Thread_Join(threadMediasList, &resultThreadMediasList);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Thread_Join - ARDATATRANSFER_MediasDownloader_QueueThreadRun", resultSys);
        test_manager_assert(resultSys == 0);
        
        result = ARDATATRANSFER_MediasDownloader_Delete(manager);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_Delete", result);
        test_manager_assert(result == ARDATATRANSFER_OK);
        
        ARSAL_Sem_Destroy(&threadList.sem);
        
        resultSys = ARSAL_Sem_Trywait(&semRunning);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Sem_Timedwait", resultSys);
    }
    while (resultSys != 0);
    
    ARDATATRANSFER_Manager_Delete(&manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARDATATRANSFER_Manager_Delete");
           
    ARSAL_Sem_Destroy(&semRunning);
}

void test_manager_checking_running_signal()
{
    eARDATATRANSFER_ERROR result;
    int resultSys;

    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");

    resultSys = ARSAL_Sem_Post(&semRunning);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Sem_Post", resultSys);

    result = ARDATATRANSFER_DataDownloader_CancelThread(managerRunning);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_DataDownloader_CancelThread", result);

    result = ARDATATRANSFER_MediasDownloader_CancelQueueThread(managerRunning);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_CancelQueueThread", result);
}

void test_manager_available_media_callback(void* arg, ARDATATRANSFER_Media_t *media, int index)
{
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s, %s, %s, %d, %d, %s", ARDISCOVERY_getProductName(media->product), media->name, media->date, (int)media->size, (int)media->thumbnailSize, (char *)arg);
    
    arg = NULL;
}

void test_manager_available_media(const char *tmp)
{
    ARDATATRANSFER_Manager_t *manager;
    ARUTILS_Manager_t *ftpListManager;
    ARUTILS_Manager_t *ftpQueueManager;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    eARUTILS_ERROR resultUtils = ARUTILS_OK;
    
    ftpListManager = ARUTILS_Manager_New(&resultUtils);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_New", resultUtils);
    
    resultUtils = ARUTILS_Manager_InitWifiFtp(ftpListManager, DEVICE_IP, DEVICE_PORT, ARUTILS_FTP_ANONYMOUS, "");
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_InitWifiFtp", resultUtils);
    
    ftpQueueManager = ARUTILS_Manager_New(&resultUtils);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_New", resultUtils);
    
    resultUtils = ARUTILS_Manager_InitWifiFtp(ftpQueueManager, DEVICE_IP, DEVICE_PORT, ARUTILS_FTP_ANONYMOUS, "");
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_Manager_InitWifiFtp", resultUtils);
    
    manager = ARDATATRANSFER_Manager_New(&result);
    
    result = ARDATATRANSFER_MediasDownloader_New(manager, ftpListManager, ftpQueueManager, "", tmp);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_New", result);
    
    result = ARDATATRANSFER_MediasDownloader_GetAvailableMediasAsync(manager, test_manager_available_media_callback, "callback");
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_GetAvailableMediasAsync", result);
    
    ARUTILS_Manager_CloseWifiFtp(ftpListManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_CloseWifiFtp");
    
    ARUTILS_Manager_Delete(&ftpListManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_Delete");
    
    ARUTILS_Manager_CloseWifiFtp(ftpQueueManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_CloseWifiFtp");
    
    ARUTILS_Manager_Delete(&ftpQueueManager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARUTILS_Manager_Delete");
}

void test_manager(const char *tmp, int opt)
{
    test_manager_data_downloader(tmp);
    //test_manager_medias_downloader(tmp);
    //test_manager_available_media(tmp);
return;
    //opt = 2;

    if (opt == 0)
    {
        test_manager_checking_parameters(tmp);
        test_manager_checking_run_cancel(tmp);
    }
    else if (opt == 1)
    {
        test_manager_checking_running(tmp);
    }
    else if (opt == 2)
    {
        test_manager_checking_running_async(tmp);
    }
    else if (opt == 3)
    {
        test_manager_available_media(tmp);
    }

    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "**************************************************************");
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "completion");
}




