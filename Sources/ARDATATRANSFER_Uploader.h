/**
 * @file ARDATATRANSFER_Uploader.h
 * @brief libARDataTransfer uploader header file.
 * @date 21/05/2014
 * @author david.flattin.ext@parrot.com
 **/

#ifndef _ARDATATRANSFER_UPLOADER_PRIVATE_H_
#define _ARDATATRANSFER_UPLOADER_PRIVATE_H_


/**
 * @brief DataDownloader structure
 * @param isInitialized Is set to 1 if Uploader initilized else 0
 * @param isCanceled Is set to 1 if Uploader Thread is canceled else 0
 * @param isRunning Is set to 1 if Uploader Thread is running else 0
 * @param ftp The FTP Uploader connection
 * @param localDirectory The local directory where Uploader download files
 * @param sem The semaphore to cancel the Uploader Thread and its FTP connection
 * @see ARDATATRANSFER_Uploader_New ()
 */
typedef struct
{
    int isCanceled;
    int isRunning;
    eARDATATRANSFER_UPLOADER_RESUME resume;
    ARUTILS_Manager_t *ftpManager;
    char remotePath[ARUTILS_FTP_MAX_PATH_SIZE];
    char localPath[ARUTILS_FTP_MAX_PATH_SIZE];
    ARSAL_Sem_t threadSem;
    
    ARDATATRANSFER_Uploader_ProgressCallback_t progressCallback;
    void *progressArg;
    ARDATATRANSFER_Uploader_CompletionCallback_t completionCallback;
    void *completionArg;
    
} ARDATATRANSFER_Uploader_t;


void ARDATATRANSFER_Uploader_Ftp_ProgressCallback(void* arg, float percent);


#endif /* _ARDATATRANSFER_UPLOADER_PRIVATE_H_ */

