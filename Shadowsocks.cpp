#include "Shadowsocks.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <thread>
#include <regex>

using std::string;
using std::thread;

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
//#include "boost/thread.hpp"

using router::DataTransfer;
using router::PluginTools;


#define BUF_SIZE 256

Shadowsocks::Shadowsocks() {
}

void startShadowsocks() {
    system("/ss/bin/autorun.sh");
}

void
Shadowsocks::onLaunched(const std::vector <std::string> &parameters) {
    std::thread subthread(startShadowsocks);
    subthread.detach();

};


std::string exec(const char *cmd) {
    char buffer[128];
    std::string result = "";
    FILE *pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}


std::string
Shadowsocks::onParameterRecieved(const std::string &params) {
    std::string method = getMethod(params);
    JSONObject data;
    if (method == "") {
        return JSONObject::error(999, "method can not be null");
    } else if (method == "nodeConnect") {

        //save ss_model
        std::string ss_mode = getDataByKey(params, "ss_mode");
        std::string exec1 = "echo '" + ss_mode + "'>/ss/config/ss_mode";
        system(exec1.data());

        //save dns_mode
        std::string dns_mode = "pdnsd";
        std::string exec2 = "echo '" + dns_mode + "'>/ss/config/dns_mode";
        system(exec2.data());


        //save ss config
        std::string SSConfig = getDataByKey(params, "SSConfig");
        saveSSConfig(SSConfig);

        //save Dns config
        std::string DnsConfig = getDataByKey(params, "DnsConfig");
        saveDnsConfig(DnsConfig);

        //save server ip
        std::string serverIp = getDataByKey(DnsConfig, "server");
        std::string exec3 = "echo '" + serverIp + "'>/ss/config/server_ip";
        system(exec3.data());

        //save ss_acl_default_mode
        std::string ss_acl_default_mode ="1";
        std::string exec4 = "echo '" + ss_acl_default_mode + "'>/ss/config/ss_acl_default_mode";
        system(exec4.data());

        //save dns_red_enable
        std::string dns_red_enable;
        router::DataTransfer::getData("dns_red_enable", dns_red_enable);
        std::string exec5 = "echo '" + dns_red_enable + "'>/ss/config/dns_red_enable";
        system(exec5.data());


        //save dns_red_ip
        std::string dns_red_ip;
        router::DataTransfer::getData("dns_red_ip", dns_red_ip);
        dns_red_ip="lanip";
        std::string exec6 = "echo '" + dns_red_ip + "'>/ss/config/dns_red_ip";
        system(exec6.data());



        return JSONObject::success();
    } else if (method == "getConfig") {
        std::string type = getDataByKey(params, "type");
        string config = "";
        if (type == "base") {
            config = exec("cat /etc/Shadowsocks_config.ini");
        } else if (type == "user") {
            config = exec("cat /etc/Shadowsocks_user_config.ini");
        }
        return
                JSONObject::success(config);
    } else if (method == "getStatus") {
        std::string version = exec("cat /proc/xiaoqiang/model");
        std::string status = exec("ps |grep 'ss-local'|grep -v 'grep'|grep -v '/bin/sh -c'|awk '{print $1}'");
//exec("ps |grep 'ss/bin/ss-local'|grep -v 'grep'|grep -v '/bin/sh -c'|awk '{print $1}'>pid");
        data.put("version", version);
        data.put("status", status);
        std::string local = exec("ps |grep 'ss-local'|grep -v 'grep'|grep -v '/bin/sh -c'|awk '{print $1}'");
        std::string redir = exec("ps |grep 'ss-redir'|grep -v 'grep'|grep -v '/bin/sh -c'|awk '{print $1}'");
        data.put("local", local);
        data.put("redir", redir);


        return JSONObject::success(data);


    } else if (method == "runSS") {
        router::DataTransfer::saveData("run_status", "1");
        runShadowsocks();
        return JSONObject::success();
    } else if (method == "stopSS") {
        router::DataTransfer::saveData("run_status", "0");

        stopShadowsocks();

        return

                JSONObject::success();
    } else if (method == "restartShadowsocks") {
        stopShadowsocks();

        runShadowsocks();

        return
                JSONObject::success(data);
    } else if (method == "shell") {
        std::string shell = getData(params);
        std::string shellLog = exec(shell.data());

        return
                JSONObject::success(shellLog);
    }


    return JSONObject::error(1, "parameter missing");


}

std::string
Shadowsocks::getMethod(const std::string &params) {
    const char *ch = params.data();
    struct json_object *jsonObject = NULL;
    jsonObject = json_tokener_parse(ch);
    std::string method = "";
    if ((long) jsonObject > 0) {/**Json格式无错误**/
        jsonObject = json_object_object_get(jsonObject, "method");
        method = json_object_get_string(jsonObject);
    }
    json_object_put(jsonObject);
    return method;
}


std::string
Shadowsocks::getData(const std::string &params) {
    const char *ch = params.data();
    struct json_object *jsonObject = NULL;
    jsonObject = json_tokener_parse(ch);
    std::string data = "";
    if ((long) jsonObject > 0) {/**Json格式无错误**/
        jsonObject = json_object_object_get(jsonObject, "data");
        data = json_object_get_string(jsonObject);
    }
    json_object_put(jsonObject);
    return data;
}

std::string
Shadowsocks::getDataByKey(const std::string &params, std::string key) {
    const char *ch = params.data();
    struct json_object *jsonObject = NULL;
    jsonObject = json_tokener_parse(ch);
    std::string data = "";
    if ((long) jsonObject > 0) {/**Json格式无错误**/
        jsonObject = json_object_object_get(jsonObject, key.data());
        data = json_object_get_string(jsonObject);
    }
    json_object_put(jsonObject);
    return data;
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
    std::string run_status;
    router::DataTransfer::getData("run_status", run_status);
    std::string configType;
    router::DataTransfer::getData("configType", configType);

    FILE *fp = NULL;
    fp = fopen("/ss/bin/autorun.sh", "w+");
    fputs("#!/bin/ash\n", fp);
    fputs("/ss/bin/ss-redir -c /ss/config/shadowsocks.json -f /ss_redir_pid\n", fp);
    fputs("/ss/bin/ss-local -c /ss/config/dns2socks.conf -f /ss_local_pid\n", fp);
    fputs("echo \"on\"\n", fp);
    fclose(fp);

    if (run_status == "1") {
        //system("./ss/bin/autorun.sh");
        std::thread subthread(startShadowsocks);
        subthread.detach();
    }
}

void Shadowsocks::stopShadowsocks() {
    system("killall ss/bin/ss-local");
    system("killall ss/bin/ssr-local");
    system("killall ss/bin/ss-redir");
    system("killall ss/bin/ssr-redir");
    system("killall ss/bin/autorun.sh");

    FILE *fp = NULL;
    fp = fopen("/ss/bin/autorun.sh", "w+");
    fputs("#!/bin/ash\n", fp);
    fputs("echo \"off\"\n", fp);
    fputs("exit\n", fp);

    fclose(fp);


}


Shadowsocks shadowsocks;
