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

#define NO_COMMAND 0x3f3f3f3fULL

class CommandInterpreter {
    // 从identifier hash映射到pid
    std::map<unsigned long long, unsigned long long> mp;
    // std::set<unsigned long long> pid_set;
    Saver &saver = Saver::get_saver();
    Logger &logger = Logger::get_logger();
    std::string DATA_STORAGE_NAME = "CommandInterpreter::map_relation";

    static unsigned long long get_hash(std::string s);
    // bool pid_exist(unsigned long long pid);
    bool identifier_exist(unsigned long long iid);
    std::string escape(char ch);
    std::vector<std::string> separator(std::string &s);
    bool load();
    bool save();
public:
    CommandInterpreter();
    ~CommandInterpreter();
    bool FIRST_START = false;
    bool add_identifier(std::string identifier, unsigned long long pid);
    bool delete_identifier(std::string identifier);
    std::pair<unsigned long long, std::vector<std::string>> get_command();
    bool clear_data();
};



                        /* class Terminal */

unsigned long long CommandInterpreter::get_hash(std::string s) {
    unsigned long long hash = 0, seed = 13331;
    for (auto &ch : s) {
        hash = hash * seed + ch;
    }
    return hash;
}

// bool CommandInterpreter::pid_exist(unsigned long long pid) {
//     return pid_set.count(pid);
// }

bool CommandInterpreter::identifier_exist(unsigned long long iid) {
    return mp.count(iid);
}

std::vector<std::string> CommandInterpreter::separator(std::string &s) {
    std::vector<std::string> res;
    std::string tmp;
    for (int i = 0; i < s.size(); i++) {
        if (s[i] != ' ') tmp.push_back(s[i]);
        if (s[i] == ' ' || i == s.size() - 1) {
            if (!tmp.empty()) {
                res.push_back(tmp);
                tmp.clear();
            }
        }
    }
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
        data.back().push_back(std::to_string(it.first));
        data.back().push_back(std::to_string(it.second));
    }
    return saver.save(DATA_STORAGE_NAME, data);
}

bool CommandInterpreter::load() {
    vvs data;
    if (!saver.load(DATA_STORAGE_NAME, data)) return false;
    mp.clear();
    // pid_set.clear();
    for (auto &pr : data) {
        if (pr.size() != 2) {
            logger.log("Command interpreter: The mapping should be a pair, and there are no pairs in the data. Please check whether the data is complete.", Logger::WARNING, __LINE__);
            mp.clear();
            return false;
        }
        if (pr[0].empty() || pr[1].empty() || !saver.is_all_digits(pr[0]) || !saver.is_all_digits(pr[1])) {
            logger.log("Command interpreter: The identifier hash or the pid is not a valid number. Please check whether the data is complete.", Logger::WARNING, __LINE__);
            mp.clear();
            return false;
        }
        unsigned long long indntifier_hash = saver.str_to_ull(pr[0]);
        unsigned long long pid = saver.str_to_ull(pr[1]);
        if (identifier_exist(indntifier_hash)) {
            logger.log("Command interpreter: There are multiple identifiers in the data, please check whether the data is correct.", Logger::WARNING, __LINE__);
            mp.clear();
            return false;
        }
        // if (pid_exist(pid)) {
        //     logger.log("Command interpreter: There are multiple pids in the data, please check whether the data is correct.", Logger::WARNING, __LINE__);
        // }
        mp[indntifier_hash] = pid;
        // pid_set.insert(pid);
    }
    return true;
}

CommandInterpreter::CommandInterpreter() {
    if (!load()) FIRST_START = true;
}

CommandInterpreter::~CommandInterpreter() {
    save();
}

bool CommandInterpreter::add_identifier(std::string identifier, unsigned long long pid) {
    unsigned long long identifier_hash = get_hash(identifier);
    if (identifier_exist(identifier_hash)) {
        logger.log("Identifier " + identifier + " already exists. Please delete the original to add a new one.", Logger::WARNING, __LINE__);
        return false;
    }
    // if (pid_exist(pid)) {
    //     logger.log("Pid " + std::to_string(pid) + " already exists. Please delete the original to add a new one.", Logger::WARNING, __LINE__);
    //     return false;
    // }
    // pid_set.insert(pid);
    mp[identifier_hash] = pid;
    return true;
}

bool CommandInterpreter::delete_identifier(std::string identifier) {
    unsigned long long identifier_hash = get_hash(identifier);
    if (!identifier_exist(identifier_hash)) {
        logger.log("Identifier " + identifier + " does not exist.", Logger::WARNING, __LINE__);
        return false;
    }
    unsigned long long pid = mp[identifier_hash];
    // pid_set.erase(pid_set.find(pid));
    mp.erase(mp.find(identifier_hash));
    return true;
}

/**
 * Commands are separated by Spaces. Backslashes can be used to escape Spaces.
 */
std::pair<unsigned long long, std::vector<std::string>> CommandInterpreter::get_command() {
    std::string cmd;
    std::getline(std::cin, cmd);
    std::vector<std::string> separated_cmd = separator(cmd);
    std::vector<std::string> escaped_cmd(separated_cmd.size());
    for (int i = 0; i < separated_cmd.size(); i++) {
        std::string &cmd = separated_cmd[i];
        for (int j = 0; j < cmd.size(); j++) {
            if (cmd[j] != '\\') {
                escaped_cmd[i].push_back(cmd[j]);
            } else if (j == cmd.size() - 1) {
                escaped_cmd[i].push_back('\\');
            } else {
                std::string escaped = escape(cmd[j + 1]);
                if (escaped.empty()) {
                    // Unknown escape: keep the backslash and the character.
                    escaped_cmd[i].push_back('\\');
                    escaped_cmd[i].push_back(cmd[j + 1]);
                } else {
                    escaped_cmd[i] += escaped;
                }
                j++;
            }
        }
    }
    if (escaped_cmd.empty()) {
        return std::make_pair(NO_COMMAND, std::vector<std::string>());
    }
    unsigned long long identifier_hash = get_hash(escaped_cmd.front());
    if (!identifier_exist(identifier_hash)) {
        logger.log("Command not found: " + escaped_cmd.front(), Logger::WARNING, __LINE__);
        return std::make_pair(NO_COMMAND, escaped_cmd);
    } else {
        escaped_cmd.erase(escaped_cmd.begin());
        return std::make_pair(mp[identifier_hash], escaped_cmd);
    }
}

bool CommandInterpreter::clear_data() {
    mp.clear();
    // pid_set.clear();
    return true;
}

                        /* ===== test ===== */
int test_command_interpreter() {
// int main() {
    Logger &logger = Logger::get_logger();
    CommandInterpreter ci;
    // ci.add_identifier("wovinibaba", 1423);
    // ci.delete_identifier("wovinibaba");
    // ci.add_identifier("wovinidx", 1423);
    // ci.add_identifier("c ", 1323);
    // ci.add_identifier("c1", 13444);
    std::cout << "Done" << '\n';
    // return 0;
    while (true) {
        auto t = ci.get_command();
        if (t.first == NO_COMMAND) {
            std::cout << *logger.information << '\n';
        }
        std::cout << t.first << '\n';
        for (auto it : t.second) {
            std::cout << it << ' ';
        }
        std::cout << '\n';
    }
    return 0;
}

#endif
