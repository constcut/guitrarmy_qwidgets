#ifndef AFILE_H
#define AFILE_H

#include "types.h"
#include <fstream>

#include <QFile>

class AFile
{
    std::ifstream *istream; //or ptr?
    std::ofstream *ostream; 
    char revBufer[16];

    bool wasOpened;

    QFile *qfileRead;
public:
	//refactoring awaits
    AFile(std::ifstream &inputStream);
	AFile(std::ofstream &outputStream);

    AFile(QFile *readFile);

    AFile();
    ~AFile();

    AFile(AFile &link);
    AFile(std::string &path);
    //etc

    std::string readLine();

    void flush();

    bool readBE(int &v);
    bool readBE(long long &v);
    bool readBE(short &v);
    bool readBE(char &v);

    bool readBE(byte &v);

    bool readBE(unsigned int &v);
    bool readBE(unsigned long long &v);
    bool readBE(unsigned short &v);

    bool readLE(int &v);
    //eof
    //opened
    //open

    bool open(std::string fname, bool forRead=true);
    bool close();
    bool opened();
    bool eof();
    size_t fileSize();

	bool read(void *to, size_t n); //little endian
    bool r(void *to, size_t n); //big endian
    
    bool write(void *from, size_t n);
    bool write(char *from, size_t n) { return write((void*)from,n); }
    bool write(const char *from, size_t n) { return write((void*)from,n); }
    bool w(void *from, size_t n);
    
    //<< >> with specificators then on layer of string abstraction

	//specificators for big endian and little one
};

#endif // AFILE_H
