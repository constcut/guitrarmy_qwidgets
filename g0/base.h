#ifndef BASE_H
#define BASE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <fstream>

#include "tab/tabloader.h"
#include "tab/tab.h"


class BaseStatistics {

public:

    void start(std::string path, size_t count);

private:

    std::string _path;

    std::unordered_map<int16_t, size_t> bpmStats;
    std::unordered_map<std::string, size_t> noteStats;
    std::unordered_map<int16_t, size_t> midiNoteStats;
    std::unordered_map<int16_t, size_t> drumNoteStats;
    std::unordered_map<std::string, size_t> barSizeStats;
    std::unordered_map<std::string, size_t> durStats;
    std::unordered_map<std::string, size_t> pauseDurStats;
    std::unordered_map<int16_t, size_t> stringStats;
    std::unordered_map<int16_t, size_t> fretStats;
    std::unordered_map<std::string, size_t> tuneStats;
    std::unordered_map<int, size_t> melStats;
    std::unordered_map<int16_t, size_t> absMelStats;
    std::unordered_map<int, size_t> harmStats;
    std::unordered_map<int16_t, size_t> absHarmStats;
    std::unordered_map<int16_t, size_t> instrumentStats;
    std::unordered_map<int16_t, size_t> notesVolumeStats;
    std::unordered_map<int16_t, size_t> totalTracksStats;
    std::unordered_map<int16_t, size_t> totalBarsStats;
    std::unordered_map<int16_t, size_t> totalBeatsStats;
    std::unordered_map<int16_t, size_t> totalNotesStats;

    const std::vector<std::string> noteNames = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    const std::vector<std::string> durationNames = {"2/1", "1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/64", "1/128"};

    GTabLoader loader;

    void reset();

    void makeTabStats(std::unique_ptr<Tab>& tab);
    void makeBeatStats(std::unique_ptr<Beat>& beat, GuitarTuning& tune);
    void makeNoteStats(std::unique_ptr<Note>& note, size_t beatSize, bool isDrums,
                       GuitarTuning& tune,int& prevNote);

    void addTuneStats(GuitarTuning& tune);

    void writeAllCSV();

    template<typename T>
    void saveStats(std::unordered_map<T, size_t>&container, std::string name) {
        std::cout << container.size() << " " << name << std::endl;
        std::ofstream os(_path + name + ".csv");
        os << "value,count" << std::endl;
        using ValuePair = std::pair<T, size_t>;
        std::vector<ValuePair> sortedData(container.begin(), container.end());
        std::sort(sortedData.begin(), sortedData.end(), [](auto lhs, auto rhs) { return lhs.second > rhs.second; });
        for (auto& p: sortedData)
            os << p.first << "," << p.second << std::endl;
    }
};



#endif // BASE_H
