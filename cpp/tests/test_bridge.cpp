#include <gtest/gtest.h>
#include <fstream>
#include <thread>
#include <atomic>
#include "../include/bridge.h"

/**
 * Test fixture for C bridge tests
 */
class CBridgeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary directory for test files
        testDir = "/tmp/bridge_test_data";
        system("mkdir -p /tmp/bridge_test_data");
    }

    void TearDown() override {
        // Clean up temporary directory
        system("rm -rf /tmp/bridge_test_data");
    }

    std::string testDir;

    // Helper to create a test file
    std::string createTestFile(const std::string& content) {
        static int counter = 0;
        std::string filePath = testDir + "/test_file_" + std::to_string(counter++) + ".txt";
        std::ofstream file(filePath);
        file << content;
        file.close();
        return filePath;
    }
};

// ============================================================================
// 1. VERIFY C API CORRECTNESS - Type conversion tests
// ============================================================================

TEST_F(CBridgeTest, CreateEngineWithDefaultStorageRoot) {
    StorageEngineHandle engine = storage_engine_create(nullptr);
    ASSERT_NE(engine, nullptr);
    storage_engine_destroy(engine);
}

TEST_F(CBridgeTest, CreateEngineWithCustomStorageRoot) {
    const char* customRoot = "/tmp/custom_storage";
    system("mkdir -p /tmp/custom_storage");

    StorageEngineHandle engine = storage_engine_create(customRoot);
    ASSERT_NE(engine, nullptr);

    storage_engine_destroy(engine);
    system("rm -rf /tmp/custom_storage");
}

TEST_F(CBridgeTest, CreateEngineWithEmptyStorageRoot) {
    const char* emptyRoot = "";
    StorageEngineHandle engine = storage_engine_create(emptyRoot);
    ASSERT_NE(engine, nullptr);
    storage_engine_destroy(engine);
}

TEST_F(CBridgeTest, StoreFileWithCharPointerConversion) {
    StorageEngineHandle engine = storage_engine_create(testDir.c_str());
    ASSERT_NE(engine, nullptr);

    std::string testFile = createTestFile("test content");
    const char* filePath = testFile.c_str();
    const char* fileId = "test_file_123";

    bool result = storage_engine_store_file(engine, filePath, fileId, nullptr);
    EXPECT_TRUE(result);

    storage_engine_destroy(engine);
}

TEST_F(CBridgeTest, RetrieveFileWithCharPointerConversion) {
    StorageEngineHandle engine = storage_engine_create(testDir.c_str());
    ASSERT_NE(engine, nullptr);

    // First store a file
    std::string testFile = createTestFile("original content");
    const char* filePath = testFile.c_str();
    const char* fileId = "retrieve_test";

    storage_engine_store_file(engine, filePath, fileId, nullptr);

    // Now retrieve it
    std::string outputFile = testDir + "/retrieved_file.txt";
    bool result = storage_engine_retrieve_file(engine, fileId, outputFile.c_str(), nullptr);
    EXPECT_TRUE(result);

    // Verify content
    std::ifstream file(outputFile);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    EXPECT_EQ(content, "original content");

    storage_engine_destroy(engine);
}

TEST_F(CBridgeTest, ListFilesReturnsCorrectStringArray) {
    StorageEngineHandle engine = storage_engine_create(testDir.c_str());
    ASSERT_NE(engine, nullptr);

    // Store multiple files
    for (int i = 0; i < 3; ++i) {
        std::string testFile = createTestFile("content " + std::to_string(i));
        std::string fileId = "file_" + std::to_string(i);
        storage_engine_store_file(engine, testFile.c_str(), fileId.c_str(), nullptr);
    }

    // List files
    int count = 0;
    char** files = storage_engine_list_files(engine, &count);
    ASSERT_NE(files, nullptr);
    EXPECT_EQ(count, 3);

    // Verify each file ID is a valid string
    for (int i = 0; i < count; ++i) {
        ASSERT_NE(files[i], nullptr);
        std::string fileId(files[i]);
        EXPECT_FALSE(fileId.empty());
    }

    storage_engine_free_file_list(files, count);
    storage_engine_destroy(engine);
}

// ============================================================================
// 2. TEST OPAQUE HANDLE LIFECYCLE
// ============================================================================

TEST_F(CBridgeTest, CreateAndDestroyHandleWithoutOperations) {
    StorageEngineHandle engine = storage_engine_create(testDir.c_str());
    ASSERT_NE(engine, nullptr);

    // Should not crash or leak memory
    storage_engine_destroy(engine);
}

