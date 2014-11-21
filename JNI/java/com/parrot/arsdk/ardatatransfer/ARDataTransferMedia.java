/*
    Copyright (C) 2014 Parrot SA

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the 
      distribution.
    * Neither the name of Parrot nor the names
      of its contributors may be used to endorse or promote products
      derived from this software without specific prior written
      permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
    OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
    OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/

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
    private String uuid = null;
    private float size = 0.f;
    private byte[] thumbnail = null;

    /*  Java Methods */

    /**
     * Private ARDataTransfer Media constructor
     * @note private ARDataTransfer Media constructor
     * @param name String Media Name
     * @param filePath String Media Path
     * @param date String Media Date
     * @param uuid String Media UUID
     * @param size float Media Size
     * @param thumbnail byte[] Media Thumbnail
     * @return void
     */
    protected ARDataTransferMedia(int productValue, String name, String filePath, String date, String uuid, float size, byte[] thumbnail)
    {
        this.product = ARDISCOVERY_PRODUCT_ENUM.getFromValue(productValue);
        this.name = name;
        this.filePath = filePath;
        this.date = date;
        this.uuid = uuid;
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
     * Gets the Media UUID
     * @note get the Media UUID
     * @return String media UUID
     */
    public String getUUID()
    {
        return this.uuid;
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
