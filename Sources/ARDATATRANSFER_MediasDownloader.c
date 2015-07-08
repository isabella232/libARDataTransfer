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
#include <libARUtils/ARUTILS_Manager.h>
#include <libARUtils/ARUTILS_Ftp.h>
#include <libARUtils/ARUTILS_FileSystem.h>
#include <libARDiscovery/ARDISCOVERY_Discovery.h>

#include "libARDataTransfer/ARDATATRANSFER_Error.h"
#include "libARDataTransfer/ARDATATRANSFER_Manager.h"
#include "libARDataTransfer/ARDATATRANSFER_Downloader.h"
#include "libARDataTransfer/ARDATATRANSFER_Uploader.h"
#include "libARDataTransfer/ARDATATRANSFER_DataDownloader.h"
#include "libARDataTransfer/ARDATATRANSFER_MediasDownloader.h"
#include "ARDATATRANSFER_Downloader.h"
#include "ARDATATRANSFER_Uploader.h"
#include "ARDATATRANSFER_MediasQueue.h"
#include "ARDATATRANSFER_DataDownloader.h"
#include "ARDATATRANSFER_MediasDownloader.h"
#include "ARDATATRANSFER_Manager.h"

#define ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG                "MediasDownloader"

#define ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_MEDIA          "media"
#define ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_THUMB          "thumb"

#define ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_JPG            "jpg"
#define ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_MP4            "mp4"
#define ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_MOV            "mov"

/*****************************************
 *
 *             Public implementation:
 *
 *****************************************/

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_New(ARDATATRANSFER_Manager_t *manager, ARUTILS_Manager_t *ftpListManager, ARUTILS_Manager_t *ftpQueueManager, const char *remoteDirectory, const char *localDirectory)
{
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    int resultSys = 0;
    
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG, "");
    
    if ((manager == NULL) || (ftpListManager == NULL) || (ftpQueueManager == NULL))
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
        resultSys = ARSAL_Sem_Init(&manager->mediasDownloader->threadSem, 0, 0);
        
        if (resultSys != 0)
        {
            result = ARDATATRANSFER_ERROR_SYSTEM;
        }
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        resultSys = ARSAL_Mutex_Init(&manager->mediasDownloader->mediasLock);
    
        if (resultSys != 0)
        {
            result = ARDATATRANSFER_ERROR_SYSTEM;
        }
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        manager->mediasDownloader->medias.medias = NULL;
        manager->mediasDownloader->medias.count = 0;
        manager->mediasDownloader->ftpListManager = ftpListManager;
        manager->mediasDownloader->ftpQueueManager = ftpQueueManager;
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        ARDATATRANSFER_MediasQueue_New(&manager->mediasDownloader->queue);
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        manager->mediasDownloader->isRunning = 0;
        manager->mediasDownloader->isCanceled = 0;
        
        result = ARDATATRANSFER_MediasDownloader_Initialize(manager, ftpListManager, ftpQueueManager, remoteDirectory, localDirectory);
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
                ARDATATRANSFER_MediasDownloader_Clear(manager);
                
                ARSAL_Sem_Destroy(&manager->mediasDownloader->queueSem);
                ARSAL_Sem_Destroy(&manager->mediasDownloader->threadSem);
                
                ARDATATRANSFER_MediasQueue_Delete(&manager->mediasDownloader->queue);
                
                ARSAL_Mutex_Destroy(&manager->mediasDownloader->mediasLock);
                ARDATATRANSFER_MediasDownloader_FreeMediaList(&manager->mediasDownloader->medias);
                
                free(manager->mediasDownloader);
                manager->mediasDownloader = NULL;
            }
        }
    }
    
    return result;
}

