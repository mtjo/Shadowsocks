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
//#include "boost/thread.hpp"

using router::DataTransfer;


#define BUF_SIZE 256

Shadowsocks::Shadowsocks() {
}
void startShadowsocks(){
    system("./ss/autorun.sh");
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
        std::string type = getDataByKey(params,"type");
        router::DataTransfer::saveData("configType", type);
        if (type == "user") {
            std::string jsondata = getData(params);

            FILE *fp = NULL;
            fp = fopen("/etc/Shadowsocks_user_config.ini", "w+");
            fputs(jsondata.data(), fp);
            fclose(fp);
            return JSONObject::success();

        } else if(type == "base") {
            const char *ch = params.data();
            struct json_object *jsonObject = NULL;
            jsonObject = json_tokener_parse(ch);
            if ((long) jsonObject > 0) {/**Json格式无错误**/
                jsonObject = json_object_object_get(jsonObject, "data");
                saveConfig(jsonObject);
            }
            json_object_put(jsonObject);
            return JSONObject::success();
        }
        return JSONObject::error(1, "save's type missing");
    } else if (method == "getConfig") {
        std::string type = getDataByKey(params,"type");
        string config = "";
        if (type == "base") {
            config = exec("cat /etc/Shadowsocks_config.ini");
        } else if (type == "user") {
            config = exec("cat /etc/Shadowsocks_user_config.ini");
        }
        return JSONObject::success(config);
    } else if (method == "getStatus") {
        std::string version = exec("cat /proc/xiaoqiang/model");

        std::string status = exec("ps |grep 'Shadowsocks'|grep -v 'grep'|grep -v '/ss/sh -c'|awk '{print $1}'");
        exec("ps |grep 'Shadowsocks'|grep -v 'grep'|grep -v '/ss/sh -c'|awk '{print $1}'>pid");
        data.put("version", version);
        data.put("status", status);

        return JSONObject::success(data);


    } else if (method == "runShadowsocks") {
        router::DataTransfer::saveData("run_status", "1");
        runShadowsocks();
        return JSONObject::success();
    } else if (method == "stopShadowsocks") {
        router::DataTransfer::saveData("run_status", "0");
        std::string run_status;
        router::DataTransfer::getData("run_status", run_status);
        data.put("run_status", run_status);
        stopShadowsocks();
        return JSONObject::success(data);
    }else if (method == "restartShadowsocks") {
        stopShadowsocks();
        runShadowsocks();
        return JSONObject::success(data);
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
Shadowsocks::getDataByKey(const std::string &params,std::string key) {
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
Shadowsocks::saveConfig(struct json_object *configData) {
    exec("rm -f etc/Shadowsocks_config.ini");
    const char *file = "etc/Shadowsocks_config.ini";
    json_object_object_foreach(configData, section, val)
    {
        json_object_object_foreach(val, key, value)
        {
            write_profile_string(section, key, json_object_get_string(value), file);
        }
    }
}

void Shadowsocks::runShadowsocks() {
    std::string run_status;
    router::DataTransfer::getData("run_status", run_status);
    std::string configType;
    router::DataTransfer::getData("configType", configType);

    FILE *fp = NULL;
    fp = fopen("/ss/autorun.sh", "w+");
    fputs("#!/ss/ash\n", fp);
    if(configType=="base"){
        fputs("/ss/Shadowsocks -c /etc/Shadowsocks_config.ini &>/dev/null\n", fp);
    }else{
        fputs("/ss/Shadowsocks -c /etc/Shadowsocks_user_config.ini &>/dev/null\n", fp);
    }
    fputs("echo \"on\"\n", fp);
    fclose(fp);

    if (run_status == "1") {
        //system("./ss/autorun.sh");
        std::thread subthread(startShadowsocks);
        subthread.detach();
    }
}

void Shadowsocks::stopShadowsocks() {
    system("killall ss/Shadowsocks");
    system("killall ss/autorun.sh");

    FILE *fp = NULL;

    fp = fopen("/ss/autorun.sh", "w+");
    fputs("#!/ss/ash\n", fp);
    fputs("echo \"off\"\n", fp);
    fputs("exit\n", fp);

    fclose(fp);


}


Shadowsocks shadowsocks;
