/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SPH_CONFIG_STORE_H
#define SPH_CONFIG_STORE_H

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace sph {
namespace backend {

/*
 * TODO
 *
 * Right now, we only support single key-value maps, no array values or such.
 * In the future, it might be a good idea to parse settings values from JSON
 * instead, e.g. using https://github.com/nlohmann/json.
 */

class ConfigStore {
    // generic callback function template
    typedef std::function<void(const std::string &key, const std::string &value)> CallbackFunction;

public:
    ~ConfigStore();

    // Singleton, thread safe
    static ConfigStore &Instance() {
        static ConfigStore _Instance;
        return _Instance;
    }

    bool open(const std::string &path);

    // get value for key
    std::string get_value(const std::string &key) const;
    // set value for key
    bool set_value(const std::string &key, const std::string &value);
    // get all keys and values, separated by newlines
    std::string get_settings() const;

private:
    ConfigStore();
    // stub functions
    ConfigStore(ConfigStore const &);
    void operator=(ConfigStore const &);

    std::string m_conf_path;
    std::map<std::string, bool> m_configs;
    std::map<std::string, std::string> m_kvmap;

    // special tokens
    char m_delim_token;
    char m_comment_token;
};

} // namespace backend
} // namespace sph

#endif // SPH_CONFIG_STORE_H
