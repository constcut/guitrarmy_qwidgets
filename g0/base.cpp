#include "base.h"

#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <map>

#include "tab/tab.h"
#include "tab/tabloader.h"



void checkBase(std::string path, size_t count) { //TODO сделать классом, убрать все словари внутрь
                                                 //Инициализировать лямбды заранее, если возможно (или сделать из них функции с четкии типами)

    const std::filesystem::path basePath{path};
    size_t filesCount = 0;
    size_t fineFiles = 0;
    GTabLoader loader;
    std::unordered_map<uint8_t, size_t> bpmStats;
    std::unordered_map<std::string, size_t> noteStats;
    std::unordered_map<uint8_t, size_t> midiNoteStats;
    std::unordered_map<uint8_t, size_t> drumNoteStats;
    std::map<std::pair<uint8_t, uint8_t>, size_t> barSizeStats; //TODO as tune
    std::unordered_map<uint8_t, size_t> durStats;
    std::unordered_map<uint8_t, size_t> pauseDurStats;
    std::unordered_map<uint8_t, size_t> stringStats;
    std::unordered_map<uint8_t, size_t> fretStats;
    std::unordered_map<std::string, size_t> tuneStats;
    std::unordered_map<int, size_t> melStats;
    std::unordered_map<uint8_t, size_t> absMelStats;
    std::unordered_map<int, size_t> harmStats;
    std::unordered_map<uint8_t, size_t> absHarmStats;
    std::vector noteNames = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

    auto addToMap = [](auto& container, auto value) {
        if (container.count(value))
            container[value] += 1;
        else
            container[value] = 1;
    };

    auto makeNoteStats = [&](auto& note, size_t beatSize, bool isDrums, auto& tune, int& prevNote) {
        auto stringNum = note->getStringNumber();
        if (isDrums == false) {
            addToMap(stringStats, stringNum);
            auto midiNote = note->getMidiNote(tune.getTune(stringNum));

            if (beatSize == 1) {
                if (prevNote != -1) {
                    int diff = midiNote - prevNote;
                    addToMap(melStats, diff);
                    addToMap(absMelStats, std::abs(diff));
                }
                prevNote = midiNote;
            }
            else
                prevNote = -1;
            addToMap(midiNoteStats, midiNote);
            addToMap(noteStats, noteNames[midiNote % 12]);
            addToMap(fretStats, note->getFret());
        }
        else {
            addToMap(drumNoteStats, note->getFret());
        }
    };

    auto makeBeatStats = [&](auto& beat, auto& tune) {
        if (beat->getPause())
            addToMap(pauseDurStats, beat->getDuration());
        else {
            addToMap(durStats, beat->getDuration());
            if (beat->size() == 2) {
                auto& note1 = beat->at(0);
                auto stringNum1 = note1->getStringNumber();
                auto& note2 = beat->at(1);
                auto stringNum2 = note2->getStringNumber();
                auto midiNote1 = note1->getMidiNote(tune.getTune(stringNum1));
                auto midiNote2 = note2->getMidiNote(tune.getTune(stringNum2));
                int diff = midiNote2 - midiNote1;
                addToMap(harmStats, diff);
                addToMap(absHarmStats, std::abs(diff));
            }
        }
    };

    auto addTuneStats = [&](auto& tune) {
        std::string tuneString;
        for (size_t tuneI = 0; tuneI < tune.getStringsAmount(); ++tuneI)
            tuneString += noteNames[tune.getTune(tuneI) % 12];
        addToMap(tuneStats, tuneString);
    };

    for(auto const& file: std::filesystem::directory_iterator{basePath}) {
        ++filesCount;
        if (filesCount >= count)
            break;

        std::string filePath = file.path();
        if (loader.open(filePath)) { //TODO в отдельную функцию
            auto tab = std::move(loader.getTab());
            ++fineFiles;
            addToMap(bpmStats, tab->getBPM());

            for (size_t i = 0; i < tab->size(); ++i) {
                auto& track = tab->at(i);
                auto tune = track->tuning;
                addTuneStats(tune);

                int prevNote = -1;
                for (size_t barI = 0; barI < track->size(); ++barI) {
                    auto& bar = track->at(barI);
                    if (i == 0)
                        addToMap(barSizeStats, std::make_pair(bar->getSignNum(), bar->getSignDenum()));

                    for (size_t beatI = 0; beatI < bar->size(); ++beatI) {
                        auto& beat = bar->at(beatI);
                        makeBeatStats(beat, tune);
                        for (size_t noteI = 0; noteI < beat->size(); ++noteI)
                            makeNoteStats(beat->at(noteI), beat->size(), track->isDrums(), tune, prevNote);
                    }
                }
            }
        }
        if (filesCount % 100 == 0)
            std::cout << filesCount << " files done" << std::endl;
    }


    auto saveStatsUInt = [](auto& container, std::string name) {
        std::cout << container.size() << " " << name << std::endl;
        std::ofstream os("/home/punnalyse/dev/g/base/" + name + ".csv");
        os << "value,count" << std::endl;
        using ValuePair = std::pair<uint8_t, size_t>; //TODO попробовать сделать универсально
        std::vector<ValuePair> sortedData(container.begin(), container.end());
        std::sort(sortedData.begin(), sortedData.end(), [](auto lhs, auto rhs) { return lhs.second > rhs.second; });
        for (auto& p: sortedData)
            os << (int)p.first << "," << p.second << std::endl;
    };

    auto saveStatsStr = [](auto& container, std::string name) {
        std::cout << container.size() << " " << name << std::endl;
        std::ofstream os("/home/punnalyse/dev/g/base/" + name + ".csv");
        os << "value,count" << std::endl;
        using ValuePair = std::pair<std::string, size_t>;
        std::vector<ValuePair> sortedData(container.begin(), container.end());
        std::sort(sortedData.begin(), sortedData.end(), [](auto lhs, auto rhs) { return lhs.second > rhs.second; });
        for (auto& p: sortedData)
            os << p.first << "," << p.second << std::endl;
    };

    std::cout << "Total files processed: " << fineFiles << std::endl;

    saveStatsUInt(bpmStats, "bpm");
    saveStatsStr(noteStats, "notes");
    saveStatsUInt(midiNoteStats, "midiNotes");
    saveStatsUInt(drumNoteStats, "drumNotes");
    saveStatsUInt(durStats, "durSize");
    saveStatsUInt(pauseDurStats, "pauseDurSize");
    saveStatsUInt(stringStats, "strings");
    saveStatsUInt(fretStats, "frets");
    saveStatsStr(tuneStats, "tunes");
    saveStatsUInt(absMelStats, "absMelody");
    saveStatsUInt(absHarmStats, "absHarmony");

    std::cout << barSizeStats.size() << " bar size records" << std::endl;
    std::ofstream barSizeCsv("/home/punnalyse/dev/g/base/barSize.csv");
    barSizeCsv << "value,count" << std::endl;
    for (auto& p: barSizeStats)
        barSizeCsv << (int)p.first.first << "/" << (int)p.first.second << "," << p.second << std::endl;
}
