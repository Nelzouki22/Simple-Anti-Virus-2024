#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <map>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <windows.h>

namespace fs = std::filesystem;

// Struct to hold file information
struct FileInfo {
    fs::file_time_type last_write_time;
    int change_count;
};

// Store file info
std::map<fs::path, FileInfo> file_info_map;

// Function to format time
std::string format_time(const fs::file_time_type& time) {
    // Convert file_time_type to system_clock time_point
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(time - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
    auto time_t = std::chrono::system_clock::to_time_t(sctp);

    std::tm local_tm;
    localtime_s(&local_tm, &time_t); // Using localtime_s for safety
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S"); // Formatting the date and time
    return oss.str();
}

// Function to set console text color
void set_console_color(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// Function to get current time as string
std::string get_current_time() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm;
    localtime_s(&local_tm, &now_time_t); // Using localtime_s for safety
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// Function to check for changes
void monitor_directory(const fs::path& path) {
    while (true) {
        for (auto& entry : fs::directory_iterator(path)) {
            if (fs::is_regular_file(entry.status())) {
                fs::path file_path = entry.path();
                auto last_write_time = fs::last_write_time(file_path);

                // If the file is new or modified
                if (file_info_map.find(file_path) == file_info_map.end()) {
                    // New file
                    file_info_map[file_path] = { last_write_time, 1 };
                    set_console_color(2); // Green for new file
                    std::cout << "[" << get_current_time() << "] "
                        << "File created: " << file_path
                        << " at " << format_time(last_write_time) << std::endl;
                }
                else {
                    // Existing file
                    if (file_info_map[file_path].last_write_time != last_write_time) {
                        file_info_map[file_path].last_write_time = last_write_time;
                        file_info_map[file_path].change_count++;
                        set_console_color(6); // Yellow for modified file
                        std::cout << "[" << get_current_time() << "] "
                            << "File modified: " << file_path
                            << " at " << format_time(last_write_time)
                            << " (Total changes: " << file_info_map[file_path].change_count << ")"
                            << std::endl;
                    }
                }
            }
        }

        // Check for deleted files
        for (auto it = file_info_map.begin(); it != file_info_map.end();) {
            if (!fs::exists(it->first)) {
                set_console_color(4); // Red for deleted file
                std::cout << "[" << get_current_time() << "] "
                    << "File deleted: " << it->first << std::endl;
                it = file_info_map.erase(it); // Remove from map
            }
            else {
                ++it;
            }
        }

        set_console_color(7); // Reset to default color
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Wait for a second before the next check
    }
}

int main() {
    fs::path path_to_monitor = "C:\\Users\\Famliy\\source\\repos\\Replicates Itself VIRUS\\Replicates Itself VIRUS";

    std::cout << "Monitoring changes in: " << path_to_monitor << std::endl;
    monitor_directory(path_to_monitor);

    return 0;
}
