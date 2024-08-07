#ifndef FILE_INFO_H
#define FILE_INFO_H

#include "FS.h"
#include "common.h"
using namespace hlavo;

class FileInfo
{
    public:
        FileInfo(fs::FS &fs, const char * path);
        virtual ~FileInfo(void)
            {}

        char* getPath();
        bool exists();
        void write(const char * message);
        void append(const char * message);
        void read();
        void rename(const char * new_path);
        void remove();
        FileInfo copy(const char * new_path);
    
    private:
        fs::FS *_fs;
        char _path[max_filepath_length];
};

FileInfo::FileInfo(fs::FS &fs, const char * path)
{
    _fs = &fs;
    snprintf(_path, sizeof(_path),"%s", path);
}

char* FileInfo::getPath()
{
  return _path;
}

bool FileInfo::exists(){
  return _fs->exists(_path);
}

void FileInfo::write(const char * message){
    // Serial.printf("Writing file: %s\n", _path);

    File file = _fs->open(_path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        // Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void FileInfo::append(const char * message){
    //Serial.printf("Appending to file: %s\n", _path);

    File file = _fs->open(_path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        //Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void FileInfo::read(){
    // Serial.printf("Reading file: %s\n", _path);

    File file = _fs->open(_path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    // Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void FileInfo::rename(const char * new_path){
    Serial.printf("Renaming file %s to %s\n", _path, new_path);
    if (_fs->rename(_path, new_path)) {
        snprintf(_path, sizeof(_path),"%s", new_path);
        // Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void FileInfo::remove(){
    Serial.printf("Deleting file: %s\n", _path);
    if(_fs->remove(_path)){
        // Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

FileInfo FileInfo::copy(const char * new_path)
{
  Serial.printf("Copying file: %s to %s\n", _path, new_path);

  // Open source file
  File sourceFile = _fs->open(_path, "r");
  if (!sourceFile) {
    Serial.println("Failed to open source file");
    return FileInfo(*_fs, "");
  }

  // Open destination file
  File destinationFile = _fs->open(new_path, "w");
  if (!destinationFile) {
    Serial.println("Failed to open destination file");
    sourceFile.close(); // Close the source file before returning
    return FileInfo(*_fs, "");
  }

  // Copy contents from source file to destination file
  while (sourceFile.available()) {
    destinationFile.write(sourceFile.read());
  }

  // Close files
  sourceFile.close();
  destinationFile.close();
  // Serial.println("Copying finished.");
  return FileInfo(*_fs, new_path);
}



#endif //FILE_INFO_H