int ARDATATRANSFER_MediasDownloader_GetAvailableMediasSync(ARDATATRANSFER_Manager_t *manager, int withThumbnail, eARDATATRANSFER_ERROR *error)
{
    char remotePath[ARUTILS_FTP_MAX_PATH_SIZE];
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
    eARUTILS_ERROR resultUtils = ARUTILS_OK;
    int count = 0;
    
    if (manager == NULL)
    {
        result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
    }
    
    if ((result == ARDATATRANSFER_OK) && (manager->mediasDownloader == NULL))
    {
        result = ARDATATRANSFER_ERROR_NOT_INITIALIZED;
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        ARSAL_Mutex_Lock(&manager->mediasDownloader->mediasLock);
        
        ARDATATRANSFER_MediasDownloader_FreeMediaList(&manager->mediasDownloader->medias);
        
        if ((result == ARDATATRANSFER_OK)
            && ((manager->mediasDownloader->medias.medias != NULL) || (manager->mediasDownloader->medias.count != 0)))
        {
            result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
        }
        
        if (result == ARDATATRANSFER_OK)
        {
            strncpy(remotePath, manager->mediasDownloader->remoteDirectory, ARUTILS_FTP_MAX_PATH_SIZE);
            remotePath[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
            
            resultUtils = ARUTILS_Manager_Ftp_List(manager->mediasDownloader->ftpListManager, remotePath, &productFtpList, &productFtpListLen);
            
            if (resultUtils != ARUTILS_OK)
            {
                result = ARDATATRANSFER_ERROR_FTP;
            }
        }
        
        product = 0;
        while ((result == ARDATATRANSFER_OK) && (product < ARDISCOVERY_PRODUCT_MAX))
        {
            resultUtils = ARUTILS_Manager_Ftp_Connection_IsCanceled(manager->mediasDownloader->ftpListManager);
            
            if (resultUtils != ARUTILS_OK)
            {
                result = ARDATATRANSFER_ERROR_CANCELED;
            }
            
            if (result == ARDATATRANSFER_OK)
            {
                char lineDataProduct[ARUTILS_FTP_MAX_PATH_SIZE];
                ARDISCOVERY_getProductPathName(product, productPathName, sizeof(productPathName));
                nextProduct = NULL;
                fileName = ARUTILS_Ftp_List_GetNextItem(productFtpList, &nextProduct, productPathName, 1, NULL, NULL, lineDataProduct,ARUTILS_FTP_MAX_PATH_SIZE);
                
                if ((fileName != NULL) && strcmp(fileName, productPathName) == 0)
                {
                    strncpy(remoteProduct, manager->mediasDownloader->remoteDirectory, ARUTILS_FTP_MAX_PATH_SIZE);
                    remoteProduct[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
                    strncat(remoteProduct, "/", ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteProduct) - 1);
                    strncat(remoteProduct, productPathName, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteProduct) - 1);
                    strncat(remoteProduct, "/" ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_MEDIA "/", ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteProduct) - 1);
                    
                    resultUtils = ARUTILS_Manager_Ftp_List(manager->mediasDownloader->ftpListManager, remoteProduct, &mediaFtpList, &mediaFtpListLen);
                    if (resultUtils == ARUTILS_OK)
                    {
                        char lineDataMedia[ARUTILS_FTP_MAX_PATH_SIZE];
                        int fileType;
                        const char *index;
                        
                        nextMedia = NULL;
                        while ((result == ARDATATRANSFER_OK)
                               && (fileName = ARUTILS_Ftp_List_GetNextItem(mediaFtpList, &nextMedia, NULL, 0, &lineItem, &lineSize, lineDataMedia,ARUTILS_FTP_MAX_PATH_SIZE)) != NULL)
                        {
                            resultUtils = ARUTILS_Manager_Ftp_Connection_IsCanceled(manager->mediasDownloader->ftpListManager);
                            
                            if (resultUtils != ARUTILS_OK)
                            {
                                result = ARDATATRANSFER_ERROR_CANCELED;
                            }
                            
                            //Check file type
                            fileType = 0;
                            index = fileName + strlen(fileName);
                            while (index > fileName && *index != '.')
                            {
                                index--;
                            }
                            if (*index == '.')
                            {
                                index++;
                                if (strcmp(index, ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_JPG) == 0)
                                {
                                    fileType = 1;
                                }
                                else if (strcmp(index, ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_MP4) == 0)
                                {
                                    fileType = 1;
                                }
                                else if (strcmp(index, ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_MOV) == 0)
                                {
                                    fileType = 1;
                                }
                            }
                            
                            if (strncmp(fileName, productPathName, strlen(productPathName)) != 0)
                            {
                                fileType = 0;
                            }
                            
                            if (result == ARDATATRANSFER_OK && (fileType == 1))
                            {
                                char remotePath[ARUTILS_FTP_MAX_PATH_SIZE];
                                ARDATATRANSFER_Media_t **oldMedias;
                                ARDATATRANSFER_Media_t *media = NULL;
                                double fileSize;
                                const char *begin = NULL;;
                                const char *tag = NULL;
                                const char *end = NULL;
                                const char *index;
                                
                                strncpy(remotePath, remoteProduct, ARUTILS_FTP_MAX_PATH_SIZE);
                                remotePath[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
                                strncat(remotePath, fileName, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remotePath) - 1);
                                
                                //do not pertorm ARUTILS_Ftp_Size that is too long, prefer decoding the FTP LIST
                                if (ARUTILS_Ftp_List_GetItemSize(lineItem, lineSize, &fileSize) != NULL)
                                {
                                    resultUtils = ARUTILS_OK;
                                }
                                else
                                {
                                    resultUtils = ARUTILS_ERROR_FTP_CODE;
                                }
                                
                                if ((result == ARDATATRANSFER_OK) && (resultUtils == ARUTILS_OK))
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
                                        
                                        strncpy(media->uuid, "", ARDATATRANSFER_MEDIA_UUID_SIZE);
                                        media->uuid[ARDATATRANSFER_MEDIA_UUID_SIZE - 1] = '\0';
                                        //Jumping_Sumo_1970-01-01T000317+0000_3902B87F947BE865A9D137CFA63492B8.mp4

                                        index = media->name;
                                        while ((index = strstr(index, "_")) != NULL)
                                        {
                                            begin = tag;
                                            tag = ++index;
                                        }
                                        
                                        if ((begin != NULL) && (tag != NULL))
                                        {
                                            end = strstr(begin, ".");
                                        }
                                        
                                        if ((begin != NULL)  && (tag != NULL) && (end != NULL))
                                        {
                                            int len = tag - begin - 1;
                                            len = (len < ARDATATRANSFER_MEDIA_DATE_SIZE) ? len : (ARDATATRANSFER_MEDIA_DATE_SIZE - 1);
                                            strncpy(media->date, begin, len);
                                            media->date[len] = '\0';
                                            
                                            len = end - tag;
                                            len = (len < ARDATATRANSFER_MEDIA_UUID_SIZE) ? len : (ARDATATRANSFER_MEDIA_UUID_SIZE - 1);
                                            strncpy(media->uuid, tag, len);
                                            media->uuid[len] = '\0';
                                        }
                                        
                                        media->size = fileSize;
                                        
                                        if (withThumbnail == 1)
                                        {
                                            ARDATATRANSFER_MediasDownloader_GetThumbnail(manager, media);
                                        }
                                    }
                                }
                                
                                if (result == ARDATATRANSFER_OK)
                                {
                                    oldMedias = manager->mediasDownloader->medias.medias;
                                    manager->mediasDownloader->medias.medias = (ARDATATRANSFER_Media_t **)realloc(manager->mediasDownloader->medias.medias, (manager->mediasDownloader->medias.count + 1) * sizeof(ARDATATRANSFER_Media_t *));
                                    
                                    if (manager->mediasDownloader->medias.medias == NULL)
                                    {
                                        manager->mediasDownloader->medias.medias = oldMedias;
                                        result = ARDATATRANSFER_ERROR_ALLOC;
                                        free(media);
                                    }
                                    else
                                    {
                                        manager->mediasDownloader->medias.medias[manager->mediasDownloader->medias.count] = media;
                                        manager->mediasDownloader->medias.count++;
                                    }
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
        
        if (result == ARDATATRANSFER_OK)
        {
            count = manager->mediasDownloader->medias.count;
        }
        else
        {
            ARDATATRANSFER_MediasDownloader_FreeMediaList(&manager->mediasDownloader->medias);
        }
        
        ARSAL_Mutex_Unlock(&manager->mediasDownloader->mediasLock);
    }
    
    *error = result;
    return count;
}

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_GetAvailableMediasAsync(ARDATATRANSFER_Manager_t *manager, ARDATATRANSFER_MediasDownloader_AvailableMediaCallback_t availableMediaCallback, void *availableMediaArg)
{
    ARDATATRANSFER_Media_t *media;
    ARDATATRANSFER_Media_t tmpMedia;
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    eARUTILS_ERROR resultUtils = ARUTILS_OK;
    int i;
    
    if ((manager == NULL) || (availableMediaCallback == NULL))
    {
        result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
    }
    
    if ((result == ARDATATRANSFER_OK) && (manager->mediasDownloader == NULL))
    {
        result = ARDATATRANSFER_ERROR_NOT_INITIALIZED;
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        if (manager->mediasDownloader->medias.count == 0)
        {
            ARDATATRANSFER_MediasDownloader_GetAvailableMediasSync(manager, 0, &result);
        }
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        for (i=manager->mediasDownloader->medias.count-1; (resultUtils == ARUTILS_OK) && (i>=0); i--)
        {
            eARDATATRANSFER_ERROR resultThumbnail;
            resultUtils = ARUTILS_Manager_Ftp_Connection_IsCanceled(manager->mediasDownloader->ftpListManager);
            
            if (resultUtils != ARUTILS_OK)
            {
                result = ARDATATRANSFER_ERROR_CANCELED;
            }
            else
            {
                ARSAL_Mutex_Lock(&manager->mediasDownloader->mediasLock);
                
                media = manager->mediasDownloader->medias.medias[i];
                if (media != NULL)
                {
                    memcpy(&tmpMedia, media, sizeof(ARDATATRANSFER_Media_t));
                }
                else
                {
                    memset(&tmpMedia, 0, sizeof(ARDATATRANSFER_Media_t));
                }
                
                ARSAL_Mutex_Unlock(&manager->mediasDownloader->mediasLock);
                
                if ((media != NULL) && (tmpMedia.thumbnail == NULL))
                {
                    resultThumbnail = ARDATATRANSFER_MediasDownloader_GetThumbnail(manager, &tmpMedia);
                    if ((resultThumbnail == ARDATATRANSFER_OK) && (tmpMedia.thumbnail != NULL))
                    {
                        ARSAL_Mutex_Lock(&manager->mediasDownloader->mediasLock);
                        
                        media = manager->mediasDownloader->medias.medias[i];
                        if (media != NULL)
                        {
                            media->thumbnail = tmpMedia.thumbnail;
                            media->thumbnailSize = tmpMedia.thumbnailSize;
                        }
                        else
                        {
                            free(tmpMedia.thumbnail);
                            tmpMedia.thumbnail = NULL;
                        }
                        
                        ARSAL_Mutex_Unlock(&manager->mediasDownloader->mediasLock);
                        availableMediaCallback(availableMediaArg, &tmpMedia, i);
                    }
                }
            }
        }
    }
    
    return result;
}

ARDATATRANSFER_Media_t * ARDATATRANSFER_MediasDownloader_GetAvailableMediaAtIndex(ARDATATRANSFER_Manager_t *manager, int index, eARDATATRANSFER_ERROR *error)
{
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    ARDATATRANSFER_Media_t *media = NULL;
    
    if (manager == NULL)
    {
        result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
    }
    
    if ((result == ARDATATRANSFER_OK) && (manager->mediasDownloader == NULL))
    {
        result = ARDATATRANSFER_ERROR_NOT_INITIALIZED;
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        ARSAL_Mutex_Lock(&manager->mediasDownloader->mediasLock);
        
        if (((index >= 0) && (index < manager->mediasDownloader->medias.count)))
        {
            media = manager->mediasDownloader->medias.medias[index];
        }
        else
        {
            result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
        }
        
        ARSAL_Mutex_Unlock(&manager->mediasDownloader->mediasLock);
    }
    
    *error = result;
    return media;
}

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_DeleteMedia(ARDATATRANSFER_Manager_t *manager, ARDATATRANSFER_Media_t *media, ARDATATRANSFER_MediasDownloader_DeleteMediaCallback_t deleteMediaCallBack, void *deleteMediaArg)
{
    char remotePath[ARUTILS_FTP_MAX_PATH_SIZE];
    char remoteThumbnail[ARUTILS_FTP_MAX_PATH_SIZE];
    char productPathName[ARUTILS_FTP_MAX_PATH_SIZE];
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    eARUTILS_ERROR resultUtils = ARUTILS_OK;
    
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
            else if (strcmp(index, ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_MOV) == 0)
            {
                prefixType = index;
            }
        }
        
        ARDISCOVERY_getProductPathName(media->product, productPathName, sizeof(productPathName));
        
        strncpy(remotePath, manager->mediasDownloader->remoteDirectory, ARUTILS_FTP_MAX_PATH_SIZE);
        remotePath[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
        strncat(remotePath, "/", ARUTILS_FTP_MAX_PATH_SIZE - strlen(remotePath) - 1);
        strncat(remotePath, productPathName, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remotePath) - 1);
        strncat(remotePath, "/" ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_MEDIA "/", ARUTILS_FTP_MAX_PATH_SIZE - strlen(remotePath) - 1);
        strncat(remotePath, media->name, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remotePath) - 1);
        
        strncpy(remoteThumbnail, manager->mediasDownloader->remoteDirectory, ARUTILS_FTP_MAX_PATH_SIZE);
        remoteThumbnail[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
        strncat(remoteThumbnail, "/", ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteThumbnail) - 1);
        strncat(remoteThumbnail, productPathName, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteThumbnail) - 1);
        strncat(remoteThumbnail, "/" ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_THUMB "/", ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteThumbnail) - 1);
        strncat(remoteThumbnail, media->name, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteThumbnail) - 1);
        if (strcmp(prefixType, ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_MP4) == 0 ||
            strcmp(prefixType, ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_MOV) == 0)
        {
            strncat(remoteThumbnail, "." ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_JPG, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteThumbnail) - 1);
        }
        
        resultUtils = ARUTILS_Manager_Ftp_Delete(manager->mediasDownloader->ftpQueueManager, remotePath);
        
        if (resultUtils != ARUTILS_OK)
        {
            result = ARDATATRANSFER_ERROR_FTP;
        }
        
        ARUTILS_Manager_Ftp_Delete(manager->mediasDownloader->ftpQueueManager, remoteThumbnail);
        
        if (deleteMediaCallBack != NULL)
        {
            deleteMediaCallBack(deleteMediaArg, media, result);
        }

        if (result == ARDATATRANSFER_OK)
        {
            result = ARDATATRANSFER_MediasDownloader_RemoveMediaFromMediaList(manager, media);
        }
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
                if ((ftpMedia->completionCallback != NULL) && (manager->mediasDownloader->isCanceled == 0))
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
        
        ARDATATRANSFER_MediasDownloader_ResetQueueThread(manager);
    }

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG, "exit");

    return NULL;
}

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_ResetQueueThread(ARDATATRANSFER_Manager_t *manager)
{
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
    
    if (result == ARDATATRANSFER_OK)
    {
        manager->mediasDownloader->isCanceled = 0;
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        while (ARSAL_Sem_Trywait(&manager->mediasDownloader->threadSem) == 0)
        {
            /* Do nothing*/
        }
    
        while (ARSAL_Sem_Trywait(&manager->mediasDownloader->queueSem) == 0)
        {
            /* Do nothing*/
        }
        
        ARUTILS_Manager_Ftp_Connection_Reset(manager->mediasDownloader->ftpQueueManager);
    }
    
    return result;
}

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_CancelQueueThread(ARDATATRANSFER_Manager_t *manager)
{
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    eARUTILS_ERROR resultUtils = ARUTILS_OK;
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
    
    if (result == ARDATATRANSFER_OK)
    {
        resultUtils = ARUTILS_Manager_Ftp_Connection_Cancel(manager->mediasDownloader->ftpQueueManager);
        
        if (resultUtils != ARUTILS_OK)
        {
            result = ARDATATRANSFER_ERROR_FTP;
        }
    }
    
    return result;
}

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_ResetGetAvailableMedias(ARDATATRANSFER_Manager_t *manager)
{
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    eARUTILS_ERROR resultUtils = ARUTILS_OK;
    
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
        resultUtils = ARUTILS_Manager_Ftp_Connection_Reset(manager->mediasDownloader->ftpListManager);
        if (resultUtils != ARUTILS_OK)
        {
            result = ARDATATRANSFER_ERROR_FTP;
        }
    }
    
    return result;
}

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_CancelGetAvailableMedias(ARDATATRANSFER_Manager_t *manager)
{
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    eARUTILS_ERROR resultUtils = ARUTILS_OK;
    
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
        resultUtils = ARUTILS_Manager_Ftp_Connection_Cancel(manager->mediasDownloader->ftpListManager);
        if (resultUtils != ARUTILS_OK)
        {
            result = ARDATATRANSFER_ERROR_FTP;
        }
    }

    return result;
}

