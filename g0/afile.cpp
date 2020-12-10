#include "afile.h"
#include <memory.h>


AFile::AFile(std::ifstream &inputStream):istream(&inputStream), ostream(0),wasOpened(false)
  ,qfileRead(0)
{

}

AFile::AFile(std::ofstream &outputStream):istream(0),ostream(&outputStream),wasOpened(false)
  ,qfileRead(0)
{
	
}

AFile::AFile() :istream(0),ostream(0), wasOpened(false)
  ,qfileRead(0)
{

}

AFile::AFile(QFile *readFile)
    :istream(0),ostream(0), wasOpened(true),qfileRead(readFile)
{

}

AFile::~AFile()
{
   //close();
}

void AFile::flush()
{
    if (wasOpened)
    {
        if (ostream)
            ostream->flush();
    }
}

bool AFile::close()
{
    if (wasOpened)
    {
        if (istream)
        {
            if (istream->is_open())
                istream->close();

            delete istream;
            istream = 0;
        }
        if (ostream)
        {
            if (ostream->is_open())
                ostream->close();

            delete ostream;
            ostream = 0;
        }
        wasOpened =false;
        return true;
    }
    return false;
}

AFile::AFile(AFile &link):istream(link.istream)
{

}

/*
AFile::AFile(std::string &path)
{
    //new //flag it!

}
*/

void reverseEndian(void *p,int s)
{
    char *bytes = (char*)p;



    if (s == 4)
    {
    char b = bytes[0];
    bytes[0] = bytes[3];
    bytes[3] = b;
    b = bytes[1];
    bytes[1] = bytes[2];
    bytes[2] = b;
    }
    if (s == 2)
    {
        char b = bytes[0];
        bytes[0] = bytes[1];
        bytes[1] = b;
    }
    /* - firstuse only for 2 bytes;
    for (int i = 0; i < s; ++i)
    {
        char cur = bytes[i];
    }
    */
}


bool AFile::r(void *to, size_t n)
{
	if (istream == 0) return false;
		//OK ATTENTION	HERE
	//Now r function used for midi access and provides 
	//Big Endian
	read(to,n);
	reverseEndian(to,n);
     return true;
}

bool AFile::read(void *to, size_t n)
{
    if (istream == 0)
    {
        if (qfileRead)
        {
            char *place = (char*)to;
            qfileRead->read(place,n);
            return true;
        }
        else
            return false;
    }
	char *place = (char*)to;
	istream->read(place,n);
	return true;
}

std::string AFile::readLine()
{
    char buf[1024];


    for (int i = 0; i < 1024; ++i)
    {
        istream->read(&buf[i],1);


        if (buf[i]=='\n')
        {
            buf[i]=0;
            std::string resp=std::string(buf);
            return resp;
        }
    }

}


bool AFile::write(void *from, size_t n)
{
	if (ostream == 0)
		return false;

	char *place = (char*)from;
	ostream->write(place,n);

	return true;
}

bool AFile::w(void *from, size_t n)
{
	if (ostream == 0)
		return false;
	
	//Now w function used for midi access and provides 
	//Big Endian
	
	char *place = (char*)from;
	
	for (size_t i = 0; i < n; ++i)
		revBufer[i] = place[i];
	
	reverseEndian(revBufer,n);
	write(revBufer,n);
	
    return true;
}



bool AFile::readBE(long long &v)
{//
   // deb("llong")

    char buf4[5];
    buf4[4]=0;

    //istream->read(buf4,4);
    r(buf4,4);
    long long value = 0;
    memcpy(&value,buf4,4);
    v = value;

    //deb(buf4);
    return true;
}
bool AFile::readBE(short &v)
{
    //deb("short")
    short value;
    r(&value,2);
    v = value;
    return true;

}

/*
 * //etc
bool AFile::readBE(int &v)
{
    //deb("int")
    return true;
}

bool AFile::readBE(char &v)
{
    //deb("char")
    return true;
}
bool AFile::readBE(byte &v)
{
    //deb("byte")
    return true;
}
bool AFile::readBE(unsigned int &v)
{
    //deb("uint")
    return true;
}
bool AFile::readBE(unsigned long long &v)
{
   // deb("ullong")
    return true;
}
bool AFile::readBE(unsigned short &v)
{
  //  deb("ushort")
   return true;
}
bool AFile::readLE(int &v)
{
   // deb("LE v")
   return true;
}
//eof
//opened
//open

*/

bool AFile::open(std::string fname, bool forRead)
{
    if (forRead)
    {
        istream = new std::ifstream;
        istream->open(fname.c_str(),std::ifstream::binary);
        wasOpened = true;
        return istream->is_open();
    }
    else
    {
        ostream = new std::ofstream;
        ostream->open(fname.c_str(),std::ofstream::binary);
        wasOpened = true;
        return ostream->is_open();
    }

    return true;
}

bool AFile::opened()
{
    return true;
}

bool AFile::eof()
{
    //int eofLine = istream->
    //int someOther =istream->tellg();
    //std::cout <<eofLine<<" : "<<someOther<<std::endl;
    return true;
}

size_t AFile::fileSize()
{
    return 0;
}