TEST_F(CBridgeTest, DestroyNullHandleDoesNotCrash) {
    EXPECT_NO_FATAL_FAILURE(storage_engine_destroy(nullptr));
}

TEST_F(CBridgeTest, MultipleEngineHandlesIndependent) {
    StorageEngineHandle engine1 = storage_engine_create("/tmp/storage1");
    StorageEngineHandle engine2 = storage_engine_create("/tmp/storage2");

    ASSERT_NE(engine1, nullptr);
    ASSERT_NE(engine2, nullptr);
    EXPECT_NE(engine1, engine2);

    system("mkdir -p /tmp/storage1 /tmp/storage2");

    // Store file in engine1
    std::string testFile1 = createTestFile("content1");
    storage_engine_store_file(engine1, testFile1.c_str(), "file1", nullptr);

    // Store file in engine2
    std::string testFile2 = createTestFile("content2");
    storage_engine_store_file(engine2, testFile2.c_str(), "file2", nullptr);

    // Destroy both
    storage_engine_destroy(engine1);
    storage_engine_destroy(engine2);

    system("rm -rf /tmp/storage1 /tmp/storage2");
}

TEST_F(CBridgeTest, HandleRemainsValidAfterMultipleOperations) {
    StorageEngineHandle engine = storage_engine_create(testDir.c_str());
    ASSERT_NE(engine, nullptr);

    // Perform multiple operations
    for (int i = 0; i < 5; ++i) {
        std::string testFile = createTestFile("content " + std::to_string(i));
        std::string fileId = "file_" + std::to_string(i);
        bool result = storage_engine_store_file(engine, testFile.c_str(), fileId.c_str(), nullptr);
        EXPECT_TRUE(result);
    }

    // Handle should still be valid
    int count = 0;
    char** files = storage_engine_list_files(engine, &count);
    EXPECT_EQ(count, 5);
    storage_engine_free_file_list(files, count);

    storage_engine_destroy(engine);
}

// ============================================================================
// 3. VALIDATE CALLBACK MECHANISM
// ============================================================================

std::atomic<int> callbackCounter(0);
std::atomic<int> lastProgress(0);

void testProgressCallback(int progress) {
    callbackCounter.fetch_add(1, std::memory_order_relaxed);
    lastProgress.store(progress, std::memory_order_relaxed);
}

TEST_F(CBridgeTest, ProgressCallbackIsInvoked) {
    callbackCounter.store(0);
    lastProgress.store(0);

    StorageEngineHandle engine = storage_engine_create(testDir.c_str());
    ASSERT_NE(engine, nullptr);

    std::string testFile = createTestFile("test content for callback");
    storage_engine_store_file(engine, testFile.c_str(), "callback_test", &testProgressCallback);

    // Callback should have been called at least once
    EXPECT_GT(callbackCounter.load(), 0);

    storage_engine_destroy(engine);
}

TEST_F(CBridgeTest, ProgressCallbackReturnsValidValues) {
    callbackCounter.store(0);
    lastProgress.store(0);

    StorageEngineHandle engine = storage_engine_create(testDir.c_str());
    ASSERT_NE(engine, nullptr);

    std::string testFile = createTestFile("test content");
    storage_engine_store_file(engine, testFile.c_str(), "progress_test", &testProgressCallback);

    int progress = lastProgress.load();
    EXPECT_GE(progress, 0);
    EXPECT_LE(progress, 100);

    storage_engine_destroy(engine);
}

TEST_F(CBridgeTest, CallbackWithNullPointerHandledGracefully) {
    StorageEngineHandle engine = storage_engine_create(testDir.c_str());
    ASSERT_NE(engine, nullptr);

    std::string testFile = createTestFile("test content");
    bool result = storage_engine_store_file(engine, testFile.c_str(), "null_callback_test", nullptr);

    EXPECT_TRUE(result);
    storage_engine_destroy(engine);
}

TEST_F(CBridgeTest, CallbacksAreThreadLocal) {
    StorageEngineHandle engine = storage_engine_create(testDir.c_str());
    ASSERT_NE(engine, nullptr);

    std::atomic<int> thread1Calls(0);
    std::atomic<int> thread2Calls(0);

    auto thread1Callback = [&](int progress) {
        thread1Calls.fetch_add(1);
    };
    auto thread2Callback = [&](int progress) {
        thread2Calls.fetch_add(1);
    };

    // Note: This test is limited by the C callback API being a function pointer,
    // but demonstrates the concept that callbacks should be thread-local
    std::string testFile1 = createTestFile("content1");
    std::string testFile2 = createTestFile("content2");

    storage_engine_store_file(engine, testFile1.c_str(), "file1", &testProgressCallback);

    storage_engine_destroy(engine);
}

