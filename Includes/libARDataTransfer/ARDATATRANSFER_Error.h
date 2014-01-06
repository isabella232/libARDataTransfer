
/**
 * @file libARDataTransfer/ARDATATRANSFER_Error.h
 * @brief libARDataTransfer error header. This file contains all error of ARDataTransfer
 * @date 19/12/2013
 * @author david.flattin.ext@parrot.com
 **/

#ifndef _LIBARDATATRANSFER_ERROR_H_
#define _LIBARDATATRANSFER_ERROR_H_

/**
 * @brief libARDataTransfer errors known.
 */
typedef enum
{
    ARDATATRANSFER_OK = 0, /**< No error */
    ARDATATRANSFER_ERROR = -1000, /**< Unknown generic error */
    ARDATATRANSFER_ERROR_ALLOC, /**< Memory allocation error */
    ARDATATRANSFER_ERROR_BAD_PARAMETER, /**< Bad parameters error */
    ARDATATRANSFER_ERROR_NOT_INITIALIZED, /**< Not initialized error */
    ARDATATRANSFER_ERROR_ALREADY_INITIALIZED, /**< Already initialized error */
    ARDATATRANSFER_ERROR_THREAD_ALREADY_RUNNING, /**< Thread already running error */
    ARDATATRANSFER_ERROR_THREAD_PROCESSING, /**< Thread processing error */
    ARDATATRANSFER_ERROR_CANCELED, /**< Canceled received */
    ARDATATRANSFER_ERROR_SYSTEM, /**< System error */
    ARDATATRANSFER_ERROR_FTP, /**< Ftp error */
    ARDATATRANSFER_ERROR_FILE, /**< File error */
    
} eARDATATRANSFER_ERROR;


/**
 * @brief Gets the error string associated with an eARDATATRANSFER_ERROR
 * @param error The error to describe
 * @return A static string describing the error
 *
 * @note User should NEVER try to modify a returned string
 */
char* ARDATATRANSFER_Error_ToString (eARDATATRANSFER_ERROR error);

#endif /* _LIBARDATATRANSFER_ERROR_H_ */
