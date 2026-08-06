/**
   ___ _                 _                      
  / __| |__   __ _ _ __ | |_    /\/\   ___  ___ 
 / /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\/\ |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/    \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee 
*/

#ifndef COMMAND_INTERPRETER_CPP
#define COMMAND_INTERPRETER_CPP

#include "saver.cpp"
#include "logger.cpp"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <climits>

#define NO_COMMAND 0x3f3f3f3fULL

class CommandInterpreter {
    // 从标识符原文映射到 pid
    std::map<std::string, unsigned long long> mp;
    Saver &saver = Saver::get_saver();
    Logger &logger = Logger::get_logger();
    std::string DATA_STORAGE_NAME = "CommandInterpreter::map_relation";

    bool identifier_exist(std::string identifier);
    std::string escape(char ch);
    std::vector<std::string> separator(std::string &s);
    bool load();
    bool save();
public:
    CommandInterpreter();
    ~CommandInterpreter();
    bool FIRST_START = false;
    bool loaded_ok = false;
    bool dirty = false;
    bool add_identifier(std::string identifier, unsigned long long pid);
    bool delete_identifier(std::string identifier);
    std::pair<unsigned long long, std::vector<std::string>> get_command();
    bool clear_data();
};



                        /* class Terminal */

bool CommandInterpreter::identifier_exist(std::string identifier) {
    return mp.count(identifier) > 0;
}

/**
 * 单趟解析：切分空格的同时处理转义，语义与原来的"先切分再转义"完全一致。
 * 规则：
 * \s -> 空格，\t -> Tab，\\ -> 反斜杠；
 * 未知转义保留反斜杠和原字符（如 \U 变成 \U）；
 * 行尾孤立的 \ 保留为字面反斜杠；
 * 反斜杠后跟空格时只保留反斜杠，空格仍然作为分隔符。
 */
std::vector<std::string> CommandInterpreter::separator(std::string &s) {
    std::vector<std::string> res;
    std::string tmp;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == ' ') {
            if (!tmp.empty()) {
                res.push_back(tmp);
                tmp.clear();
            }
        } else if (s[i] != '\\') {
            tmp.push_back(s[i]);
        } else if (i == s.size() - 1) {
            tmp.push_back('\\');
        } else {
            std::string escaped = escape(s[i + 1]);
            if (escaped.empty()) {
                // Unknown escape: keep the backslash and the character.
                tmp.push_back('\\');
                if (s[i + 1] != ' ') {
                    tmp.push_back(s[i + 1]);
                    i++;
                }
                // 反斜杠后是空格时不消费它，下一轮按分隔符处理（与原两趟解析一致）。
            } else {
                tmp += escaped;
                i++;
            }
        }
    }
    if (!tmp.empty()) res.push_back(tmp);
    return res;
}

std::string CommandInterpreter::escape(char ch) {
    static std::vector<std::pair<char, std::string>> fr_to({
        {'s', " "},
        {'t', "\t"},
        {'\\', "\\"}
    });
    for (auto &it : fr_to) {
        if (it.first == ch) return it.second;
    }
    return "";
}

bool CommandInterpreter::save() {
    vvs data;
    for (auto &it : mp) {
        data.push_back(std::vector<std::string>());
        data.back().push_back(it.first);
        data.back().push_back(std::to_string(it.second));
    }
    return saver.save(DATA_STORAGE_NAME, data);
}

bool CommandInterpreter::load() {
    vvs data;
    if (!saver.load(DATA_STORAGE_NAME, data)) return false;
    mp.clear();
    for (auto &pr : data) {
        if (pr.size() != 2) {
            logger.log("Command interpreter: The mapping should be a pair. Please check whether the data is complete.", Logger::WARNING, __LINE__);
            mp.clear();
            return false;
        }
        if (pr[0].empty() || pr[1].empty() || !saver.is_all_digits(pr[1]) || pr[1].size() > 20) {
            logger.log("Command interpreter: The identifier or the pid is invalid. Please check whether the data is complete.", Logger::WARNING, __LINE__);
            mp.clear();
            return false;
        }
        unsigned long long pid = 0;
        for (size_t i = 0; i < pr[1].size(); i++) {
            unsigned long long d = pr[1][i] - '0';
            if (pid > (ULLONG_MAX - d) / 10) {
                logger.log("Command interpreter: The identifier or the pid is invalid. Please check whether the data is complete.", Logger::WARNING, __LINE__);
                mp.clear();
                return false;
            }
            pid = pid * 10 + d;
        }
        if (identifier_exist(pr[0])) {
            logger.log("Command interpreter: There are multiple identifiers in the data, please check whether the data is correct.", Logger::WARNING, __LINE__);
            mp.clear();
            return false;
        }
        mp[pr[0]] = pid;
    }
    return true;
}