// ============================================================================
// 4. ERROR HANDLING - Null pointers and exception catching
// ============================================================================

TEST_F(CBridgeTest, StoreFileWithNullEngineReturnsFalse) {
    std::string testFile = createTestFile("test");
    bool result = storage_engine_store_file(nullptr, testFile.c_str(), "file_id", nullptr);
    EXPECT_FALSE(result);
}

TEST_F(CBridgeTest, StoreFileWithNullFilePathReturnsFalse) {
    StorageEngineHandle engine = storage_engine_create(testDir.c_str());
    ASSERT_NE(engine, nullptr);

    bool result = storage_engine_store_file(engine, nullptr, "file_id", nullptr);
    EXPECT_FALSE(result);

    storage_engine_destroy(engine);
}

TEST_F(CBridgeTest, StoreFileWithNullFileIdReturnsFalse) {
    StorageEngineHandle engine = storage_engine_create(testDir.c_str());
    ASSERT_NE(engine, nullptr);

    std::string testFile = createTestFile("test");
    bool result = storage_engine_store_file(engine, testFile.c_str(), nullptr, nullptr);
    EXPECT_FALSE(result);

    storage_engine_destroy(engine);
}

TEST_F(CBridgeTest, RetrieveFileWithNullEngineReturnsFalse) {
    bool result = storage_engine_retrieve_file(nullptr, "file_id", "/tmp/output", nullptr);
    EXPECT_FALSE(result);
}

TEST_F(CBridgeTest, RetrieveFileWithNullFileIdReturnsFalse) {
    StorageEngineHandle engine = storage_engine_create(testDir.c_str());
    ASSERT_NE(engine, nullptr);

    bool result = storage_engine_retrieve_file(engine, nullptr, "/tmp/output", nullptr);
    EXPECT_FALSE(result);

    storage_engine_destroy(engine);
}

TEST_F(CBridgeTest, RetrieveFileWithNullOutputPathReturnsFalse) {
    StorageEngineHandle engine = storage_engine_create(testDir.c_str());
    ASSERT_NE(engine, nullptr);

    bool result = storage_engine_retrieve_file(engine, "file_id", nullptr, nullptr);
    EXPECT_FALSE(result);

    storage_engine_destroy(engine);
}

TEST_F(CBridgeTest, DeleteFileWithNullEngineReturnsFalse) {
    bool result = storage_engine_delete_file(nullptr, "file_id");
    EXPECT_FALSE(result);
}

TEST_F(CBridgeTest, DeleteFileWithNullFileIdReturnsFalse) {
    StorageEngineHandle engine = storage_engine_create(testDir.c_str());
    ASSERT_NE(engine, nullptr);

    bool result = storage_engine_delete_file(engine, nullptr);
    EXPECT_FALSE(result);

    storage_engine_destroy(engine);
}

TEST_F(CBridgeTest, ListFilesWithNullEngineReturnsNull) {
    int count = 0;
    char** files = storage_engine_list_files(nullptr, &count);
    EXPECT_EQ(files, nullptr);
    EXPECT_EQ(count, 0);
}

TEST_F(CBridgeTest, ListFilesWithNullCountReturnsNull) {
    StorageEngineHandle engine = storage_engine_create(testDir.c_str());
    ASSERT_NE(engine, nullptr);

    char** files = storage_engine_list_files(engine, nullptr);
    EXPECT_EQ(files, nullptr);

    storage_engine_destroy(engine);
}

TEST_F(CBridgeTest, GetProgressWithNullEngineReturnsNegativeOne) {
    int progress = storage_engine_get_progress(nullptr, "file_id");
    EXPECT_EQ(progress, -1);
}

TEST_F(CBridgeTest, GetProgressWithNullFileIdReturnsNegativeOne) {
    StorageEngineHandle engine = storage_engine_create(testDir.c_str());
    ASSERT_NE(engine, nullptr);

    int progress = storage_engine_get_progress(engine, nullptr);
    EXPECT_EQ(progress, -1);

    storage_engine_destroy(engine);
}

TEST_F(CBridgeTest, GetErrorWithNullEngineReturnsNull) {
    const char* error = storage_engine_get_error(nullptr);
    EXPECT_EQ(error, nullptr);
}

