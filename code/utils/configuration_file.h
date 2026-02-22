#ifndef GUI_CONFIGURATION_FILE_HEADER_ALREADY_DEFINED
#define GUI_CONFIGURATION_FILE_HEADER_ALREADY_DEFINED

/*
 * configuration_file.h - Read/write a string-to-string dictionary to a file
 * via FileManager. One line per entry (key value). Keys cannot contain spaces;
 * keys and values cannot contain newline. File is read once on construction,
 * written on each UpdateEntry.
 */

#include <map>
#include <string>

class FileManager;

class ConfigurationFile {
public:
  /* fm must outlive this object. path is the file path used for read/write. */
  ConfigurationFile(FileManager *fm, std::string path);

  /* Return value for key, or "" if key does not exist. */
  std::string GetEntry(const std::string &key) const;

  /* Set key to value and write file. Throws if key contains space/newline or
   * value contains newline. */
  void UpdateEntry(const std::string &key, const std::string &value);

private:
  FileManager *fm_;
  std::string path_;
  std::map<std::string, std::string> entries_;
};

#endif // GUI_CONFIGURATION_FILE_HEADER_ALREADY_DEFINED
