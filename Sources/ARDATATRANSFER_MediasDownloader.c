/**
 * @file ARDATATRANSFER_MediasManager.c
 * @brief libARDataTransfer MediasManager c file.
 * @date 19/12/2013
 * @author david.flattin.ext@parrot.com
 **/

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <libARSAL/ARSAL_Sem.h>
#include <libARSAL/ARSAL_Mutex.h>
#include <libARSAL/ARSAL_Print.h>
#include <libARUtils/ARUTILS_Error.h>
#include <libARUtils/ARUTILS_Ftp.h>
#include <libARUtils/ARUTILS_FileSystem.h>

#include "libARDataTransfer/ARDATATRANSFER_Error.h"
#include "libARDataTransfer/ARDATATRANSFER_Manager.h"
#include "libARDataTransfer/ARDATATRANSFER_MediasDownloader.h"
#include "ARDATATRANSFER_MediasQueue.h"
#include "ARDATATRANSFER_DataDownloader.h"
#include "ARDATATRANSFER_MediasDownloader.h"
#include "ARDATATRANSFER_Manager.h"

#define ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG                "MediasDownloader"

#define ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_ROOT           "medias"
#define ARDATATRANSFER_MEDIAS_DOWNLOADER_PREFIX_THUMBNAIL   "thumbnail_"
#define ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_JPG            "jpg"

