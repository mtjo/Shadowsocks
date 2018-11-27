#include "Tools.h"
#include <string>
#include "PluginTools.h"
#include <string>
#include "json/json.h"
#include "JSON.h"

using router::PluginTools;
using std::string;

std::string
Tools::runCommand(std::string command) {
    std::string output = "";
    router::PluginTools::sCallSystem(command, output);
    return output;
}

int
Tools::saveData(const std::string key, const std::string value) {
    return router::PluginTools::saveData(key, value);
}

std::string
Tools::getData(std::string key) {
    std::string value = "";
    router::PluginTools::getData(key, value);
    return value;
}

std::string
Tools::getParams(const std::string params) {
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
Tools::getParamsByKey(const std::string params, std::string key) {
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



