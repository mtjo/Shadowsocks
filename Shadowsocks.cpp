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
#include "CommonTools.h"
#include <curl/curl.h>
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
    std::string SSversion = Tools::runCommand("/ss/bin/ss-local -h|grep shadowsocks-libev|awk '{print $2}'");
    Tools::saveData("SSversion",SSversion);

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

        //save index
        std::string nodeIndex = Tools::getParamsByKey(params, "nodeIndex");
        Tools::saveData("nodeIndex",nodeIndex);

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
    } else if (method == "getSSStatus") {
        //std::string status = Tools::runCommand("ps |grep 'ss-local'|grep -v 'grep'|grep -v '/bin/sh -c'|awk '{print $1}'");
        //std::string status = Tools::runCommand("pidof ss/ss-local);
        //std::string local = Tools::runCommand("ps |grep 'ss-local'|grep -v 'grep'|grep -v '/bin/sh -c'|awk '{print $1}'");
        std::string local = Tools::runCommand("pidof ss-local");
        std::string redir = Tools::runCommand("pidof ss-redir");

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
    } else if (method == "runCommand") {
        std::string command = Tools::getParamsByKey(params, "command");

        return JSONObject::success(Tools::runCommand(command));
    }
    else if (method == "curltest") {
        // get 请求
        std::string strURL = "http://www.baidu.com";
        std::string strResponse;
        CURLcode nRes = CommonTools::HttpGet(strURL, strResponse,300);
        size_t nSrcLength = strResponse.length();

        //下载文件
        std::string url = "http://www.baidu.com/aaa.dat";
        char local_file[50] = {0};
        //sprintf(local_file,"./pb_%d_%s.dat",1,"aaaa");
        int iDownlaod_Success = CommonTools::download_file(url.c_str(),local_file);
        if(iDownlaod_Success<0){
            char download_failure_info[100] ={0};
            //sprintf(download_failure_info,"download file :%s ,failure,url:%s",local_file,url);
            //FATAL(download_failure_info);
        }
        data.put("get",strResponse);
        data.put("size_t",(int)nSrcLength);
        data.put("download",iDownlaod_Success);

        return JSONObject::success(data);
    }

    return JSONObject::error(1, method + " not defined");


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
