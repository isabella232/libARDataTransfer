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
#include <libARDiscovery/ARDISCOVERY_Discovery.h>

#include "libARDataTransfer/ARDATATRANSFER_Error.h"
#include "libARDataTransfer/ARDATATRANSFER_Manager.h"
#include "libARDataTransfer/ARDATATRANSFER_MediasDownloader.h"
#include "ARDATATRANSFER_MediasQueue.h"
#include "ARDATATRANSFER_DataDownloader.h"
#include "ARDATATRANSFER_MediasDownloader.h"
#include "ARDATATRANSFER_Manager.h"

#define ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG                "MediasDownloader"

#define ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_ROOT           ""
#define ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_MEDIA          "media"
#define ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_THUMB          "thumb"

#define ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_JPG            "jpg"
#define ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_MP4            "mp4"

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

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_GetAvailableMediasAsync(ARDATATRANSFER_Manager_t *manager, ARDATATRANSFER_MediasDownloader_AvailableMediaCallback_t availableMediaCallback, void *availableMediaArg)
{
    ARDATATRANSFER_MediaList_t mediaList;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    
    memset(&mediaList, 0, sizeof(ARDATATRANSFER_MediaList_t));
    
    result = ARDATATRANSFER_MediasDownloader_GetAvailableMediasInternal(manager, &mediaList, availableMediaCallback, availableMediaArg, 1);
    
    ARDATATRANSFER_MediasDownloader_FreeMediaList(&mediaList);
    
    return result;
}

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_GetAvailableMediasSync (ARDATATRANSFER_Manager_t *manager, ARDATATRANSFER_MediaList_t *mediaList, int withThumbnail)
{
    return ARDATATRANSFER_MediasDownloader_GetAvailableMediasInternal(manager,  mediaList, NULL, NULL, withThumbnail);
}

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_GetAvailableMediasInternal(ARDATATRANSFER_Manager_t *manager, ARDATATRANSFER_MediaList_t *mediaList, ARDATATRANSFER_MediasDownloader_AvailableMediaCallback_t availableMediaCallback, void *availableMediaArg, int withThumbnail)
{
    char remoteProduct[ARUTILS_FTP_MAX_PATH_SIZE];
    char productPathName[ARUTILS_FTP_MAX_PATH_SIZE];
    char *productFtpList = NULL;
    uint32_t productFtpListLen = 0;
    char *mediaFtpList = NULL;
    uint32_t mediaFtpListLen = 0;
    const char *nextProduct = NULL;
    const char *nextMedia = NULL;
    const char *lineItem;
    int lineSize;
    const char *fileName;
    int product;
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
        error = ARUTILS_Ftp_List(manager->mediasDownloader->listFtp, ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_ROOT, &productFtpList, &productFtpListLen);
        
        if (error != ARUTILS_OK)
        {
            result = ARDATATRANSFER_ERROR_FTP;
        }
    }
    
    product = 0;
    while ((result == ARDATATRANSFER_OK) && (product < ARDISCOVERY_PRODUCT_MAX))
    {
        error = ARUTILS_Ftp_IsCanceled(manager->mediasDownloader->listFtp);
        
        if (error != ARUTILS_OK)
        {
            result = ARDATATRANSFER_ERROR_CANCELED;
        }
        
        if (result == ARDATATRANSFER_OK)
        {
            ARDISCOVERY_getProductPathName(product, productPathName, sizeof(productPathName));
            nextProduct = NULL;
            fileName = ARUTILS_Ftp_List_GetNextItem(productFtpList, &nextProduct, productPathName, 1, NULL, NULL);
            
            if (fileName != NULL)
            {
                strncpy(remoteProduct, ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_ROOT "/", ARUTILS_FTP_MAX_PATH_SIZE);
                remoteProduct[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
                strncat(remoteProduct, productPathName, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteProduct) - 1);
                strncat(remoteProduct, "/" ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_MEDIA "/", ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteProduct) - 1);
                
                error = ARUTILS_Ftp_List(manager->mediasDownloader->listFtp, remoteProduct, &mediaFtpList, &mediaFtpListLen);
                if (error == ARUTILS_OK)
                {
                    nextMedia = NULL;
                    while ((result == ARDATATRANSFER_OK)
                           && (fileName = ARUTILS_Ftp_List_GetNextItem(mediaFtpList, &nextMedia, NULL, 0, &lineItem, &lineSize)) != NULL)
                    {
                        error = ARUTILS_Ftp_IsCanceled(manager->mediasDownloader->listFtp);
                        
                        if (error != ARUTILS_OK)
                        {
                            result = ARDATATRANSFER_ERROR_CANCELED;
                        }
                        
                        if (result == ARDATATRANSFER_OK)
                        {
                            char remotePath[ARUTILS_FTP_MAX_PATH_SIZE];
                            ARDATATRANSFER_Media_t **oldMedias;
                            ARDATATRANSFER_Media_t *media = NULL;
                            double fileSize;
                            const char *index;
                            const char *begin;
                            const char *end;

                            strncpy(remotePath, remoteProduct, ARUTILS_FTP_MAX_PATH_SIZE);
                            remotePath[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
                            strncat(remotePath, fileName, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remotePath) - 1);
                            
                            //do not pertorm ARUTILS_Ftp_Size that is too long, prefer decoding the FTP LIST
                            if (ARUTILS_Ftp_List_GetItemSize(lineItem, lineSize, &fileSize) != NULL)
                            {
                                error = ARUTILS_OK;
                            }
                            else
                            {
                                error = ARUTILS_ERROR_FTP_CODE;
                            }
                            
                            if ((result == ARDATATRANSFER_OK) && (error == ARUTILS_OK))
                            {
                                media = (ARDATATRANSFER_Media_t *)calloc(1, sizeof(ARDATATRANSFER_Media_t));

                                if (media == NULL)
                                {
                                    result = ARDATATRANSFER_ERROR_ALLOC;
                                }
                                else
                                {
                                    media->product = product;
                                    strncpy(media->name, fileName, ARDATATRANSFER_MEDIA_NAME_SIZE);
                                    media->name[ARDATATRANSFER_MEDIA_NAME_SIZE - 1] = '\0';
                                    
                                    strncpy(media->filePath, manager->mediasDownloader->localDirectory, ARDATATRANSFER_MEDIA_PATH_SIZE);
                                    media->filePath[ARDATATRANSFER_MEDIA_PATH_SIZE - 1] = '\0';
                                    strncat(media->filePath, fileName, ARDATATRANSFER_MEDIA_PATH_SIZE - strlen(media->filePath) - 1);
                                    
                                    strncpy(media->date, "", ARDATATRANSFER_MEDIA_DATE_SIZE);
                                    media->date[ARDATATRANSFER_MEDIA_DATE_SIZE - 1] = '\0';
                                    index = media->name;
                                    while ((index = strstr(index, "_")) != NULL)
                                    {
                                        begin = ++index;
                                    }
                                    
                                    end = NULL;
                                    if (begin != NULL)
                                    {
                                        end = strstr(begin, ".");
                                    }
                                    if ((begin != NULL) && (end != NULL))
                                    {
                                        int len = end - begin;
                                        len = (len < ARDATATRANSFER_MEDIA_DATE_SIZE) ? len : (ARDATATRANSFER_MEDIA_DATE_SIZE - 1);
                                        strncpy(media->date, begin, len);
                                        media->date[len] = '\0';
                                    }

                                    media->size = fileSize;

                                    if (withThumbnail == 1)
                                    {
                                        ARDATATRANSFER_MediasDownloader_GetThumbnail(manager, mediaFtpList, remoteProduct, media);
                                    }
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
                            
                            if ((result == ARDATATRANSFER_OK) && (availableMediaCallback != NULL) && (mediaList->count > 0))
                            {
                                availableMediaCallback(availableMediaArg, mediaList->medias[mediaList->count - 1]);
                            }
                        }
                    }
                }

                if (mediaFtpList != NULL)
                {
                    free(mediaFtpList);
                    mediaFtpList = NULL;
                    mediaFtpListLen = 0;
                }
            }
        }
        product++;
    }
    
    if (productFtpList != NULL)
    {
        free(productFtpList);
    }
    
    if (result != ARDATATRANSFER_OK)
    {
        ARDATATRANSFER_MediasDownloader_FreeMediaList(mediaList);
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

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_DeleteMedia(ARDATATRANSFER_Manager_t *manager, ARDATATRANSFER_Media_t *media)
{
    char remotePath[ARUTILS_FTP_MAX_PATH_SIZE];
    char remoteThumbnail[ARUTILS_FTP_MAX_PATH_SIZE];
    char productPathName[ARUTILS_FTP_MAX_PATH_SIZE];
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    eARUTILS_ERROR error = ARUTILS_OK;
    
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
        const char *prefixType = "";
        char *index = media->name + strlen(media->name);
        while (index > media->name && *index != '.')
        {
            index--;
        }
        if (*index == '.')
        {
            index++;
            
            if (strcmp(index, ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_JPG) == 0)
            {
                prefixType = index;
            }
            else if (strcmp(index, ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_MP4) == 0)
            {
                prefixType = index;
            }
        }
        
        ARDISCOVERY_getProductPathName(media->product, productPathName, sizeof(productPathName));
        
        strncpy(remotePath, ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_ROOT "/", ARUTILS_FTP_MAX_PATH_SIZE);
        remotePath[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
        strncat(remotePath, productPathName, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remotePath) - 1);
        strncat(remotePath, "/" ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_MEDIA "/", ARUTILS_FTP_MAX_PATH_SIZE - strlen(remotePath) - 1);
        strncat(remotePath, media->name, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remotePath) - 1);
        
        strncpy(remoteThumbnail, ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_ROOT "/", ARUTILS_FTP_MAX_PATH_SIZE);
        remoteThumbnail[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
        strncpy(remoteThumbnail, productPathName, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteThumbnail) - 1);
        strncat(remoteThumbnail, "/" ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_THUMB "/", ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteThumbnail) - 1);
        strncat(remoteThumbnail, media->name, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteThumbnail) - 1);
        if (strcmp(prefixType, ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_MP4) == 0)
        {
            strncat(remoteThumbnail, "." ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_JPG, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteThumbnail) - 1);
        }
        
        error = ARUTILS_Ftp_Delete(manager->mediasDownloader->deleteFtp, remotePath);
        
        if (error != ARUTILS_OK)
        {
            result = ARDATATRANSFER_ERROR_FTP;
        }
        
        ARUTILS_Ftp_Delete(manager->mediasDownloader->deleteFtp, remoteThumbnail);
    }

    return result;
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
            strncpy(newFtpMedia->media.filePath, media->filePath, ARDATATRANSFER_MEDIA_PATH_SIZE);
            newFtpMedia->media.filePath[ARDATATRANSFER_MEDIA_PATH_SIZE - 1] = '\0';
            strncpy(newFtpMedia->media.date, media->date, ARDATATRANSFER_MEDIA_DATE_SIZE);
            newFtpMedia->media.date[ARDATATRANSFER_MEDIA_DATE_SIZE - 1] = '\0';
            newFtpMedia->media.size = media->size;
            newFtpMedia->media.product = media->product;

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

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_CancelGetAvailableMedias(ARDATATRANSFER_Manager_t *manager)
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
        resultSys = ARSAL_Sem_Post(&manager->mediasDownloader->listSem);
        
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
        strncat(manager->mediasDownloader->localDirectory, "/" ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_MEDIA "/" , ARUTILS_FTP_MAX_PATH_SIZE - strlen(manager->mediasDownloader->localDirectory) - 1);
        
        resultSys = mkdir(manager->mediasDownloader->localDirectory, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        
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
        manager->mediasDownloader->deleteFtp = ARUTILS_Ftp_Connection_New(NULL, deviceIP, port, ARUTILS_FTP_ANONYMOUS, "", &error);
        
        if (error != ARUTILS_OK)
        {
            result = ARDATATRANSFER_ERROR_FTP;
        }
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        manager->mediasDownloader->ftp = ARUTILS_Ftp_Connection_New(&manager->mediasDownloader->threadSem, deviceIP, port, ARUTILS_FTP_ANONYMOUS, "", &error);
        
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
        
        if (manager->mediasDownloader->deleteFtp != NULL)
        {
            ARUTILS_Ftp_Connection_Delete(&manager->mediasDownloader->deleteFtp);
        }
        
        if (manager->mediasDownloader->ftp != NULL)
        {
            ARUTILS_Ftp_Connection_Delete(&manager->mediasDownloader->ftp);
        }
        
        manager->mediasDownloader->isCanceled = 0;
    }
}

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_GetThumbnail(ARDATATRANSFER_Manager_t *manager, const char *fileList, const char *remoteDir, ARDATATRANSFER_Media_t *media)
{
    char remoteThumbnail[ARUTILS_FTP_MAX_PATH_SIZE];
    char productPathName[ARUTILS_FTP_MAX_PATH_SIZE];
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    eARUTILS_ERROR error = ARUTILS_OK;
    
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG, "");
    
    if ((manager == NULL) || (fileList == NULL) || (media == NULL))
    {
        result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        const char *prefixType = "";
        char *index = media->name + strlen(media->name);
        while (index > media->name && *index != '.')
        {
            index--;
        }
        if (*index == '.')
        {
            index++;
            
            if (strcmp(index, ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_JPG) == 0)
            {
                prefixType = index;
            }
            else if (strcmp(index, ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_MP4) == 0)
            {
                prefixType = index;
            }
        }
        
        ARDISCOVERY_getProductPathName(media->product, productPathName, sizeof(productPathName));
        strncpy(remoteThumbnail, ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_ROOT "/", ARUTILS_FTP_MAX_PATH_SIZE);
        remoteThumbnail[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
        strncat(remoteThumbnail, productPathName, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteThumbnail) - 1);
        strncat(remoteThumbnail, "/" ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_THUMB "/", ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteThumbnail) - 1);
        strncat(remoteThumbnail, media->name, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteThumbnail) - 1);
        if (strcmp(prefixType, ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_MP4) == 0)
        {
            strncat(remoteThumbnail, "." ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_JPG, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteThumbnail) - 1);
        }
        
        error = ARUTILS_Ftp_Get_WithBuffer(manager->mediasDownloader->listFtp, remoteThumbnail, &media->thumbnail, &media->thumbnailSize, NULL, NULL);
        
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
    char restorePath[ARUTILS_FTP_MAX_PATH_SIZE];
    char productPathName[ARUTILS_FTP_MAX_PATH_SIZE];
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
        ARDISCOVERY_getProductPathName(ftpMedia->media.product, productPathName, sizeof(productPathName));
        strncpy(remotePath, ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_ROOT "/", ARUTILS_FTP_MAX_PATH_SIZE);
        remotePath[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
        strncat(remotePath, productPathName, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remotePath) - 1);
        strncat(remotePath, "/" ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_MEDIA "/", ARUTILS_FTP_MAX_PATH_SIZE - strlen(remotePath) - 1);
        strncat(remotePath, ftpMedia->media.name, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remotePath) - 1);
        
        strncpy(restorePath, ftpMedia->media.filePath, ARUTILS_FTP_MAX_PATH_SIZE);
        restorePath[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';

        strncpy(localPath, manager->mediasDownloader->localDirectory, ARUTILS_FTP_MAX_PATH_SIZE);
        localPath[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
        strncat(localPath, ARDATATRANSFER_MANAGER_DOWNLOADER_PREFIX_DOWNLOADING, ARUTILS_FTP_MAX_PATH_SIZE - strlen(localPath) - 1);
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
        error = ARUTILS_FileSystem_Rename(localPath, restorePath);
        
        if (error != ARUTILS_OK)
        {
            result = ARDATATRANSFER_ERROR_FILE;
        }
    }

    return result;
}
