/**
 * @file ARDATATRANSFER_MediasDownloader.h
 * @brief libARDataTransfer MediasDownloader header file.
 * @date 19/12/2013
 * @author david.flattin.ext@parrot.com
 **/

#ifndef _ARDATATRANSFER_MEDIAS_DOWNLOADER_H_
#define _ARDATATRANSFER_MEDIAS_DOWNLOADER_H_

/**
 * @brief Defines the media name size in characters
 * @see ARDATATRANSFER_Media_t
 */
#define ARDATATRANSFER_MEDIA_NAME_SIZE  64
/**
 * @brief Defines the media date size in characters
 * @see ARDATATRANSFER_Media_t
 */
#define ARDATATRANSFER_MEDIA_DATE_SIZE  32

/**
 * @brief Media structure
 * @param name The name of the media
 * @param date The date of the media
 * @param size The size of the media
 * @param thumbnail The media thumbnail data
 * @param thumbnailSize The size of the media thumbnail
 * @see ARDATATRANSFER_MediaList_t
 */
typedef struct
{
    char name[ARDATATRANSFER_MEDIA_NAME_SIZE];
    char date[ARDATATRANSFER_MEDIA_DATE_SIZE];
    uint8_t *thumbnail;
    uint32_t thumbnailSize;
    double size;
    
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
 * @retval On success, returns ARDATATRANSFER_OK. Otherwise, it returns an error number of eARDATATRANSFER_ERROR.
 * @see ARDATATRANSFER_MediasDownloader_ThreadRun ()
 */
eARDATATRANSFER_ERROR ARDATATRANSFER_MediasDownloader_GetAvailableMedias (ARDATATRANSFER_Manager_t *manager, ARDATATRANSFER_MediaList_t *mediaList);

/**
 * @brief Free a medias list
 * @param mediaList The list of medias
 * @see ARDATATRANSFER_MediasDownloader_GetAvailableMedias ()
 */
void ARDATATRANSFER_MediasDownloader_FreeMediaList(ARDATATRANSFER_MediaList_t *mediaList);

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


