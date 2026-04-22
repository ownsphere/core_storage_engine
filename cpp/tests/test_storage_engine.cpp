#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>
#include <algorithm>
#include "storage_engine.h"

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
    return "file_" + std::to_string(rand());
}

void cleanup(const std::string& path) {
    std::remove(path.c_str());
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

// ================= ADVANCED TESTS =================

// 🔥 Missing chunk
TEST(StorageEngineAdvancedTest, MissingChunkFailure) {
    StorageEngine engine;

    std::string input = "corrupt.txt";
    std::string output = "corrupt_out.txt";
    std::string fileId = uniqueId();

    createFile(input, "Important Data");

    ASSERT_TRUE(engine.storeFile(input, fileId));

    std::string chunkPath = "data/chunks/" + fileId + "_chunk_0";
    std::remove(chunkPath.c_str());

    EXPECT_FALSE(engine.retrieveFile(fileId, output));

    cleanup(input);
    cleanup(output);
}

// 🔥 Corruption
TEST(StorageEngineAdvancedTest, ChecksumCorruption) {
    StorageEngine engine;

    std::string input = "checksum.txt";
    std::string output = "checksum_out.txt";
    std::string fileId = uniqueId();

    createFile(input, "Secure Data");

    ASSERT_TRUE(engine.storeFile(input, fileId));

    std::string chunkPath = "data/chunks/" + fileId + "_chunk_0";
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