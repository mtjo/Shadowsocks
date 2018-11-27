#pragma once

#include <string>

#include "json/json.h"


class Tools {

public:
    static std::string runCommand(std::string command);

    static int saveData(std::string key, std::string value);

    static std::string getData(std::string key);

    static std::string getParams(const std::string params);

    static std::string getParamsByKey(const std::string params, std::string key);
};
