#include "simple_fs.hpp"
#include "simple_fs_types_win.hpp"
#include "text.hpp"

#ifndef UNICODE
#define UNICODE
#endif
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include "Windows.h"
#include "Memoryapi.h"
#include "Shlobj.h"
#include <cstdlib>

#pragma comment(lib, "Shlwapi.lib")

namespace simple_fs {
	file::~file() {
		if(mapping_handle) {
			if(content.data)
				UnmapViewOfFile(content.data);
			CloseHandle(mapping_handle);
		}
		if(file_handle != INVALID_HANDLE_VALUE) {
			CloseHandle(file_handle);
		}
	}

	file::file(file&& other) noexcept : absolute_path(std::move(other.absolute_path)) {
		mapping_handle = other.mapping_handle;
		file_handle = other.file_handle;
		other.mapping_handle = nullptr;
		other.file_handle = INVALID_HANDLE_VALUE;
		content = other.content;
	}
	void file::operator=(file&& other) noexcept {
		mapping_handle = other.mapping_handle;
		file_handle = other.file_handle;
		other.mapping_handle = nullptr;
		other.file_handle = INVALID_HANDLE_VALUE;
		content = other.content;
		absolute_path = std::move(other.absolute_path);
	}

	file::file(native_string const& full_path) {
		file_handle = CreateFileW(full_path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
		if(file_handle != INVALID_HANDLE_VALUE) {
			absolute_path = full_path;
			mapping_handle = CreateFileMappingW(file_handle, nullptr, PAGE_READONLY, 0, 0, nullptr);
			if(mapping_handle) {
				content.data = (char const*)MapViewOfFile(mapping_handle, FILE_MAP_READ, 0, 0, 0);
				if(content.data) {
					_LARGE_INTEGER pvalue;
					GetFileSizeEx(file_handle, &pvalue);
					content.file_size = uint32_t(pvalue.QuadPart);
				}
			}
		}
	}
	file::file(HANDLE file_handle, native_string const& full_path) : file_handle(file_handle) {
		absolute_path = full_path;
		mapping_handle = CreateFileMappingW(file_handle, nullptr, PAGE_READONLY, 0, 0, nullptr);
		if(mapping_handle) {
			content.data = (char const*)MapViewOfFile(mapping_handle, FILE_MAP_READ, 0, 0, 0);
			if(content.data) {
				_LARGE_INTEGER pvalue;
				GetFileSizeEx(file_handle, &pvalue);
				content.file_size = uint32_t(pvalue.QuadPart);
			}
		}
	}

	std::optional<file> open_file(unopened_file const& f) {
		std::optional<file> result(file{ f.absolute_path });
		if(!result->content.data) {
			result = std::optional<file>{};
		}
		return result;
	}

	void reset(file_system& fs) {
		fs.ordered_roots.clear();
	}

	void add_root(file_system& fs, native_string_view root_path) {
		fs.ordered_roots.emplace_back(root_path);
	}

	void add_relative_root(file_system& fs, native_string_view root_path) {
		WCHAR module_name[MAX_PATH] = {};
		int32_t path_used = GetModuleFileNameW(nullptr, module_name, MAX_PATH);
		while(path_used >= 0 && module_name[path_used] != L'\\') {
			module_name[path_used] = 0;
			--path_used;
		}

		fs.ordered_roots.push_back(native_string(module_name) + native_string(root_path));
	}

	directory get_root(file_system const& fs) {
		return directory(&fs, NATIVE(""));
	}

	native_string extract_state(file_system const& fs) {
		native_string result;
		for (auto const& str : fs.ordered_roots) {
			result += NATIVE(';') + str;
		}
		result += NATIVE('?');
		for (auto const& replace_path : fs.replace_paths) {
			result += replace_path.first + NATIVE('=') + replace_path.second + NATIVE(';');
		}
		return result;
	}
	void restore_state(file_system& fs, native_string_view data) {
		// Parse ordered roots
		{
			fs.ordered_roots.clear();
			auto position = data.data();
			auto end = std::find(position, data.data() + data.length(), NATIVE('?'));
			while (position < end) {
				auto next_semicolon = std::find(position, end, NATIVE(';'));
				fs.ordered_roots.emplace_back(position, next_semicolon);
				position = next_semicolon + 1;
			}
		}
		// Replaced paths
		{
			fs.replace_paths.clear();
			auto position = std::find(data.data(), data.data() + data.length(), NATIVE('?'));
			auto end = data.data() + data.length();
			while (position < end) {
				auto next_equal_sign = std::find(position, end, NATIVE('='));
				if(next_equal_sign >= end)
					break;
				native_string replace_path(position, next_equal_sign);
				position = next_equal_sign + 1;
				auto next_semicolon = std::find(position, end, NATIVE(';'));
				native_string new_path(position, next_semicolon);
				position = next_semicolon + 1;
				fs.replace_paths.insert(std::pair{ replace_path, new_path });
			}
		}
	}

	std::vector<unopened_file> list_files(directory const& dir, native_char const* extension) {
		std::vector<unopened_file> accumulated_results;
		if(dir.parent_system) {
			for(size_t i = dir.parent_system->ordered_roots.size(); i-- > 0; ) {
				const auto dir_path = dir.parent_system->ordered_roots[i] + dir.relative_path;
				if(simple_fs::is_ignored_path(*dir.parent_system, dir_path)) {
					continue;
				}
				const auto appended_path = dir_path + NATIVE("\\*") + extension;
				WIN32_FIND_DATAW find_result;
				auto find_handle = FindFirstFileW(appended_path.c_str(), &find_result);
				if(find_handle != INVALID_HANDLE_VALUE) {
					do {
						if(!(find_result.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
							if(auto search_result = std::find_if(accumulated_results.begin(), accumulated_results.end(), [n = find_result.cFileName](const auto& f) {
								return f.file_name.compare(n) == 0;
								}); search_result == accumulated_results.end()) {

								accumulated_results.emplace_back(dir.parent_system->ordered_roots[i] + dir.relative_path + NATIVE("\\") + find_result.cFileName, find_result.cFileName);
							}
						}
					} while(FindNextFileW(find_handle, &find_result) != 0);
					FindClose(find_handle);
				}
			}
		} else {
			const auto appended_path = dir.relative_path + NATIVE("\\*") + extension;
			WIN32_FIND_DATAW find_result;
			auto find_handle = FindFirstFileW(appended_path.c_str(), &find_result);
			if(find_handle != INVALID_HANDLE_VALUE) {
				do {
					if(!(find_result.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
						accumulated_results.emplace_back(dir.relative_path + NATIVE("\\") + find_result.cFileName, find_result.cFileName);
						
					}
				} while(FindNextFileW(find_handle, &find_result) != 0);
				FindClose(find_handle);
			}
		}
		return accumulated_results;
	}
	std::vector<directory> list_subdirectories(directory const& dir) {
		std::vector<directory> accumulated_results;
		if(dir.parent_system) {
			for(size_t i = dir.parent_system->ordered_roots.size(); i-- > 0; ) {
				const auto dir_path = dir.parent_system->ordered_roots[i] + dir.relative_path;
				if(simple_fs::is_ignored_path(*dir.parent_system, dir_path)) {
					continue;
				}
				const auto appended_path = dir_path + NATIVE("\\*");
				WIN32_FIND_DATAW find_result;
				auto find_handle = FindFirstFileW(appended_path.c_str(), &find_result);
				if(find_handle != INVALID_HANDLE_VALUE) {
					do {
						if(find_result.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
							native_string const rel_name = dir.relative_path + NATIVE("\\") + find_result.cFileName;
							if(find_result.cFileName[0] != NATIVE('.')
								&& std::find_if(accumulated_results.begin(), accumulated_results.end(),
									[&rel_name](const auto& s) {
										return s.relative_path.compare(rel_name) == 0;
									}) == accumulated_results.end()) {
								accumulated_results.emplace_back(dir.parent_system, rel_name);
							}
						}
					} while(FindNextFileW(find_handle, &find_result) != 0);
					FindClose(find_handle);
				}
			}
		} else {
			const auto appended_path = dir.relative_path + NATIVE("\\*");
			WIN32_FIND_DATAW find_result;
			auto find_handle = FindFirstFileW(appended_path.c_str(), &find_result);
			if(find_handle != INVALID_HANDLE_VALUE) {
				do {
					if(find_result.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
						native_string const rel_name = dir.relative_path + NATIVE("\\") + find_result.cFileName;
						if(find_result.cFileName[0] != NATIVE('.')) {
							accumulated_results.emplace_back(nullptr, rel_name);
						}
					}
				} while(FindNextFileW(find_handle, &find_result) != 0);
				FindClose(find_handle);
			}
		}
		return accumulated_results;
	}

	directory open_directory(directory const& dir, native_string_view directory_name) {
		return directory(dir.parent_system, dir.relative_path + NATIVE('\\') + native_string(directory_name));
	}

	native_string get_full_name(directory const& dir) {
		return dir.relative_path;
	}

	std::optional<file> open_file(directory const& dir, native_string_view file_name) {
		if(dir.parent_system) {
			for(size_t i = dir.parent_system->ordered_roots.size(); i-- > 0; ) {
				native_string dir_path = dir.parent_system->ordered_roots[i] + dir.relative_path;
				if(simple_fs::is_ignored_path(*dir.parent_system, dir_path)) {
					continue;
				}
				native_string full_path = dir_path + NATIVE('\\') + native_string(file_name);
				HANDLE file_handle = CreateFileW(full_path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
				if(file_handle != INVALID_HANDLE_VALUE) {
					return std::optional<file>(file(file_handle, full_path));
				}
			}
		} else {
			native_string full_path = dir.relative_path + NATIVE('\\') + native_string(file_name);
			HANDLE file_handle = CreateFileW(full_path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
			if(file_handle != INVALID_HANDLE_VALUE) {
				return std::optional<file>(file(file_handle, full_path));
			}
		}
		return std::optional<file>{};
	}

	std::optional<unopened_file> peek_file(directory const& dir, native_string_view file_name) {
		if(dir.parent_system) {
			for(size_t i = dir.parent_system->ordered_roots.size(); i-- > 0; ) {
				native_string dir_path = dir.parent_system->ordered_roots[i] + dir.relative_path;
				if(simple_fs::is_ignored_path(*dir.parent_system, dir_path)) {
					continue;
				}
				native_string full_path = dir_path + NATIVE('\\') + native_string(file_name);
				DWORD dwAttrib = GetFileAttributesW(full_path.c_str());
				if(dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
					return std::optional<unopened_file>(unopened_file(full_path, file_name));
				}
			}
		} else {
			native_string full_path = dir.relative_path + NATIVE('\\') + native_string(file_name);
			DWORD dwAttrib = GetFileAttributesW(full_path.c_str());
			if(dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
				return std::optional<unopened_file>(unopened_file(full_path, file_name));
			}
		}
		return std::optional<unopened_file>{};
	}

	void add_replace_path_rule(file_system& fs, native_string_view replaced_path, native_string_view new_path) {
		fs.replace_paths.insert(std::pair{ replaced_path, new_path });
	}

	std::vector<native_string> list_roots(file_system const& fs) {
		return fs.ordered_roots;
	}

	bool is_ignored_path(file_system const& fs, native_string_view path) {
		for(const auto& replace_path : fs.replace_paths)
			if(replace_path.first == path)
				return true;
		return false;
	}

	native_string get_full_name(unopened_file const& f) {
		return f.absolute_path;
	}

	native_string get_file_name(unopened_file const& f) {
		return f.file_name;
	}

	native_string get_full_name(file const& f) {
		return f.absolute_path;
	}

	void write_file(directory const& dir, native_string_view file_name, char const* file_data, uint32_t file_size) {
		if(dir.parent_system)
			std::abort();
		
		native_string full_path = dir.relative_path + NATIVE('\\') + native_string(file_name);

		HANDLE file_handle = CreateFileW(full_path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
		if(file_handle != INVALID_HANDLE_VALUE) {
			WriteFile(file_handle, file_data, DWORD(file_size), nullptr, nullptr);
			SetEndOfFile(file_handle);
			CloseHandle(file_handle);
		}
	}

	file_contents view_contents(file const& f) {
		return f.content;
	}

	directory get_or_create_settings_directory() {
		wchar_t* local_path_out = nullptr;
		std::wstring base_path;
		if(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local_path_out) == S_OK) {
			base_path = std::wstring(local_path_out) + L"\\Project Alice";
		}
		CoTaskMemFree(local_path_out);
		if(base_path.length() > 0) {
			CreateDirectoryW(base_path.c_str(), nullptr);
		}
		return directory(nullptr, base_path);
	}

	directory get_or_create_save_game_directory() {
		wchar_t* local_path_out = nullptr;
		std::wstring base_path;
		if(SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &local_path_out) == S_OK) {
			base_path = std::wstring(local_path_out) + L"\\Project Alice";
		}
		CoTaskMemFree(local_path_out);
		if(base_path.length() > 0) {
			CreateDirectoryW(base_path.c_str(), nullptr);
			base_path += NATIVE("\\saved games");
			CreateDirectoryW(base_path.c_str(), nullptr);
		}
		return directory(nullptr, base_path);
	}

	directory get_or_create_scenario_directory() {
		wchar_t* local_path_out = nullptr;
		std::wstring base_path;
		if(SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &local_path_out) == S_OK) {
			base_path = std::wstring(local_path_out) + L"\\Project Alice";
		}
		CoTaskMemFree(local_path_out);
		if(base_path.length() > 0) {
			CreateDirectoryW(base_path.c_str(), nullptr);
			base_path += NATIVE("\\scenarios");
			CreateDirectoryW(base_path.c_str(), nullptr);
		}
		return directory(nullptr, base_path);
	}


	native_string win1250_to_native(std::string_view data_in) {
		native_string result;
		for(auto ch : data_in) {
			result += wchar_t(text::win1250toUTF16(ch));
		}
		return result;
	}

	native_string utf8_to_native(std::string_view str) {
		WCHAR* buffer = new WCHAR[str.length() * 2];

		auto chars_written = MultiByteToWideChar(
			CP_UTF8,
			MB_PRECOMPOSED,
			str.data(),
			int32_t(str.length()),
			buffer,
			int32_t(str.length() * 2)
		);

		std::wstring result(buffer, size_t(chars_written));

		delete[] buffer;
		return result;
	}

	std::string native_to_utf8(native_string_view str) {
		char* buffer = new char[str.length() * 4];

		auto chars_written = WideCharToMultiByte(
			CP_UTF8,
			0,
			str.data(),
			int32_t(str.length()),
			buffer,
			int32_t(str.length() * 4),
			nullptr,
			nullptr
		);

		std::string result(buffer, size_t(chars_written));

		delete[] buffer;
		return result;
	}


	std::string remove_double_backslashes(std::string_view data_in) {
		std::string res;
		res.reserve(data_in.size());
		for(uint32_t i = 0; i < data_in.size(); ++i) {
			if(data_in[i] == '\\') {
				res += '\\';
				if(i + 1 < data_in.size() && data_in[i + 1] == '\\')
					++i;
			} else {
				res += data_in[i];
			}
		}
		return res;
	}

	native_string correct_slashes(native_string path) {
		std::wstring res;
		res.reserve(path.size());
		for(size_t i = 0; i < path.size(); i++) {
			res += path[i] == '/' ? '\\' : path[i];
		}
		return res;
	}
}
