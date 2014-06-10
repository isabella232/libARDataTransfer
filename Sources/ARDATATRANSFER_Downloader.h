/**
 * @file ARDATATRANSFER_Downloader.h
 * @brief libARDataTransfer downloader header file.
 * @date 21/05/2014
 * @author david.flattin.ext@parrot.com
 **/

#ifndef _ARDATATRANSFER_DOWNLOADER_PRIVATE_H_
#define _ARDATATRANSFER_DOWNLOADER_PRIVATE_H_

/**
 * @brief DataDownloader structure
 * @param isInitialized Is set to 1 if Downloader initilized else 0
 * @param isCanceled Is set to 1 if Downloader Thread is canceled else 0
 * @param isRunning Is set to 1 if Downloader Thread is running else 0
 * @param ftp The FTP Downloader connection
 * @param localDirectory The local directory where Downloader download files
 * @param sem The semaphore to cancel the Downloader Thread and its FTP connection
 * @see ARDATATRANSFER_Downloader_New ()
 */
typedef struct
{
    int isCanceled;
    int isRunning;
    eARDATATRANSFER_DOWNLOADER_RESUME resume;
    ARUTILS_Manager_t *ftpManager;
    char remotePath[ARUTILS_FTP_MAX_PATH_SIZE];
    char localPath[ARUTILS_FTP_MAX_PATH_SIZE];
    ARSAL_Sem_t threadSem;
    
    ARDATATRANSFER_Downloader_ProgressCallback_t progressCallback;
    void *progressArg;
    ARDATATRANSFER_Downloader_CompletionCallback_t completionCallback;
    void *completionArg;
    
} ARDATATRANSFER_Downloader_t;


void ARDATATRANSFER_Downloader_Ftp_ProgressCallback(void* arg, float percent);

#endif /* _ARDATATRANSFER_DOWNLOADER_PRIVATE_H_ */