/*****************************************
 *
 *             Public implementation:
 *
 *****************************************/

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_New(ARDATATRANSFER_Manager_t *manager, const char *deviceIP, int port, const char *localDirectory)
{
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    int resultSys = 0;
    
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG, "");
    
    if (manager == NULL)
    {
        result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        if (manager->mediasDownloader != NULL)
        {
            result = ARDATATRANSFER_ERROR_ALREADY_INITIALIZED;
        }
        else
        {
            manager->mediasDownloader = (ARDATATRANSFER_MediasDownloader_t *)calloc(1, sizeof(ARDATATRANSFER_MediasDownloader_t));
            
            if (manager->mediasDownloader == NULL)
            {
                result = ARDATATRANSFER_ERROR_ALLOC;
            }
        }
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        resultSys = ARSAL_Sem_Init(&manager->mediasDownloader->queueSem, 0, 0);
        
        if (resultSys != 0)
        {
            result = ARDATATRANSFER_ERROR_SYSTEM;
        }
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        resultSys = ARSAL_Sem_Init(&manager->mediasDownloader->listSem, 0, 0);
        
        if (resultSys != 0)
        {
            result = ARDATATRANSFER_ERROR_SYSTEM;
        }
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        resultSys = ARSAL_Sem_Init(&manager->mediasDownloader->threadSem, 0, 0);
        
        if (resultSys != 0)
        {
            result = ARDATATRANSFER_ERROR_SYSTEM;
        }
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        ARDATATRANSFER_MediasQueue_New(&manager->mediasDownloader->queue);
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        manager->mediasDownloader->isRunning = 0;
        manager->mediasDownloader->isCanceled = 0;
        
        result = ARDATATRANSFER_MediasDownloader_Initialize(manager, deviceIP, port, localDirectory);
    }
    
    if (result != ARDATATRANSFER_OK && result != ARDATATRANSFER_ERROR_ALREADY_INITIALIZED)
    {
        ARDATATRANSFER_MediasDownloader_Delete(manager);
    }
    
    return result;
}

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_Delete(ARDATATRANSFER_Manager_t *manager)
{
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG, "");
    
    
    if (manager == NULL)
    {
        result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
    }
    else
    {
        if (manager->mediasDownloader == NULL)
        {
            result = ARDATATRANSFER_ERROR_NOT_INITIALIZED;
        }
        else
        {
            if (manager->mediasDownloader->isRunning != 0)
            {
                result = ARDATATRANSFER_ERROR_THREAD_PROCESSING;
            }
            else
            {
                ARSAL_Sem_Post(&manager->mediasDownloader->listSem);
                ARDATATRANSFER_MediasDownloader_CancelQueueThread(manager);
                
                ARDATATRANSFER_MediasDownloader_Clear(manager);
                
                ARSAL_Sem_Destroy(&manager->mediasDownloader->queueSem);
                ARSAL_Sem_Destroy(&manager->mediasDownloader->listSem);
                ARSAL_Sem_Destroy(&manager->mediasDownloader->threadSem);
                
                ARDATATRANSFER_MediasQueue_Delete(&manager->mediasDownloader->queue);
                
                free(manager->mediasDownloader);
                manager->mediasDownloader = NULL;
            }
        }
    }
    
    return result;
}

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_GetAvailableMedias(ARDATATRANSFER_Manager_t *manager, ARDATATRANSFER_MediaList_t *mediaList)
{
    char *resultList = NULL;
    uint32_t resultListLen = 0;
    const char *nextItem = NULL;
    const char *fileName;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    eARUTILS_ERROR error = ARUTILS_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG, "");

    if ((manager == NULL) || (mediaList == NULL))
    {
        result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
    }

    if (result == ARDATATRANSFER_OK && (manager->mediasDownloader == NULL))
    {
        result = ARDATATRANSFER_ERROR_NOT_INITIALIZED;
    }

    if ((result == ARDATATRANSFER_OK)
        && ((mediaList->medias != NULL) || (mediaList->count != 0)))
    {
        result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
    }

    if (result == ARDATATRANSFER_OK)
    {
        error = ARUTILS_Ftp_List(manager->mediasDownloader->listFtp, ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_ROOT, &resultList, &resultListLen);
        
        if (error != ARUTILS_OK)
        {
            result = ARDATATRANSFER_ERROR_FTP;
        }
    }

    while ((result == ARDATATRANSFER_OK)
           && (fileName = ARDATATRANSFER_Manager_List_GetNextItem(resultList, &nextItem, NULL, 0)) != NULL)
    {
        if (strncmp(fileName, ARDATATRANSFER_MEDIAS_DOWNLOADER_PREFIX_THUMBNAIL, strlen(ARDATATRANSFER_MEDIAS_DOWNLOADER_PREFIX_THUMBNAIL)) != 0)
        {
            char remotePath[ARUTILS_FTP_MAX_PATH_SIZE];
            ARDATATRANSFER_Media_t **oldMedias;
            ARDATATRANSFER_Media_t *media = NULL;
            double fileSize;
            const char *begin;
            const char *end;

            strncpy(remotePath, ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_ROOT "/", ARUTILS_FTP_MAX_PATH_SIZE);
            remotePath[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
            strncat(remotePath, fileName, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remotePath) - 1);
            error = ARUTILS_Ftp_Size(manager->mediasDownloader->listFtp, remotePath, &fileSize);
            
            if (error != ARUTILS_OK)
            {
                result = ARDATATRANSFER_ERROR_FTP;
            }

            if (result == ARDATATRANSFER_OK)
            {
                media = (ARDATATRANSFER_Media_t *)calloc(1, sizeof(ARDATATRANSFER_Media_t));

                if (media == NULL)
                {
                    result = ARDATATRANSFER_ERROR_ALLOC;
                }
                else
                {
                    strncpy(media->name, fileName, ARDATATRANSFER_MEDIA_NAME_SIZE);
                    media->name[ARDATATRANSFER_MEDIA_NAME_SIZE - 1] = '\0';
                    strncpy(media->date, "", ARDATATRANSFER_MEDIA_DATE_SIZE);
                    media->date[ARDATATRANSFER_MEDIA_DATE_SIZE - 1] = '\0';
                    begin = strstr(media->name, "_");
                    end = strstr(media->name, ".");
                    if ((begin != NULL) && (end != NULL))
                    {
                        int len = end - (++begin);
                        strncpy(media->date, begin, len);
                        media->date[len] = '\0';
                    }

                    media->size = fileSize;

                    ARDATATRANSFER_MediasDownloader_GetThumbnail(manager, media);
                }
            }

            if (result == ARDATATRANSFER_OK)
            {
                oldMedias = mediaList->medias;
                mediaList->medias = (ARDATATRANSFER_Media_t **)realloc(mediaList->medias, (mediaList->count + 1) * sizeof(ARDATATRANSFER_Media_t *));

                if (mediaList->medias == NULL)
                {
                    mediaList->medias = oldMedias;
                    result = ARDATATRANSFER_ERROR_ALLOC;
                    free(media);
                }
                else
                {
                    mediaList->medias[mediaList->count] = media;
                    mediaList->count++;
                }
            }
        }
    }

    if (resultList != NULL)
    {
        free(resultList);
    }

    return result;
}

