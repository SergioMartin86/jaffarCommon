#pragma once

/**
 * @file json.hpp
 * @brief Contains common functions related to JSON manipulation
 */

#include <cstdlib>
#include "../extern/json/single_include/nlohmann/json.hpp"
#include "exceptions.hpp"

namespace jaffarCommon
{

namespace json
{

#define JSON_GET_STRING(JSON, ENTRY) jaffarCommon::jsonGetString(JSON, ENTRY)
__INLINE__ const std::string jsonGetString(const nlohmann::json& json, const std::string& entry)
{
  if (json.is_object() == false) JAFFAR_THROW_LOGIC("[Error] JSON passed is not a key/value object. Happened when trying to obtain string entry '%s'. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  if (json.contains(entry) == false) JAFFAR_THROW_LOGIC("[Error] JSON contains no field called '%s'. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  if (json[entry].is_string() == false) JAFFAR_THROW_LOGIC("[Error] Configuration entry '%s' is not a string. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  return json.at(entry).get<std::string>();
}

#define JSON_GET_OBJECT(JSON, ENTRY) jaffarCommon::jsonGetObject(JSON, ENTRY)
__INLINE__ const nlohmann::json& jsonGetObject(const nlohmann::json& json, const std::string& entry)
{
  if (json.is_object() == false) JAFFAR_THROW_LOGIC("[Error] JSON passed is not a key/value object. Happened when trying to obtain string entry '%s'. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  if (json.contains(entry) == false) JAFFAR_THROW_LOGIC("[Error] JSON contains no field called '%s'. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  if (json[entry].is_object() == false) JAFFAR_THROW_LOGIC("[Error] Configuration entry '%s' is not a key/value object. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  return json.at(entry);
}

#define JSON_GET_ARRAY(JSON, ENTRY) jaffarCommon::jsonGetArray(JSON, ENTRY)
__INLINE__ const nlohmann::json& jsonGetArray(const nlohmann::json& json, const std::string& entry)
{
  if (json.is_object() == false) JAFFAR_THROW_LOGIC("[Error] JSON passed is not a key/value object. Happened when trying to obtain string entry '%s'. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  if (json.contains(entry) == false) JAFFAR_THROW_LOGIC("[Error] JSON contains no field called '%s'. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  if (json[entry].is_array() == false) JAFFAR_THROW_LOGIC("[Error] Configuration entry '%s' is not an array. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  return json.at(entry);
}

#define JSON_GET_NUMBER(T, JSON, ENTRY) jaffarCommon::jsonGetNumber<T>(JSON, ENTRY)
template <typename T> __INLINE__ const T jsonGetNumber(const nlohmann::json& json, const std::string& entry)
{
  if (json.is_object() == false) JAFFAR_THROW_LOGIC("[Error] JSON passed is not a key/value object. Happened when trying to obtain string entry '%s'. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  if (json.contains(entry) == false) JAFFAR_THROW_LOGIC("[Error] JSON contains no field called '%s'. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  if (json[entry].is_number() == false) JAFFAR_THROW_LOGIC("[Error] Configuration entry '%s' is not a number. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  return json.at(entry).get<T>();
}

#define JSON_GET_BOOLEAN(JSON, ENTRY) jaffarCommon::jsonGetBoolean(JSON, ENTRY)
__INLINE__ const bool jsonGetBoolean(const nlohmann::json& json, const std::string& entry)
{
  if (json.is_object() == false) JAFFAR_THROW_LOGIC("[Error] JSON passed is not a key/value object. Happened when trying to obtain string entry '%s'. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  if (json.contains(entry) == false) JAFFAR_THROW_LOGIC("[Error] JSON contains no field called '%s'. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  if (json[entry].is_boolean() == false) JAFFAR_THROW_LOGIC("[Error] Configuration entry '%s' is not a boolean. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  return json.at(entry).get<bool>();
}

} // namespace json

} // namespace jaffarCommon