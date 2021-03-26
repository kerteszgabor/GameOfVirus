#pragma once
#include <string>

struct Config {

    const size_t size;
    const size_t nPersons;
    const double pInfected;
    const double pInfects;
    const size_t nSamplePlace;
    const size_t nQuarantineDays;
    const double pMove;

    const size_t minHealDay;
    const size_t maxHealDay;
    const double pHeal;

    const size_t minDiseaseDay;
    const size_t maxDiseaseDay;
    const double pDisease;

    Config(std::ifstream& ifs);
};

extern const Config params;

size_t getAnIntegerValue(std::ifstream& ifs, const std::string& key);
double getADoubleValue(std::ifstream& ifs, const std::string& key);
std::stringstream getRawValue(std::ifstream& ifs, const std::string& key);