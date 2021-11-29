#include "midirender.h"

#include "libs/sf/tml.h"
#include "libs/sf/tsf.h"

#include <QElapsedTimer>
#include <QDebug>



MidiRender::MidiRender()
{
    renderFrameSize = 1024; //must make simply configurable
    freq = 8000; //or 44100
    totalMsSpent = 0;
}

bool MidiRender::openMidiFile(QString midiFilename)
{
    QByteArray fileNameBytes = midiFilename.toLocal8Bit();
    tml_message* midiMessages = tml_load_filename(fileNameBytes.constData());

    if (midiMessages==0)
        return false;

    midiFile = midiMessages;

    return true;
}

bool MidiRender::openSoundFont(QString sfFilename)
{
     QByteArray fileNameBytes = sfFilename.toLocal8Bit();
      tsf*  sf  = tsf_load_filename(fileNameBytes.constData());

      if (sf == 0)
          return false;

     tsf_set_output(sf, TSF_STEREO_INTERLEAVED, freq, -18.0f);

      soundFont = sf;
      return true;
}

QByteArray MidiRender::renderShort(QString midiFilename)
{
    QElapsedTimer timer;
    timer.start();

    if ((openMidiFile(midiFilename)==false))
        return QByteArray();

    qint64 loadingFiles = timer.restart();

    QByteArray response;
    msRendered = 0.0;

    while (midiFile)
        response += renderShortNext(renderFrameSize);

     tsf_note_off_all((tsf*)soundFont);

    qint64 renderMidi = timer.elapsed();

    qDebug() <<"For "<<msRendered<<" rendered "<<loadingFiles<<" "<<renderMidi;
    totalMsSpent += loadingFiles + renderMidi;

    return response;
}

QByteArray MidiRender::renderFloat(QString midiFilename)
{
    if ((openMidiFile(midiFilename)==false))
        return QByteArray();

    QByteArray response;
    msRendered = 0.0; //rename to the milisecondsRendered
    //nice to have also milisecondsTotal - to know% better

    while (midiFile)
        response += renderFloatNext(renderFrameSize);

     tsf_note_off_all((tsf*)soundFont);

    return response;
}

QByteArray MidiRender::renderShort(QString midiFilename, QString sfFilename)
{
    if ((openSoundFont(sfFilename)==false))
        return QByteArray();

    return renderShort(midiFilename);
}


QByteArray MidiRender::renderFloat(QString midiFilename, QString sfFilename)
{
    if ((openSoundFont(sfFilename)==false))
        return QByteArray();

    return renderFloat(midiFilename);
}


QByteArray MidiRender::renderShortNext(int len)
{
    tml_message* g_MidiMessage = (tml_message*)midiFile;

    QByteArray result; //(const char*)data,len); - old way
    result.resize(len);      //may have some tail in the end

    char *stream = result.data();

    int SampleBlock, SampleCount = (len / (2 * sizeof(short)));
    for (SampleBlock = TSF_RENDER_EFFECTSAMPLEBLOCK; SampleCount; SampleCount -= SampleBlock, stream += (SampleBlock * (2 * sizeof(short))))
    {
        if (SampleBlock > SampleCount) SampleBlock = SampleCount; //this is a moment when would have tail can cut

        for (msRendered += SampleBlock * (1000.0 / freq); g_MidiMessage && msRendered>= g_MidiMessage->time; g_MidiMessage = g_MidiMessage->next)
        {

            switch (g_MidiMessage->type)
            {
                //TODO check other file like awake there are missin volume, panoram anf some other things

                //Возможно проблема в ситуациях, когда инструмент не устанавливается изначально
                //Вести логи - изучить. По крайней мере многие клавиши у нас работают
                case TML_PROGRAM_CHANGE: //channel program (preset) change
                    g_MidiChannelPreset[g_MidiMessage->channel] = tsf_get_presetindex((tsf*)soundFont, 0, g_MidiMessage->program);
                    if (g_MidiChannelPreset[g_MidiMessage->channel] < 0) g_MidiChannelPreset[g_MidiMessage->channel] = 0;
                    break;
                case TML_NOTE_ON: //play a note
                    tsf_note_on((tsf*)soundFont, g_MidiChannelPreset[g_MidiMessage->channel], g_MidiMessage->key, g_MidiMessage->velocity / 127.0f);
                    break;
                case TML_NOTE_OFF: //stop a note
                    tsf_note_off((tsf*)soundFont, g_MidiChannelPreset[g_MidiMessage->channel], g_MidiMessage->key);
                    break;
            }
        }
        tsf_render_short((tsf*)soundFont, (short*)stream, SampleBlock, 0);
    }

   midiFile = g_MidiMessage;

   return result;
}


