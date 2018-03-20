#pragma once
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;
typedef fs::path path_type;

namespace fx {
namespace system {

bool get_temp_dir(path_type& path);
bool get_exe_path(path_type& path);

#if defined(_WIN32)
bool get_appdata_dir(path_type& path);
#endif

} // namespace system
} // namespace fx
