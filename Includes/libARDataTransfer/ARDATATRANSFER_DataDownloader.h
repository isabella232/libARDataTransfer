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
 * @brief Create a new ARDataTransfer DataDownloader
 * @warning This function allocates memory
 * @param manager The address of the pointer on the ARDataTransfer Manager
 * @retval On success, returns ARDATATRANSFER_OK. Otherwise, it returns an error number of eARDATATRANSFER_ERROR.
 * @see ARDATATRANSFER_DataDownloader_Delete ()
 */
eARDATATRANSFER_ERROR ARDATATRANSFER_DataDownloader_New (ARDATATRANSFER_Manager_t *manager, ARUTILS_Manager_t *ftpManager, const char *localDirectory);

/**
 * @brief Delete an ARDataTransfer DataDownloader
 * @warning This function frees memory
 * @param manager The address of the pointer on the ARDataTransfer Manager
 * @retval On success, returns ARDATATRANSFER_OK. Otherwise, it returns an error number of eARDATATRANSFER_ERROR.
 * @see ARDATATRANSFER_DataDownloader_New ()
 */
eARDATATRANSFER_ERROR ARDATATRANSFER_DataDownloader_Delete (ARDATATRANSFER_Manager_t *manager);

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