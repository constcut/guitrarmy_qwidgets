#ifndef TESTS_H
#define TESTS_H

#include <string>

namespace gtmy {

    class AConfig;

    int getTime();
    bool testScenario();
    void connectConfigs(AConfig& config);
    bool midiPrint(std::string fileName);
    bool greatCheckScenarioCase(uint32_t scen, uint32_t from, uint32_t to, uint32_t v);

}

#endif // TESTS_H
