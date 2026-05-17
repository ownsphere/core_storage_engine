#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <thread>
#include <vector>
#include <algorithm>
#include <atomic>
#include <chrono>
#include "storage_engine.h"
#include "chunk_manager.h"
#include "metadata_manager.h"
#include "write_ahead_log.h"
#include "checksum.h"
#include "encryption.h"

// ================= HELPERS =================

std::string readFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());
}

void createFile(const std::string& path, const std::string& content) {
    std::ofstream out(path, std::ios::binary);
    out << content;
    out.close();
}

std::string uniqueId() {
    static std::atomic<unsigned long long> counter{0};
    const auto timestamp = static_cast<unsigned long long>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());
    return "file_" + std::to_string(timestamp) + "_" + std::to_string(counter.fetch_add(1));
}

void cleanup(const std::string& path) {
    std::remove(path.c_str());
}

void cleanupDirectory(const std::string& path) {
    std::filesystem::remove_all(path);
}

std::vector<char> toBytes(const std::string& value) {
    return std::vector<char>(value.begin(), value.end());
}

std::vector<std::string> chunkFilesFor(const std::string& fileId) {
    std::vector<std::string> chunkFiles;
    const std::string prefix = fileId + "_";
    const std::filesystem::path chunkDir("data/chunks");

    if (!std::filesystem::exists(chunkDir)) {
        return chunkFiles;
    }

    for (const auto& entry : std::filesystem::directory_iterator(chunkDir)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const std::string name = entry.path().filename().string();
        if (name.rfind(prefix, 0) == 0) {
            chunkFiles.push_back(name);
        }
    }

    return chunkFiles;
}

std::string makeStorageRoot() {
    return (std::filesystem::temp_directory_path() / uniqueId()).string();
}

// ================= BASIC TESTS =================

TEST(StorageEngineTest, StoreAndRetrieveFile) {
    StorageEngine engine;

    std::string input = "input.txt";
    std::string output = "output.txt";
    std::string fileId = uniqueId();

    createFile(input, "Hello Storage Engine");

    EXPECT_TRUE(engine.storeFile(input, fileId));
    EXPECT_TRUE(engine.retrieveFile(fileId, output));
    EXPECT_EQ(readFile(input), readFile(output));

    cleanup(input);
    cleanup(output);
}

TEST(StorageEngineTest, EmptyFileHandling) {
    StorageEngine engine;

    std::string input = "empty.txt";
    std::string output = "empty_out.txt";
    std::string fileId = uniqueId();

    createFile(input, "");

    EXPECT_TRUE(engine.storeFile(input, fileId));
    EXPECT_TRUE(engine.retrieveFile(fileId, output));
    EXPECT_EQ(readFile(output), "");

    cleanup(input);
    cleanup(output);
}

TEST(StorageEngineTest, DeleteFile) {
    StorageEngine engine;

    std::string input = "delete.txt";
    std::string output = "delete_out.txt";
    std::string fileId = uniqueId();

    createFile(input, "delete");

    EXPECT_TRUE(engine.storeFile(input, fileId));
    EXPECT_TRUE(engine.deleteFile(fileId));
    EXPECT_FALSE(engine.retrieveFile(fileId, output));

    cleanup(input);
    cleanup(output);
}

TEST(StorageEngineTest, DeleteFileReturnsFalseWhenChunkDeletionFails) {
    const std::string storageRoot = makeStorageRoot();
    StorageEngine engine(storageRoot);
    MetadataManager metadataManager(storageRoot);

    const std::string input = "delete_failure.txt";
    const std::string fileId = uniqueId();
    const std::filesystem::path chunkDir = std::filesystem::path(storageRoot) / "chunks";

    createFile(input, "delete failure");

    ASSERT_TRUE(engine.storeFile(input, fileId));
    ASSERT_FALSE(metadataManager.loadChunks(fileId).empty());

    std::filesystem::permissions(
        chunkDir,
        std::filesystem::perms::owner_read | std::filesystem::perms::owner_exec |
            std::filesystem::perms::group_read | std::filesystem::perms::group_exec |
            std::filesystem::perms::others_read | std::filesystem::perms::others_exec,
        std::filesystem::perm_options::replace);

    EXPECT_FALSE(engine.deleteFile(fileId));

    std::filesystem::permissions(
        chunkDir,
        std::filesystem::perms::owner_all | std::filesystem::perms::group_read |
            std::filesystem::perms::group_exec | std::filesystem::perms::others_read |
            std::filesystem::perms::others_exec,
        std::filesystem::perm_options::replace);

    cleanup(input);
    cleanupDirectory(storageRoot);
}

