#include "storage_engine.h"
#include "metadata_manager.h"
#include "logger.h"

#include <iostream>
#include <limits>

// ======================= MENU =======================
void showMenu() {
    std::cout << "\n=== OwnSphere Storage Engine ===\n";
    std::cout << "1. Store file\n";
    std::cout << "2. Read file\n";
    std::cout << "3. List files\n";
    std::cout << "4. Delete file\n";
    std::cout << "5. Exit\n";
    std::cout << "Choose option: ";
}

// ======================= PROGRESS BAR =======================
void showProgress(int percent) {
    int width = 40;
    int pos = (percent * width) / 100;

    std::cout << "\r[";
    for (int i = 0; i < width; i++) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << percent << "%";
    std::cout.flush();
}

// ======================= MAIN =======================
int main() {
    // ✅ Init logger FIRST
    Logger::init("logs/app.log");
    LOG_INFO("Application started");

    StorageEngine engine;

    // ✅ Cleanup temp metadata from previous crash
    MetadataManager().cleanupTempFiles();
    LOG_INFO("Temporary metadata cleanup completed");

    int choice;

    while (true) {
        showMenu();

        if (!(std::cin >> choice)) {
            LOG_ERROR("Invalid input detected");
            std::cout << "❌ Invalid input\n";

            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        // ================= STORE =================
        if (choice == 1) {
            std::string filePath, fileId;

            std::cout << "Enter file path: ";
            std::getline(std::cin, filePath);

            std::cout << "Enter file ID: ";
            std::getline(std::cin, fileId);

            LOG_INFO("Store request: fileId=" + fileId + ", path=" + filePath);

            if (engine.storeFile(filePath, fileId, showProgress)) {
                std::cout << "\n✅ File stored\n";
                LOG_INFO("File stored successfully: " + fileId);
            } else {
                std::cout << "\n❌ Failed to store file\n";
                LOG_ERROR("Failed to store file: " + fileId);
            }
        }

        // ================= READ =================
        else if (choice == 2) {
            std::string fileId, outputPath;

            std::cout << "Enter file ID: ";
            std::getline(std::cin, fileId);

            std::cout << "Enter output path: ";
            std::getline(std::cin, outputPath);

            LOG_INFO("Retrieve request: fileId=" + fileId + ", output=" + outputPath);

            if (engine.retrieveFile(fileId, outputPath, showProgress)) {
                std::cout << "\n✅ File retrieved\n";
                LOG_INFO("File retrieved successfully: " + fileId);
            } else {
                std::cout << "\n❌ Failed to retrieve file\n";
                LOG_ERROR("Failed to retrieve file: " + fileId);
            }
        }

        // ================= LIST =================
        else if (choice == 3) {
            LOG_INFO("List files request");

            auto files = engine.listFiles();

            if (files.empty()) {
                std::cout << "No files stored.\n";
                LOG_INFO("No files found");
            } else {
                std::cout << "Stored files:\n";
                for (const auto& f : files) {
                    std::cout << "  - " << f << "\n";
                }
                LOG_INFO("Listed files count: " + std::to_string(files.size()));
            }
        }

        // ================= DELETE =================
        else if (choice == 4) {
            std::string fileId;

            std::cout << "Enter file ID: ";
            std::getline(std::cin, fileId);

            LOG_INFO("Delete request: fileId=" + fileId);

            if (engine.deleteFile(fileId)) {
                std::cout << "✅ File deleted\n";
                LOG_INFO("File deleted: " + fileId);
            } else {
                std::cout << "❌ Failed to delete file\n";
                LOG_ERROR("Failed to delete file: " + fileId);
            }
        }

        // ================= EXIT =================
        else if (choice == 5) {
            LOG_INFO("Application shutting down");
            std::cout << "Exiting...\n";
            break;
        }

        // ================= INVALID =================
        else {
            LOG_ERROR("Invalid menu option selected");
            std::cout << "❌ Invalid option\n";
        }
    }

    return 0;
}