#pragma once

/**
 * @file json.hpp
 * @brief Contains common functions related to JSON manipulation
 */

#include <cstdlib>
#include "../extern/json/single_include/nlohmann/json.hpp"
#include "logger.hpp"

namespace jaffarCommon
{

#define JSON_GET_STRING(JSON, ENTRY) jaffarCommon::jsonGetString(JSON, ENTRY)
static inline const std::string jsonGetString(const nlohmann::json& json, const std::string& entry)
{
  if (json.is_object() == false) EXIT_WITH_ERROR("[Error] JSON passed is not a key/value object. Happened when trying to obtain string entry '%s'. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  if (json.contains(entry) == false) EXIT_WITH_ERROR("[Error] JSON contains no field called '%s'. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  if (json[entry].is_string() == false) EXIT_WITH_ERROR("[Error] Configuration entry '%s' is not a string. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  return json.at(entry).get<std::string>();
}

#define JSON_GET_OBJECT(JSON, ENTRY) jaffarCommon::jsonGetObject(JSON, ENTRY)
static inline const nlohmann::json& jsonGetObject(const nlohmann::json& json, const std::string& entry)
{
  if (json.is_object() == false) EXIT_WITH_ERROR("[Error] JSON passed is not a key/value object. Happened when trying to obtain string entry '%s'. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  if (json.contains(entry) == false) EXIT_WITH_ERROR("[Error] JSON contains no field called '%s'. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  if (json[entry].is_object() == false) EXIT_WITH_ERROR("[Error] Configuration entry '%s' is not a key/value object. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  return json.at(entry);
}

#define JSON_GET_ARRAY(JSON, ENTRY) jaffarCommon::jsonGetArray(JSON, ENTRY)
static inline const nlohmann::json& jsonGetArray(const nlohmann::json& json, const std::string& entry)
{
  if (json.is_object() == false) EXIT_WITH_ERROR("[Error] JSON passed is not a key/value object. Happened when trying to obtain string entry '%s'. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  if (json.contains(entry) == false) EXIT_WITH_ERROR("[Error] JSON contains no field called '%s'. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  if (json[entry].is_array() == false) EXIT_WITH_ERROR("[Error] Configuration entry '%s' is not an array. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  return json.at(entry);
}

#define JSON_GET_NUMBER(T, JSON, ENTRY) jaffarCommon::jsonGetNumber<T>(JSON, ENTRY)
template <typename T> static inline const T jsonGetNumber(const nlohmann::json& json, const std::string& entry)
{
  if (json.is_object() == false) EXIT_WITH_ERROR("[Error] JSON passed is not a key/value object. Happened when trying to obtain string entry '%s'. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  if (json.contains(entry) == false) EXIT_WITH_ERROR("[Error] JSON contains no field called '%s'. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  if (json[entry].is_number() == false) EXIT_WITH_ERROR("[Error] Configuration entry '%s' is not a number. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  return json.at(entry).get<T>();
}

#define JSON_GET_BOOLEAN(JSON, ENTRY) jaffarCommon::jsonGetBoolean(JSON, ENTRY)
static inline const bool jsonGetBoolean(const nlohmann::json& json, const std::string& entry)
{
  if (json.is_object() == false) EXIT_WITH_ERROR("[Error] JSON passed is not a key/value object. Happened when trying to obtain string entry '%s'. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  if (json.contains(entry) == false) EXIT_WITH_ERROR("[Error] JSON contains no field called '%s'. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  if (json[entry].is_boolean() == false) EXIT_WITH_ERROR("[Error] Configuration entry '%s' is not a boolean. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  return json.at(entry).get<bool>();
}

}