TEST(StorageEngineTest, InvalidInputFile) {
    StorageEngine engine;
    EXPECT_FALSE(engine.storeFile("invalid.txt", uniqueId()));
}

TEST(StorageEngineTest, ListFiles) {
    StorageEngine engine;

    std::string input = "list.txt";
    createFile(input, "data");

    std::string f1 = uniqueId();
    std::string f2 = uniqueId();

    engine.storeFile(input, f1);
    engine.storeFile(input, f2);

    auto files = engine.listFiles();

    EXPECT_NE(std::find(files.begin(), files.end(), f1), files.end());
    EXPECT_NE(std::find(files.begin(), files.end(), f2), files.end());

    cleanup(input);
}

TEST(StorageEngineTest, OverwriteSameFileId) {
    StorageEngine engine;

    std::string input1 = "file1.txt";
    std::string input2 = "file2.txt";
    std::string output = "overwrite.txt";
    std::string fileId = uniqueId();

    createFile(input1, "First");
    createFile(input2, "Second");

    engine.storeFile(input1, fileId);
    engine.storeFile(input2, fileId);

    EXPECT_TRUE(engine.retrieveFile(fileId, output));
    EXPECT_EQ(readFile(output), "Second");

    cleanup(input1);
    cleanup(input2);
    cleanup(output);
}

TEST(StorageEngineTest, OverwriteTracksMetadataVersions) {
    const std::string storageRoot = makeStorageRoot();
    StorageEngine engine(storageRoot);
    MetadataManager metadataManager(storageRoot);

    const std::string input1 = "versioned_file_one.txt";
    const std::string input2 = "versioned_file_two.txt";
    const std::string fileId = uniqueId();

    createFile(input1, "First");
    createFile(input2, "Second");

    ASSERT_TRUE(engine.storeFile(input1, fileId));
    FileMetadata firstMetadata;
    ASSERT_TRUE(metadataManager.loadMetadata(fileId, firstMetadata));
    ASSERT_FALSE(firstMetadata.versionId.empty());
    EXPECT_TRUE(firstMetadata.previousVersionId.empty());

    ASSERT_TRUE(engine.storeFile(input2, fileId));
    FileMetadata latestMetadata;
    ASSERT_TRUE(metadataManager.loadMetadata(fileId, latestMetadata));
    ASSERT_FALSE(latestMetadata.versionId.empty());
    EXPECT_EQ(latestMetadata.previousVersionId, firstMetadata.versionId);

    const auto versions = metadataManager.listMetadataVersions(fileId);
    ASSERT_EQ(versions.size(), 2u);
    EXPECT_EQ(versions[0].fileSize, std::string("Second").size());
    EXPECT_EQ(versions[1].fileSize, std::string("First").size());

    FileMetadata historicMetadata;
    ASSERT_TRUE(metadataManager.loadMetadataVersion(fileId, firstMetadata.versionId, historicMetadata));
    EXPECT_EQ(historicMetadata.fileSize, std::string("First").size());
    ASSERT_EQ(historicMetadata.chunks.size(), 1u);
    EXPECT_EQ(historicMetadata.chunks.front().id, firstMetadata.chunks.front().id);

    cleanup(input1);
    cleanup(input2);
    cleanupDirectory(storageRoot);
}

TEST(StorageEngineTest, ProgressCallbackWorks) {
    StorageEngine engine;

    std::string input = "progress.txt";
    std::string fileId = uniqueId();

    createFile(input, "some large data...");

    int last = 0;
    auto cb = [&](int p) { last = p; };

    engine.storeFile(input, fileId, cb);

    EXPECT_EQ(last, 100);

    cleanup(input);
}

TEST(StorageEngineTest, SupportsCustomStorageRoot) {
    const std::string storageRoot = makeStorageRoot();
    StorageEngine engine(storageRoot);
    MetadataManager metadataManager(storageRoot);

    std::string input = "custom_root_input.txt";
    std::string output = "custom_root_output.txt";
    std::string fileId = uniqueId();

    createFile(input, "stored outside default data path");

    EXPECT_TRUE(engine.storeFile(input, fileId));
    EXPECT_TRUE(engine.retrieveFile(fileId, output));
    EXPECT_EQ(readFile(output), "stored outside default data path");
    EXPECT_TRUE(std::filesystem::exists(metadataManager.metadataPath(fileId)));
    EXPECT_FALSE(std::filesystem::exists("data/metadata/" + fileId + ".meta"));

    cleanup(input);
    cleanup(output);
    cleanupDirectory(storageRoot);
}

