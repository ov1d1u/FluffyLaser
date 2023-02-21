
#ifndef ARDUINO_FILEREADER_H
#define ARDUINO_FILEREADER_H
#include <Arduino.h>
#include <LittleFS.h>

class FileReader  
{
	private:
		File file;
	public:
		FileReader(const char *path);
		bool read(std::unique_ptr<char[]> *buffer, int *length);
		~FileReader();

};
#endif
