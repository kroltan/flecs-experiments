#pragma once

#include <string>
#include <sstream>
#include <fstream>

static std::string read_file_to_end(const std::string &path) {
    std::ifstream file(path);

    std::ostringstream content;
    content << file.rdbuf();

    return content.str();
}

static void replace_all(std::string &haystack, std::string_view from, std::string_view to) {
    std::string::size_type position = 0;
    while ((position = haystack.find(from, position)) != std::string::npos) {
        haystack.replace(position, from.length(), to);
        position += to.length();
    }
}