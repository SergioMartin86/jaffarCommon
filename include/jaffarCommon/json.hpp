#pragma once

/**
 * @file json.hpp
 * @brief Contains common functions related to JSON manipulation
 */

#include <cstdlib>
#include <json/single_include/nlohmann/json.hpp>
#include "exceptions.hpp"

namespace jaffarCommon
{

namespace json
{

__INLINE__ const void checkEntry(const nlohmann::json& json, const std::string& entry)
{
  if (json.is_object() == false) JAFFAR_THROW_LOGIC("[Error] JSON passed is not a key/value object. Happened when trying to obtain string entry '%s'. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  if (json.contains(entry) == false) JAFFAR_THROW_LOGIC("[Error] JSON contains no field called '%s'. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
}

__INLINE__ const std::string getString(const nlohmann::json& json, const std::string& entry)
{
  checkEntry(json, entry);
  if (json[entry].is_string() == false) JAFFAR_THROW_LOGIC("[Error] Configuration entry '%s' is not a string. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  return json.at(entry).get<std::string>();
}

__INLINE__ const nlohmann::json& getObject(const nlohmann::json& json, const std::string& entry)
{
  checkEntry(json, entry);
  if (json[entry].is_object() == false) JAFFAR_THROW_LOGIC("[Error] Configuration entry '%s' is not a key/value object. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  return json.at(entry);
}

template <typename T> __INLINE__ const std::vector<T> getArray(const nlohmann::json& json, const std::string& entry)
{
  checkEntry(json, entry);
  if (json[entry].is_array() == false) JAFFAR_THROW_LOGIC("[Error] Configuration entry '%s' is not an array. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  return json.at(entry).get<std::vector<T>>();
}

template <typename T> __INLINE__ const T getNumber(const nlohmann::json& json, const std::string& entry)
{
  checkEntry(json, entry);
  if (json[entry].is_number() == false) JAFFAR_THROW_LOGIC("[Error] Configuration entry '%s' is not a number. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  return json.at(entry).get<T>();
}

__INLINE__ const bool getBoolean(const nlohmann::json& json, const std::string& entry)
{
  checkEntry(json, entry);
  if (json[entry].is_boolean() == false) JAFFAR_THROW_LOGIC("[Error] Configuration entry '%s' is not a boolean. JSON Dump: %s\n", entry.c_str(), json.dump(2).c_str());
  return json.at(entry).get<bool>();
}

} // namespace json

} // namespace jaffarCommon