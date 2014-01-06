
package com.parrot.arsdk.ardatatransfer;

/**
 * ARDataTransfer MediasDownloader CompletionListener
 * @author david.flattin.ext@parrot.com
 * @date 19/12/2013
 */
public interface ARDataTransferMediasDownloaderCompletionListener
{
    /**
     * Gives the ARDataTransferMedia media download complete status
     * @param arg Object complete Listener arg
     * @param media {@link ARDataTransferMedia} media download complete
     * @param error ARDATATRANSFER_OK if success, else an {@link ARDATATRANSFER_ERROR_ENUM} error code
     * @return void
     */
    void didComplete(Object arg, ARDataTransferMedia media, ARDATATRANSFER_ERROR_ENUM error);
}
