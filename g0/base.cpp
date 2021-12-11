#include "base.h"


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
    barSizeStats.clear();
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

//TODO total tracks
//TODO total bars
//TODO total beats
//TODO total notes

void BaseStatistics::makeBeatStats(std::unique_ptr<Beat>& beat, GuitarTuning& tune) {
    auto dur = beat->getDuration();
    if (beat->getPause()) {
        if (dur < durationNames.size())
            addToMap(pauseDurStats, durationNames[dur]);
        else
            addToMap(pauseDurStats, std::to_string(dur));
    }
    else {
        if (dur < durationNames.size())
            addToMap(durStats, durationNames[dur]);
        else
            addToMap(durStats, std::to_string(dur));

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
        addToMap(instrumentStats,track->getInstrument());
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
        if (loader.open(filePath)) {
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
    saveStats(bpmStats, "bpm");
    saveStats(noteStats, "notes");
    saveStats(midiNoteStats, "midiNotes");
    saveStats(drumNoteStats, "drumNotes");
    saveStats(durStats, "durSize");
    saveStats(pauseDurStats, "pauseDurSize");
    saveStats(stringStats, "strings");
    saveStats(fretStats, "frets");
    saveStats(tuneStats, "tunes");
    saveStats(absMelStats, "absMelody");
    saveStats(melStats, "melody");
    saveStats(absHarmStats, "absHarmony");
    saveStats(harmStats, "harmony");
    saveStats(barSizeStats, "barSize");
    saveStats(instrumentStats, "instruments");
    saveStats(notesVolumeStats, "noteVolumes");
}
