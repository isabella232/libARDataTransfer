/*
 * GENERATED FILE
 *  Do not modify this file, it will be erased during the next configure run
 */

/**
 * @file ARDATATRANSFER_Error.c
 * @brief ToString function for eARDATATRANSFER_ERROR enum
 */

#include <libARDataTransfer/ARDATATRANSFER_Error.h>

char* ARDATATRANSFER_Error_ToString (eARDATATRANSFER_ERROR error)
{
    switch (error)
    {
    case ARDATATRANSFER_OK:
        return "No error";
        break;
    case ARDATATRANSFER_ERROR:
        return "Unknown generic error";
        break;
    case ARDATATRANSFER_ERROR_ALLOC:
        return "Memory allocation error";
        break;
    case ARDATATRANSFER_ERROR_BAD_PARAMETER:
        return "Bad parameters error";
        break;
    case ARDATATRANSFER_ERROR_NOT_INITIALIZED:
        return "Not initialized error";
        break;
    case ARDATATRANSFER_ERROR_ALREADY_INITIALIZED:
        return "Already initialized error";
        break;
    case ARDATATRANSFER_ERROR_THREAD_ALREADY_RUNNING:
        return "Thread already running error";
        break;
    case ARDATATRANSFER_ERROR_THREAD_PROCESSING:
        return "Thread processing error";
        break;
    case ARDATATRANSFER_ERROR_CANCELED:
        return "Canceled received";
        break;
    case ARDATATRANSFER_ERROR_SYSTEM:
        return "System error";
        break;
    case ARDATATRANSFER_ERROR_FTP:
        return "Ftp error";
        break;
    case ARDATATRANSFER_ERROR_FILE:
        return "File error";
        break;
    default:
        return "Unknown value";
        break;
    }
    return "Unknown value";
}
