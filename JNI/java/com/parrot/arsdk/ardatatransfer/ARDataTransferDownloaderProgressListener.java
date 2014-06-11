
package com.parrot.arsdk.ardatatransfer;

/**
 * ARDataTransfer Downloader ProgressListener
 * @author david.flattin.ext@parrot.com
 * @date 09/06/2014
 */


public interface ARDataTransferDownloaderProgressListener
{
    /**
     * Gives the ARDataTransferDownloader download progress
     * @param arg Object progress Listener arg
     * @param percent percent of progress
     * @return void
     */
    void didDownloadProgress(Object arg, float percent);
}