TEST(StorageEngineTest, RetrieveUsesCachedMetadataWhenDiskMetadataIsMissing) {
    const std::string storageRoot = makeStorageRoot();
    StorageEngine engine(storageRoot);
    MetadataManager metadataManager(storageRoot);

    const std::string input = "cached_metadata_input.txt";
    const std::string output = "cached_metadata_output.txt";
    const std::string fileId = uniqueId();

    createFile(input, "metadata cache keeps reads fast");

    ASSERT_TRUE(engine.storeFile(input, fileId));
    ASSERT_TRUE(std::filesystem::remove(metadataManager.metadataPath(fileId)));

    EXPECT_TRUE(engine.retrieveFile(fileId, output));
    EXPECT_EQ(readFile(output), "metadata cache keeps reads fast");

    cleanup(input);
    cleanup(output);
    cleanupDirectory(storageRoot);
}

TEST(StorageEngineTest, StartupGarbageCollectionRemovesOrphanedChunks) {
    const std::string storageRoot = makeStorageRoot();
    StorageEngine engine(storageRoot);
    MetadataManager metadataManager(storageRoot);
    ChunkManager chunkManager(storageRoot);

    const std::string input = "gc_orphan_input.txt";
    const std::string output = "gc_orphan_output.txt";
    const std::string fileId = uniqueId();
    const std::string orphanChunkId = fileId + "_orphan_chunk";

    createFile(input, "live data must survive cleanup");

    ASSERT_TRUE(engine.storeFile(input, fileId));
    const auto liveChunks = metadataManager.loadChunks(fileId);
    ASSERT_FALSE(liveChunks.empty());

    ASSERT_TRUE(chunkManager.writeChunk(orphanChunkId, encryptData(toBytes("orphan"), "mysecretkey")));
    ASSERT_TRUE(std::filesystem::exists(std::filesystem::path(chunkManager.chunksDir()) / orphanChunkId));

    StorageEngine recoveredEngine(storageRoot);

    EXPECT_FALSE(std::filesystem::exists(std::filesystem::path(chunkManager.chunksDir()) / orphanChunkId));
    EXPECT_TRUE(std::filesystem::exists(std::filesystem::path(chunkManager.chunksDir()) / liveChunks.front().id));
    EXPECT_TRUE(recoveredEngine.retrieveFile(fileId, output));
    EXPECT_EQ(readFile(output), "live data must survive cleanup");

    cleanup(input);
    cleanup(output);
    cleanupDirectory(storageRoot);
}

// ================= ADVANCED TESTS =================

// 🔥 Missing chunk
TEST(StorageEngineAdvancedTest, MissingChunkFailure) {
    StorageEngine engine;
    MetadataManager metadataManager;

    std::string input = "corrupt.txt";
    std::string output = "corrupt_out.txt";
    std::string fileId = uniqueId();

    createFile(input, "Important Data");

    ASSERT_TRUE(engine.storeFile(input, fileId));

    const auto chunks = metadataManager.loadChunks(fileId);
    ASSERT_FALSE(chunks.empty());

    std::string chunkPath = "data/chunks/" + chunks.front().id;
    std::remove(chunkPath.c_str());

    EXPECT_FALSE(engine.retrieveFile(fileId, output));

    cleanup(input);
    cleanup(output);
}

// 🔥 Corruption
TEST(StorageEngineAdvancedTest, ChecksumCorruption) {
    StorageEngine engine;
    MetadataManager metadataManager;

    std::string input = "checksum.txt";
    std::string output = "checksum_out.txt";
    std::string fileId = uniqueId();

    createFile(input, "Secure Data");

    ASSERT_TRUE(engine.storeFile(input, fileId));

    const auto chunks = metadataManager.loadChunks(fileId);
    ASSERT_FALSE(chunks.empty());

    std::string chunkPath = "data/chunks/" + chunks.front().id;
    std::ofstream corrupt(chunkPath, std::ios::app);
    corrupt << "XXX";
    corrupt.close();

    EXPECT_FALSE(engine.retrieveFile(fileId, output));

    cleanup(input);
    cleanup(output);
}