CommandInterpreter::CommandInterpreter() {
    if (saver.exists(DATA_STORAGE_NAME)) {
        if (load()) {
            loaded_ok = true;
            if (mp.empty()) FIRST_START = true;
        } else {
            loaded_ok = false;
            FIRST_START = true;
        }
    } else {
        loaded_ok = true;
        FIRST_START = true;
    }
}

CommandInterpreter::~CommandInterpreter() {
    if (dirty && loaded_ok && !save()) {
        logger.log("Failed to save the identifier mapping on exit.", Logger::WARNING, __LINE__);
    }
}

bool CommandInterpreter::add_identifier(std::string identifier, unsigned long long pid) {
    if (identifier.empty()) {
        logger.log("The identifier cannot be empty.", Logger::WARNING, __LINE__);
        return false;
    }
    // 标识符里不能有空格或 Tab，否则注册后永远无法通过命令行输入。
    for (size_t i = 0; i < identifier.size(); i++) {
        if (identifier[i] == ' ' || identifier[i] == '\t') {
            logger.log("The identifier cannot contain spaces or tabs.", Logger::WARNING, __LINE__);
            return false;
        }
    }
    if (pid == NO_COMMAND) {
        logger.log("The pid " + std::to_string(pid) + " is a reserved value and cannot be used.", Logger::WARNING, __LINE__);
        return false;
    }
    if (identifier == "exit" || identifier == "quit") {
        logger.log("The identifier " + identifier + " is reserved and cannot be registered.", Logger::WARNING, __LINE__);
        return false;
    }
    if (identifier_exist(identifier)) {
        logger.log("Identifier " + identifier + " already exists. Please delete the original to add a new one.", Logger::WARNING, __LINE__);
        return false;
    }
    mp[identifier] = pid;
    dirty = true;
    return true;
}

bool CommandInterpreter::delete_identifier(std::string identifier) {
    if (!identifier_exist(identifier)) {
        logger.log("Identifier " + identifier + " does not exist.", Logger::WARNING, __LINE__);
        return false;
    }
    mp.erase(mp.find(identifier));
    dirty = true;
    return true;
}

/**
 * Commands are separated by Spaces. Use \s for a space, \t for a tab,
 * and \\ for a backslash.
 */
std::pair<unsigned long long, std::vector<std::string>> CommandInterpreter::get_command() {
    std::string cmd;
    std::getline(std::cin, cmd);
    if (!std::cin) {
        return std::make_pair(NO_COMMAND, std::vector<std::string>());
    }
    if (!cmd.empty() && cmd.back() == '\r') cmd.pop_back();
    if (cmd.size() > 65536) {
        logger.log("The command line is too long.", Logger::WARNING, __LINE__);
        cmd.clear();
    }
    std::vector<std::string> separated_cmd = separator(cmd);
    if (separated_cmd.empty()) {
        return std::make_pair(NO_COMMAND, std::vector<std::string>());
    }
    if (!identifier_exist(separated_cmd.front())) {
        // exit/quit 是保留字，不能注册成别名，所以直接在这里识别，不再报 Command not found。
        if (separated_cmd.front() == "exit" || separated_cmd.front() == "quit") {
            return std::make_pair(NO_COMMAND, separated_cmd);
        }
        logger.log("Command not found: " + separated_cmd.front(), Logger::WARNING, __LINE__);
        return std::make_pair(NO_COMMAND, separated_cmd);
    } else {
        unsigned long long pid = mp[separated_cmd.front()];
        separated_cmd.erase(separated_cmd.begin());
        return std::make_pair(pid, separated_cmd);
    }
}

bool CommandInterpreter::clear_data() {
    mp.clear();
    dirty = true;
    return true;
}

#endif
