#include "Shadowsocks.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <thread>
#include <regex>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <stdio.h>
#include "json/json.h"
#include "JSON.h"
#include "inifile.h"
#include "DataTransfer.h"
#include "PluginTools.h"
#include "VpnControllor.h"
#include "Tools.h"
//#include "boost/thread.hpp"

using router::DataTransfer;
using router::PluginTools;
using std::string;
using std::thread;


#define BUF_SIZE 256

Shadowsocks::Shadowsocks() {
}

void startShadowsocks() {
    Tools::runCommand("/ss/bin/autorun.sh");
}

void
Shadowsocks::onLaunched(const std::vector <std::string> &parameters) {
    //std::thread subthread(startShadowsocks);
    //subthread.detach();

};


std::string
Shadowsocks::onParameterRecieved(const std::string &params) {
    std::string method = Tools::getParamsByKey(params, "method");
    JSONObject data;
    if (method == "") {
        return JSONObject::error(999, "method can not be null");
    } else if (method == "nodeConnect") {

        //save ss_model
        std::string ss_mode = Tools::getParamsByKey(params, "ss_mode");
        std::string command1 = "echo '" + ss_mode + "'>/ss/config/ss_mode";
        Tools::runCommand(command1);

        //save dns_mode
        std::string dns_mode = "pdnsd";
        std::string command2 = "echo '" + dns_mode + "'>/ss/config/dns_mode";
        Tools::runCommand(command2);


        //save ss config
        std::string SSConfig = Tools::getParamsByKey(params, "SSConfig");
        saveSSConfig(SSConfig);

        //save Dns config
        std::string DnsConfig = Tools::getParamsByKey(params, "DnsConfig");
        saveDnsConfig(DnsConfig);

        //save server ip
        std::string serverIp = Tools::getParamsByKey(DnsConfig, "server");
        std::string command3 = "echo '" + serverIp + "'>/ss/config/server_ip";
        Tools::runCommand(command3);

        //save ss_acl_default_mode
        std::string ss_acl_default_mode = "1";
        std::string command4 = "echo '" + ss_acl_default_mode + "'>/ss/config/ss_acl_default_mode";
        Tools::runCommand(command4);

        //save dns_red_enable
        std::string dns_red_enable = Tools::getData("dns_red_enable");
        std::string command5 = "echo '" + dns_red_enable + "'>/ss/config/dns_red_enable";
        Tools::runCommand(command5);


        //save dns_red_ip
        std::string dns_red_ip = Tools::getData("dns_red_ip");
        dns_red_ip = "lanip";
        std::string command6 = "echo '" + dns_red_ip + "'>/ss/config/dns_red_ip";
        Tools::runCommand(command6);


        return JSONObject::success();
    } else if (method == "getConfig") {
        std::string type = Tools::getParamsByKey(params, "type");
        string config = "";
        if (type == "base") {
            config = Tools::runCommand("cat /etc/Shadowsocks_config.ini");
        } else if (type == "user") {
            config = Tools::runCommand("cat /etc/Shadowsocks_user_config.ini");
        }
        return JSONObject::success(config);
    } else if (method == "getStatus") {
        std::string version = Tools::runCommand("cat /proc/xiaoqiang/model");
        std::string status = Tools::runCommand(
                "ps |grep 'ss-local'|grep -v 'grep'|grep -v '/bin/sh -c'|awk '{print $1}'");
//Tools::runCommand("ps |grep 'ss/bin/ss-local'|grep -v 'grep'|grep -v '/bin/sh -c'|awk '{print $1}'>pid");
        data.put("version", version);
        data.put("status", status);
        std::string local = Tools::runCommand(
                "ps |grep 'ss-local'|grep -v 'grep'|grep -v '/bin/sh -c'|awk '{print $1}'");
        std::string redir = Tools::runCommand(
                "ps |grep 'ss-redir'|grep -v 'grep'|grep -v '/bin/sh -c'|awk '{print $1}'");
        data.put("local", local);
        data.put("redir", redir);


        return JSONObject::success(data);


    } else if (method == "enableSS") {

        Tools::saveData("runStatus", "1");
        Tools::runCommand("echo 'ON'>/ss/config/enableSS");

        return JSONObject::success();
    } else if (method == "disableSS") {
        Tools::saveData("runStatus", "0");
        Tools::runCommand("echo 'OFF'>/ss/config/enableSS");
        return JSONObject::success();
    } else if (method == "restartShadowsocks") {

        return JSONObject::success(data);
    }

    return JSONObject::error(1, "parameter missing");


}


void
Shadowsocks::saveSSConfig(const std::string config) {
    FILE *fp = NULL;
    fp = fopen("/ss/config/shadowsocks.json", "w+");
    fputs(config.data(), fp);
    fclose(fp);

}

void
Shadowsocks::saveDnsConfig(const std::string config) {
    FILE *fp = NULL;
    fp = fopen("/ss/config/dns2socks.conf", "w+");
    fputs(config.data(), fp);
    fclose(fp);
}


void Shadowsocks::runShadowsocks() {
    std::string runStatus = Tools::getData("runStatus");
    if (runStatus == "1") {

    }
}

void Shadowsocks::stopShadowsocks() {
    Tools::runCommand("killall ss/bin/ss-local");
    Tools::runCommand("killall ss/bin/ssr-local");
    Tools::runCommand("killall ss/bin/ss-redir");
    Tools::runCommand("killall ss/bin/ssr-redir");
    Tools::runCommand("killall ss/bin/autorun.sh");

    FILE *fp = NULL;
    fp = fopen("/ss/bin/autorun.sh", "w+");
    fputs("#!/bin/ash\n", fp);
    fputs("echo \"off\"\n", fp);
    fputs("exit\n", fp);

    fclose(fp);


}


Shadowsocks shadowsocks;