QByteArray MidiRender::renderFloatNext(int len)
{
    tml_message* g_MidiMessage = (tml_message*)midiFile;

    QByteArray result; //(const char*)data,len); - old way
    result.resize(len);      //may have some tail in the end

    char *stream = result.data();

    int SampleBlock, SampleCount = (len / (2 * sizeof(float)));
    for (SampleBlock = TSF_RENDER_EFFECTSAMPLEBLOCK; SampleCount; SampleCount -= SampleBlock, stream += (SampleBlock * (2 * sizeof(float))))
    {
        if (SampleBlock > SampleCount) SampleBlock = SampleCount; //this is a moment when would have tail can cut

        for (msRendered += SampleBlock * (1000.0 / freq); g_MidiMessage && msRendered >= g_MidiMessage->time; g_MidiMessage = g_MidiMessage->next)
        {
            //qDebug() << "Render message time "<<g_MidiMessage->time;

            switch (g_MidiMessage->type)
            {
                case TML_PROGRAM_CHANGE: //channel program (preset) change
                    g_MidiChannelPreset[g_MidiMessage->channel] = tsf_get_presetindex((tsf*)soundFont, 0, g_MidiMessage->program);
                    if (g_MidiChannelPreset[g_MidiMessage->channel] < 0) g_MidiChannelPreset[g_MidiMessage->channel] = 0;
                    break;
                case TML_NOTE_ON: //play a note
                    tsf_note_on((tsf*)soundFont, g_MidiChannelPreset[g_MidiMessage->channel], g_MidiMessage->key, g_MidiMessage->velocity / 127.0f);
                    break;
                case TML_NOTE_OFF: //stop a note
                    tsf_note_off((tsf*)soundFont, g_MidiChannelPreset[g_MidiMessage->channel], g_MidiMessage->key);
                    break;
            }
        }
        tsf_render_float((tsf*)soundFont, (float*)stream, SampleBlock, 0);
    }

   midiFile = g_MidiMessage;

   return result;
}

//=================================Memory functions - what means midi file class usage=======================

QByteArray MidiRender::renderFromMemoryShort(MidiTrack &track)
{
    QByteArray response;
    msRendered = 0.0;

    midiTrack = &track;
    trackPosition = 0;

    while (trackPosition < midiTrack->size())
        response += renderMemoryShortNext(renderFrameSize);

     tsf_note_off_all((tsf*)soundFont);

    return response;
}

QByteArray MidiRender::renderFromMemoryFloat(MidiTrack &track)
{
    QByteArray response;
    msRendered = 0.0;

    midiTrack = &track;
    trackPosition = 0;

    while (trackPosition < midiTrack->size())
        response += renderMemoryFloatNext(renderFrameSize);

     tsf_note_off_all((tsf*)soundFont);

    return response;
}


//Memory functions means usage of our MidiFile class - not loading file from hard disk
QByteArray MidiRender::renderMemoryFloatNext(int len)
{
    QByteArray result;
    result.resize(len);
    char *stream = result.data();


    int SampleBlock, SampleCount = (len / (2 * sizeof(float)));
    for (SampleBlock = TSF_RENDER_EFFECTSAMPLEBLOCK; SampleCount;
         SampleCount -= SampleBlock, stream += (SampleBlock * (2 * sizeof(float))))
    {
        if (SampleBlock > SampleCount) SampleBlock = SampleCount;

        for (msRendered += SampleBlock * (1000.0 / freq);
             (trackPosition < midiTrack->size()) && (msRendered >= midiTrack->at(trackPosition)->absoluteTime); //REPLACE 0 with time from midi signals
             ++trackPosition)
        {
            auto& signal = midiTrack->at(trackPosition); //later replace such place with ->at(i)

            switch (signal->byte0 & 0xf0)
            {
                case TML_PROGRAM_CHANGE:
                    g_MidiChannelPreset[signal->getChannel()] = tsf_get_presetindex((tsf*)soundFont, 0, signal->param1);
                    if (g_MidiChannelPreset[signal->getChannel()] < 0) g_MidiChannelPreset[signal->getChannel()] = 0;
                    break;
                case TML_NOTE_ON: //play a note
                    tsf_note_on((tsf*)soundFont, g_MidiChannelPreset[signal->getChannel()], signal->param1, signal->param2 / 127.0f);
                    break;
                case TML_NOTE_OFF: //stop a note
                    tsf_note_off((tsf*)soundFont, g_MidiChannelPreset[signal->getChannel()], signal->param1);
                    break;
            }

        }
        tsf_render_float((tsf*)soundFont, (float*)stream, SampleBlock, 0);

        if (trackPosition >= midiTrack->size())
            break;
    }

   return result;
}


QByteArray MidiRender::renderMemoryShortNext(int len)
{
    QByteArray result;
    result.resize(len);
    char *stream = result.data();

    int SampleBlock, SampleCount = (len / (2 * sizeof(short)));
    for (SampleBlock = TSF_RENDER_EFFECTSAMPLEBLOCK; SampleCount;
         SampleCount -= SampleBlock, stream += (SampleBlock * (2 * sizeof(short))))
    {
        if (SampleBlock > SampleCount) SampleBlock = SampleCount;

        for (msRendered += SampleBlock * (1000.0 / freq);
             (trackPosition < midiTrack->size()) && (msRendered >= midiTrack->at(trackPosition)->absoluteTime); //REPLACE 0 with time from midi signals
             ++trackPosition)
        {
            auto& signal = midiTrack->at(trackPosition); //later replace such place with ->at(i)

            qDebug() << "Render "<<signal->byte0<<" "<<signal->absoluteTime;

            switch (signal->byte0 & 0xf0)
            {
                case TML_PROGRAM_CHANGE:
                    g_MidiChannelPreset[signal->getChannel()] = tsf_get_presetindex((tsf*)soundFont, 0, signal->param1);
                    if (g_MidiChannelPreset[signal->getChannel()] < 0) g_MidiChannelPreset[signal->getChannel()] = 0;
                    break;
                case TML_NOTE_ON: //play a note
                    tsf_note_on((tsf*)soundFont, g_MidiChannelPreset[signal->getChannel()], signal->param1, signal->param2 / 127.0f);
                    break;
                case TML_NOTE_OFF: //stop a note
                    tsf_note_off((tsf*)soundFont, g_MidiChannelPreset[signal->getChannel()], signal->param1);
                    break;
            }
        }

        tsf_render_short((tsf*)soundFont, (short*)stream, SampleBlock, 0);

        if (trackPosition >= midiTrack->size())
            break;
    }

   return result;
}
