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

    void saveSSConfig(const std::string configData);
    void saveDnsConfig(const std::string configData);
    virtual void onLaunched(const std::vector<std::string> &parameters);
    virtual std::string onParameterRecieved(const std::string &params);
};