void ARDATATRANSFER_MediasDownloader_FreeMediaList(ARDATATRANSFER_MediaList_t *mediaList)
{
    int i = 0;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG, "");

    if (mediaList != NULL)
    {
        if (mediaList->medias != NULL)
        {
            for (i=0; i<mediaList->count; i++)
            {
                ARDATATRANSFER_Media_t *media = mediaList->medias[i];

                if (media->thumbnail != NULL)
                {
                    free(media->thumbnail);
                }

                free(media);
            }
        }

        mediaList->medias = NULL;
        mediaList->count = 0;
    }
}

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_AddMediaToQueue(ARDATATRANSFER_Manager_t *manager, ARDATATRANSFER_Media_t *media,  ARDATATRANSFER_MediasDownloader_MediaDownloadProgressCallback_t progressCallback, void *progressArg, ARDATATRANSFER_MediasDownloader_MediaDownloadCompletionCallback_t completionCallback, void *completionArg)
{
    ARDATATRANSFER_FtpMedia_t *newFtpMedia = NULL;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG, "");

    if ((manager == NULL) || (media == NULL))
    {
        result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
    }

    if (result == ARDATATRANSFER_OK && (manager->mediasDownloader == NULL))
    {
        result = ARDATATRANSFER_ERROR_NOT_INITIALIZED;
    }

    if (result == ARDATATRANSFER_OK)
    {
        newFtpMedia = calloc(1, sizeof(ARDATATRANSFER_FtpMedia_t));

        if (newFtpMedia == NULL)
        {
            result = ARDATATRANSFER_ERROR_ALLOC;
        }
        else
        {
            strncpy(newFtpMedia->media.name, media->name, ARDATATRANSFER_MEDIA_NAME_SIZE);
            newFtpMedia->media.name[ARDATATRANSFER_MEDIA_NAME_SIZE - 1] = '\0';
            strncpy(newFtpMedia->media.date, media->date, ARDATATRANSFER_MEDIA_DATE_SIZE);
            newFtpMedia->media.date[ARDATATRANSFER_MEDIA_DATE_SIZE - 1] = '\0';
            newFtpMedia->media.size = media->size;

            newFtpMedia->progressCallback = progressCallback;
            newFtpMedia->progressArg = progressArg;
            newFtpMedia->completionCallback = completionCallback;
            newFtpMedia->completionArg = completionArg;
        }
    }

    if (result == ARDATATRANSFER_OK)
    {
        result = ARDATATRANSFER_MediasQueue_Add(&manager->mediasDownloader->queue, newFtpMedia);
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        ARSAL_Sem_Post(&manager->mediasDownloader->queueSem);
    }

    if (result != ARDATATRANSFER_OK)
    {
        free(newFtpMedia);
    }

    return result;
}

void* ARDATATRANSFER_MediasDownloader_QueueThreadRun(void *managerArg)
{
    ARDATATRANSFER_Manager_t *manager = (ARDATATRANSFER_Manager_t *)managerArg;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG, "");

    if (manager == NULL)
    {
        result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
    }

    if (result == ARDATATRANSFER_OK && (manager->mediasDownloader == NULL))
    {
        result = ARDATATRANSFER_ERROR_NOT_INITIALIZED;
    }
    
    if (result == ARDATATRANSFER_OK && (manager->mediasDownloader->isCanceled != 0))
    {
        result = ARDATATRANSFER_ERROR_CANCELED;
    }

    if (result == ARDATATRANSFER_OK && (manager->mediasDownloader->isRunning != 0))
    {
        result = ARDATATRANSFER_ERROR_THREAD_ALREADY_RUNNING;
    }

    if (result == ARDATATRANSFER_OK)
    {
        manager->mediasDownloader->isRunning = 1;
    }

    if (result == ARDATATRANSFER_OK)
    {
        eARDATATRANSFER_ERROR error = ARDATATRANSFER_OK;
        ARDATATRANSFER_FtpMedia_t *ftpMedia = NULL;
        int resultSys;

        do
        {
            resultSys = ARSAL_Sem_Wait(&manager->mediasDownloader->queueSem);
            
            if (resultSys != 0)
            {
                result = ARDATATRANSFER_ERROR_SYSTEM;
            }
            
            if (result == ARDATATRANSFER_OK)
            {
                ftpMedia = ARDATATRANSFER_MediasQueue_Pop(&manager->mediasDownloader->queue, &error);
            }

            if ((result == ARDATATRANSFER_OK)
                && (error == ARDATATRANSFER_OK)
                && (ftpMedia != NULL)
                && (manager->mediasDownloader->isCanceled == 0))
            {
                error = ARDATATRANSFER_MediasDownloader_DownloadMedia(manager, ftpMedia);
            }

            if (ftpMedia != NULL)
            {
                if (ftpMedia->completionCallback != NULL)
                {
                    ftpMedia->completionCallback(ftpMedia->completionArg, &ftpMedia->media, error);
                }

                free(ftpMedia);
            }
        }
        while (manager->mediasDownloader->isCanceled == 0);
    }

    if (manager != NULL && manager->mediasDownloader != NULL)
    {
        manager->mediasDownloader->isRunning = 0;
    }

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG, "exit");

    return NULL;
}

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_CancelQueueThread(ARDATATRANSFER_Manager_t *manager)
{
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    int resultSys = 0;
    
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG, "");
    
    if (manager == NULL)
    {
        result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
    }
    
    if (result == ARDATATRANSFER_OK && (manager->mediasDownloader == NULL))
    {
        result = ARDATATRANSFER_ERROR_NOT_INITIALIZED;
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        manager->mediasDownloader->isCanceled = 1;
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        result = ARDATATRANSFER_MediasQueue_RemoveAll(&manager->mediasDownloader->queue);
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        resultSys = ARSAL_Sem_Post(&manager->mediasDownloader->threadSem);
        
        if (resultSys != 0)
        {
            result = ARDATATRANSFER_ERROR_SYSTEM;
        }
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        resultSys = ARSAL_Sem_Post(&manager->mediasDownloader->queueSem);
        
        if (resultSys != 0)
        {
            result = ARDATATRANSFER_ERROR_SYSTEM;
        }
    }
    
    return result;
}

