#include "base.h"

#include <filesystem>
#include <iostream>
#include <unordered_map>

#include <algorithm>
#include <map>



auto addToMap = [](auto& container, auto value) {
    if (container.count(value))
        container[value] += 1;
    else
        container[value] = 1;
};


void BaseStatistics::reset() {
    bpmStats.clear();
    noteStats.clear();
    midiNoteStats.clear();
    drumNoteStats.clear();
    barSizeStats.clear(); //Так же как tune TODO
    durStats.clear();
    pauseDurStats.clear();
    stringStats.clear();
    fretStats.clear();
    tuneStats.clear();
    melStats.clear();
    absMelStats.clear();
    harmStats.clear();
    absHarmStats.clear();
    instrumentStats.clear();
    notesVolumeStats.clear();
}

//TODO effects on beats
//TODO effects on notes
//TODO scales
//TODO bpm change from-to (as string)

void BaseStatistics::makeBeatStats(std::unique_ptr<Beat>& beat, GuitarTuning& tune) {
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
}


void BaseStatistics::makeNoteStats(std::unique_ptr<Note>& note, size_t beatSize, bool isDrums,
                   GuitarTuning& tune,int& prevNote) {

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
        addToMap(notesVolumeStats, note->getVolume());
    }
    else {
        addToMap(drumNoteStats, note->getFret());
    }
}


void BaseStatistics::addTuneStats(GuitarTuning& tune) {
    std::string tuneString;
    for (size_t tuneI = 0; tuneI < tune.getStringsAmount(); ++tuneI)
        tuneString += noteNames[tune.getTune(tuneI) % 12];
    addToMap(tuneStats, tuneString);
}


void BaseStatistics::makeTabStats(std::unique_ptr<Tab>& tab) {
    addToMap(bpmStats, tab->getBPM());

    for (size_t i = 0; i < tab->size(); ++i) {
        auto& track = tab->at(i);
        auto tune = track->tuning;
        addToMap(instrumentStats,track->getVolume());
        if (track->isDrums() == false)
            addTuneStats(tune);

        int prevNote = -1;
        for (size_t barI = 0; barI < track->size(); ++barI) {
            auto& bar = track->at(barI);
            if (i == 0)
                addToMap(barSizeStats, std::to_string(bar->getSignNum()) + "/" +
                         std::to_string(bar->getSignDenum()));

            for (size_t beatI = 0; beatI < bar->size(); ++beatI) {
                auto& beat = bar->at(beatI);
                makeBeatStats(beat, tune);
                for (size_t noteI = 0; noteI < beat->size(); ++noteI)
                    makeNoteStats(beat->at(noteI), beat->size(), track->isDrums(), tune, prevNote);
            }
        }
    }
}


void BaseStatistics::start(std::string path, size_t count) {

    _path = path;
    reset();
    const std::filesystem::path basePath{path + "base"};
    size_t filesCount = 0;
    size_t fineFiles = 0;

    for(auto const& file: std::filesystem::directory_iterator{basePath}) {
        ++filesCount;
        if (filesCount >= count)
            break;

        std::string filePath = file.path();
        if (loader.open(filePath)) { //TODO в отдельную функцию
            auto tab = std::move(loader.getTab());
            ++fineFiles;
            makeTabStats(tab);
        }
        if (filesCount % 100 == 0)
            std::cout << filesCount << " files done" << std::endl;
    }

    std::cout << "Total files processed: " << fineFiles << std::endl;
    writeAllCSV();
}


void BaseStatistics::writeAllCSV() {


    auto saveStatsUInt = [this](auto& container, std::string name) {
        std::cout << container.size() << " " << name << std::endl;
        std::ofstream os(_path + name + ".csv");
        os << "value,count" << std::endl;
        using ValuePair = std::pair<uint8_t, size_t>; //TODO попробовать сделать универсально
        std::vector<ValuePair> sortedData(container.begin(), container.end());
        std::sort(sortedData.begin(), sortedData.end(), [](auto lhs, auto rhs) { return lhs.second > rhs.second; });
        for (auto& p: sortedData)
            os << (int)p.first << "," << p.second << std::endl;
    };
    //TODO template function for first type of unordered_map
    auto saveStatsStr = [this](auto& container, std::string name) {
        std::cout << container.size() << " " << name << std::endl;
        std::ofstream os(_path + name + ".csv");
        os << "value,count" << std::endl;
        using ValuePair = std::pair<std::string, size_t>;
        std::vector<ValuePair> sortedData(container.begin(), container.end());
        std::sort(sortedData.begin(), sortedData.end(), [](auto lhs, auto rhs) { return lhs.second > rhs.second; });
        for (auto& p: sortedData)
            os << p.first << "," << p.second << std::endl;
    };

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
    saveStatsStr(barSizeStats, "barSize");
    saveStatsUInt(instrumentStats, "instruments");
    saveStatsUInt(notesVolumeStats, "noteVolumes");
}
