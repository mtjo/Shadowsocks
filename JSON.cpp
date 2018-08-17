/**
 * @author Baiming Ma (mabaiming@xiaomi.com)
 * @date 2015-06-04
 */

#include "JSON.h"

#include <string>

#include "json/json.h"

JSONObject&
JSONObject::put(const std::string& key, bool value) {
  json_object_object_add(holder, key.c_str(), json_object_new_boolean(value));
  return *this;
}

JSONObject&
JSONObject::put(const std::string& key, int32_t value) {
  json_object_object_add(holder, key.c_str(), json_object_new_int(value));
  return *this;
}

JSONObject&
JSONObject::put(const std::string& key, int64_t value) {
  json_object_object_add(holder, key.c_str(), json_object_new_int64(value));
  return *this;
}

JSONObject&
JSONObject::put(const std::string& key, double value) {
  json_object_object_add(holder, key.c_str(), json_object_new_double(value));
  return *this;
}

JSONObject&
JSONObject::put(const std::string& key, const std::string& value) {
  json_object_object_add(holder, key.c_str(), json_object_new_string(value.c_str()));
  return *this;
}

JSONObject&
JSONObject::put(const std::string& key, JSONObject& value) {
  json_object_object_add(holder, key.c_str(), value.holder);
  value.owner = false;
  return *this;
}

JSONObject&
JSONObject::put(const std::string& key, JSONArray& value) {
  json_object_object_add(holder, key.c_str(), value.holder);
  value.owner = false;
  return *this;
}

std::string
JSONObject::toString() const {
  return json_object_get_string(holder);
}

JSONArray&
JSONArray::append(bool value) {
  json_object_array_add(holder, json_object_new_boolean(value));
  return *this;
}

JSONArray&
JSONArray::append(int32_t value) {
  json_object_array_add(holder, json_object_new_int(value));
  return *this;
}

JSONArray&
JSONArray::append(int64_t value) {
  json_object_array_add(holder, json_object_new_int64(value));
  return *this;
}

JSONArray&
JSONArray::append(double value) {
  json_object_array_add(holder, json_object_new_double(value));
  return *this;
}

JSONArray&
JSONArray::append(const std::string& value) {
  json_object_array_add(holder, json_object_new_string(value.c_str()));
  return *this;
}

JSONArray&
JSONArray::append(JSONObject& value) {
  json_object_array_add(holder, value.holder);
  value.owner = false;
  return *this;
}

JSONArray&
JSONArray::append(JSONArray& value) {
  json_object_array_add(holder, value.holder);
  value.owner = false;
  return *this;
}

int
JSONArray::length() const {
  return json_object_array_length(holder);
}

std::string
JSONArray::toString() const {
  return json_object_get_string(holder);
}
