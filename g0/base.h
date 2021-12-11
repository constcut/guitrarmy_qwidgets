#ifndef BASE_H
#define BASE_H

#include <string>
#include <unordered_map>
#include <vector>
#include "tab/tabloader.h"
#include "tab/tab.h"


class BaseStatistics {

public:

    void checkBase(std::string path, size_t count);

    void reset();

private:
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


};

#endif // BASE_H
