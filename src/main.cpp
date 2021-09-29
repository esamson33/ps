#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>

#include "string_utils.hpp"

namespace fs = std::filesystem;

using pid_vec = std::vector<pid_t>;
using pid_map = std::multimap<pid_t, pid_t>;
using child_set = std::set<uint32_t>;
using child_map = std::map<uint32_t, child_set>;
using str_map = std::map<std::string, std::string>;

str_map get_proc_pid_status(fs::path const &p)
{
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

void dump_child_map(child_map const &map)
{
  std::cout << "child map:\n";
  for (auto&[k, v]: map) {
    std::cout << k << ": [ ";
    for (auto &i: v) {
      std::cout << i << " ";
    }
    std::cout << "]\n";
  }
}

void dump_index(pid_map const &map)
{
  std::cout << "child map:\n";
  std::for_each(map.cbegin(), map.cend(), [](auto const &item) {
      std::cout << item.first << ": " << item.second << "\n";
  });
}

void dump_pid_vec(pid_vec const &vec)
{
  std::cout << "pid vec: [ ";
  std::for_each(vec.cbegin(), vec.cend(), [](auto const &item) {
      std::cout << item << " ";
  });
  std::cout << "]\n";
}

uint32_t to_number(const char *str)
{
  char *p;
  uint32_t converted = std::strtol(str, &p, 10);
  if (*p) {
    return 0;
  } else {
    return converted;
  }
}

void scan_children_(child_map &map)
{
  fs::path proc("/proc");

  for (auto const &dir_entry: fs::directory_iterator(proc)) {
    if (dir_entry.is_directory()) {
      uint32_t pid;
      if ((pid = to_number(dir_entry.path().filename().c_str())) > 0) {
        str_map proc_pid_status = get_proc_pid_status(proc / std::to_string(pid) / "status");
        uint32_t ppid = to_number(proc_pid_status["PPid"].c_str());
        if (map.contains(ppid)) {
          map[ppid].insert(pid);
        }
      }
    }
  }
}

std::vector<pid_t> get_children(pid_map const &pid_map_, pid_t pid)
{
  std::vector<pid_t> vec;
  auto range = pid_map_.equal_range(pid);
  for (auto i = range.first; i != range.second; ++i) {
    vec.push_back(i->second);
  }
  return vec;
}

void index_proc(pid_map &pid_map_)
{
  fs::path proc("/proc");
  for (auto const &dir_entry: fs::directory_iterator(proc)) {
    if (dir_entry.is_directory()) {
      uint32_t pid;
      if ((pid = to_number(dir_entry.path().filename().c_str())) > 0) {
        str_map proc_pid_status = get_proc_pid_status(proc / std::to_string(pid) / "status");
        uint32_t ppid = to_number(proc_pid_status["PPid"].c_str());
        pid_map_.insert({ppid, pid});
      }
    }
  }
}

pid_vec process(pid_t root, pid_map const &map, int depth)
{
  pid_vec result;
  pid_vec res = get_children(map, root);
  if (res.size() > 0) {
    result.insert(result.end(), res.begin(), res.end());
    if (depth < 4) {
      for (auto const &pid: res) {
        pid_vec res_ = process(pid, map, depth + 1);
        result.insert(result.end(), res_.begin(), res_.end());
      }
    }
  }
  return result;
}

void usage()
{
  std::cout << "Usage: ps <pid>\n";
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    usage();
    std::exit(1);
  }

  pid_map pid_map_;

  auto time_before = std::chrono::high_resolution_clock::now();
  index_proc(pid_map_);
  std::chrono::duration<double> index_elapsed = std::chrono::high_resolution_clock::now() - time_before;

  //dump_index(pid_map_);
  //dump_pid_vec(children);

  pid_t root = static_cast<pid_t>(std::stoul(argv[1], nullptr, 10));
  time_before = std::chrono::high_resolution_clock::now();
  pid_vec result = process(root, pid_map_, 0);
  std::chrono::duration<double> proc_elapsed = std::chrono::high_resolution_clock::now() - time_before;

  dump_pid_vec(result);

  std::cout << "Times: \n"
            << "\tindex time: " << std::chrono::duration_cast<std::chrono::milliseconds>(index_elapsed).count()
            << " ms\n"
            << "\tprocess time: " << std::chrono::duration_cast<std::chrono::microseconds>(proc_elapsed).count()
            << " us\n";
}
