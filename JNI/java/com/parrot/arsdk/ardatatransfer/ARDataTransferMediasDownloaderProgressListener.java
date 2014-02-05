
package com.parrot.arsdk.ardatatransfer;

/**
 * ARDataTransfer MediasDownloader ProgressListener
 * @author david.flattin.ext@parrot.com
 * @date 19/12/2013
 */
public interface ARDataTransferMediasDownloaderProgressListener
{
    /**
     * Gives the ARDataTransferMedia media download progress state
     * @param arg Object progress Listener arg
     * @param media {@link ARDataTransferMedia} media in download progress
     * @param percent The percent size of the media file already downloaded
     * @return void
     */
     void didMediaProgress(Object arg, ARDataTransferMedia media, int percent);
}
