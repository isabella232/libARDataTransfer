/**
 * @file ARDATATRANSFER_Manager.c
 * @brief libARDataTransfer Manager c file.
 * @date 19/12/2013
 * @author david.flattin.ext@parrot.com
 **/

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include <libARSAL/ARSAL_Sem.h>
#include <libARSAL/ARSAL_Mutex.h>
#include <libARSAL/ARSAL_Print.h>
#include <libARUtils/ARUTILS_Error.h>
#include <libARUtils/ARUTILS_Ftp.h>
#include <libARUtils/ARUTILS_FileSystem.h>

#include "libARDataTransfer/ARDATATRANSFER_Error.h"
#include "libARDataTransfer/ARDATATRANSFER_Manager.h"
#include "libARDataTransfer/ARDATATRANSFER_DataDownloader.h"
#include "libARDataTransfer/ARDATATRANSFER_MediasDownloader.h"
#include "ARDATATRANSFER_MediasQueue.h"
#include "ARDATATRANSFER_DataDownloader.h"
#include "ARDATATRANSFER_MediasDownloader.h"
#include "ARDATATRANSFER_Manager.h"

#define ARDATATRANSFER_MANAGER_TAG          "Manager"

/*****************************************
 *
 *             Public implementation:
 *
 *****************************************/

ARDATATRANSFER_Manager_t * ARDATATRANSFER_Manager_New(eARDATATRANSFER_ERROR *error)
{
    ARDATATRANSFER_Manager_t *newManager = NULL;
    eARDATATRANSFER_ERROR result = 0;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_MANAGER_TAG, "");

    newManager = calloc(1, sizeof(ARDATATRANSFER_Manager_t));

    if (newManager == NULL)
    {
        result = ARDATATRANSFER_ERROR_ALLOC;
    }

    if (result != ARDATATRANSFER_OK)
    {
        ARDATATRANSFER_Manager_Delete(&newManager);
    }

    *error = result;
    return newManager;
}

void ARDATATRANSFER_Manager_Delete(ARDATATRANSFER_Manager_t **managerPtrAddr)
{
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARDATATRANSFER_MANAGER_TAG, "");

    if (managerPtrAddr != NULL)
    {
        ARDATATRANSFER_Manager_t *manager = *managerPtrAddr;
        
        if (manager != NULL)
        {
            free(manager);
            *managerPtrAddr = NULL;
        }
    }
}

/*****************************************
 *
 *             Private implementation:
 *
 *****************************************/

