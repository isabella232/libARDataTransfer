/**
 * @file ARDATATRANSFER_Manager.h
 * @brief libARDataTransfer Manager header file.
 * @date 19/12/2013
 * @author david.flattin.ext@parrot.com
 **/

#ifndef _ARDATATRANSFER_MANAGER_PRIVATE_H_
#define _ARDATATRANSFER_MANAGER_PRIVATE_H_

#define ARDATATRANSFER_MANAGER_DOWNLOADER_PREFIX_DOWNLOADING    "downloading_"
#define ARDATATRANSFER_MANAGER_DOWNLOADER_PRODUCT_ID_MAX_SIZE   10

/**
 * @brief Manager structure
 * @param dataDownloader The DataDownloader
 * @param mediasDownloader The MediasDownloader
 * @see
 */
struct ARDATATRANSFER_Manager_t
{
    ARDATATRANSFER_Downloader_t *downloader;
    ARDATATRANSFER_Uploader_t *uploader;
    ARDATATRANSFER_DataDownloader_t *dataDownloader;
    ARDATATRANSFER_MediasDownloader_t *mediasDownloader;
};

#endif /* _ARDATATRANSFER_MANAGER_PRIVATE_H_ */