
package com.parrot.arsdk.ardatatransfer;

import com.parrot.arsdk.ardiscovery.ARDISCOVERY_PRODUCT_ENUM;

/**
 * ARDataTransfer Media
 * @author david.flattin.ext@parrot.com
 * @date 19/12/2013
 */
public class ARDataTransferMedia
{
    /*  Members  */
    private ARDISCOVERY_PRODUCT_ENUM product = null;
    private String name = null;
    private String filePath = null;
    private String date = null;
    private float size = 0.f;
    private byte[] thumbnail = null;

    /*  Java Methods */

    /**
     * Private ARDataTransfer Media constructor
     * @note private ARDataTransfer Media constructor
     * @param name String Media Name
     * @param filePath String Media Path
     * @param date String Media Date
     * @param size float Media Size
     * @param thumbnail byte[] Media Thumbnail
     * @return void
     */
    protected ARDataTransferMedia(int productValue, String name, String filePath, String date, float size, byte[] thumbnail)
    {
        this.product = ARDISCOVERY_PRODUCT_ENUM.getFromValue(productValue);
        this.name = name;
        this.filePath = filePath;
        this.date = date;
        this.size = size;
        this.thumbnail = thumbnail;
    }

    /**
     * Gets the Media product it belong to
     * @note get the Media Product
     * @return String media Product
     */
    public ARDISCOVERY_PRODUCT_ENUM getProduct()
    {
        return this.product;
    }

    /**
     * Gets the Media product it belong to
     * @note get the Media Product
     * @return String media Product
     */
    public int getProductValue()
    {
        return this.product.getValue();
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
     * Gets the Media file Path
     * @note get the Media file Path
     * @return String media file Path
     */
    public String getFilePath()
    {
        return this.filePath;
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
