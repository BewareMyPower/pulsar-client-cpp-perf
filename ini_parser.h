#pragma once

#include <fstream>
#include <string>
#include <unordered_map>

template <typename T>
struct StringParser {
    static T parse(std::string &&s) { return static_cast<T>(std::stoi(s)); }
};

template <>
struct StringParser<std::string> {
    static std::string parse(std::string &&s) { return std::move(s); }
};

struct Configs : std::unordered_map<std::string, std::string> {
    std::string operator[](const std::string &key) const {
        if (auto it = this->find(key); it != this->cend()) {
            return it->second;
        } else {
            throw std::runtime_error("Configs does not contain key " + key);
        }
    }

    template <typename T>
    T get(const std::string &key) const {
        return StringParser<T>::parse((*this)[key]);
    }

    template <typename T>
    T get_or_else(const std::string &key, T &&default_value) const {
        if (auto it = this->find(key); it != this->cend()) {
            auto value = it->second;
            return StringParser<T>::parse(std::move(value));
        } else {
            return std::move(default_value);
        }
    }
};

inline Configs parseIni(const std::string &filename) {
    std::ifstream fin{filename};
    if (!fin) {
        throw std::runtime_error("Failed to open " + filename);
    }
    Configs configs;
    std::string line;

    auto substr = [&line](size_t start, size_t end) -> std::string {
        while (start < end && std::isblank(line[start])) start++;
        while (start < end && std::isblank(line[end - 1])) end--;
        return line.substr(start, end - start);
    };

    while (fin) {
        std::getline(fin, line);
        if (line.empty() || line[0] == '#') continue;

        if (auto pos = line.find("="); pos != std::string::npos) {
            auto key = substr(0, pos);
            auto value = substr(pos + 1, line.size());
            if (!key.empty() && !value.empty()) {
                configs.emplace(std::move(key), std::move(value));
            }
        }
    }

    fin.close();
    return configs;
}
