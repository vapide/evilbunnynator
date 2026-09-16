#include "paths.hpp"

#include <windows.h>

#include <filesystem>

namespace fs = std::filesystem;

fs::path executable_directory;

void set_executable_dir(const char* argv0) {
  wchar_t buffer[MAX_PATH];

  DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);

  if (length > 0) {
    fs::path exe_path(buffer);
    executable_directory = exe_path.parent_path();
  } else if (argv0 != nullptr) {
    fs::path exe_path(argv0);

    if (exe_path.has_parent_path()) {
      executable_directory = fs::absolute(exe_path).parent_path();
    } else {
      executable_directory = fs::current_path();
    }
  } else {
    executable_directory = fs::current_path();
  }
}

const fs::path& executable_dir() { return executable_directory; }

std::string find_data_file(const std::string& relative) {
  fs::path file_path(relative);

  // use full path is given
  if (file_path.is_absolute()) {
    return relative;
  }

  // look by executable
  fs::path candidate = executable_directory / file_path;

  if (fs::exists(candidate)) {
    return candidate.string();
  }

  // one folder above the exe
  candidate = executable_directory.parent_path() / file_path;

  if (fs::exists(candidate)) {
    return candidate.string();
  }

  // look into working dir
  candidate = fs::current_path() / file_path;

  if (fs::exists(candidate)) {
    return candidate.string();
  }

  // return original path since not found
  return relative;
}