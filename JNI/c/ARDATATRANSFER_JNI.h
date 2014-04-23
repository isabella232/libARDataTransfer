/**
 * @file ARDATATRANSFER_JNI.h
 * @brief libARDataTransfer JNI header file.
 * @date 19/12/2013
 * @author david.flattin.ext@parrot.com
 **/

#ifndef _ARDATATRANSFER_JNI_H_
#define _ARDATATRANSFER_JNI_H_

#ifndef JNI_OK
#define JNI_OK      0
#endif
#ifndef JNI_FAILED
#define JNI_FAILED  -1
#endif

extern JavaVM* ARDATATRANSFER_JNI_Manager_VM;
extern jclass classMDMedia;

/**
 * @brief MediasDownloader Callbacks structure
 * @param jMedia The media
 * @param jProgressListener The progress Listener
 * @param jProgressArg The progress Arg object
 * @param jCompletionListener The completion Listener
 * @param jCompletionArg The completion Arg object
 * @see ARDATATRANSFER_JNI_MediasDownloader_FreeMediasDownloaderCallbacks
 */
typedef struct _ARDATATRANSFER_JNI_MediasDownloaderCallbacks_t_
{
    jobject jMedia;
    jobject jProgressListener;
    jobject jProgressArg;
    jobject jCompletionListener;
    jobject jCompletionArg;
    jobject jAvailableMediaListener;
    jobject jAvailableMediaArg;

} ARDATATRANSFER_JNI_MediasDownloaderCallbacks_t;

/**
 * @brief Throw a new ARDataTransferException
 * @param env The java env
 * @param nativeError The error
 * @retval void
 * @see ARDATATRANSFER_JNI_Manager_NewARDataTransferException
 */
void ARDATATRANSFER_JNI_Manager_ThrowARDataTransferException(JNIEnv *env, eARDATATRANSFER_ERROR nativeError);

/**
 * @brief Create a new ARDataTransferException
 * @param env The java env
 * @param nativeError The error
 * @retval the new ARDataTransferException
 * @see ARDATATRANSFER_JNI_Manager_ThrowARDataTransferException
 */
jobject ARDATATRANSFER_JNI_Manager_NewARDataTransferException(JNIEnv *env, eARDATATRANSFER_ERROR nativeError);

/**
 * @brief Get the ARDataTransferException JNI class
 * @param env The java env
 * @retval JNI_TRUE if Success, else JNI_FALSE
 * @see ARDATATRANSFER_JNI_Manager_FreeARDataTransferExceptionJNI
 */
int ARDATATRANSFER_JNI_Manager_NewARDataTransferExceptionJNI(JNIEnv *env);

/**
 * @brief Free the ARDataTransferException JNI class
 * @param env The java env
 * @retval void
 * @see ARDATATRANSFER_JNI_Manager_NewARDataTransferExceptionJNI
 */
void ARDATATRANSFER_JNI_Manager_FreeARDataTransferExceptionJNI(JNIEnv *env);

/**
 * @brief Create a new ARDATATRANSFER_ERROR_ENUM
 * @param env The java env
 * @param nativeError The error
 * @retval the new ARDATATRANSFER_ERROR_ENUM
 * @see ARDATATRANSFER_JNI_Manager_NewERROR_ENUM_JNI
 */
jobject ARDATATRANSFER_JNI_Manager_NewERROR_ENUM(JNIEnv *env, eARDATATRANSFER_ERROR nativeError);

/**
 * @brief Get the ARDATATRANSFER_ERROR_ENUM JNI class
 * @param env The java env
 * @retval JNI_TRUE if Success, else JNI_FALSE
 * @see ARDATATRANSFER_JNI_Manager_FreeERROR_ENUM_JNI
 */
int ARDATATRANSFER_JNI_Manager_NewERROR_ENUM_JNI(JNIEnv *env);

/**
 * @brief Free the ARDATATRANSFER_ERROR_ENUM JNI class
 * @param env The java env
 * @retval void
 * @see ARDATATRANSFER_JNI_Manager_NewERROR_ENUM_JNI
 */
void ARDATATRANSFER_JNI_Manager_FreeERROR_ENUM_JNI(JNIEnv *env);

