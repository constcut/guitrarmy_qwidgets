#include "regression.h"

#include <string>
#include <unordered_map>
#include <fstream>
#include <filesystem>

#include "tab/tab.h"
#include "tab/gtpfiles.h"
#include "midi/midiexport.h"

bool checkHasRegression() {

    std::unordered_map<size_t, size_t> groupLength = {
        {1, 12},
        {2, 38},
        {3, 70},
        {4, 109}
    };

    Gp4Import importer;
    uint16_t regressionCount = 0;
    for (size_t groupIdx = 1; groupIdx <= 4; ++groupIdx) {
        size_t from = 1;
        size_t to = groupLength[groupIdx] - 1;

        if (groupIdx == 3)
            break;

        for (size_t fileIndx = from; fileIndx <= to; ++fileIndx) {
            std::string testName = std::to_string(groupIdx) + "." + std::to_string(fileIndx);
            std::string testLocation = "/home/punnalyse/dev/g/_wgtab/gtab/og/"; //AConfig::getInstance().testsLocation;
            std::string midiFile = testLocation + std::string("regression_check/") + testName + std::string(".mid");
            std::string midiFileCheck = testLocation + std::string("regression/") + testName + std::string(".mid");
            std::string gp4File = testLocation + std::string("g4/") + testName + std::string(".gp4");

            std::ifstream itfile;
            itfile.open(gp4File.c_str(),std::ifstream::binary);

            Tab tab;
            importer.import(itfile,&tab); //Перегрузить на просто ссылку и на умный указатель TODO
            tab.postGTP();
            tab.connectTracks(); //new for chains refac

            auto f = exportMidi(&tab);
            std::ofstream midiOut(midiFile, std::ios::binary);
            size_t bytesWritten = f->writeStream(midiOut);
            std::cerr << "MidiSize: " << bytesWritten << " to " << testName <<  std::endl;
            //TODO сделать сравнение со старой версией, убедиться что за последние 10 дней работы не было внесено новой регрессии
            auto fSize = std::filesystem::file_size(midiFileCheck);

            if (fSize != bytesWritten) {
                std::cerr << "Files " << testName << " got a regression " << std::endl;
                ++regressionCount;
                return true;
            }
            else {
                //TODO compare binnary each byte
            }

        }
    }

    if (regressionCount) {
        std::cerr << "Total " << regressionCount << " files got regression" << std::endl;
        return true;
    }
    return false;
}
