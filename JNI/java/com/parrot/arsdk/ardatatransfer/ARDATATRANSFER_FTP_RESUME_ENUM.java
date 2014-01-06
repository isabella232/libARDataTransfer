/*
 * GENERATED FILE
 *  Do not modify this file, it will be erased during the next configure run
 */

package com.parrot.arsdk.ardatatransfer;

import java.util.HashMap;

/**
 * Java copy of the eARDATATRANSFER_FTP_RESUME enum
 */
public enum ARDATATRANSFER_FTP_RESUME_ENUM {
   FTP_RESUME_FALSE (0),
   FTP_RESUME_TRUE (1);

    private final int value;
    private final String comment;
    static HashMap<Integer, ARDATATRANSFER_FTP_RESUME_ENUM> valuesList;

    ARDATATRANSFER_FTP_RESUME_ENUM (int value) {
        this.value = value;
        this.comment = null;
    }

    ARDATATRANSFER_FTP_RESUME_ENUM (int value, String comment) {
        this.value = value;
        this.comment = comment;
    }

    /**
     * Gets the int value of the enum
     * @return int value of the enum
     */
    public int getValue () {
        return value;
    }

    /**
     * Gets the ARDATATRANSFER_FTP_RESUME_ENUM instance from a C enum value
     * @param value C value of the enum
     * @return The ARDATATRANSFER_FTP_RESUME_ENUM instance, or null if the C enum value was not valid
     */
    public static ARDATATRANSFER_FTP_RESUME_ENUM getFromValue (int value) {
        if (null == valuesList) {
            ARDATATRANSFER_FTP_RESUME_ENUM [] valuesArray = ARDATATRANSFER_FTP_RESUME_ENUM.values ();
            valuesList = new HashMap<Integer, ARDATATRANSFER_FTP_RESUME_ENUM> (valuesArray.length);
            for (ARDATATRANSFER_FTP_RESUME_ENUM entry : valuesArray) {
                valuesList.put (entry.getValue (), entry);
            }
        }
        return valuesList.get (value);
    }

    /**
     * Returns the enum comment as a description string
     * @return The enum description
     */
    public String toString () {
        if (this.comment != null) {
            return this.comment;
        }
        return super.toString ();
    }
}
