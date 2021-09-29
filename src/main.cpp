#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>

#include "string_utils.hpp"

namespace fs = std::filesystem;

using child_set = std::set<uint32_t>;
using child_map = std::map<uint32_t, child_set>;

using str_map = std::map<std::string, std::string>;

str_map get_proc_pid_status(fs::path const &p) {
  str_map m;
  std::ifstream fh(p);
  if (fh) {
    std::string line;
    while (std::getline(fh, line)) {
      std::string::size_type pos;
      if ((pos = line.find(":")) != std::string::npos) {
        auto key = ps::trim_copy(line.substr(0, pos));
        auto value = ps::trim_copy(line.substr(pos + 1));
        m[key] = value;
      }
    }
  }
  return m;
}

void dump_child_map(child_map const &map) {
  std::cout << "child map:\n";
  for (auto&[k, v]: map) {
    std::cout << k << ": [ ";
    for (auto &i: v) {
      std::cout << i << " ";
    }
    std::cout << "]\n";
  }
}

uint32_t to_number(const char *str) {
  char *p;
  uint32_t converted = std::strtol(str, &p, 10);
  if (*p) {
    return 0;
  } else {
    return converted;
  }
}

void scan_children(child_map &map) {
  fs::path proc("/proc");

  auto update_child_map = [&](uint32_t pid) {
      str_map proc_pid_status = get_proc_pid_status(proc / std::to_string(pid) / "status");
      uint32_t ppid = to_number(proc_pid_status["PPid"].c_str());
      if (map.contains(ppid)) {
        map[ppid].insert(pid);
      }
  };

  for (auto const &dir_entry: fs::directory_iterator(proc)) {
    if (dir_entry.is_directory()) {
      uint32_t pid;
      if ((pid = to_number(dir_entry.path().filename().c_str())) > 0) {
        //update_child_map(pid);
        str_map proc_pid_status = get_proc_pid_status(proc / std::to_string(pid) / "status");
        uint32_t ppid = to_number(proc_pid_status["PPid"].c_str());
        if (map.contains(ppid)) {
          map[ppid].insert(pid);
        }
      }
    }
  }

}

int main(int argc, char *argv[]) {
  child_set cs{};
  child_map m{{static_cast<uint32_t>(std::stoul(argv[1], nullptr, 10)), cs}};
  auto time_before = std::chrono::high_resolution_clock::now();
  scan_children(m);
  std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - time_before;
  dump_child_map(m);
  std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << " ms"
            << std::endl;
}