/**
 * @brief Create a new ARDataTransferMedia
 * @param env The java env
 * @param media The native media
 * @retval the new ARDataTransferMedia
 * @see ARDATATRANSFER_JNI_MediasDownloader_AddMedia
 */
jobject ARDATATRANSFER_JNI_MediasDownloader_NewMedia(JNIEnv *env, ARDATATRANSFER_Media_t *media);

/**
 * @brief Get an ARDataTransferMedia from a native media
 * @param env The java env
 * @param media The native media
 * @retval JNI_TRUE if Success, else JNI_FALSE
 * @see ARDATATRANSFER_JNI_MediasDownloader_AddMedia
 */
int ARDATATRANSFER_JNI_MediasDownloader_GetMedia(JNIEnv *env, jobject jMedia, ARDATATRANSFER_Media_t *media);

/**
 * @brief Get the ARDataTransferMedia JNI class
 * @param env The java env
 * @retval JNI_TRUE if Success, else JNI_FALSE
 * @see ARDATATRANSFER_JNI_MediasDownloader_FreeMediaJNI
 */
int ARDATATRANSFER_JNI_MediasDownloader_NewMediaJNI(JNIEnv *env);

/**
 * @brief Free the ARDataTransferMedia JNI class
 * @param env The java env
 * @retval void
 * @see ARDATATRANSFER_JNI_MediasDownloader_NewMediaJNI
 */
void ARDATATRANSFER_JNI_MediasDownloader_FreeMediaJNI(JNIEnv *env);

/**
 * @brief Get the ARDataTransferMediasDownloaderProgressListener and ARDataTransferMediasDownloaderCompletionListener JNI classes
 * @param env The java env
 * @retval JNI_TRUE if Success, else JNI_FALSE
 * @see ARDATATRANSFER_JNI_MediasDownloader_FreeListenersJNI
 */
int ARDATATRANSFER_JNI_MediasDownloader_NewListenersJNI(JNIEnv *env);

/**
 * @brief Free the ARDataTransferMediasDownloaderProgressListener and ARDataTransferMediasDownloaderCompletionListener JNI classes
 * @param env The java env
 * @retval void
 * @see ARDATATRANSFER_JNI_MediasDownloader_NewListenersJNI
 */
void ARDATATRANSFER_JNI_MediasDownloader_FreeListenersJNI(JNIEnv *env);

/**
 * @brief Callback that give the media download progress persent
 * @param arg The arg
 * @param media The media
 * @param percent The progress percent
 * @retval void
 * @see ARDATATRANSFER_JNI_MediasDownloader_FreeListenersJNI
 */
void ARDATATRANSFER_JNI_MediasDownloader_ProgressCallback(void* arg, ARDATATRANSFER_Media_t *media, uint8_t percent);

/**
 * @brief Callback that give the media download completion status
 * @param arg The arg
 * @param media The media
 * @param nativeError The error status of the media download
 * @retval void
 * @see ARDATATRANSFER_JNI_MediasDownloader_FreeListenersJNI
 */
void ARDATATRANSFER_JNI_MediasDownloader_CompletionCallback(void* arg, ARDATATRANSFER_Media_t *media, eARDATATRANSFER_ERROR nativeError);

/**
 * @brief Callback that give e new discoverd media
 * @param arg The arg
 * @param media The media
 * @retval void
 * @see ARDATATRANSFER_JNI_MediasDownloader_FreeListenersJNI
 */
void ARDATATRANSFER_JNI_MediasDownloader_AvailableMediaCallback(void* arg, ARDATATRANSFER_Media_t *media);

/**
 * @brief Free Callbacks structure
 * @warning This function frees memory
 * @param env The java env
 * @param callbacks The callbacks structure
 * @retval void
 * @see ARDATATRANSFER_JNI_MediasDownloaderCallbacks_t
 */
void ARDATATRANSFER_JNI_MediasDownloader_FreeMediasDownloaderCallbacks(JNIEnv *env, ARDATATRANSFER_JNI_MediasDownloaderCallbacks_t **callbacks);

#endif /* _ARDATATRANSFER_JNI_H_ */

