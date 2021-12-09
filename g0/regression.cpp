#include "regression.h"

#include <string>
#include <unordered_map>
#include <fstream>
#include <filesystem>

#include "tab/tab.h"
#include "tab/gtpfiles.h"
#include "midi/midiexport.h"
#include "tab/gmyfile.h"
#include "g0/config.h"

bool checkHasRegression() {

    std::unordered_map<size_t, size_t> groupLength = {
        {1, 12},
        {2, 39},
        {3, 70},
        {4, 109}
    };

    Gp4Import importer;
    GmyFile exporter;
    uint16_t regressionCountMidi = 0;
    uint32_t regressionCountGmy = 0;
    for (size_t groupIdx = 1; groupIdx <= 4; ++groupIdx) {
        size_t from = 1;
        size_t to = groupLength[groupIdx] - 1;

        if (groupIdx == 3)
            break;

        for (size_t fileIndx = from; fileIndx <= to; ++fileIndx) {
            std::string testName = std::to_string(groupIdx) + "." + std::to_string(fileIndx);
            std::string testLocation = AConfig::getInst().testsLocation;
            std::string midiFile = testLocation + std::string("regression_check/") + testName + std::string(".mid");
            std::string midiFileCheck = testLocation + std::string("regression/") + testName + std::string(".mid");
            std::string gp4File = testLocation + testName + std::string(".gp4");

            std::ifstream itfile;
            itfile.open(gp4File.c_str(),std::ifstream::binary);

            Tab tab;
            importer.import(itfile, &tab); //Перегрузить на просто ссылку и на умный указатель TODO
            tab.postGTP();
            tab.connectTracks(); //new for chains refac

            size_t bytesWritten = 0;
            auto f = exportMidi(&tab);
            {
                std::ofstream midiOut(midiFile, std::ios::binary);
                bytesWritten = f->writeStream(midiOut);
                std::cerr << "MidiSize: " << bytesWritten << " to " << testName <<  std::endl;
                //TODO сделать сравнение со старой версией, убедиться что за последние 10 дней работы не было внесено новой регрессии
            }
            auto fSize = std::filesystem::file_size(midiFileCheck);

            if (fSize != bytesWritten) {
                std::cerr << "Files " << testName << " got a MIDI regression " << std::endl;
                ++regressionCountMidi;
            }
            else {
                //TODO compare binnary each byte
            }

            std::string gmyFile = testLocation + std::string("regression_check/") + testName + std::string(".gmy");
            std::string gmyFileCheck = testLocation + std::string("regression/") + testName + std::string(".gmy");

            {
                std::ofstream gmyOut(gmyFile, std::ios::binary);
                exporter.saveToFile(gmyOut, &tab);
            }


            auto sizeNew = std::filesystem::file_size(gmyFile);
            auto sizeOld = std::filesystem::file_size(gmyFileCheck);

            if (sizeNew != sizeOld) {
                std::cerr << "Files " << testName << " got a GMY regression " << std::endl;
                ++regressionCountGmy;
            }

        }
    }

    if (regressionCountMidi || regressionCountGmy) {
        std::cerr << "Total " << regressionCountMidi << " files got MIDI regression"
                  << std::endl << "Total " << regressionCountGmy << " files got GMY regression" << std::endl;
        return true;
    }
    return false;
}