// 🔥 Large file
TEST(StorageEngineAdvancedTest, LargeFileHandling) {
    StorageEngine engine;

    std::string input = "large.txt";
    std::string output = "large_out.txt";
    std::string fileId = uniqueId();

    std::ofstream out(input);
    for (int i = 0; i < 2 * 1024 * 1024; i++) out << 'A';
    out.close();

    EXPECT_TRUE(engine.storeFile(input, fileId));
    EXPECT_TRUE(engine.retrieveFile(fileId, output));

    cleanup(input);
    cleanup(output);
}

// 🔥 Multiple files
TEST(StorageEngineAdvancedTest, MultipleFiles) {
    StorageEngine engine;

    for (int i = 0; i < 20; i++) {
        std::string f = "file_" + std::to_string(i);
        createFile(f, "data");
        EXPECT_TRUE(engine.storeFile(f, f));
        cleanup(f);
    }

    auto files = engine.listFiles();
    EXPECT_GE(files.size(), 20);
}

// 🔥 Concurrency
TEST(StorageEngineAdvancedTest, ConcurrentWrites) {
    StorageEngine engine;

    auto task = [&](int i) {
        std::string f = "c_" + std::to_string(i);
        createFile(f, "data");
        engine.storeFile(f, f);
        cleanup(f);
    };

    std::vector<std::thread> threads;

    for (int i = 0; i < 10; i++)
        threads.emplace_back(task, i);

    for (auto& t : threads) t.join();

    auto files = engine.listFiles();
    EXPECT_GE(files.size(), 10);
}

TEST(StorageEngineAdvancedTest, ConcurrentWritesToSameFileIdAreSerialized) {
    const std::string storageRoot = makeStorageRoot();
    StorageEngine engine(storageRoot);
    MetadataManager metadataManager(storageRoot);

    const std::string fileId = uniqueId();
    const std::string input1 = "same_id_writer_one.txt";
    const std::string input2 = "same_id_writer_two.txt";
    const std::string output = "same_id_writer_out.txt";
    const std::filesystem::path chunkDir = std::filesystem::path(storageRoot) / "chunks";
    const std::string payload1(6 * 1024 * 1024, 'A');
    const std::string payload2(6 * 1024 * 1024, 'B');

    createFile(input1, payload1);
    createFile(input2, payload2);

    std::atomic<int> ready{0};

    auto store = [&](const std::string& inputPath) {
        ready.fetch_add(1);
        while (ready.load() < 2) {
            std::this_thread::yield();
        }
        EXPECT_TRUE(engine.storeFile(inputPath, fileId));
    };

    std::thread first(store, std::cref(input1));
    std::thread second(store, std::cref(input2));

    first.join();
    second.join();

    ASSERT_TRUE(engine.retrieveFile(fileId, output));

    const std::string finalContent = readFile(output);
    EXPECT_TRUE(finalContent == payload1 || finalContent == payload2);

    const auto chunks = metadataManager.loadChunks(fileId);
    ASSERT_FALSE(chunks.empty());
    size_t chunkFileCount = 0;
    for (const auto& entry : std::filesystem::directory_iterator(chunkDir)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const std::string name = entry.path().filename().string();
        if (name.rfind(fileId + "_", 0) == 0) {
            chunkFileCount++;
        }
    }
    EXPECT_GE(chunkFileCount, chunks.size());
    EXPECT_FALSE(std::filesystem::exists(metadataManager.metadataPath(fileId) + ".tmp"));
    EXPECT_FALSE(std::filesystem::exists(storageRoot + "/wal/" + fileId + ".wal.tmp"));

    cleanup(input1);
    cleanup(input2);
    cleanup(output);
    cleanupDirectory(storageRoot);
}

