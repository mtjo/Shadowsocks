/**
 * @author Baiming Ma (mabaiming@xiaomi.com)
 * @date 2015-06-04
 */

#pragma once

#include <string>

#include "json/json.h"

class JSONArray;

class JSONObject {
public:
  static std::string success() {
    JSONObject data;
    return data.put("code", 0).put("msg", "").toString();
  }

  static std::string success(JSONObject data) {
    JSONObject result;
    return result.put("code", 0).put("msg", "").put("data", data).toString();
  }
  static std::string success(const std::string string) {
    JSONObject result;
    return result.put("code", 0).put("msg", "").put("data", string).toString();
  }

  static std::string error(int code, const std::string& msg) {
    JSONObject data;
    return data.put("code", code).put("msg", msg).toString();
  }

  JSONObject() :
    holder(json_object_new_object()),
    owner(true) {}

  ~JSONObject() {
    if (owner) {
      json_object_put(holder);
    }
  }

  JSONObject& put(const std::string &key, bool value);
  JSONObject& put(const std::string& key, int32_t value);
  JSONObject& put(const std::string& key, int64_t value);
  JSONObject& put(const std::string& key, double value);
  JSONObject& put(const std::string& key, const std::string& value);
  JSONObject& put(const std::string& key, JSONObject& value);
  JSONObject& put(const std::string& key, JSONArray& value);
  std::string toString() const;

private:
  struct json_object* holder;
  bool owner;

friend class JSONArray;
};

class JSONArray {
public:
  JSONArray() :
    holder(json_object_new_array()),
    owner(true) {}

  ~JSONArray() {
    if (owner) {
      json_object_put(holder);
    }
  }

  JSONArray& append(bool value);
  JSONArray& append(int32_t value);
  JSONArray& append(int64_t value);
  JSONArray& append(double value);
  JSONArray& append(const std::string& value);
  JSONArray& append(JSONObject& value);
  JSONArray& append(JSONArray& value);
  int length() const;
  std::string toString() const;

private:
  struct json_object* holder;
  bool owner;

friend class JSONObject;
};
