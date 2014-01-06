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
#define DEVICE_IP       "172.20.5.117"  /* PC */
//#define DEVICE_IP       "192.168.1.1" /* ARDrone2 */
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

void test_manager_medias_downloader_progress_callback(void* arg, ARDATATRANSFER_Media_t *media, uint8_t percent)
{
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d%%", media->name, percent);
}

void test_manager_medias_downloader_completion_callback(void* arg, ARDATATRANSFER_Media_t *media, eARDATATRANSFER_ERROR error)
{
    const char *tmp = (const char *)arg;
    eARUTILS_ERROR errorFile = ARUTILS_OK;
    char localFile[ARUTILS_FTP_MAX_PATH_SIZE];
    
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", media->name, (int)error);
    
    if (tmp != NULL)
    {
        strcpy(localFile, tmp);
        strcat(localFile, "/medias/");
        strcat(localFile, media->name);
        
        errorFile = ARUTILS_FileSystem_RemoveFile(localFile);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARUTILS_FileSystem_RemoveFile", errorFile);
    }
}

void test_manager_data_downloader(const char *tmp)
{
    ARDATATRANSFER_Manager_t *manager;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    ARSAL_Thread_t threadDataDownloader;
    int resultSys;
    void *resultThread = NULL;

    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "**************************************************************");

    manager = ARDATATRANSFER_Manager_New(&result);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_Manager_New", result);

    result = ARDATATRANSFER_DataDownloader_New(manager, DEVICE_IP, DEVICE_PORT, tmp);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_DataDownloader_New", result);

    resultSys = ARSAL_Thread_Create(&threadDataDownloader, ARDATATRANSFER_DataDownloader_ThreadRun, manager);

    //result = ARDATATRANSFER_DataDownloader_CancelThread(manager);
    //ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_DataDownloader_CancelThread", result);

    resultSys = ARSAL_Thread_Join(threadDataDownloader, &resultThread);

    result = ARDATATRANSFER_DataDownloader_Delete(manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_DataDownloader_Delete", result);
    
    ARDATATRANSFER_Manager_Delete(&manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_Manager_Delete", result);
}

void test_manager_medias_downloader(const char *tmp)
{
    ARDATATRANSFER_MediaList_t mediaList;
    ARDATATRANSFER_Manager_t *manager;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    ARSAL_Thread_t threadMediasDownloader;
    int resultSys;
    void *resultThread = NULL;
    int i;

    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "**************************************************************");

    manager = ARDATATRANSFER_Manager_New(&result);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_Manager_New", result);

    result = ARDATATRANSFER_MediasDownloader_New(manager, DEVICE_IP, DEVICE_PORT, tmp);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_New", result);

    memset(&mediaList, 0, sizeof(ARDATATRANSFER_MediaList_t));

    result = ARDATATRANSFER_MediasDownloader_GetAvailableMedias(manager, &mediaList);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_GetAvailableMedias", result);

    for (i=0; i<mediaList.count; i++)
    {
        ARDATATRANSFER_Media_t *media = mediaList.medias[i];

        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "media, name: %s, date: %s, size: %0.f, thumbnail: %d\n", media->name, media->date, media->size, media->thumbnailSize);

        result = ARDATATRANSFER_MediasDownloader_AddMediaToQueue(manager, media, test_manager_medias_downloader_progress_callback, NULL, test_manager_medias_downloader_completion_callback, (void *)tmp);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_AddMediaToQueue", result);
    }

    ARDATATRANSFER_MediasDownloader_FreeMediaList(&mediaList);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s:", "ARDATATRANSFER_MediasDownloader_FreeMediaList");

    resultSys = ARSAL_Thread_Create(&threadMediasDownloader, ARDATATRANSFER_MediasDownloader_QueueThreadRun, manager);

    result = ARDATATRANSFER_MediasDownloader_GetAvailableMedias(manager, &mediaList);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_GetAvailableMedias", result);

    for (i=0; i<mediaList.count; i++)
    {
        ARDATATRANSFER_Media_t *media = mediaList.medias[i];

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
}

