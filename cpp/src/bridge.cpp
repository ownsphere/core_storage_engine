#include "bridge.h"
#include "storage_engine.h"
#include <map>
#include <mutex>
#include <memory>
#include <string>
#include <cstring>

/**
 * Internal wrapper structure to hold C++ StorageEngine and error state
 */
struct StorageEngineWrapper {
    std::unique_ptr<StorageEngine> engine;
    std::string lastError;
    mutable std::mutex errorMutex;

    void setError(const std::string& msg) {
        std::lock_guard<std::mutex> lock(errorMutex);
        lastError = msg;
    }

    std::string getLastError() const {
        std::lock_guard<std::mutex> lock(errorMutex);
        return lastError;
    }
};

/**
 * Thread-local storage for current C callback
 * This allows the C API to associate a callback with each store/retrieve operation
 */
thread_local StorageEngineProgressCallback g_current_callback = nullptr;

/**
 * Internal adapter: Convert C callback to C++ std::function callback
 */
void invoke_c_callback(int progress) {
    if (g_current_callback) {
        g_current_callback(progress);
    }
}

// ============================================================================
// Implementation of C API functions
// ============================================================================

StorageEngineHandle storage_engine_create(const char* storageRoot) {
    try {
        auto wrapper = new StorageEngineWrapper();
        if (storageRoot && strlen(storageRoot) > 0) {
            wrapper->engine = std::make_unique<StorageEngine>(std::string(storageRoot));
        } else {
            wrapper->engine = std::make_unique<StorageEngine>();
        }
        return static_cast<StorageEngineHandle>(wrapper);
    } catch (const std::exception& e) {
        return nullptr;
    }
}

void storage_engine_destroy(StorageEngineHandle engine) {
    if (engine) {
        StorageEngineWrapper* wrapper = static_cast<StorageEngineWrapper*>(engine);
        delete wrapper;
    }
}

bool storage_engine_store_file(StorageEngineHandle engine,
                               const char* filePath,
                               const char* fileId,
                               StorageEngineProgressCallback callback) {
    if (!engine || !filePath || !fileId) {
        return false;
    }

    try {
        StorageEngineWrapper* wrapper = static_cast<StorageEngineWrapper*>(engine);

        // Set thread-local callback
        g_current_callback = callback;

        // Create C++ callback that adapts to the C callback
        std::function<void(int)> cpp_callback = nullptr;
        if (callback) {
            cpp_callback = invoke_c_callback;
        }

        bool result = wrapper->engine->storeFile(
            std::string(filePath),
            std::string(fileId),
            cpp_callback
        );

        // Clear thread-local callback
        g_current_callback = nullptr;

        return result;
    } catch (const std::exception& e) {
        StorageEngineWrapper* wrapper = static_cast<StorageEngineWrapper*>(engine);
        wrapper->setError(e.what());
        return false;
    }
}

bool storage_engine_retrieve_file(StorageEngineHandle engine,
                                  const char* fileId,
                                  const char* outputPath,
                                  StorageEngineProgressCallback callback) {
    if (!engine || !fileId || !outputPath) {
        return false;
    }

    try {
        StorageEngineWrapper* wrapper = static_cast<StorageEngineWrapper*>(engine);

        // Set thread-local callback
        g_current_callback = callback;

        // Create C++ callback that adapts to the C callback
        std::function<void(int)> cpp_callback = nullptr;
        if (callback) {
            cpp_callback = invoke_c_callback;
        }

        bool result = wrapper->engine->retrieveFile(
            std::string(fileId),
            std::string(outputPath),
            cpp_callback
        );

        // Clear thread-local callback
        g_current_callback = nullptr;

        return result;
    } catch (const std::exception& e) {
        StorageEngineWrapper* wrapper = static_cast<StorageEngineWrapper*>(engine);
        wrapper->setError(e.what());
        return false;
    }
}

char** storage_engine_list_files(StorageEngineHandle engine, int* count) {
    if (!engine || !count) {
        return nullptr;
    }

    try {
        StorageEngineWrapper* wrapper = static_cast<StorageEngineWrapper*>(engine);
        std::vector<std::string> files = wrapper->engine->listFiles();

        *count = static_cast<int>(files.size());

        // Allocate array of pointers
        char** result = static_cast<char**>(malloc(sizeof(char*) * (*count)));
        if (!result) {
            return nullptr;
        }

        // Allocate and copy each string
        for (int i = 0; i < *count; ++i) {
            size_t len = files[i].length();
            result[i] = static_cast<char*>(malloc(len + 1));
            if (!result[i]) {
                // Clean up on allocation failure
                for (int j = 0; j < i; ++j) {
                    free(result[j]);
                }
                free(result);
                *count = 0;
                return nullptr;
            }
            strcpy(result[i], files[i].c_str());
        }

        return result;
    } catch (const std::exception& e) {
        if (engine) {
            StorageEngineWrapper* wrapper = static_cast<StorageEngineWrapper*>(engine);
            wrapper->setError(e.what());
        }
        *count = 0;
        return nullptr;
    }
}

void storage_engine_free_file_list(char** fileList, int count) {
    if (!fileList) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        free(fileList[i]);
    }
    free(fileList);
}

bool storage_engine_delete_file(StorageEngineHandle engine, const char* fileId) {
    if (!engine || !fileId) {
        return false;
    }

    try {
        StorageEngineWrapper* wrapper = static_cast<StorageEngineWrapper*>(engine);
        return wrapper->engine->deleteFile(std::string(fileId));
    } catch (const std::exception& e) {
        StorageEngineWrapper* wrapper = static_cast<StorageEngineWrapper*>(engine);
        wrapper->setError(e.what());
        return false;
    }
}

bool storage_engine_delete_all_files(StorageEngineHandle engine) {
    if (!engine) {
        return false;
    }

    try {
        StorageEngineWrapper* wrapper = static_cast<StorageEngineWrapper*>(engine);
        return wrapper->engine->deleteAllFiles();
    } catch (const std::exception& e) {
        StorageEngineWrapper* wrapper = static_cast<StorageEngineWrapper*>(engine);
        wrapper->setError(e.what());
        return false;
    }
}

int storage_engine_get_progress(StorageEngineHandle engine, const char* fileId) {
    if (!engine || !fileId) {
        return -1;
    }

    try {
        StorageEngineWrapper* wrapper = static_cast<StorageEngineWrapper*>(engine);
        return wrapper->engine->getProgress(std::string(fileId));
    } catch (const std::exception& e) {
        StorageEngineWrapper* wrapper = static_cast<StorageEngineWrapper*>(engine);
        wrapper->setError(e.what());
        return -1;
    }
}

void storage_engine_wait_for_background_tasks(StorageEngineHandle engine) {
    if (!engine) {
        return;
    }

    try {
        StorageEngineWrapper* wrapper = static_cast<StorageEngineWrapper*>(engine);
        wrapper->engine->waitForBackgroundTasks();
    } catch (const std::exception& e) {
        StorageEngineWrapper* wrapper = static_cast<StorageEngineWrapper*>(engine);
        wrapper->setError(e.what());
    }
}

const char* storage_engine_get_error(StorageEngineHandle engine) {
    if (!engine) {
        return nullptr;
    }

    try {
        StorageEngineWrapper* wrapper = static_cast<StorageEngineWrapper*>(engine);
        std::string error = wrapper->getLastError();
        if (error.empty()) {
            return nullptr;
        }
        // Return pointer to internal string buffer
        // NOTE: This is only valid until the next operation on this engine handle
        return wrapper->lastError.c_str();
    } catch (const std::exception&) {
        return nullptr;
    }
}

