#ifndef BASE_H
#define BASE_H

#include <string>
#include <unordered_map>
#include <vector>
#include "tab/tabloader.h"
#include "tab/tab.h"


class BaseStatistics {

public:

    void start(std::string path, size_t count);

    void reset();

private:

    std::string _path;

    std::unordered_map<uint8_t, size_t> bpmStats;
    std::unordered_map<std::string, size_t> noteStats;
    std::unordered_map<uint8_t, size_t> midiNoteStats;
    std::unordered_map<uint8_t, size_t> drumNoteStats;
    std::unordered_map<std::string, size_t> barSizeStats; //Так же как tune TODO
    std::unordered_map<uint8_t, size_t> durStats;
    std::unordered_map<uint8_t, size_t> pauseDurStats;
    std::unordered_map<uint8_t, size_t> stringStats;
    std::unordered_map<uint8_t, size_t> fretStats;
    std::unordered_map<std::string, size_t> tuneStats;
    std::unordered_map<int, size_t> melStats;
    std::unordered_map<uint8_t, size_t> absMelStats;
    std::unordered_map<int, size_t> harmStats;
    std::unordered_map<uint8_t, size_t> absHarmStats;
    std::unordered_map<uint8_t, size_t> instrumentStats;
    std::unordered_map<uint8_t, size_t> notesVolumeStats;
    const std::vector<std::string> noteNames = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    GTabLoader loader;

    void writeAllCSV();

    void makeBeatStats(std::unique_ptr<Beat>& beat, GuitarTuning& tune);
    void makeNoteStats(std::unique_ptr<Note>& note, size_t beatSize, bool isDrums,
                       GuitarTuning& tune,int& prevNote);

    void addTuneStats(GuitarTuning& tune);
    void makeTabStats(std::unique_ptr<Tab>& tab);
};

#endif // BASE_H
