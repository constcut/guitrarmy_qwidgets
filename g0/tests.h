#ifndef TESTS_H
#define TESTS_H

#include <string>

class AConfig;

int getTime(); //TODO find a better place for all those functions
bool testScenario();
void connectConfigs(AConfig& config);
bool midiPrint(std::string fileName);
bool greatCheckScenarioCase(uint32_t scen, uint32_t from, uint32_t to, uint32_t v);


#endif // TESTS_H