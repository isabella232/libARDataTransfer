
package com.parrot.arsdk.ardatatransfer;

/**
 * ARDataTransfer Media
 * @author david.flattin.ext@parrot.com
 * @date 19/12/2013
 */
public class ARDataTransferMedia
{
    /*  Members  */
    private String name = null;
    private String date = null;
    private float size = 0.f;
    private byte[] thumbnail = null;

    /*  Java Methods */
    
    /**
     * Private ARDataTransfer Media constructor
     * @note private ARDataTransfer Media constructor
     * @param name String Media Name
     * @param date String Media Date
     * @param size float Media Size
     * @param thumbnail byte[] Media Thumbnail
     * @return void
     */
    protected ARDataTransferMedia(String name, String date, float size, byte[] thumbnail)
    {
        this.name = name;
        this.date = date;
        this.size = size;
        this.thumbnail = thumbnail;
    }
    
    /**
     * Gets the Media Name
     * @note get the Media Name
     * @return String media Name
     */
    public String getName()
    {
        return this.name;
    }
    
    /**
     * Gets the Media Date
     * @note get the Media Date
     * @return String media Date
     */
    public String getDate()
    {
        return this.date;
    }
    
    /**
     * Gets the Media Size
     * @note get the Media Size
     * @return float media Size
     */
    public float getSize()
    {
        return this.size;
    }
    
    /**
     * Gets the Media Thumbnail
     * @note get the Media Thumbnail
     * @return byte[] media Thumbnail
     */
    public byte[] getThumbnail()
    {
        return this.thumbnail;
    }
}
