
package com.parrot.arsdk.ardatatransfer;

/**
 * ARDataTransfer Uploader ProgressListener
 * @author david.flattin.ext@parrot.com
 * @date 09/06/2014
 */


public interface ARDataTransferUploaderProgressListener
{
    /**
     * Gives the ARDataTransferUploader upload progress
     * @param arg Object progress Listener arg
     * @param percent percent of progress
     * @return void
     */
    void didUploadProgress(Object arg, float percent);
}