/*****************************************
 *
 *             Private implementation:
 *
 *****************************************/

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_Initialize(ARDATATRANSFER_Manager_t *manager, ARUTILS_Manager_t *ftpListManager, ARUTILS_Manager_t *ftpQueueManager, const char *remoteDirectory, const char *localDirectory)
{
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    //eARUTILS_ERROR error = ARUTILS_OK;
    int resultSys = 0;
    
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG, "%s, %s", localDirectory ? localDirectory : "null", remoteDirectory ? remoteDirectory : "null");
    
    if ((manager == NULL) || (ftpListManager == NULL) || (ftpQueueManager == NULL) || (localDirectory == NULL))
    {
        result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        strncpy(manager->mediasDownloader->remoteDirectory, remoteDirectory, ARUTILS_FTP_MAX_PATH_SIZE);
        manager->mediasDownloader->remoteDirectory[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
        strncpy(manager->mediasDownloader->localDirectory, localDirectory, ARUTILS_FTP_MAX_PATH_SIZE);
        manager->mediasDownloader->localDirectory[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
        strncat(manager->mediasDownloader->localDirectory, "/" , ARUTILS_FTP_MAX_PATH_SIZE - strlen(manager->mediasDownloader->localDirectory) - 1);
        
        resultSys = mkdir(manager->mediasDownloader->localDirectory, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        
        if ((resultSys != 0) && (errno != EEXIST))
        {
            result = ARDATATRANSFER_ERROR_SYSTEM;
        }
    }
    
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG, "return %d", result);
    
    return result;
}

void ARDATATRANSFER_MediasDownloader_Clear(ARDATATRANSFER_Manager_t *manager)
{
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG, "");
    
    if ((manager != NULL) && (manager->mediasDownloader != NULL))
    {
        manager->mediasDownloader->isCanceled = 0;
    }
}

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_GetThumbnail(ARDATATRANSFER_Manager_t *manager, ARDATATRANSFER_Media_t *media)
{
    char remoteThumbnail[ARUTILS_FTP_MAX_PATH_SIZE];
    char productPathName[ARUTILS_FTP_MAX_PATH_SIZE];
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    eARUTILS_ERROR error = ARUTILS_OK;
    
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_MEDIAS_DOWNLOADER_TAG, "");
    
    if ((manager == NULL) || (media == NULL))
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
            else if (strcmp(index, ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_MOV) == 0)
            {
                prefixType = index;
            }
        }
        
        ARDISCOVERY_getProductPathName(media->product, productPathName, sizeof(productPathName));
        strncpy(remoteThumbnail, manager->mediasDownloader->remoteDirectory, ARUTILS_FTP_MAX_PATH_SIZE);
        remoteThumbnail[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
        strncat(remoteThumbnail, "/", ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteThumbnail) - 1);
        strncat(remoteThumbnail, productPathName, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteThumbnail) - 1);
        strncat(remoteThumbnail, "/" ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_THUMB "/", ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteThumbnail) - 1);
        strncat(remoteThumbnail, media->name, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteThumbnail) - 1);
        if (strcmp(prefixType, ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_MP4) == 0 ||
            strcmp(prefixType, ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_MOV) == 0)
        {
            strncat(remoteThumbnail, "." ARDATATRANSFER_MEDIAS_DOWNLOADER_EXT_JPG, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remoteThumbnail) - 1);
        }
        
        error = ARUTILS_Manager_Ftp_Get_WithBuffer(manager->mediasDownloader->ftpListManager, remoteThumbnail, &media->thumbnail, &media->thumbnailSize, NULL, NULL);
        
        if (error != ARUTILS_OK)
        {
            result = ARDATATRANSFER_ERROR_FTP;
        }
    }
    
    return result;
}