void test_manager_checking_parameters(const char *tmp)
{
    ARDATATRANSFER_MediaList_t mediaList;
    ARDATATRANSFER_Media_t media;
    ARDATATRANSFER_Manager_t *manager = NULL;
    ARSAL_Thread_t threadMediasDownloader;
    ARSAL_Thread_t threadDataDownloader;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    int resultSys;
    void *resultThread = NULL;

    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "**************************************************************");

    memset(&mediaList, 0, sizeof(ARDATATRANSFER_MediaList_t));
    memset(&media, 0, sizeof(ARDATATRANSFER_Media_t));

    //Without Manager
    ARDATATRANSFER_Manager_Delete(&manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARDATATRANSFER_Manager_Delete");

    result = ARDATATRANSFER_DataDownloader_New(manager, DEVICE_IP, DEVICE_PORT, tmp);
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

    result = ARDATATRANSFER_MediasDownloader_New(manager, DEVICE_IP, DEVICE_PORT, tmp);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_New", result);
    test_manager_assert(result == ARDATATRANSFER_ERROR_BAD_PARAMETER);
    
    result = ARDATATRANSFER_MediasDownloader_Delete(manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_Delete", result);
    test_manager_assert(result == ARDATATRANSFER_ERROR_BAD_PARAMETER);

    result = ARDATATRANSFER_MediasDownloader_GetAvailableMedias(manager, &mediaList);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_GetAvailableMedias", result);
    test_manager_assert(result == ARDATATRANSFER_ERROR_BAD_PARAMETER);

    ARDATATRANSFER_MediasDownloader_FreeMediaList(&mediaList);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARDATATRANSFER_MediasDownloader_FreeMediaList");

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
    result = ARDATATRANSFER_MediasDownloader_GetAvailableMedias(manager, &mediaList);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_GetAvailableMedias", result);
    test_manager_assert(result == ARDATATRANSFER_ERROR_NOT_INITIALIZED);

    ARDATATRANSFER_MediasDownloader_FreeMediaList(&mediaList);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARDATATRANSFER_MediasDownloader_FreeMediaList");

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
    result = ARDATATRANSFER_DataDownloader_New(manager, DEVICE_IP, DEVICE_PORT, tmp);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_DataDownloader_New", result);
    test_manager_assert(result == ARDATATRANSFER_OK);

    result = ARDATATRANSFER_DataDownloader_New(manager, DEVICE_IP, DEVICE_PORT, tmp);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_DataDownloader_New", result);
    test_manager_assert(result == ARDATATRANSFER_ERROR_ALREADY_INITIALIZED);

    result = ARDATATRANSFER_MediasDownloader_New(manager, DEVICE_IP, DEVICE_PORT, tmp);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_New", result);
    test_manager_assert(result == ARDATATRANSFER_OK);

    result = ARDATATRANSFER_MediasDownloader_New(manager, DEVICE_IP, DEVICE_PORT, tmp);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_New", result);
    test_manager_assert(result == ARDATATRANSFER_ERROR_ALREADY_INITIALIZED);

    ARDATATRANSFER_Manager_Delete(&manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARDATATRANSFER_Manager_Delete");
}

void test_manager_checking_run_cancel(const char *tmp)
{
    ARDATATRANSFER_MediaList_t mediaList;
    ARDATATRANSFER_Manager_t *manager;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    ARSAL_Thread_t threadDataDownloader;
    ARSAL_Thread_t threadMediasDownloader;
    int resultSys;
    void *resultThread = NULL;
    int i;

    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "**************************************************************");

    manager = ARDATATRANSFER_Manager_New(&result);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_Manager_New", result);
    test_manager_assert(result == ARDATATRANSFER_OK);

    //Data Downloader
    result = ARDATATRANSFER_DataDownloader_New(manager, DEVICE_IP, DEVICE_PORT, tmp);
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
    result = ARDATATRANSFER_MediasDownloader_New(manager, DEVICE_IP, DEVICE_PORT, tmp);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_New", result);
    test_manager_assert(result == ARDATATRANSFER_OK);

    memset(&mediaList, 0, sizeof(ARDATATRANSFER_MediaList_t));

    result = ARDATATRANSFER_MediasDownloader_GetAvailableMedias(manager, &mediaList);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_GetAvailableMedias", result);
    test_manager_assert(result == ARDATATRANSFER_OK);

    for (i=0; i<mediaList.count; i++)
    {
        ARDATATRANSFER_Media_t *media = mediaList.medias[i];

        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "media, name: %s, date: %s, size: %0.f, thumbnail: %d\n", media->name, media->date, media->size, media->thumbnailSize);

        result = ARDATATRANSFER_MediasDownloader_AddMediaToQueue(manager, media, test_manager_medias_downloader_progress_callback, NULL, test_manager_medias_downloader_completion_callback, NULL);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_AddMediaToQueue", result);
        test_manager_assert(result == ARDATATRANSFER_OK);
    }

    ARDATATRANSFER_MediasDownloader_FreeMediaList(&mediaList);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARDATATRANSFER_MediasDownloader_FreeMediaList");

    resultSys = ARSAL_Thread_Create(&threadMediasDownloader, ARDATATRANSFER_MediasDownloader_QueueThreadRun, manager);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Thread_Create - ARDATATRANSFER_MediasDownloader_QueueThreadRun", resultSys);
    test_manager_assert(resultSys == 0);

    result = ARDATATRANSFER_MediasDownloader_GetAvailableMedias(manager, &mediaList);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_GetAvailableMedias", result);
    test_manager_assert(result == ARDATATRANSFER_OK);

    for (i=0; i<mediaList.count; i++)
    {
        ARDATATRANSFER_Media_t *media = mediaList.medias[i];

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
}

void test_manager_checking_running(const char *tmp)
{
    ARDATATRANSFER_MediaList_t mediaList;
    ARSAL_Thread_t threadMediasDownloader;
    ARSAL_Thread_t threadDataDownloader;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    int resultSys;
    void *resultThread = NULL;

    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "**************************************************************");

    memset(&mediaList, 0, sizeof(ARDATATRANSFER_MediaList_t));
    ARSAL_Sem_Init(&semRunning, 0, 0);

    managerRunning = ARDATATRANSFER_Manager_New(&result);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_Manager_New", result);
    test_manager_assert(result == ARDATATRANSFER_OK);

    //Data
    result = ARDATATRANSFER_DataDownloader_New(managerRunning, DEVICE_IP, DEVICE_PORT, tmp);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_DataDownloader_New", result);
    test_manager_assert(result == ARDATATRANSFER_OK);

    resultSys = ARSAL_Thread_Create(&threadDataDownloader, ARDATATRANSFER_DataDownloader_ThreadRun, managerRunning);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Thread_Create - ARDATATRANSFER_DataDownloader_ThreadRun", resultSys);
    test_manager_assert(resultSys == 0);

    //Medias
    result = ARDATATRANSFER_MediasDownloader_New(managerRunning, DEVICE_IP, DEVICE_PORT, tmp);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_New", result);
    test_manager_assert(result == ARDATATRANSFER_OK);

    resultSys = ARSAL_Thread_Create(&threadMediasDownloader, ARDATATRANSFER_MediasDownloader_QueueThreadRun, managerRunning);
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARSAL_Thread_Create - ARDATATRANSFER_MediasDownloader_QueueThreadRun", resultSys);
    test_manager_assert(resultSys == 0);

    do
    {
        struct timespec tm;
        int i;

        result = ARDATATRANSFER_MediasDownloader_GetAvailableMedias(managerRunning, &mediaList);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_GetAvailableMedias", result);
        test_manager_assert(result == ARDATATRANSFER_OK);

        for (i=0; i<mediaList.count; i++)
        {
            ARDATATRANSFER_Media_t *media = mediaList.medias[i];

            ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "media, name: %s, date: %s, size: %0.f, thumbnail: %d\n", media->name, media->date, media->size, media->thumbnailSize);

            result = ARDATATRANSFER_MediasDownloader_AddMediaToQueue(managerRunning, media, test_manager_medias_downloader_progress_callback, NULL, test_manager_medias_downloader_completion_callback, NULL);
            ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_AddMediaToQueue", result);
            test_manager_assert(result == ARDATATRANSFER_OK);
        }

        ARDATATRANSFER_MediasDownloader_FreeMediaList(&mediaList);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARDATATRANSFER_MediasDownloader_FreeMediaList");

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

    ARSAL_Sem_Destroy(&semRunning);
}

