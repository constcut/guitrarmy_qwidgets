#include "base.h"

#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <map>

#include "tab/tab.h"
#include "tab/tabloader.h"



void checkBase(std::string path, size_t count) {

    const std::filesystem::path basePath{path};

    size_t filesCount = 0;
    size_t fineFiles = 0;
    GTabLoader loader;
    std::unordered_map<uint8_t, size_t> bpmStats;
    std::unordered_map<uint8_t, size_t> noteStats;
    std::unordered_map<uint8_t, size_t> midiNoteStats;
    std::unordered_map<uint8_t, size_t> drumNoteStats;
    std::map<std::pair<uint8_t, uint8_t>, size_t> barSizeStats;
    std::unordered_map<uint8_t, size_t> durStats;
    std::unordered_map<uint8_t, size_t> stringStats;
    std::unordered_map<uint8_t, size_t> fretStats;
    //std::map<

    auto addToMap = [](auto& container, auto value) {
        if (container.count(value))
            container[value] += 1;
        else
            container[value] = 1;
    };

    for(auto const& file: std::filesystem::directory_iterator{basePath}) {
        ++filesCount;
        //if (filesCount < 150000)
            //continue;
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

                for (size_t barI = 0; barI < track->size(); ++barI) {
                    auto& bar = track->at(barI);

                    if (i == 0) //Не повторяемся
                        addToMap(barSizeStats, std::make_pair(bar->getSignNum(), bar->getSignDenum()));

                    for (size_t beatI = 0; beatI < bar->size(); ++beatI) {
                        auto& beat = bar->at(beatI);
                        addToMap(durStats, beat->getDuration());

                        if (beat->getPause())
                            continue; //TODO More stats

                        for (size_t noteI = 0; noteI < beat->size(); ++noteI) {
                            auto& note = beat->at(noteI);
                            auto stringNum = note->getStringNumber();

                            if (track->isDrums() == false) {
                                addToMap(stringStats, stringNum);
                                auto midiNote = note->getMidiNote(tune.getTune(stringNum));
                                addToMap(midiNoteStats, midiNote);
                                addToMap(noteStats, midiNote % 12);
                                addToMap(fretStats, note->getFret());
                                //TODO melodic, harmonic
                            }
                            else {
                                addToMap(drumNoteStats, note->getFret());
                            }
                        }
                    }
                }
                //TODO - scale + tuning
            }
        }

        if (filesCount % 100 == 0)
            std::cout << filesCount << " files done" << std::endl;
    }
        //std::cout << dir_entry << '\n';



    //std::vector<BpmPair> sortedBPM(bpmStats.begin(), bpmStats.end());

    auto saveStats = [](auto& container, std::ofstream& os) {
        using ValuePair = std::pair<uint8_t, size_t>;
        std::vector<ValuePair> sortedData(container.begin(), container.end());
        std::sort(sortedData.begin(), sortedData.end(), [](auto lhs, auto rhs) { return lhs.second > rhs.second; });
        for (auto& p: container) //sortedData
            os << p.first << "," << p.second << std::endl;
    };

    //TODO lamda for most of
    std::ofstream bpmCsv("/home/punnalyse/dev/g/base/bpm.csv");
    std::ofstream notesCsv("/home/punnalyse/dev/g/base/notes.csv");
    std::ofstream midiNotesCsv("/home/punnalyse/dev/g/base/midiNotes.csv");
    std::ofstream drumNotesCsv("/home/punnalyse/dev/g/base/drumNotes.csv");
    std::ofstream barSizeCsv("/home/punnalyse/dev/g/base/barSize.csv");
    std::ofstream durCsv("/home/punnalyse/dev/g/base/durSize.csv");
    std::ofstream stringCsv("/home/punnalyse/dev/g/base/strings.csv");
    std::ofstream fretCsv("/home/punnalyse/dev/g/base/fret.csv");

    std::cout << "Total " << fineFiles << " loaded files" << std::endl;
    std::cout << bpmStats.size() << " bpm records" << std::endl;
    std::cout << noteStats.size() << " note records" << std::endl;
    std::cout << midiNoteStats.size() << " midi note records" << std::endl;
    std::cout << drumNoteStats.size() << " drum note records" << std::endl;
    std::cout << barSizeStats.size() << " bar size records" << std::endl;
    std::cout << durStats.size() << " duration records" << std::endl;
    std::cout << stringStats.size() << " string records" << std::endl;
    std::cout << fretStats.size() << " fret records" << std::endl;

    saveStats(bpmStats, bpmCsv);
    saveStats(noteStats, notesCsv);
    saveStats(midiNoteStats, midiNotesCsv);
    saveStats(drumNoteStats, drumNotesCsv);
    saveStats(durStats, durCsv);
    saveStats(stringStats, stringCsv);
    saveStats(fretStats, fretCsv);

    for (auto& p: barSizeStats)
        barSizeCsv << p.first.first << "," << p.first.second << "," << p.second << std::endl;
}