void ARDATATRANSFER_MediasDownloader_FtpProgressCallback(void* arg, float percent)
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
        strncpy(remotePath, manager->mediasDownloader->remoteDirectory, ARUTILS_FTP_MAX_PATH_SIZE);
        remotePath[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
        strncat(remotePath, "/", ARUTILS_FTP_MAX_PATH_SIZE - strlen(remotePath) - 1);
        strncat(remotePath, productPathName, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remotePath) - 1);
        strncat(remotePath, "/" ARDATATRANSFER_MEDIAS_DOWNLOADER_FTP_MEDIA "/", ARUTILS_FTP_MAX_PATH_SIZE - strlen(remotePath) - 1);
        strncat(remotePath, ftpMedia->media.name, ARUTILS_FTP_MAX_PATH_SIZE - strlen(remotePath) - 1);
        
        strncpy(restorePath, ftpMedia->media.filePath, ARUTILS_FTP_MAX_PATH_SIZE);
        restorePath[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';

        strncpy(localPath, manager->mediasDownloader->localDirectory, ARUTILS_FTP_MAX_PATH_SIZE);
        localPath[ARUTILS_FTP_MAX_PATH_SIZE - 1] = '\0';
        strncat(localPath, ARDATATRANSFER_MANAGER_DOWNLOADER_DOWNLOADING_PREFIX, ARUTILS_FTP_MAX_PATH_SIZE - strlen(localPath) - 1);
        strncat(localPath, ftpMedia->media.name, ARUTILS_FTP_MAX_PATH_SIZE - strlen(localPath) - 1);

        errorResume = ARUTILS_FileSystem_GetFileSize(localPath, &localSize);
    }

    if (result == ARDATATRANSFER_OK)
    {
        error = ARUTILS_Manager_Ftp_Get(manager->mediasDownloader->ftpQueueManager, remotePath, localPath, ARDATATRANSFER_MediasDownloader_FtpProgressCallback, ftpMedia, (errorResume == ARUTILS_OK) ? FTP_RESUME_TRUE : FTP_RESUME_FALSE);
        
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
    else
    {
        remove(localPath);
    }

    return result;
}

eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_RemoveMediaFromMediaList(ARDATATRANSFER_Manager_t *manager, ARDATATRANSFER_Media_t *media)
{
    eARDATATRANSFER_ERROR result = ARDATATRANSFER_OK;
    ARDATATRANSFER_Media_t *curMedia;
    int foundIndex = -1;
    int i;
    
    if ((manager == NULL) || (manager->mediasDownloader == NULL) || (manager->mediasDownloader->medias.medias == NULL) || (manager->mediasDownloader->medias.count == 0))
    {
        result = ARDATATRANSFER_ERROR_BAD_PARAMETER;
    }
    
    if (result == ARDATATRANSFER_OK)
    {
        ARSAL_Mutex_Lock(&manager->mediasDownloader->mediasLock);
        
        for (i = 0; (foundIndex == -1) && (i < manager->mediasDownloader->medias.count); i++)
        {
            curMedia = manager->mediasDownloader->medias.medias[i];
            
            if ((curMedia != NULL) && (strcmp(curMedia->filePath, media->filePath) == 0))
            {
                foundIndex = i;
            }
        }
        
        if (foundIndex != -1)
        {
            curMedia = manager->mediasDownloader->medias.medias[foundIndex];
            manager->mediasDownloader->medias.medias[foundIndex] = NULL;
            free(curMedia);
        }
        
        ARSAL_Mutex_Unlock(&manager->mediasDownloader->mediasLock);
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
                
                if (media != NULL)
                {
                    if (media->thumbnail != NULL)
                    {
                        free(media->thumbnail);
                    }
                    
                    free(media);
                }
            }
            mediaList->medias = NULL;
        }
        
        mediaList->count = 0;
    }
}

