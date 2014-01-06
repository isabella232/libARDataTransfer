
package com.parrot.arsdk.ardatatransfer;

/**
 * ARDataTransfer Manager
 * @author david.flattin.ext@parrot.com
 * @date 19/12/2013
 */
public class ARDataTransferManager
{	
    /* Native Functions */
    private native static boolean nativeStaticInit();
    private native long nativeNew() throws ARDataTransferException;
    private native void nativeDelete(long manager);
 
    /*  Members  */
    private long nativeManager = 0;
    private boolean isInit = false;
    ARDataTransferDataDownloader dataDownloader = null;
    ARDataTransferMediasDownloader mediasDownloader = null;
    
    /*  Java Methods */
    
    /**
     * Creates ARDataTransfer Manager
     * @return void
     * @throws ARDataTransferException if error
     */
    public void createManager() throws ARDataTransferException
    {
        if (isInit == false)
        {
            nativeManager = nativeNew();
        }

        if (nativeManager != 0)
        {
            isInit = true;
        }
    }
    
	/**
     * Closes ARDataTransfer Manager
     * @return void
     */    
    public void closeManager()
    {
    	if (nativeManager != 0)
        {
            nativeDelete(nativeManager);
            nativeManager = 0;
            isInit = false;
        }
    }
    
    /**
     * Gets the ARDataTransfer Manager status Initialized or not
     * @return true if the Manager is already created else false
     */     
    public boolean isInitialized()
    {
        return isInit;
    }
    
    /**
     * Gets the ARDataTransfer DataDownloader object {@link ARDataTransferDataDownloader}
     * @return an ARDataTransfer DataDownloader object, null if Manager not initialized
     */     
    public ARDataTransferDataDownloader getARDataTransferDataDownloader()
    {
        if (isInit == false)
        {
            return null;
        }
        
        if (dataDownloader == null)
        {
            dataDownloader = new ARDataTransferDataDownloader(nativeManager);
        }
        
        return dataDownloader;
    }
    
    /**
     * Gets the ARDataTransfer MediasDownloader object {@link ARDataTransferMediasDownloader}
     * @return an ARDataTransfer MediasDownloader object, null if Manager not initialized
     */
    public ARDataTransferMediasDownloader getARDataTransferMediasDownloader()
    {
        if (isInit == false)
        {
            return null;
        }
        
        if (mediasDownloader == null)
        {
            mediasDownloader = new ARDataTransferMediasDownloader(nativeManager);
        }
        
        return mediasDownloader;
    }
 
    /*  Static Block */
    static
    {
        nativeStaticInit();
    }  
}
