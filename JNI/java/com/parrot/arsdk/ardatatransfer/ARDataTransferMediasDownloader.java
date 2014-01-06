
package com.parrot.arsdk.ardatatransfer;

import java.lang.Runnable;
import java.util.Vector;
import java.util.List;

/**
 * ARDataTransfer MediasDownloader module
 * @author david.flattin.ext@parrot.com
 * @date 19/12/2013
 */
public class ARDataTransferMediasDownloader
{
    /* Native Functions */
    private native static boolean nativeStaticInit();
    private native int nativeNew(long manager, String deviceIP, int port, String localDirectory);
    private native int nativeDelete(long manager);    
    private native int nativeGetAvailableMedias(long manager, List<ARDataTransferMedia> mediaList);
    private native int nativeAddMediaToQueue(long manager, ARDataTransferMedia media, ARDataTransferMediasDownloaderProgressListener progressListener, Object progressArg, ARDataTransferMediasDownloaderCompletionListener completionListener, Object completionArg);
    private native void nativeQueueThreadRun(long manager);
    private native int nativeCancelQueueThread(long manager);
    
    /*  Members  */
    private boolean isInit = false;
    private long nativeManager = 0;
    private Runnable downloaderRunnable = null;
    
    /*  Java Methods */
    
    /**
     * Private ARDataTransfer MediasDownloader constructor
     * @return void
     */
    protected ARDataTransferMediasDownloader(long _nativeManager)
    {
        this.nativeManager = _nativeManager;
        
        this.downloaderRunnable = new Runnable () {
            public void run() {
                nativeQueueThreadRun(nativeManager);    
            }
        };
    }
    
    /**
     * Creates the ARDataTransfer MediasDownloader
     * @return void
     * @throws ARDataTransferException if error
     */
    public void createMediasDownloader(String deviceIP, int port, String localDirectory) throws ARDataTransferException
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
     * Creates the ARDataTransfer MediasDownloader
     * @return ARDATATRANSFER_OK if success, else an {@link ARDATATRANSFER_ERROR_ENUM} error code
     */
    public ARDATATRANSFER_ERROR_ENUM closeMediasDownloader()
    {
        int result = nativeDelete(nativeManager);
        
        ARDATATRANSFER_ERROR_ENUM error = ARDATATRANSFER_ERROR_ENUM.getFromValue(result);
        return error;
    }
    
    /**
     * Gets the {@link List} of available {@link ARDataTransferMedia} medias in the ARDataTransfer MediasDownloader
     * @return List<ARDataTransferMedia> available ARDataTransferMedia medias List
     * @throws ARDataTransferException if error
     */
    public List<ARDataTransferMedia> getAvailableMedias() throws ARDataTransferException
    {
        Vector<ARDataTransferMedia> mediaList = new Vector<ARDataTransferMedia>();
        
        int result = nativeGetAvailableMedias(nativeManager, mediaList);
        
        ARDATATRANSFER_ERROR_ENUM error = ARDATATRANSFER_ERROR_ENUM.getFromValue(result);
        
        if (error != ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK)
        {
            throw new ARDataTransferException(error);
        }
        
        return mediaList;
    }
    
    /**
     * Adds an {@link ARDataTransferMedia} media to the ARDataTransfer MediasDownloader Runnable Queue to start as new Thread
     * @param media ARDataTransferMedia media to add
     * @param progressListener ARDataTransferMediasDownloaderProgressListener progress Listener
     * @param progressArg Object progress Listener arg
     * @param completionListener ARDataTransferMediasDownloaderCompletionListener completion Listener
     * @param completionArg Object completion Listener arg
     * @return void
     * @throws ARDataTransferException if error
     */
    public void addMediaToQueue(ARDataTransferMedia media, ARDataTransferMediasDownloaderProgressListener progressListener, Object progressArg, ARDataTransferMediasDownloaderCompletionListener completionListener, Object completionArg) throws ARDataTransferException
    {
        int result = nativeAddMediaToQueue(nativeManager, media, progressListener, progressArg, completionListener, completionArg);
        
        ARDATATRANSFER_ERROR_ENUM error = ARDATATRANSFER_ERROR_ENUM.getFromValue(result);
        
        if (error != ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK)
        {
            throw new ARDataTransferException(error);
        }
    }
        
    /**
     * Gets the ARDataTransfer MediasDownloader {@link Runnable} Queue to start as new {@link Thread}
     * @return MediasDownloader Runnable
     */
    public Runnable getDownloaderQueueRunnable()
    {
        Runnable runnable = null;
        
        if (isInit == true)
        {
            runnable = this.downloaderRunnable;
        }
        
        return runnable;
    }
        
    /**
     * Cancels the ARDataTransfer MediasDownloader Runnable Queue Thread
     * @return ARDATATRANSFER_OK if success, else an {@link ARDATATRANSFER_ERROR_ENUM} error code
     */
    public ARDATATRANSFER_ERROR_ENUM cancelQueueThread()
    {
        int result = nativeCancelQueueThread(nativeManager);
        
        ARDATATRANSFER_ERROR_ENUM error = ARDATATRANSFER_ERROR_ENUM.getFromValue(result);
                
        return error;
    }
    
    /*  Static Block */
    static
    {
        nativeStaticInit();
    }        
}

