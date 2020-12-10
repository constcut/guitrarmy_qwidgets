#ifndef MIDITOPCM_H
#define MIDITOPCM_H

#include <string>
#include <vector>

class MidiToPcm
{
    public:

    int convert(std::string midiFile, std::string waveFile); //returns -1 on fail, on ok - amount of ms spent

    int rawConvert(std::vector<unsigned char> *midiFile, std::vector<unsigned char> *waveFile); //works same way, as default one convertor, but don't uses files

    MidiToPcm(std::string cfgFile);
    ~MidiToPcm();

    protected:

    int rate;
    int bits;
    int channels;
    int volume;

    std::string cfgFile;

};

#endif // MIDITOPCM_H
