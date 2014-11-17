/**
 * @file ARDATATRANSFER_DataDownloader.h
 * @brief libARDataTransfer DataDownloader header file..
 * @date 19/12/2013
 * @author david.flattin.ext@parrot.com
 **/

#include <libARUtils/ARUtils.h>

#ifndef _ARDATATRANSFER_DATA_DOWNLOADER_H_
#define _ARDATATRANSFER_DATA_DOWNLOADER_H_

/**
 * @brief Completion callback of the Media download
 * @param arg The pointer of the user custom argument
 * @param media The media to be downloaded
 * @param error The error status to indicate the media downloaded status
 * @see ARDATATRANSFER_MediasDownloader_ThreadRun ()
 */
typedef void (*ARDATATRANSFER_DataDownloader_DataCompletionCallback_t) (void* arg, const char *fileName, eARDATATRANSFER_ERROR error);

/**
 * @brief Create a new ARDataTransfer DataDownloader
 * @warning This function allocates memory
 * @param manager The address of the pointer on the ARDataTransfer Manager
 * @param ftpListManager The ftp list manager
 * @param ftpDataManager The ftp data manager
 * @param remoteDirectory The remote directory
 * @param localDirectory The local directory
 * @retval On success, returns ARDATATRANSFER_OK. Otherwise, it returns an error number of eARDATATRANSFER_ERROR.
 * @see ARDATATRANSFER_DataDownloader_Delete ()
 */
eARDATATRANSFER_ERROR ARDATATRANSFER_DataDownloader_New (ARDATATRANSFER_Manager_t *manager, ARUTILS_Manager_t *ftpListManager, ARUTILS_Manager_t *ftpDataManager, const char *remoteDirectory, const char *localDirectory, ARDATATRANSFER_DataDownloader_DataCompletionCallback_t dataCompletionCallback, void *dataCompletionArg);

/**
 * @brief Delete an ARDataTransfer DataDownloader
 * @warning This function frees memory
 * @param manager The address of the pointer on the ARDataTransfer Manager
 * @retval On success, returns ARDATATRANSFER_OK. Otherwise, it returns an error number of eARDATATRANSFER_ERROR.
 * @see ARDATATRANSFER_DataDownloader_New ()
 */
eARDATATRANSFER_ERROR ARDATATRANSFER_DataDownloader_Delete (ARDATATRANSFER_Manager_t *manager);

/**
 * @brief Get Available Files of ARDataTransfer DataDownloader
 * @param manager The address of the pointer on the ARDataTransfer Manager
 * @param filesNumber The address of the pointer that will received the available files number
 * @retval On success, returns ARDATATRANSFER_OK. Otherwise, it returns an error number of eARDATATRANSFER_ERROR.
 * @see ARDATATRANSFER_DataDownloader_New ()
 */
eARDATATRANSFER_ERROR ARDATATRANSFER_DataDownloader_GetAvailableFiles (ARDATATRANSFER_Manager_t *manager, long *filesNumber);

/**
 * @brief Process of the device data download (flights data or ride data)
 * @param manager The pointer of the ARDataTransfer Manager
 * @retval returns NULL
 * @see ARDATATRANSFER_DataDownloader_Init ()
 */
void* ARDATATRANSFER_DataDownloader_ThreadRun (void *manager);

/**
 * @brief Send a cancel to the device data downloader thread
 * @param manager The pointer of the ARDataTransfer Manager
 * @retval On success, returns ARDATATRANSFER_OK. Otherwise, it returns an error number of eARDATATRANSFER_ERROR.
 * @see ARDATATRANSFER_DataDownloader_ThreadRun ()
 */
eARDATATRANSFER_ERROR ARDATATRANSFER_DataDownloader_CancelThread (ARDATATRANSFER_Manager_t *manager);

#endif /* _ARDATATRANSFER_DATA_DOWNLOADER_H_ */