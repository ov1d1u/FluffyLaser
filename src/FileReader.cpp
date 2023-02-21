#include "FileReader.h"

FileReader::FileReader(const char *path) {
  LittleFS.begin();
  file = LittleFS.open(path, "r");
}

bool FileReader::read(std::unique_ptr<char[]> *buffer, int *length) {
  if (!file) {
    return false;
  }
  
  size_t size = file.size();
  std::unique_ptr<char[]> buf(new char[size]);
  file.readBytes(buf.get(), size);

  *buffer = std::move(buf);
  *length = static_cast<int>(size);

  return true;
}

FileReader::~FileReader() {
  file.close();
  LittleFS.end();
}