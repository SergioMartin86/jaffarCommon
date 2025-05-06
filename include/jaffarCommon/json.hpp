#pragma once

/**
 * @file json.hpp
 * @brief Contains common functions related to JSON manipulation
 */
#include "exceptions.hpp"
#include <json/single_include/nlohmann/json.hpp>
#include <stdlib.h>

namespace jaffarCommon
{

namespace json
{

/**
 *  Masks the use of nlohmann namespace in favor or an implementation-agnostic naming
 */
typedef nlohmann::json object;

/**
 * Verifies the given key exists in the given json object
 *
 * @note An exception will occur if the input is not a json object (e.g., is an array)
 * @note An exception will occur if the given key does not exist in the json object
 *
 * @param[in] json The json object to look into
 * @param[in] key The key to look for inside the json object
 */
__JAFFARCOMMON__INLINE__ const void checkEntry(const object& json, const std::string& key)
{
  if (json.is_object() == false)
    JAFFAR_THROW_LOGIC("[Error] JSON passed is not a key/value object. Happened when trying to obtain string key '%s'. JSON Dump: %s\n", key.c_str(), json.dump(2).c_str());
  if (json.contains(key) == false) JAFFAR_THROW_LOGIC("[Error] JSON contains no field called '%s'. JSON Dump: %s\n", key.c_str(), json.dump(2).c_str());
}

/**
 * Returns the string stored in the given key of the given json object
 *
 * @note An exception will occur if the input is not a json object (e.g., is an array)
 * @note An exception will occur if the given key does not exist in the json object
 * @note An exception will occur if the given key is not a string
 *
 * @param[in] json The json object to look into
 * @param[in] key The key to look for inside the json object
 * @return A string contained in the key
 */
__JAFFARCOMMON__INLINE__ const std::string getString(const object& json, const std::string& key)
{
  checkEntry(json, key);
  if (json[key].is_string() == false) JAFFAR_THROW_LOGIC("[Error] Configuration key '%s' is not a string. JSON Dump: %s\n", key.c_str(), json.dump(2).c_str());
  return json.at(key).get<std::string>();
}

/**
 * Returns the object stored in the given key of the given json object
 *
 * @note An exception will occur if the input is not a json object (e.g., is an array)
 * @note An exception will occur if the given key does not exist in the json object
 * @note An exception will occur if the given key is not an object
 *
 * @param[in] json The json object to look into
 * @param[in] key The key to look for inside the json object
 * @return A json object contained in the key
 */
__JAFFARCOMMON__INLINE__ const object& getObject(const object& json, const std::string& key)
{
  checkEntry(json, key);
  if (json[key].is_object() == false) JAFFAR_THROW_LOGIC("[Error] Configuration key '%s' is not a key/value object. JSON Dump: %s\n", key.c_str(), json.dump(2).c_str());
  return json.at(key);
}

/**
 * Returns the array of the specified type stored in the given key of the given json object
 *
 * @note An exception will occur if the input is not a json object (e.g., is an array)
 * @note An exception will occur if the given key does not exist in the json object
 * @note An exception will occur if the given key is not an array
 *
 * @param[in] json The json object to look into
 * @param[in] key The key to look for inside the json object
 * @return A vector of the type specified containing all the elements of the array of key entry
 */
template <typename T>
__JAFFARCOMMON__INLINE__ const std::vector<T> getArray(const object& json, const std::string& key)
{
  checkEntry(json, key);
  if (json[key].is_array() == false) JAFFAR_THROW_LOGIC("[Error] Configuration key '%s' is not an array. JSON Dump: %s\n", key.c_str(), json.dump(2).c_str());
  return json.at(key).get<std::vector<T>>();
}

/**
 * Returns the number of the specified type stored the given key of the given json object
 *
 * @note An exception will occur if the input is not a json object (e.g., is an array)
 * @note An exception will occur if the given key does not exist in the json object
 * @note An exception will occur if the given key is not a number
 *
 * @param[in] json The json object to look into
 * @param[in] key The key to look for inside the json object
 * @return The number stored in the key entry
 */
template <typename T>
__JAFFARCOMMON__INLINE__ const T getNumber(const object& json, const std::string& key)
{
  checkEntry(json, key);
  if (json[key].is_number() == false) JAFFAR_THROW_LOGIC("[Error] Configuration key '%s' is not a number. JSON Dump: %s\n", key.c_str(), json.dump(2).c_str());
  return json.at(key).get<T>();
}

/**
 * Returns the boolean stored the given key of the given json object
 *
 * @note An exception will occur if the input is not a json object (e.g., is an array)
 * @note An exception will occur if the given key does not exist in the json object
 * @note An exception will occur if the given key is not a boolean
 *
 * @param[in] json The json object to look into
 * @param[in] key The key to look for inside the json object
 * @return The boolean stored in the key entry
 */
__JAFFARCOMMON__INLINE__ const bool getBoolean(const object& json, const std::string& key)
{
  checkEntry(json, key);
  if (json[key].is_boolean() == false) JAFFAR_THROW_LOGIC("[Error] Configuration key '%s' is not a boolean. JSON Dump: %s\n", key.c_str(), json.dump(2).c_str());
  return json.at(key).get<bool>();
}

} // namespace json

} // namespace jaffarCommon