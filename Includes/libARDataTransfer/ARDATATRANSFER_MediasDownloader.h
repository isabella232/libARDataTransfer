/**
 * @file ARDATATRANSFER_MediasDownloader.h
 * @brief libARDataTransfer MediasDownloader header file.
 * @date 19/12/2013
 * @author david.flattin.ext@parrot.com
 **/

#include <libARDiscovery/ARDISCOVERY_Discovery.h>

#ifndef _ARDATATRANSFER_MEDIAS_DOWNLOADER_H_
#define _ARDATATRANSFER_MEDIAS_DOWNLOADER_H_

/**
 * @brief Defines the media name size in characters
 * @see ARDATATRANSFER_Media_t
 */
#define ARDATATRANSFER_MEDIA_NAME_SIZE  128

/**
 * @brief Defines the media path size in characters
 * @see ARDATATRANSFER_Media_t
 */
#define ARDATATRANSFER_MEDIA_PATH_SIZE  256

/**
 * @brief Defines the media date size in characters
 * @see ARDATATRANSFER_Media_t
 */
#define ARDATATRANSFER_MEDIA_DATE_SIZE  32

/**
 * @brief Media structure
 * @param product The the product that the media belong to
 * @param name The name of the media
 * @param fileName The file name of the media
 * @param date The date of the media
 * @param size The size of the media
 * @param thumbnail The media thumbnail data
 * @param thumbnailSize The size of the media thumbnail
 * @see ARDATATRANSFER_MediaList_t
 */
typedef struct
{
    eARDISCOVERY_PRODUCT product;
    char name[ARDATATRANSFER_MEDIA_NAME_SIZE];
    char filePath[ARDATATRANSFER_MEDIA_PATH_SIZE];
    char date[ARDATATRANSFER_MEDIA_DATE_SIZE];
    double size;
    uint8_t *thumbnail;
    uint32_t thumbnailSize;
    
} ARDATATRANSFER_Media_t;

/**
 * @brief Initialize the MediasDownloader
 * @param medias The pointer address of the media list
 * @param count The number of medias in the media list
 * @see ARDATATRANSFER_MediasDownloader_GetAvailableMedias (), ARDATATRANSFER_Media_t
 */
typedef struct
{
    ARDATATRANSFER_Media_t **medias;
    int count;
    
} ARDATATRANSFER_MediaList_t;

/**
 * @brief Available media callback called for each media found
 * @param arg The pointer of the user custom argument
 * @param media The availble media found
 * @see ARDATATRANSFER_MediasDownloader_GetAvailableMedias ()
 */
typedef void (*ARDATATRANSFER_MediasDownloader_AvailableMediaCallback_t) (void* arg, ARDATATRANSFER_Media_t *media);

/**
 * @brief Progress callback of the Media download
 * @param arg The pointer of the user custom argument
 * @param media The media in progress
 * @param percent The percent size of the media file already downloaded
 * @see ARDATATRANSFER_MediasDownloader_ThreadRun ()
 */
typedef void (*ARDATATRANSFER_MediasDownloader_MediaDownloadProgressCallback_t) (void* arg, ARDATATRANSFER_Media_t *media, uint8_t percent);

/**
 * @brief Completion callback of the Media download
 * @param arg The pointer of the user custom argument
 * @param media The media to be downloaded
 * @param error The error status to indicate the media downloaded status
 * @see ARDATATRANSFER_MediasDownloader_ThreadRun ()
 */
typedef void (*ARDATATRANSFER_MediasDownloader_MediaDownloadCompletionCallback_t) (void* arg, ARDATATRANSFER_Media_t *media, eARDATATRANSFER_ERROR error);

/**
 * @brief Create a new ARDataTransfer MediasDownloader
 * @warning This function allocates memory
 * @param manager The address of the pointer on the ARDataTransfer Manager
 * @retval On success, returns ARDATATRANSFER_OK. Otherwise, it returns an error number of eARDATATRANSFER_ERROR.
 * @see ARDATATRANSFER_MediasDownloader_Delete ()
 */
eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_New (ARDATATRANSFER_Manager_t *manager, const char *deviceIP, int port, const char *localDirectory);

/**
 * @brief Delete an ARDataTransfer MediasDownloader
 * @warning This function frees memory
 * @param manager The address of the pointer on the ARDataTransfer Manager
 * @retval On success, returns ARDATATRANSFER_OK. Otherwise, it returns an error number of eARDATATRANSFER_ERROR.
 * @see ARDATATRANSFER_MediasDownloader_New ()
 */
eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_Delete (ARDATATRANSFER_Manager_t *manager);

/**
 * @brief Get the medias list available form the Device
 * @warning This function allocates memory
 * @param manager The pointer of the ARDataTransfer Manager
 * @param [out] mediaList The list of medias
 * @param withThumbnail The flag to return thumbnail, 0 no thumbnail is returned, 1 thumbnails are returned
 * @retval On success, returns ARDATATRANSFER_OK. Otherwise, it returns an error number of eARDATATRANSFER_ERROR.
 * @see ARDATATRANSFER_MediasDownloader_FreeMediaList ()
 */
eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_GetAvailableMediasSync (ARDATATRANSFER_Manager_t *manager, ARDATATRANSFER_MediaList_t *mediaList, int withThumbnail);

/**
 * @brief Get the medias list available form the Device
 * @warning This function allocates memory
 * @param manager The pointer of the ARDataTransfer Manager
 * @param availableMediaCallback The available media callback
 * @param availableMediaArg The pointer of the user custom argument
 * @retval On success, returns ARDATATRANSFER_OK. Otherwise, it returns an error number of eARDATATRANSFER_ERROR.
 * @see ARDATATRANSFER_MediasDownloader_AvailableMediaCallback_t ()
 */
eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_GetAvailableMediasAsync (ARDATATRANSFER_Manager_t *manager, ARDATATRANSFER_MediasDownloader_AvailableMediaCallback_t availableMediaCallback, void *availableMediaArg);

/**
 * @brief Send a cancel to the get media list function
 * @param manager The pointer of the ARDataTransfer Manager
 * @retval On success, returns ARDATATRANSFER_OK. Otherwise, it returns an error number of eARDATATRANSFER_ERROR.
 * @see ARDATATRANSFER_MediasDownloader_GetAvailableMediasSync (), ARDATATRANSFER_MediasDownloader_GetAvailableMediasAsync ()
 */
eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_CancelGetAvailableMedias(ARDATATRANSFER_Manager_t *manager);

/**
 * @brief Free a medias list
 * @param mediaList The list of medias
 * @see ARDATATRANSFER_MediasDownloader_GetAvailableMedias ()
 */
void ARDATATRANSFER_MediasDownloader_FreeMediaList(ARDATATRANSFER_MediaList_t *mediaList);

/**
 * @brief Delete the media form the Device
 * @param manager The pointer of the ARDataTransfer Manager
 * @param media The media to delete
 * @retval On success, returns ARDATATRANSFER_OK. Otherwise, it returns an error number of eARDATATRANSFER_ERROR.
 * @see ARDATATRANSFER_MediasDownloader_GetAvailableMedias ()
 */
eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_DeleteMedia(ARDATATRANSFER_Manager_t *manager, ARDATATRANSFER_Media_t *media);

/**
 * @brief Add a media to the download process queue
 * @param manager The pointer of the ARDataTransfer Manager
 * @param media The media to add
 * @param progressCallback The progress callback for this media download
 * @param progressArg The progress callback user argument for this media download
 * @param completionCallback The completion callback for this media download
 * @param completionArg The completion callback user argument for this media download
 * @retval On success, returns ARDATATRANSFER_OK. Otherwise, it returns an error number of eARDATATRANSFER_ERROR.
 * @see ARDATATRANSFER_MediasDownloader_ThreadRun ()
 */
eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_AddMediaToQueue (ARDATATRANSFER_Manager_t *manager, ARDATATRANSFER_Media_t *media,  ARDATATRANSFER_MediasDownloader_MediaDownloadProgressCallback_t progressCallback, void *progressArg, ARDATATRANSFER_MediasDownloader_MediaDownloadCompletionCallback_t completionCallback, void *completionArg);

/**
 * @brief Process of the media download queue
 * @param manager The pointer of the ARDataTransfer Manager
 * @retval returns NULL
 * @see ARDATATRANSFER_MediasDownloader_Init ()
 */
void* ARDATATRANSFER_MediasDownloader_QueueThreadRun (void *managerArg);

/**
 * @brief Send a cancel to the medias downloader process queue
 * @param manager The pointer of the ARDataTransfer Manager
 * @retval On success, returns ARDATATRANSFER_OK. Otherwise, it returns an error number of eARDATATRANSFER_ERROR.
 * @see ARDATATRANSFER_MediasDownloader_QueueThreadRun ()
 */
eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_CancelQueueThread (ARDATATRANSFER_Manager_t *manager);

#endif /* _ARDATATRANSFER_MEDIAS_DOWNLOADER_H_ */


