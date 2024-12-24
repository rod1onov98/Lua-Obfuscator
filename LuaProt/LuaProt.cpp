#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <regex>
#include <sys/stat.h>


const std::unordered_set<std::string> lua_keywords = {
    "and", "break", "do", "else", "elseif", "end", "false", "for", "function",
    "goto", "if", "in", "local", "nil", "not", "or", "repeat", "return",
    "then", "true", "until", "while"
};

std::string obfs(size_t length) {
    const char charset[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const size_t max_index = (sizeof(charset) - 1);
    std::string random_string;
    for (size_t i = 0; i < length; ++i) {
        random_string += charset[rand() % max_index];
    }
    return random_string;
}

std::string obfslua(const std::string& code) {
    std::unordered_map<std::string, std::string> name_map;
    std::string obfuscated_code = code;
    std::regex word_regex(R"(\b[a-zA-Z_][a-zA-Z0-9_]*\b)");
    auto words_begin = std::sregex_iterator(code.begin(), code.end(), word_regex);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::string word = (*i).str();
        if (lua_keywords.find(word) != lua_keywords.end()) {
            continue; 
        }
        if (name_map.find(word) == name_map.end()) {
            name_map[word] = obfs(10);
        }
        obfuscated_code = std::regex_replace(obfuscated_code, std::regex("\\b" + word + "\\b"), name_map[word]);
    }

    return obfuscated_code;
}

std::string readfile(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("could not open file: " + file_path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void writefile(const std::string& file_path, const std::string& content) {
    std::ofstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("could not write to file: " + file_path);
    }
    file << content;
}

bool fileExists(const std::string& file_path) {
    struct stat buffer;
    return (stat(file_path.c_str(), &buffer) == 0);
}

int main(int argc, char* argv[]) {
    std::string lua_file_path;

    if (argc != 2) {
        std::cout << "drag and drop a lua file onto the executable or enter the file path: ";
        std::getline(std::cin, lua_file_path);
    }
    else {
        lua_file_path = argv[1];
    }

    if (!fileExists(lua_file_path) || lua_file_path.substr(lua_file_path.find_last_of(".") + 1) != "lua") {
        std::cerr << "please provide a valid Lua file" << std::endl;
        return 1;
    }

    try {
        std::string lua_code = readfile(lua_file_path);

        srand(static_cast<unsigned int>(time(0)));

        std::string obfuscated_code = obfslua(lua_code);
        std::string output_file_path = lua_file_path + ".prot.lua";

        writefile(output_file_path, obfuscated_code);

        std::cout << "obfs lua file saved to " << output_file_path << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "obfs failed: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