/*****************************************
 *
 *             Private implementation:
 *
 *****************************************/

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_Initialize(ARDATATRANSFER_Manager_t *manager, const char *deviceIP, int port, const char *localDirectory)
{
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    eARUTILS_ERROR error = ARUTILS_OK;
    int resultSys = 0;
    
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG, "%s, %s, %d", deviceIP ? deviceIP : "null", localDirectory ? localDirectory : "null", port);
    
    if ((manager == NULL) || (deviceIP == NULL) || (localDirectory == NULL))
    {
        result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        strncpy(manager->mediasDownloader->localDirectory, localDirectory, ARUTILS_FTP_MAX_PATH_SIZE);
        manager->mediasDownloader->localDirectory[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
        
        char localPath[ARUTILS_FTP_MAX_PATH_SIZE];
        strncpy(localPath, localDirectory, ARUTILS_FTP_MAX_PATH_SIZE);
        localPath[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
        strncat(localPath, "/" ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_ROOT, ARUTILS_FTP_MAX_PATH_SIZE - strlen(localPath) - 1);
        
        resultSys = mkdir(localPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        
        if ((resultSys != 0) && (errno != EEXIST))
        {
            result = ARDATATRANSFER_ERROR_SYSTEM;
        }
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        manager->mediasDownloader->listFtp = ARUTILS_Ftp_Connection_New(&manager->mediasDownloader->listSem, deviceIP, port, ARUTILS_FTP_ANONYMOUS, "", &error);
        
        if (error != ARUTILS_OK)
        {
            result = ARDATATRANSFER_ERROR_FTP;
        }
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        manager->mediasDownloader->ftp = ARUTILS_Ftp_Connection_New(&manager->mediasDownloader->threadSem, deviceIP, 21, ARUTILS_FTP_ANONYMOUS, "", &error);
        
        if (error != ARUTILS_OK)
        {
            result = ARDATATRANSFER_ERROR_FTP;
        }
    }
    
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG, "return %d", result);
    
    return result;
}

void ARDATATRANSFER_MediasDownloader_Clear(ARDATATRANSFER_Manager_t *manager)
{
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG, "");
    
    if (manager != NULL)
    {
        if (manager->mediasDownloader->listFtp != NULL)
        {
            ARUTILS_Ftp_Connection_Delete(&manager->mediasDownloader->listFtp);
        }
        
        if (manager->mediasDownloader->ftp != NULL)
        {
            ARUTILS_Ftp_Connection_Delete(&manager->mediasDownloader->ftp);
        }
        
        manager->mediasDownloader->isCanceled = 0;
    }
}

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_GetThumbnail(ARDATATRANSFER_Manager_t *manager, ARDATATRANSFER_Media_t *media)
{
    char remotePath[ARUTILS_FTP_MAX_PATH_SIZE];
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    eARUTILS_ERROR error = ARUTILS_OK;
    
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG, "");
    
    if ((manager == NULL) || (media == NULL))
    {
        result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        strncpy(remotePath, ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_ROOT "/", ARUTILS_FTP_MAX_PATH_SIZE);
        remotePath[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
        strncat(remotePath, ARDATATRANSFER_MEDIAS_DOWNLOADER_PREFIX_THUMBNAIL, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remotePath) - 1);
        strncat(remotePath, media->name, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remotePath) - 1);
        char *index = remotePath + strlen(remotePath);
        while (index > remotePath && *index != '.')
        {
            index--;
        }
        
        if (*index == '.')
        {
            index++;
            strncpy(index, ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_JPG, strlen(ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_JPG));
            remotePath[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
        }
        
        error = ARUTILS_Ftp_Get_WithBuffer(manager->mediasDownloader->listFtp, remotePath, &media->thumbnail, &media->thumbnailSize, NULL, NULL);
        
        if (error != ARUTILS_OK)
        {
            result = ARDATATRANSFER_ERROR_FTP;
        }
    }
    
    return result;
}

