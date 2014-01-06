/**
 * @file ARDATATRANSFER_Manager.h
 * @brief libARDataTransfer Manager header file.
 * @date 19/12/2013
 * @author david.flattin.ext@parrot.com
 **/

#ifndef _ARDATATRANSFER_MANAGER_PRIVATE_H_
#define _ARDATATRANSFER_MANAGER_PRIVATE_H_

#define ARDATATRANSFER_MANAGER_DOWNLOADER_PREFIX_DOWNLOADING    "downloading_"

/**
 * @brief Manager structure
 * @param dataDownloader The DataDownloader
 * @param mediasDownloader The MediasDownloader
 * @see
 */
struct ARDATATRANSFER_Manager_t
{
    ARDATATRANSFER_DataDownloader_t *dataDownloader;
    ARDATATRANSFER_MediasDownloader_t *mediasDownloader;
};

/**
 * @brief File list iterator function
 * @param list The file list
 * @param nextItem The the next file
 * @param prefix The file prefix to match
 * @param isDirectory The file type requested: 1 directory or 0 file
 * @see
 */
const char * ARDATATRANSFER_Manager_List_GetNextItem(const char *list, const char **nextItem, const char *prefix, int isDirectory);

#endif /* _ARDATATRANSFER_MANAGER_PRIVATE_H_ */