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
    totalTracksStats.clear();
    totalBarsStats.clear();
    totalBeatsStats.clear();
    totalNotesStats.clear();
    noteEffectsStats.clear();
    scalesStats.clear();
    trackMostFreqNoteStats.clear();
}



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
        addToMap(trackScale, midiNote % 12);
        addToMap(fretStats, note->getFret());
        addToMap(notesVolumeStats, note->getVolume());

        for (int16_t i = 1; i < 31; ++i)
            if (note->effPack == static_cast<Effect>(i))
                addToMap(noteEffectsStats, effectNames[i]);
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
    addToMap(totalTracksStats, tab->size());

    for (size_t i = 0; i < tab->size(); ++i) {
        auto& track = tab->at(i);
        auto tune = track->tuning;
        addToMap(instrumentStats,track->getInstrument());
        if (track->isDrums() == false)
            addTuneStats(tune);
        if (i == 0)
            addToMap(totalBarsStats, track->size());
        int prevNote = -1;
        for (size_t barI = 0; barI < track->size(); ++barI) {
            auto& bar = track->at(barI);
            if (i == 0)
                addToMap(barSizeStats, std::to_string(bar->getSignNum()) + "/" +
                         std::to_string(bar->getSignDenum()));
            addToMap(totalBeatsStats, bar->size());
            for (size_t beatI = 0; beatI < bar->size(); ++beatI) {
                auto& beat = bar->at(beatI);
                makeBeatStats(beat, tune);
                addToMap(totalNotesStats, beat->size());
                for (size_t noteI = 0; noteI < beat->size(); ++noteI)
                    makeNoteStats(beat->at(noteI), beat->size(), track->isDrums(), tune, prevNote);
            }
        }
        if (track->isDrums() == false)
            addTrackScaleAndClear();
    }
}

std::string BaseStatistics::scaleStructure(int16_t freqNote) {
    auto foundIt = trackScale.find(freqNote);
    std::string scaleString;

    int prevValue = -1;
    for (auto it = foundIt; it != trackScale.end(); ++it) {
        if (prevValue == -1)
            prevValue = it->first;
        else {
            auto diff = it->first - prevValue;
            scaleString += std::to_string(diff);
            prevValue = it->first;
        }
    }

    if (foundIt != trackScale.begin())
        for (auto it = trackScale.begin(); it != foundIt; ++it) {
            auto diff = (it->first + 12) - prevValue;
            scaleString += std::to_string(diff);
            prevValue = it->first + 12;
        }

    return scaleString;
}


std::string nameDiatonicScale(std::string structure) {
    if (structure == "2212221")
        return "Ionian major";
    else if (structure == "2122212")
        return "Dorian minor";
    else if (structure == "1222122")
        return "Frigian minor";
    else if (structure == "2221221")
        return "Lidian major";
    else if (structure == "2212212")
        return "Miksolidian major";
    else if (structure == "2122122")
        return "Eolian minor";
    else if (structure == "1221222")
        return "Lokrian";
    return structure;
}

std::string namePentanotincScale(std::string structure) {
    if (structure == "22323")
        return "Major pentatonic";
    else if (structure == "32232")
        return "Minor pentatonic";
    else if (structure == "23232")
        return "Pentatonic V3";
    else if (structure == "32322")
        return "Pentatonic V4";
    else if (structure == "23223")
        return "Pentatonic V5";
    return structure;
}


void BaseStatistics::addTrackScaleAndClear() {

    if (trackScale.empty())
        return;

    auto mostFrequent = std::max_element(trackScale.begin(), trackScale.end(),
    [](const std::pair<int, int>& lhs, const std::pair<int, int>& rhs) {
        return lhs.second < rhs.second; });



    std::string noteName = noteNames[mostFrequent->first];
    addToMap(trackMostFreqNoteStats, noteName);

    if(trackScale.size() == 7) {
        auto structure = scaleStructure(mostFrequent->first);
        addToMap(scalesStats, nameDiatonicScale(structure) + " " + noteName);
    }
    else if (trackScale.size() == 5) {
        auto structure = scaleStructure(mostFrequent->first);
        addToMap(scalesStats, namePentanotincScale(structure) + noteName);
    }
    else if (trackScale.size() == 12)
        addToMap(scalesStats, "Chromatic " + noteName);
    else if (trackScale.size() < 5)
        addToMap(scalesStats, "Incomplete " + noteName);
    else if (trackScale.size() == 6)
        addToMap(scalesStats, "Pentatonic Modulation " + noteName);
    else
        addToMap(scalesStats, "Diatonic Modulation " + noteName);
    trackScale.clear();
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
    saveStats(totalTracksStats, "totalTracks");
    saveStats(totalBarsStats, "totalBars");
    saveStats(totalBeatsStats, "totalBeats");
    saveStats(totalNotesStats, "totalNotes");
    saveStats(noteEffectsStats, "noteEffects");
    saveStats(scalesStats, "scales");
    saveStats(trackMostFreqNoteStats, "trackMostFreqNote");
}