void ARDATATRANSFER_MediasDownloader_FtpProgressCallback(void* arg, uint8_t percent)
{
    ARDATATRANSFER_FtpMedia_t *ftpMedia = (ARDATATRANSFER_FtpMedia_t *)arg;

    if (ftpMedia != NULL)
    {
        if (ftpMedia->progressCallback != NULL)
        {
            ftpMedia->progressCallback(ftpMedia->progressArg, &ftpMedia->media, percent);
        }
    }
}

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_DownloadMedia(ARDATATRANSFER_Manager_t *manager, ARDATATRANSFER_FtpMedia_t *ftpMedia)
{
    char remotePath[ARUTILS_FTP_MAX_PATH_SIZE];
    char localPath[ARUTILS_FTP_MAX_PATH_SIZE];
    char downlodedPath[ARUTILS_FTP_MAX_PATH_SIZE];
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    eARUTILS_ERROR errorResume = ARUTILS_OK;
    eARUTILS_ERROR error = ARUTILS_OK;
    uint32_t localSize = 0;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG, "");

    if ((manager  == NULL) || (ftpMedia == NULL))
    {
        result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
    }

    if (result == ARDATATRANSFER_OK)
    {
        strncpy(remotePath, ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_ROOT "/", ARUTILS_FTP_MAX_PATH_SIZE);
        remotePath[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
        strncat(remotePath, ftpMedia->media.name, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remotePath) - 1);

        strncpy(downlodedPath, manager->mediasDownloader->localDirectory, ARUTILS_FTP_MAX_PATH_SIZE);
        downlodedPath[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
        strncat(downlodedPath, "/" ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_ROOT "/", ARUTILS_FTP_MAX_PATH_SIZE - strlen(downlodedPath) - 1);
        strncat(downlodedPath, ftpMedia->media.name, ARUTILS_FTP_MAX_PATH_SIZE - strlen(downlodedPath) - 1);

        strncpy(localPath, manager->mediasDownloader->localDirectory, ARUTILS_FTP_MAX_PATH_SIZE);
        localPath[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
        strncat(localPath, "/" ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_ROOT "/"ARDATATRANSFER_MANAGER_DOWNLOADER_PREFIX_DOWNLOADING, ARUTILS_FTP_MAX_PATH_SIZE - strlen(localPath) - 1);
        strncat(localPath, ftpMedia->media.name, ARUTILS_FTP_MAX_PATH_SIZE - strlen(localPath) - 1);

        errorResume = ARUTILS_FileSystem_GetFileSize(localPath, &localSize);
    }

    if (result == ARDATATRANSFER_OK)
    {
        error = ARUTILS_Ftp_Get(manager->mediasDownloader->ftp, remotePath, localPath, ARDATATRANSFER_MediasDownloader_FtpProgressCallback, ftpMedia, (errorResume == ARUTILS_OK) ? FTP_RESUME_TRUE : FTP_RESUME_FALSE);
        
        if (error == ARUTILS_ERROR_FTP_CANCELED)
        {
            result = ARDATATRANSFER_ERROR_CANCELED;
        }
        else if (error != ARUTILS_OK)
        {
            result = ARDATATRANSFER_ERROR_FTP;
        }
    }

    if (result == ARDATATRANSFER_OK)
    {
        error = ARUTILS_FileSystem_Rename(localPath, downlodedPath);
        
        if (error != ARUTILS_OK)
        {
            result = ARDATATRANSFER_ERROR_FILE;
        }
    }

    return result;
}
