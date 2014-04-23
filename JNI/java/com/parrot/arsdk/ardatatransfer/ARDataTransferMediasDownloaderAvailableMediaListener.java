
package com.parrot.arsdk.ardatatransfer;

/**
 * ARDataTransfer MediasDownloader AvailableMediaListener
 * @author david.flattin.ext@parrot.com
 * @date 27/01/2014
 */
public interface ARDataTransferMediasDownloaderAvailableMediaListener
{
    /**
     * Gives the ARDataTransferMedia media available status
     * @param arg Object complete Listener arg
     * @param media {@link ARDataTransferMedia} media available
     * @param error ARDATATRANSFER_OK if success, else an {@link ARDATATRANSFER_ERROR_ENUM} error code
     * @return void
     */
    void didMediaAvailable(Object arg, ARDataTransferMedia media, int index);
}
