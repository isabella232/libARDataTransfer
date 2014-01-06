/**
 * @file ARDATATRANSFER_Manager.h
 * @brief libARDataTransfer Manager header file.
 * @date 19/12/2013
 * @author david.flattin.ext@parrot.com
 **/

#ifndef _ARDATATRANSFER_MANAGER_H_
#define _ARDATATRANSFER_MANAGER_H_

/**
 * @brief Manager structure
 * @see ARDATATRANSFER_Manager_New ()
 */
typedef struct ARDATATRANSFER_Manager_t ARDATATRANSFER_Manager_t;

/**
 * @brief Create a new ARDataTransfer Manager
 * @warning This function allocates memory
 * @param[out] error A pointer on the error output
 * @return Pointer on the new ARDataTransfer Manager
 * @see ARDATATRANSFER_Manager_Delete ()
 */
ARDATATRANSFER_Manager_t* ARDATATRANSFER_Manager_New (eARDATATRANSFER_ERROR *error);

/**
 * @brief Delete an ARDataTransfer Manager
 * @warning This function frees memory
 * @param manager The address of the pointer on the ARDataTransfer Manager
 * @see ARDATATRANSFER_Manager_New ()
 */
void ARDATATRANSFER_Manager_Delete (ARDATATRANSFER_Manager_t **managerPtrAddr);

#endif /* _ARDATATRANSFER_MANAGER_H_ */
