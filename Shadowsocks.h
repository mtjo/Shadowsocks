#pragma once

#include <string>

#include "MRApp.h"

class Shadowsocks : public MRApp {
public:
    Shadowsocks();
    virtual ~Shadowsocks(){};

    void config();
    void runShadowsocks();
    void stopShadowsocks();

    void saveConfig(const std::string configData);
    std::string getMethod(const std::string &params);
    std::string getData(const std::string &params);
    std::string getDataByKey(const std::string &params,std::string key);
    virtual void onLaunched(const std::vector<std::string> &parameters);
    virtual std::string onParameterRecieved(const std::string &params);
};