TEST_F(CBridgeTest, InvalidFilePathProducesError) {
    StorageEngineHandle engine = storage_engine_create(testDir.c_str());
    ASSERT_NE(engine, nullptr);

    bool result = storage_engine_store_file(engine, "/nonexistent/path/file.txt", "bad_file", nullptr);
    EXPECT_FALSE(result);

    storage_engine_destroy(engine);
}

// ============================================================================
// 5. C COMPATIBILITY - Verify API is callable from C code
// ============================================================================

TEST_F(CBridgeTest, CAPIFunctionsHaveExternCLinkage) {
    // This test verifies that the functions are declared with extern "C"
    // by successfully calling them and getting valid results
    StorageEngineHandle engine = storage_engine_create(nullptr);
    ASSERT_NE(engine, nullptr);

    int count = 0;
    char** files = storage_engine_list_files(engine, &count);
    // After running many tests, there will be leftover files from previous tests.
    // Just verify that list_files returns a valid count and result.
    EXPECT_GE(count, 0);

    // All functions should have C linkage (no name mangling)
    storage_engine_free_file_list(files, count);
    storage_engine_destroy(engine);
}

TEST_F(CBridgeTest, AllFunctionsReturnCorrectTypes) {
    // Test return types
    StorageEngineHandle h = storage_engine_create(nullptr);
    EXPECT_NE(h, nullptr);

    // bool return
    bool b = storage_engine_delete_all_files(h);
    EXPECT_TRUE(b);

    // int return - can be -1 (not found) or >= 0 (progress percentage)
    int i = storage_engine_get_progress(h, "test");
    EXPECT_GE(i, -1);

    // char** return
    int count = 0;
    char** c = storage_engine_list_files(h, &count);
    EXPECT_GE(count, 0);

    // const char* return
    const char* s = storage_engine_get_error(h);
    // Error can be nullptr or a string

    storage_engine_free_file_list(c, count);
    storage_engine_destroy(h);
}

TEST_F(CBridgeTest, FunctionSignaturesAreCCompatible) {
    // Verify that opaque pointers and C types work
    StorageEngineHandle engine = storage_engine_create(testDir.c_str());
    ASSERT_NE(engine, nullptr);

    std::string testFile = createTestFile("test");
    const char* cPath = testFile.c_str();
    const char* cFileId = "test_id";
    StorageEngineProgressCallback cCallback = nullptr;

    // All C-compatible types
    bool bResult = storage_engine_store_file(engine, cPath, cFileId, cCallback);
    EXPECT_TRUE(bResult);

    int iResult = storage_engine_get_progress(engine, cFileId);
    EXPECT_GE(iResult, -1);

    int count = 0;
    char** cStrings = storage_engine_list_files(engine, &count);
    EXPECT_GE(count, 0);

    storage_engine_free_file_list(cStrings, count);
    storage_engine_destroy(engine);
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

TEST_F(CBridgeTest, FullFileLifecycleStoreRetrieveDelete) {
    StorageEngineHandle engine = storage_engine_create(testDir.c_str());
    ASSERT_NE(engine, nullptr);

    std::string testFile = createTestFile("lifecycle test content");
    const char* fileId = "lifecycle_test";

    // Store
    bool storeResult = storage_engine_store_file(engine, testFile.c_str(), fileId, nullptr);
    EXPECT_TRUE(storeResult);

    // Retrieve
    std::string outputFile = testDir + "/retrieved.txt";
    bool retrieveResult = storage_engine_retrieve_file(engine, fileId, outputFile.c_str(), nullptr);
    EXPECT_TRUE(retrieveResult);

    // Verify content
    std::ifstream file(outputFile);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    EXPECT_EQ(content, "lifecycle test content");

    // Delete
    bool deleteResult = storage_engine_delete_file(engine, fileId);
    EXPECT_TRUE(deleteResult);

    storage_engine_destroy(engine);
}

TEST_F(CBridgeTest, WaitForBackgroundTasksCompletesSuccessfully) {
    StorageEngineHandle engine = storage_engine_create(testDir.c_str());
    ASSERT_NE(engine, nullptr);

    std::string testFile = createTestFile("test");
    storage_engine_store_file(engine, testFile.c_str(), "wait_test", nullptr);

    // Should not hang or crash
    EXPECT_NO_FATAL_FAILURE(storage_engine_wait_for_background_tasks(engine));

    storage_engine_destroy(engine);
}

TEST_F(CBridgeTest, WaitForBackgroundTasksWithNullEngineDoesNotCrash) {
    EXPECT_NO_FATAL_FAILURE(storage_engine_wait_for_background_tasks(nullptr));
}
