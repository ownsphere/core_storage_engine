#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
 * Opaque handle to StorageEngine instance
 */
typedef void* StorageEngineHandle;

/**
 * Progress callback signature for C API
 * Called with progress percentage (0-100)
 */
typedef void (*StorageEngineProgressCallback)(int progress);

/**
 * Create a new StorageEngine instance
 *
 * @param storageRoot Path to storage directory (NULL for default "data")
 * @return Handle to engine instance, must be freed with storage_engine_destroy()
 */
StorageEngineHandle storage_engine_create(const char* storageRoot);

/**
 * Destroy a StorageEngine instance
 *
 * @param engine Handle returned from storage_engine_create()
 */
void storage_engine_destroy(StorageEngineHandle engine);

/**
 * Store a file in the storage engine
 *
 * @param engine Handle to engine instance
 * @param filePath Path to source file
 * @param fileId Unique identifier for the file
 * @param callback Optional progress callback (can be NULL)
 * @return true on success, false on failure
 */
bool storage_engine_store_file(StorageEngineHandle engine,
                               const char* filePath,
                               const char* fileId,
                               StorageEngineProgressCallback callback);

bool storage_engine_store_file_with_metadata(StorageEngineHandle engine,
                                             const char* filePath,
                                             const char* fileId,
                                             const char* originalFilename,
                                             const char* extension,
                                             const char* contentType,
                                             const char* checksum,
                                             long long uploadedAtEpochMs,
                                             StorageEngineProgressCallback callback);

/**
 * Retrieve a file from the storage engine
 *
 * @param engine Handle to engine instance
 * @param fileId Identifier of file to retrieve
 * @param outputPath Path where to write the retrieved file
 * @param callback Optional progress callback (can be NULL)
 * @return true on success, false on failure
 */
bool storage_engine_retrieve_file(StorageEngineHandle engine,
                                  const char* fileId,
                                  const char* outputPath,
                                  StorageEngineProgressCallback callback);

char* storage_engine_get_file_metadata_json(StorageEngineHandle engine, const char* fileId);
void storage_engine_free_string(char* value);

/**
 * List all stored files
 *
 * @param engine Handle to engine instance
 * @param count Output parameter for number of files
 * @return Array of file IDs (strings), must be freed with storage_engine_free_file_list()
 */
char** storage_engine_list_files(StorageEngineHandle engine, int* count);

/**
 * Free file list returned from storage_engine_list_files()
 *
 * @param fileList Array of strings from storage_engine_list_files()
 * @param count Number of strings in array
 */
void storage_engine_free_file_list(char** fileList, int count);

/**
 * Delete a file from storage
 *
 * @param engine Handle to engine instance
 * @param fileId Identifier of file to delete
 * @return true on success, false on failure
 */
bool storage_engine_delete_file(StorageEngineHandle engine, const char* fileId);

/**
 * Delete all files from storage
 *
 * @param engine Handle to engine instance
 * @return true on success, false on failure
 */
bool storage_engine_delete_all_files(StorageEngineHandle engine);

/**
 * Get current progress for a file operation
 *
 * @param engine Handle to engine instance
 * @param fileId Identifier of file
 * @return Progress percentage (0-100), or -1 if not found
 */
int storage_engine_get_progress(StorageEngineHandle engine, const char* fileId);

/**
 * Wait for all background tasks to complete
 *
 * @param engine Handle to engine instance
 */
void storage_engine_wait_for_background_tasks(StorageEngineHandle engine);

/**
 * Get last error message as a string
 *
 * @param engine Handle to engine instance
 * @return Error message, or NULL if no error. String is valid until next operation.
 */
const char* storage_engine_get_error(StorageEngineHandle engine);

#ifdef __cplusplus
}
#endif
