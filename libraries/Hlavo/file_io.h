#include "FS.h"

#ifndef FILEIO_H
#define FILEIO_H

class FileIO
{
    public:
        FileIO(fs::FS &fs, const char * path);
        virtual ~FileIO(void)
            {}

        void append(const char * message);
        void read();
    
    private:
        fs::FS *_fs;
        const char * _path;
};

FileIO::FileIO(fs::FS &fs, const char * path)
{
    _fs = &fs;
    _path = path;
}


void FileIO::append(const char * message){
    Serial.printf("Appending to file: %s\n", _path);

    File file = _fs->open(_path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void FileIO::read(){
    Serial.printf("Reading file: %s\n", _path);

    File file = _fs->open(_path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

#endif //FILEIO_H