TEST(StorageEngineRecoveryTest, ChunkWriteFailureRollsBackAttemptedChunks) {
    StorageEngine engine;

    std::string input = "rollback_partial_failure.txt";
    std::string fileId = uniqueId();
    const std::string walPath = "data/wal/" + fileId + ".wal";
    const std::filesystem::path chunkDir("data/chunks");

    std::ofstream out(input, std::ios::binary);
    out << std::string(5 * 1024 * 1024, 'R');
    out.close();

    std::filesystem::create_directories(chunkDir);
    std::filesystem::permissions(
        chunkDir,
        std::filesystem::perms::owner_all | std::filesystem::perms::group_read |
            std::filesystem::perms::group_exec | std::filesystem::perms::others_read |
            std::filesystem::perms::others_exec,
        std::filesystem::perm_options::replace);

    bool injectedFailure = false;

    auto cb = [&](int percent) {
        if (percent >= 100 || injectedFailure) {
            return;
        }

        const auto chunkFiles = chunkFilesFor(fileId);
        ASSERT_EQ(chunkFiles.size(), 1u);

        const std::string currentChunk = chunkFiles.front();
        const std::string suffix = "_chunk_0";
        ASSERT_NE(currentChunk.rfind(suffix), std::string::npos);

        const std::string nextChunkPath = "data/chunks/" +
            currentChunk.substr(0, currentChunk.size() - suffix.size()) + "_chunk_1";
        std::filesystem::create_directories(nextChunkPath);
        injectedFailure = true;
    };

    EXPECT_FALSE(engine.storeFile(input, fileId, cb));

    EXPECT_TRUE(injectedFailure);
    EXPECT_TRUE(chunkFilesFor(fileId).empty());
    EXPECT_FALSE(std::filesystem::exists(walPath));

    cleanup(input);
}

TEST(StorageEngineRecoveryTest, PendingWalRollsBackNewChunks) {
    StorageEngine engine;
    MetadataManager metadataManager;
    ChunkManager chunkManager;
    WriteAheadLog wal;

    std::string input = "pending_before.txt";
    std::string output = "pending_after.txt";
    std::string fileId = uniqueId();

    createFile(input, "stable data");
    ASSERT_TRUE(engine.storeFile(input, fileId));

    std::vector<std::string> oldChunkIds;
    for (const auto& chunk : metadataManager.loadChunks(fileId)) {
        oldChunkIds.push_back(chunk.id);
    }

    const std::string replacement = "new data that should be rolled back";
    ChunkInfo newChunk;
    newChunk.id = fileId + "_pending_chunk";
    newChunk.checksum = computeChecksum(toBytes(replacement));

    ASSERT_TRUE(wal.begin(fileId, replacement.size(), oldChunkIds));
    ASSERT_TRUE(wal.appendChunk(fileId, newChunk));
    ASSERT_TRUE(chunkManager.writeChunk(newChunk.id, encryptData(toBytes(replacement), "mysecretkey")));

    StorageEngine recoveredEngine;

    EXPECT_TRUE(recoveredEngine.retrieveFile(fileId, output));
    EXPECT_EQ(readFile(output), "stable data");
    EXPECT_FALSE(std::filesystem::exists("data/chunks/" + newChunk.id));
    EXPECT_FALSE(std::filesystem::exists("data/wal/" + fileId + ".wal"));

    cleanup(input);
    cleanup(output);
}

TEST(StorageEngineRecoveryTest, ApplyingWalReplaysCommittedMetadata) {
    StorageEngine engine;
    MetadataManager metadataManager;
    ChunkManager chunkManager;
    WriteAheadLog wal;

    std::string input = "applying_before.txt";
    std::string output = "applying_after.txt";
    std::string fileId = uniqueId();

    createFile(input, "old version");
    ASSERT_TRUE(engine.storeFile(input, fileId));

    std::vector<std::string> oldChunkIds;
    for (const auto& chunk : metadataManager.loadChunks(fileId)) {
        oldChunkIds.push_back(chunk.id);
    }

    const std::string replacement = "new version";
    ChunkInfo newChunk;
    newChunk.id = fileId + "_applying_chunk";
    newChunk.checksum = computeChecksum(toBytes(replacement));

    ASSERT_TRUE(wal.begin(fileId, replacement.size(), oldChunkIds));
    ASSERT_TRUE(wal.appendChunk(fileId, newChunk));
    ASSERT_TRUE(chunkManager.writeChunk(newChunk.id, encryptData(toBytes(replacement), "mysecretkey")));
    ASSERT_TRUE(wal.markApplying(fileId));

    StorageEngine recoveredEngine;

    EXPECT_TRUE(recoveredEngine.retrieveFile(fileId, output));
    EXPECT_EQ(readFile(output), replacement);
    EXPECT_FALSE(std::filesystem::exists("data/wal/" + fileId + ".wal"));

    for (const auto& chunkId : oldChunkIds) {
        EXPECT_TRUE(std::filesystem::exists("data/chunks/" + chunkId));
    }

    cleanup(input);
    cleanup(output);
}
