#include "base.h"

#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include "tab/tab.h"
#include "tab/tabloader.h"



void checkBase(std::string path, size_t count) {

    const std::filesystem::path basePath{path};

    size_t filesCount = 0;
    GTabLoader loader;
    std::unordered_map<int, size_t> bpmStats;

    for(auto const& file: std::filesystem::directory_iterator{basePath}) {
        ++filesCount;
        if (filesCount >= count)
            break;
        std::string filePath = file.path();
        //std::cout << "Opening " << filePath << std::endl;
        if (loader.open(filePath)) {
            auto tab = std::move(loader.getTab());
            auto bpm = tab->getBPM();
            if (bpmStats.count(bpm))
                bpmStats[bpm] += 1;
            else
                bpmStats[bpm] = 1;
        }
        if (filesCount % 100 == 0)
            std::cout << filesCount << " files done" << std::endl;
    }
        //std::cout << dir_entry << '\n';


    using BpmPair = std::pair<int, size_t>;
    std::vector<BpmPair> sortedBPM(bpmStats.begin(), bpmStats.end());
    std::sort(sortedBPM.begin(), sortedBPM.end(), [](BpmPair lhs, BpmPair rhs) { return lhs.second > rhs.second; });

    size_t elementsCount = 0;
    std::cout << bpmStats.size() << " bpm records" << std::endl;
    std::cout << "BPMS: " << std::endl;
    for (auto& p: sortedBPM) {
        std::cout << "BPM " << p.first << " met " << p.second << " times" << std::endl;
        ++elementsCount;
        if (elementsCount > 20)
            break;
    }


}
