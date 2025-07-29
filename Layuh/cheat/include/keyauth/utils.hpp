#pragma once
#include <filesystem> 
#include <string> 
#include <fstream>
#include "oxorany/oxorany_include.h"
#include "nlohmann/json.hpp"
#include "WinBase.h"

using json = nlohmann::json;

std::string ReadFromJson(std::string path, std::string section) 
{
	if (!std::filesystem::exists(path))
		return oxorany("File Not Found");
	std::ifstream file(path);
	json data = json::parse(file);
	return data[section];
}

bool CheckIfJsonKeyExists(std::string path, std::string section) 
{
	if (!std::filesystem::exists(path))
		return oxorany("File Not Found");
	std::ifstream file(path);
	json data = json::parse(file);
	return data.contains(section);
}

bool WriteToJson(std::string path, std::string name, std::string value, bool userpass, std::string name2, std::string value2) 
{
	json file;
	if (!userpass) 
	{
		file[name] = value;
	}
	else
	{
		file[name] = value;
		file[name2] = value2;
	}

	std::ofstream jsonfile(path, std::ios::out);
	jsonfile << file;
	jsonfile.close();
	if (!std::filesystem::exists(path))
		return false;

	return true;
}

void checkAuthenticated(std::string ownerid) {
	while (true) {
		if (GlobalFindAtomA(ownerid.c_str()) == 0) {
			exit(13);
		}
		Sleep(1000); // thread interval
	}
}
