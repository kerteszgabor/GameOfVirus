#pragma once
#include "config.h"
#include <string>
#include <fstream>
#include <string>
#include <sstream>

std::ifstream input("config.txt");
const Config params(input);

Config::Config(std::ifstream& ifs) : 
	size(getAnIntegerValue(ifs, "size")),
	nPersons(getAnIntegerValue(ifs, "nPersons")),
	nSamplePlace(getAnIntegerValue(ifs, "nSamplePlace")),
	nQuarantineDays(getAnIntegerValue(ifs, "nQuarantineDays")),
	minHealDay(getAnIntegerValue(ifs, "minHealDay")),
	maxHealDay(getAnIntegerValue(ifs, "maxHealDay")),
	minDiseaseDay(getAnIntegerValue(ifs, "minDiseaseDay")),
	maxDiseaseDay(getAnIntegerValue(ifs, "maxDiseaseDay")),

	pInfected(getADoubleValue(ifs, "pInfected")),
	pInfects(getADoubleValue(ifs, "pInfects")),
	pMove(getADoubleValue(ifs, "pMove")),
	pHeal(getADoubleValue(ifs, "pHeal")),
	pDisease(getADoubleValue(ifs, "pDisease"))
{
	ifs.close();
}


size_t getAnIntegerValue(std::ifstream& ifs, const std::string& key)
{
	size_t value;
	getRawValue(ifs, key) >> value;
	return value;
}

double getADoubleValue(std::ifstream& ifs, const std::string& key)
{
	double value;
	getRawValue(ifs, key) >> value;
	return value;
}

std::stringstream getRawValue(std::ifstream& ifs, const std::string& key) 
{
	if (ifs.is_open())
	{
		while (!ifs.eof())
		{
			std::string line;
			std::getline(ifs, line, '\n');
			size_t indexOfDelimiter = line.find('=');
			std::string property = line.substr(0, indexOfDelimiter);
			if (property == key)
			{
				std::stringstream ss;
				ss << line.substr(indexOfDelimiter + 1, line.length());
				return ss;
			}
		}
	}
}