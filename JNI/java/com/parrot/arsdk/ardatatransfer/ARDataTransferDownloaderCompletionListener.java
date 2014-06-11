
package com.parrot.arsdk.ardatatransfer;

/**
 * ARDataTransfer Downloader CompletionListener
 * @author david.flattin.ext@parrot.com
 * @date 09/06/2014
 */

public interface ARDataTransferDownloaderCompletionListener
{
    /**
     * Gives the ARDataTransferDownloader download complete status
     * @param arg Object complete Listener arg
     * @param error ARDATATRANSFER_OK if success, else an {@link ARDATATRANSFER_ERROR_ENUM} error code
     * @return void
     */
    void didDownloadComplete(Object arg, ARDATATRANSFER_ERROR_ENUM error);
}

