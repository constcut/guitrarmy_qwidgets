#ifndef WAVEFILE_H
#define WAVEFILE_H

#include <QtGlobal>
#include <string>

#include <g0/afile.h>


struct WaveHeader
{
    char RIFF_id[4];
    quint32 size;

    char WAVE_id[4];

    char HEAD_id[4];

    quint16 format;
    quint16 channels;
    quint32 sampleRate;
    quint32 bitRate;
    quint16 blockAllign;
    quint16 bitsPerSample;

};


class WaveFile
{
    WaveHeader header;

    char *rawData;

public:
    WaveFile():rawData(0)
    {
    }

    ~WaveFile()
    {
        if (rawData)
            delete rawData;
    }

    char *getRaw() { return rawData; }

    qint32 getSize() { return header.size; }

    bool read(std::string fileName)
    {
        AFile file;
        bool result =file.open(fileName);

        if (result==false)
            return result;

        result = read(&file);
        file.close();
        return result;
    }

    bool read(AFile *file)
    {
        if (file->opened()==false)
            return false;

        file->read(&header,sizeof(WaveHeader));

        char isData[5] = {0};

        //some magic number that is equal to data
        while (std::string(isData)!="data")
        {
            file->read(isData,4);
        }

        quint32 waveLength = header.size;

        if (rawData)
            delete rawData;

        rawData = new char[waveLength];

        for (quint32 i = 0; i < waveLength; ++i)
            file->read(&rawData[i],1);

        return true;

    }

};

#endif // WAVEFILE_H
