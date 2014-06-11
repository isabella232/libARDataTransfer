
package com.parrot.arsdk.ardatatransfer;

import java.lang.Runnable;
import com.parrot.arsdk.arsal.ARSALPrint;
import com.parrot.arsdk.arutils.ARUtilsManager;

/**
 * ARDataTransfer Uploader module
 * @author david.flattin.ext@parrot.com
 * @date 09/06/2014
 */
public class ARDataTransferUploader
{
    /* Native Functions */
    private native static boolean nativeStaticInit();
    private native int nativeNew(long manager, long utilsManager, String remotePath, String localPath, ARDataTransferUploaderProgressListener progressListener, Object progressArg, ARDataTransferUploaderCompletionListener completionListener, Object completionArg, int resume);
    private native int nativeDelete(long manager);
    private native void nativeThreadRun(long manager);
    private native int nativeCancelThread(long manager);

    /*  Members  */
    private static final String TAG = ARDataTransferMediasDownloader.class.getSimpleName ();
    private boolean isInit = false;
    private long nativeManager = 0;
    private Runnable uploaderRunnable = null;

    /*  Java Methods */

    /**
     * Private ARDataTransfer Uploader constructor
     * @return void
     */
    protected ARDataTransferUploader(long _nativeManager)
    {
        this.nativeManager = _nativeManager;

        this.uploaderRunnable = new Runnable () {
            public void run() {
                nativeThreadRun(nativeManager);
            }
        };
    }

    /**
     * Creates the ARDataTransfer Uploader
     * @param utilsManager The FTP server ip address
     * @param remotePath The FTP Server local directory
     * @param localPath The local system directory
     * @return void
     * @throws ARDataTransferException if error
     */
    public void createUploader(ARUtilsManager utilsManager, String remotePath, String localPath, ARDataTransferUploaderProgressListener progressListener, Object progressArg, ARDataTransferUploaderCompletionListener completionListener, Object completionArg, ARDATATRANSFER_UPLOADER_RESUME_ENUM resume) throws ARDataTransferException
    {
        int result = nativeNew(nativeManager, utilsManager.getManager(), remotePath, localPath, progressListener, progressArg, completionListener, completionArg, resume.getValue());

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
     * Deletes the ARDataTransfer Uploader
     * @return ARDATATRANSFER_OK if success, else an {@link ARDATATRANSFER_ERROR_ENUM} error code
     */
    public ARDATATRANSFER_ERROR_ENUM dispose()
    {
        ARDATATRANSFER_ERROR_ENUM error = ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK;

        if (isInit)
        {
            int result = nativeDelete(nativeManager);

            error = ARDATATRANSFER_ERROR_ENUM.getFromValue(result);
            if (error == ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK)
            {
                isInit = false;
            }
        }

        return error;
    }

    /**
     * Destructor<br>
     * This destructor tries to avoid leaks if the object was not disposed
     */
    protected void finalize () throws Throwable
    {
        try
        {
            if (isInit)
            {
                ARSALPrint.e (TAG, "Object " + this + " was not disposed !");
                ARDATATRANSFER_ERROR_ENUM error = dispose ();
                if(error != ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_OK)
                {
                    ARSALPrint.e (TAG, "Unable to dispose object " + this + " ... leaking memory !");
                }
            }
        }
        finally
        {
            super.finalize ();
        }
    }

    /**
     * Gets the ARDataTransfer Uploader {@link Runnable} to start as new {@link Thread}
     * @return Uploader Runnable
     */
    public Runnable getUploaderRunnable()
    {
        Runnable runnable = null;

        if (isInit == true)
        {
            runnable = this.uploaderRunnable;
        }

        return runnable;
    }

    /**
     * Cancels the ARDataTransfer Uploader Runnable Thread
     * @return ARDATATRANSFER_OK if success, else an {@link ARDATATRANSFER_ERROR_ENUM} error code
     */
    public ARDATATRANSFER_ERROR_ENUM cancelThread()
    {
        int result = nativeCancelThread(nativeManager);

        ARDATATRANSFER_ERROR_ENUM error = ARDATATRANSFER_ERROR_ENUM.getFromValue(result);

        return error;
    }

    /*  Static Block */
    static
    {
        nativeStaticInit();
    }
}