void * test_manager_checking_thread_medias_list(void *arg)
{
    test_manager_thread_t *thread = (test_manager_thread_t*)arg;
    ARDATATRANSFER_MediaList_t mediaList;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    int resultSys = 0;
    unsigned int timeout;
    struct timespec tm;
    int i;
    
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "");
    srand(time(NULL));
    memset(&mediaList, 0, sizeof(ARDATATRANSFER_MediaList_t));
    
    do
    {
        result = ARDATATRANSFER_MediasDownloader_GetAvailableMedias(thread->manager, &mediaList);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_GetAvailableMedias", result);
        test_manager_assert(result == ARDATATRANSFER_OK);
        
        for (i=0; i<mediaList.count; i++)
        {
            ARDATATRANSFER_Media_t *media = mediaList.medias[i];
            
            ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "media, name: %s, date: %s, size: %0.f, thumbnail: %d\n", media->name, media->date, media->size, media->thumbnailSize);
            
            result = ARDATATRANSFER_MediasDownloader_AddMediaToQueue(thread->manager, media, test_manager_medias_downloader_progress_callback, NULL, test_manager_medias_downloader_completion_callback, (void *)thread->tmp);
            ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_AddMediaToQueue", result);
            test_manager_assert(result == ARDATATRANSFER_OK);
        }
    
        ARDATATRANSFER_MediasDownloader_FreeMediaList(&mediaList);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s", "ARDATATRANSFER_MediasDownloader_FreeMediaList");
        
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
    
    srand(time(NULL));
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
        
        result = ARDATATRANSFER_MediasDownloader_New(manager, DEVICE_IP, DEVICE_PORT, tmp);
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "%s: %d", "ARDATATRANSFER_MediasDownloader_New", result);
        test_manager_assert(result == ARDATATRANSFER_OK);
        
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

void test_manager(const char *tmp, int opt)
{
    //test_manager_data_downloader(tmp);
    //test_manager_medias_downloader(tmp);

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

    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "**************************************************************");
    ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "completion");
}




