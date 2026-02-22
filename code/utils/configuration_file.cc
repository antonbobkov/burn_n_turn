#include "configuration_file.h"
#include "exception.h"
#include "file_utils.h"

#include <sstream>

namespace {

bool KeyIsValid(const std::string &key) {
  for (char c : key) {
    if (c == ' ' || c == '\n')
      return false;
  }
  return true;
}

bool ValueIsValid(const std::string &value) {
  return value.find('\n') == std::string::npos;
}

void ParseContent(const std::string &content,
                  std::map<std::string, std::string> *entries) {
  std::istringstream iss(content);
  std::string line;
  while (std::getline(iss, line)) {
    if (line.empty())
      continue;
    std::string::size_type first_space = line.find(' ');
    std::string key, value;
    if (first_space == std::string::npos) {
      key = line;
      value = "";
    } else {
      key = line.substr(0, first_space);
      value = line.substr(first_space + 1);
    }
    (*entries)[key] = value;
  }
}

} // namespace

ConfigurationFile::ConfigurationFile(FileManager *fm, std::string path)
    : fm_(fm), path_(std::move(path)) {
  std::string content = GetFileContent(fm_, path_);
  ParseContent(content, &entries_);
}

std::string ConfigurationFile::GetEntry(const std::string &key) const {
  auto it = entries_.find(key);
  if (it == entries_.end())
    return "";
  return it->second;
}

void ConfigurationFile::UpdateEntry(const std::string &key,
                                    const std::string &value) {
  if (!KeyIsValid(key))
    throw SimpleException("ConfigurationFile", "UpdateEntry",
                         "key must not contain space or newline");
  if (!ValueIsValid(value))
    throw SimpleException("ConfigurationFile", "UpdateEntry",
                         "value must not contain newline");
  entries_[key] = value;
  std::ostringstream oss;
  for (const auto &p : entries_)
    oss << p.first << " " << p.second << "\n";
  WriteContentToFile(fm_, path_, oss.str());
}
