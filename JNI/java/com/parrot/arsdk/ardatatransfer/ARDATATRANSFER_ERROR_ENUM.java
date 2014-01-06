/*
 * GENERATED FILE
 *  Do not modify this file, it will be erased during the next configure run
 */

package com.parrot.arsdk.ardatatransfer;

import java.util.HashMap;

/**
 * Java copy of the eARDATATRANSFER_ERROR enum
 */
public enum ARDATATRANSFER_ERROR_ENUM {
   /** No error */
    ARDATATRANSFER_OK (0, "No error"),
   /** Unknown generic error */
    ARDATATRANSFER_ERROR (-1000, "Unknown generic error"),
   /** Memory allocation error */
    ARDATATRANSFER_ERROR_ALLOC (-999, "Memory allocation error"),
   /** Bad parameters error */
    ARDATATRANSFER_ERROR_BAD_PARAMETER (-998, "Bad parameters error"),
   /** Not initialized error */
    ARDATATRANSFER_ERROR_NOT_INITIALIZED (-997, "Not initialized error"),
   /** Already initialized error */
    ARDATATRANSFER_ERROR_ALREADY_INITIALIZED (-996, "Already initialized error"),
   /** Thread already running error */
    ARDATATRANSFER_ERROR_THREAD_ALREADY_RUNNING (-995, "Thread already running error"),
   /** Thread processing error */
    ARDATATRANSFER_ERROR_THREAD_PROCESSING (-994, "Thread processing error"),
   /** Canceled received */
    ARDATATRANSFER_ERROR_CANCELED (-993, "Canceled received"),
   /** System error */
    ARDATATRANSFER_ERROR_SYSTEM (-992, "System error"),
   /** Ftp error */
    ARDATATRANSFER_ERROR_FTP (-991, "Ftp error"),
   /** File error */
    ARDATATRANSFER_ERROR_FILE (-990, "File error");

    private final int value;
    private final String comment;
    static HashMap<Integer, ARDATATRANSFER_ERROR_ENUM> valuesList;

    ARDATATRANSFER_ERROR_ENUM (int value) {
        this.value = value;
        this.comment = null;
    }

    ARDATATRANSFER_ERROR_ENUM (int value, String comment) {
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
     * Gets the ARDATATRANSFER_ERROR_ENUM instance from a C enum value
     * @param value C value of the enum
     * @return The ARDATATRANSFER_ERROR_ENUM instance, or null if the C enum value was not valid
     */
    public static ARDATATRANSFER_ERROR_ENUM getFromValue (int value) {
        if (null == valuesList) {
            ARDATATRANSFER_ERROR_ENUM [] valuesArray = ARDATATRANSFER_ERROR_ENUM.values ();
            valuesList = new HashMap<Integer, ARDATATRANSFER_ERROR_ENUM> (valuesArray.length);
            for (ARDATATRANSFER_ERROR_ENUM entry : valuesArray) {
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
