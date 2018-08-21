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
    system("./ss/bin/autorun.sh");
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
    } else if (method == "saveConfig") {
        std::string type = getDataByKey(params, "type");
        router::DataTransfer::saveData("configType", type);
        if (type == "user") {
            std::string jsondata = getData(params);

            FILE *fp = NULL;
            fp = fopen("/etc/Shadowsocks_user_config.ini", "w+");
            fputs(jsondata.data(), fp);
            fclose(fp);
            return JSONObject::success();

        } else if (type == "base") {

            std::string configData = getData(params);
            saveConfig(configData);
            return JSONObject::success();
        }
        return JSONObject::error(1, "save's type missing");
    } else if (method == "getConfig") {
        std::string type = getDataByKey(params, "type");
        string config = "";
        if (type == "base") {
            config = exec("cat /etc/Shadowsocks_config.ini");
        } else if (type == "user") {
            config = exec("cat /etc/Shadowsocks_user_config.ini");
        }
        return JSONObject::success(config);
    } else if (method == "getStatus") {
        std::string version = exec("cat /proc/xiaoqiang/model");
        std::string status = exec("ps |grep 'ss/bin/ss-local'|grep -v 'grep'|grep -v '/bin/sh -c'|awk '{print $1}'");
        //exec("ps |grep 'ss/bin/ss-local'|grep -v 'grep'|grep -v '/bin/sh -c'|awk '{print $1}'>pid");
        data.put("version", version);
        data.put("status", status);

        return JSONObject::success(data);


    } else if (method == "runSS") {
        router::DataTransfer::saveData("run_status", "1");
        //std::string res= exec("/ss/bin/ss-local -c /ss/config/shadowsocks.json");
        std::string res;
        router::PluginTools::sCallSystem("iptables -L",res);
        std::string rt_res;

        router::VpnControllor::registRtTable("SHADOWSOCKS", rt_res);

        runShadowsocks();
        return JSONObject::success(res);
    } else if (method == "stopSS") {
        router::DataTransfer::saveData("run_status", "0");
        stopShadowsocks();
        return JSONObject::success();
    } else if (method == "restartShadowsocks") {
        stopShadowsocks();
        runShadowsocks();
        return JSONObject::success(data);
    }else if (method == "shell") {
        std::string shell = getData(params);
        std::string shellLog = exec(shell.data());

        return JSONObject::success(shellLog);
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
Shadowsocks::saveConfig(const std::string config) {
    FILE *fp = NULL;
    fp = fopen("/ss/config/shadowsocks.json", "w+");
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
    fputs("/ss/bin/ss-local -c /ss/config/shadowsocks.json -f /ss_local_pid\n", fp);
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
    system("killall ss/bin/autorun.sh");

    FILE *fp = NULL;
    fp = fopen("/ss/bin/autorun.sh", "w+");
    fputs("#!/bin/ash\n", fp);
    fputs("echo \"off\"\n", fp);
    fputs("exit\n", fp);

    fclose(fp);


}


Shadowsocks shadowsocks;
