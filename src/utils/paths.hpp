#pragma once

#include <filesystem>
#include <string>

void set_executable_dir(const char* argv0);

const std::filesystem::path& executable_dir();

// Searches for the relative data file by looking near the executable
// Checks the following:
// 1. exe dir
// 2. parent of exe dir
// current working dir
std::string find_data_file(const std::string& relative);