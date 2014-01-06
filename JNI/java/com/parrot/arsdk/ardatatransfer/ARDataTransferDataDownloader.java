
package com.parrot.arsdk.ardatatransfer;

import java.lang.Runnable;

/**
 * ARDataTransfer DataDownloader module
 * @author david.flattin.ext@parrot.com
 * @date 19/12/2013
 */
public class ARDataTransferDataDownloader
{
    /* Native Functions */
    private native int nativeNew(long manager, String deviceIP, int port, String localDirectory);
    private native int nativeDelete(long manager);
    private native void nativeThreadRun (long manager);
    private native int nativeCancelThread (long manager);
    
    /*  Members  */
    private boolean isInit = false;
    private long nativeManager = 0;
    private Runnable downloaderRunnable = null;
    
    /*  Java Methods */
    
	/**
     * Private ARDataTransfer DataDownloader constructor
     * @return void
     */    
    protected ARDataTransferDataDownloader(long _nativeManager)
    {
        this.nativeManager = _nativeManager;
        
        this.downloaderRunnable = new Runnable () {
            public void run() {
                nativeThreadRun(nativeManager);    
            }
        };
    }
    
    /**
     * Creates a new ARDataTransfer DataDownloader
     * @return void
     * @throws ARDataTransferException if error
     */
    public void createDataDownloader(String deviceIP, int port, String localDirectory) throws ARDataTransferException
    {
        int result = nativeNew(nativeManager, deviceIP, port, localDirectory);
        
        ARDATATRANSFER_ERROR_ENUM error = ARDATATRANSFER_ERROR_ENUM.getFromValue(result);
        
        if (error != ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK)
        {
            throw new ARDataTransferException(error);
        }
        else
        {
            isInit = true;
        }
    }
    
    /**
     * Closes an ARDataTransfer DataDownloader
     * @return ARDATATRANSFER_OK if success, else an {@link ARDATATRANSFER_ERROR_ENUM} error code
     */
    public ARDATATRANSFER_ERROR_ENUM closeDataDownloader()
    {
        int result = nativeDelete(nativeManager);
        
        ARDATATRANSFER_ERROR_ENUM error = ARDATATRANSFER_ERROR_ENUM.getFromValue(result);
        return error;
    }
        
    /**
     * Gets the ARDataTransfer DataDownloader {@link Runnable} to start as new {@link Thread}
     * @return DataDownloader Runnable
     */
    public Runnable getDownloaderRunnable()
    {
        Runnable runnable = null;
        
        if (isInit == true)
        {
            runnable = this.downloaderRunnable;
        }
        
        return runnable;    
    }
    
    /**
     * Cancels the ARDataTransfer DataDownloader Runnable Thread
     * @return ARDATATRANSFER_OK if success, else an {@link ARDATATRANSFER_ERROR_ENUM} error code
     */
    public ARDATATRANSFER_ERROR_ENUM cancelThread()
    {
        int result = nativeCancelThread(nativeManager);
        
        ARDATATRANSFER_ERROR_ENUM error = ARDATATRANSFER_ERROR_ENUM.getFromValue(result);
        return error;        
    }    
}
