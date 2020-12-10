#include "miditopcm.h"


#include "timidity.h"

#include <iostream>

#include <time.h>



int MidiToPcm::rawConvert(std::vector<unsigned char> *midiFile, std::vector<unsigned char> *waveFile)
{
    return 0;
}

int MidiToPcm::convert(std::string midiFile, std::string waveFile)
{
    clock_t mom1 = clock();

    FILE * output = stdout;
    MidIStream *stream = NULL;

    MidSongOptions options;
    MidSong *song;
    sint8 buffer[4096];
    size_t bytes_read;


    output = fopen (waveFile.c_str(), "wb");
    if (output == NULL)
    {
      fprintf (stderr, "Could not open output file %s\n", waveFile.c_str());
      return -1;
    }

    //some attention here
    ///!!! POSSIBLE COULD ESCAPE MID_INIT but if not it mus be here
    //and also mid exit in the end

    stream = mid_istream_open_file (midiFile.c_str());

    if (stream == NULL)
    {
      fprintf (stderr, "Could not open file %s\n", midiFile.c_str());
      mid_exit ();
      return -1;
    }

    //HERE WE DID SKIP WAVE HEADERS
    //=======WAVE HEADER===================================
    int fullsize = 2*2721000;
    fwrite("RIFF",4,1,output);
    fwrite(&fullsize,4,1,output);
    fwrite("WAVE",4,1,output);
    fwrite("fmt ",4,1,output);


    int br = 16;
    fwrite(&br,4,1,output);

    int audioFormat = 1;
    fwrite(&audioFormat,2,1,output);
    int nChan = channels;
    fwrite(&nChan,2,1,output);
    int samR = rate;
    fwrite(&samR,4,1,output);

    int byteRate = channels*(bits/8)*rate; //block align * sample rate
    fwrite(&byteRate,4,1,output);

    int blockAlign = channels*(bits/8); //num channels * bits per samp /8;
    fwrite(&blockAlign,2,1,output);

    int bitPer = bits;
    fwrite(&bitPer,2,1,output);

    fwrite("data",4,1,output);
    fwrite(&fullsize,4,1,output);
    //===============================WAVE HEADER=======================

    options.rate = rate;
    options.format = (bits == 16) ? MID_AUDIO_S16LSB : MID_AUDIO_S8;
    options.channels = channels;
    options.buffer_size = sizeof (buffer) / (bits * channels / 8);

    song = mid_song_load (stream, &options);
    mid_istream_close (stream);

    if (song == NULL)
    {
      fprintf (stderr, "Invalid MIDI file\n");
      mid_exit ();
      return -1;
    }

    mid_song_set_volume (song, volume);
    mid_song_start (song);

    while ((bytes_read = mid_song_read_wave (song, buffer, sizeof (buffer))))
    fwrite (buffer, bytes_read, 1, output);

    mid_song_free (song);


    fclose(output);

    clock_t mom2 = clock();

    return mom2-mom1; //spent time
}

MidiToPcm::MidiToPcm(std::string cfgFile):cfgFile(cfgFile) //could have also some params as rate or else
{
    rate = 8000; //44100
    bits = 16; // 16
    channels = 1; //2
    volume = 100;

    if (mid_init (cfgFile.c_str()) < 0)
    {
      fprintf (stderr, "Could not initialise libTiMidity\n");
      //
      ///failed
    }
}

MidiToPcm::~MidiToPcm()
{
     mid_exit (); //whats logg not very good, but maybe fine - such wrapper doesn't let to create more then one generator - but must check
}


struct WaveFileHeader
{
    unsigned long chunkId;
    unsigned long chunkSize;
    unsigned long format;
    unsigned long subchunkId;
    unsigned long subchunkSize;

    unsigned short audioFormat;
    unsigned short numChannels;

    unsigned long sampleRate;
    unsigned long byteRate;

    unsigned short blockAlign;
    unsigned short bitsPerSample;

    unsigned long subchunk2Id;
    unsigned long subchunk2Size;

    void clear()
    {
        chunkId=chunkSize=format=subchunkId=subchunkSize=0;
        audioFormat=numChannels=blockAlign=bitsPerSample=0;
        sampleRate = byteRate = subchunk2Id = subchunk2Size = 0;
    }

    /*
     * //RIFF
      //fullsize-8
      //WAVE
      //fmt '
      //sub size - 16
      //enc 1
      //channels 2
      //sample rate 8000
      //byte rate
      //block align
      //bits per sample
      WaveFileHeader header;
      header.clear();*/

};


