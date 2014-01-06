
package com.parrot.arsdk.ardatatransfer;

/**
 * Exception class: ARDataTransferException of ARDataTransfer library
 * @date 19/12/2013
 * @author david.flattin.ext@parrot.com
 */
public class ARDataTransferException extends Exception
{	
    private ARDATATRANSFER_ERROR_ENUM error;
    
    /**
     * ARDataTransferException constructor
     * @return void
     */
    public ARDataTransferException()
    {
        error = ARDATATRANSFER_ERROR_ENUM.ARDATATRANSFER_ERROR;
    }
    
    /**
     * ARDataTransferException constructor
     * @param error ARDATATRANSFER_ERROR_ENUM error code
     * @return void
     */
    public ARDataTransferException(ARDATATRANSFER_ERROR_ENUM error) 
    {
        this.error = error;
    }
    
    /**
     * ARDataTransferException constructor
     * @param error int error code
     * @return void
     */
    public ARDataTransferException(int error) 
    {
        this.error = ARDATATRANSFER_ERROR_ENUM.getFromValue(error);
    }
    
    /**
     * Gets ARDataTransfer ERROR code
     * @return {@link ARDATATRANSFER_ERROR_ENUM} error code
     */
    public ARDATATRANSFER_ERROR_ENUM getError()
    {
        return error;
    }
    
    /**
     * Sets ARDataTransfer ERROR code
     * @param error {@link ARDATATRANSFER_ERROR_ENUM} error code     
     * @return void
     */
    public void setError(ARDATATRANSFER_ERROR_ENUM error)
    {
        this.error = error;
    }
    
    /**
     * Gets ARDataTransferException string representation
     * @return String Exception representation
     */
    public String toString ()
    {
        String str;
        
        if (null != error)
        {
            str = "ARDataTransferException [" + error.toString() + "]";
        }
        else
        {
            str = super.toString();
        }
        
        return str;
    }
}

