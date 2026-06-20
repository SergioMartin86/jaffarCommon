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
__JAFFAR_COMMON_INLINE__ const void checkEntry(const object& json, const std::string& key)
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
__JAFFAR_COMMON_INLINE__ const std::string getString(const object& json, const std::string& key)
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
__JAFFAR_COMMON_INLINE__ const object& getObject(const object& json, const std::string& key)
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
__JAFFAR_COMMON_INLINE__ const std::vector<T> getArray(const object& json, const std::string& key)
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
__JAFFAR_COMMON_INLINE__ const T getNumber(const object& json, const std::string& key)
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
__JAFFAR_COMMON_INLINE__ const bool getBoolean(const object& json, const std::string& key)
{
  checkEntry(json, key);
  if (json[key].is_boolean() == false) JAFFAR_THROW_LOGIC("[Error] Configuration key '%s' is not a boolean. JSON Dump: %s\n", key.c_str(), json.dump(2).c_str());
  return json.at(key).get<bool>();
}

/**
 * Consuming variants of the getters: read the value as the corresponding get*() function, then
 * erase the key from the (mutable) json object. After a parser has read every key it recognizes
 * with these, any keys left in the object are unrecognized -- pass the object to checkEmpty().
 */
__JAFFAR_COMMON_INLINE__ const std::string popString(object& json, const std::string& key)
{
  const std::string value = getString(json, key);
  json.erase(key);
  return value;
}

/// @brief Like getObject(), but returns a copy and erases the key from the parent. See popString().
__JAFFAR_COMMON_INLINE__ const object popObject(object& json, const std::string& key)
{
  const object value = getObject(json, key);
  json.erase(key);
  return value;
}

/// @brief Like getArray(), but erases the key from the json object. See popString().
template <typename T>
__JAFFAR_COMMON_INLINE__ const std::vector<T> popArray(object& json, const std::string& key)
{
  const std::vector<T> value = getArray<T>(json, key);
  json.erase(key);
  return value;
}

/// @brief Like getNumber(), but erases the key from the json object. See popString().
template <typename T>
__JAFFAR_COMMON_INLINE__ const T popNumber(object& json, const std::string& key)
{
  const T value = getNumber<T>(json, key);
  json.erase(key);
  return value;
}

/// @brief Like getBoolean(), but erases the key from the json object. See popString().
__JAFFAR_COMMON_INLINE__ const bool popBoolean(object& json, const std::string& key)
{
  const bool value = getBoolean(json, key);
  json.erase(key);
  return value;
}

/**
 * Throws if the given json object still contains any keys. Used after a parser has consumed (via the
 * pop*() functions) every key it recognizes: anything left is an unrecognized key, which is reported
 * by name together with the context (e.g. the configuration section the keys came from).
 *
 * @param[in] json The (already-consumed) json object that should be empty
 * @param[in] context A human-readable description of where the keys came from, used in the error
 */
__JAFFAR_COMMON_INLINE__ const void checkEmpty(const object& json, const std::string& context)
{
  if (json.is_object() == false) return;
  if (json.empty()) return;
  std::string keys;
  for (auto it = json.begin(); it != json.end(); ++it) keys += (keys.empty() ? "'" : ", '") + it.key() + "'";
  JAFFAR_THROW_LOGIC("[Error] Unrecognized key(s) in %s: %s. Check for typos or unsupported options. JSON Dump: %s\n", context.c_str(), keys.c_str(), json.dump(2).c_str());
}

} // namespace json

} // namespace jaffarCommon