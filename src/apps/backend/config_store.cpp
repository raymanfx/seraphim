/*
 * (C) Copyright 2019
 * The Seraphim Project Developers.
 *
 * SPDX-License-Identifier: MIT
 */

#include <fstream>
#include <iostream>
#include <limits.h>
#include <sstream>

#include "config_store.h"

using namespace sph::backend;

ConfigStore::ConfigStore() {
    m_delim_token = '=';
    m_comment_token = '#';
}

ConfigStore::~ConfigStore() {
    // dummy
}

bool ConfigStore::open(const std::string &path) {
    std::ifstream cfg_file;
    std::string cfg_line;
    std::string key, value;
    size_t delim_pos;

    cfg_file.open(path);
    if (!cfg_file.is_open()) {
        return false;
    }

    // init kvmap
    m_conf_path = path;

    // XX: set a sane locale for str->double conversion
    setlocale(LC_NUMERIC, "C");

    while (getline(cfg_file, cfg_line)) {
        // skip empty lines
        if (cfg_line.length() == 0) {
            continue;
        }

        // skip comment tokens
        if (cfg_line.at(0) == m_comment_token) {
            continue;
        }

        // look for a valid key
        delim_pos = cfg_line.find(m_delim_token);
        if (delim_pos == std::string::npos) {
            continue;
        }

        key = cfg_line.substr(0, delim_pos);
        value = cfg_line.substr(++delim_pos, cfg_line.length());
        if (key.empty() || value.empty()) {
            std::cout << "[WARNING] ConfigStore::" << __func__ << ": Malformed key-value pair: "
                      << " k=" << key << ", v=" << value << ", ignoring" << std::endl;
            continue;
        }

        m_kvmap[key] = value;
    }

    // XX: restore default locale
    setlocale(LC_NUMERIC, "");

    cfg_file.close();
    return true;
}

/*
 * Returns the value to a given key.
 * Delimiter is "=".
 */
std::string ConfigStore::get_value(const std::string &key) const {
    if (m_kvmap.find(key) == m_kvmap.end())
        return "";

    return m_kvmap.at(key);
}

/*
 * Returns all settings, separated by newlines.
 * Delimiter is "=".
 */
std::string ConfigStore::get_settings() const {
    std::string ret = "";
    std::stringstream line;

    for (auto it = m_kvmap.begin(); it != m_kvmap.end(); it++) {
        ret += "\n" + it->first + "=" + it->second;
    }

    return ret;
}

/*
 * Sets the value for a given key.
 * Note that this only applies to the running session and does not affect the config file at all.
 * All changes made here will be lost on application restart.
 *
 * Returns: true on success, false otherwise.
 */
bool ConfigStore::set_value(const std::string &key, const std::string &value) {
    if (m_kvmap.find(key) == m_kvmap.end()) {
        return false;
    }

    m_kvmap[key] = value;
    return true;
}
