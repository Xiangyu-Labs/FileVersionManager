/**
   ___ _                 _                      
  / __| |__   __ _ _ __ | |_    /\/\   ___  ___ 
 / /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\/\ |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/    \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee 
*/

#ifndef TERMINAL_CPP
#define TERMINAL_CPP

#include "file_system.cpp"
#include "command_interpreter.cpp"
#include "logger.cpp"
#include "saver.cpp"
#include <cstdlib>
#include <cctype>
#include <map>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cstdio>
#include <csignal>
#include <unistd.h>

volatile sig_atomic_t g_interrupted = 0;

void on_sigint(int) {
    g_interrupted = 1;
}

enum PARA_TYPE {
    STR = 0, ULL
};

struct CommandSpec {
    const char *name;
    std::vector<PARA_TYPE> required;
    int max_extra;   // 最多允许的额外参数个数，-1 表示不限
};

/**
 * 命令表。pid 就是下标，顺序不能变（别名数据按 pid 持久化）。
 * max_extra 只对 rmf/rmd（多删）和 ls/create_version（可选参数）非 0。
 */
static const std::vector<CommandSpec> command_specs = {
    {"add_identifier", {STR, ULL}, 0},
    {"delete_identifier", {STR}, 0},
    {"switch_version", {ULL}, 0},
    {"touch", {STR}, 0},
    {"mkdir", {STR}, 0},
    {"cd", {STR}, 0},
    {"rmf", {STR}, -1},
    {"rmd", {STR}, -1},
    {"update_name", {STR, STR}, 0},
    {"update_content", {STR, STR}, 0},
    {"cat", {STR}, 0},
    {"tree", {}, 0},
    {"cdl", {}, 0},
    {"ls", {}, 1},
    {"create_version", {}, 2},
    {"version", {}, 0},
    {"gcv", {}, 0},
    {"init", {}, 0},
    {"clear", {}, 0},
    {"vim", {STR}, 0},
    {"pwd", {}, 0},
    {"find", {STR}, 0}
};

enum CMD_ID {
    CMD_ADD_IDENTIFIER = 0,
    CMD_DELETE_IDENTIFIER,
    CMD_SWITCH_VERSION,
    CMD_TOUCH,
    CMD_MKDIR,
    CMD_CD,
    CMD_RMF,
    CMD_RMD,
    CMD_UPDATE_NAME,
    CMD_UPDATE_CONTENT,
    CMD_CAT,
    CMD_TREE,
    CMD_CDL,
    CMD_LS,
    CMD_CREATE_VERSION,
    CMD_VERSION,
    CMD_GCV,
    CMD_INIT,
    CMD_CLEAR,
    CMD_VIM,
    CMD_PWD,
    CMD_FIND,
    CMD_COUNT
};

std::string ordinal(int n) {
    int d = n % 100;
    if (d >= 11 && d <= 13) return std::to_string(n) + "th";
    if (n % 10 == 1) return std::to_string(n) + "st";
    if (n % 10 == 2) return std::to_string(n) + "nd";
    if (n % 10 == 3) return std::to_string(n) + "rd";
    return std::to_string(n) + "th";
}

std::string plural(int n) {
    return n == 1 ? "" : "s";
}

// add_identifier：pid 必须在命令表范围内。
bool check_add_identifier_args(std::vector<std::string> &parameter) {
    if (Saver::str_to_ull(parameter[1]) >= command_specs.size()) {
        Logger::get_logger().log("There is no program numbered " + parameter[1] + ". Please check whether the configuration is correct.", Logger::WARNING, __LINE__);
        return false;
    }
    return true;
}

// ls：唯一的可选参数只能是 -a。
bool check_ls_args(std::vector<std::string> &parameter) {
    if (parameter.size() == 1 && parameter[0] != "-a") {
        Logger::get_logger().log("Invalid parameter for ls: " + parameter[0] + ". Only \"-a\" is accepted.", Logger::WARNING, __LINE__);
        return false;
    }
    return true;
}

// create_version：0~2 个参数，至少一个必须是数字版本号。
bool check_create_version_args(std::vector<std::string> &parameter) {
    Logger &logger = Logger::get_logger();
    if (parameter.size() >= 1 && Saver::is_all_digits(parameter[0]) && parameter[0].size() > 18) {
        logger.log("The 1st argument has a maximum of 18 digits. Check the input.", Logger::WARNING, __LINE__);
        return false;
    }
    if (parameter.size() == 2 && !Saver::is_all_digits(parameter[0]) && Saver::is_all_digits(parameter[1]) && parameter[1].size() > 18) {
        logger.log("The 2nd argument has a maximum of 18 digits. Check the input.", Logger::WARNING, __LINE__);
        return false;
    }
    if (parameter.size() == 2 && !Saver::is_all_digits(parameter[0]) && !Saver::is_all_digits(parameter[1])) {
        logger.log("At least one of the two parameters of create_version must be an integer version number.", Logger::WARNING, __LINE__);
        return false;
    }
    return true;
}

class Terminal : private CommandInterpreter {
private:
   FileSystem file_system;
   Logger &logger = Logger::get_logger();

   bool check_parameters(unsigned long long pid, std::vector<std::string> &parameter);
   bool execute(unsigned long long pid, std::vector<std::string> parameter);
   bool initialize();
   void print_slash_path(const std::vector<std::string> &p);
   bool run_vim(std::string name);

public:
   Terminal();
   int run();
};

bool Terminal::check_parameters(unsigned long long pid, std::vector<std::string> &parameter) {
   if (pid >= command_specs.size()) {
      logger.log("There is no program numbered " + std::to_string(pid) + ". Please check whether the configuration is correct.", Logger::WARNING, __LINE__);
      return false;
   }
   const CommandSpec &spec = command_specs[pid];
   size_t min = spec.required.size();
   if (parameter.size() < min) {
      logger.log("Parameters are insufficient. " + std::to_string(min) + " parameter" + plural((int)min)
         + " were required but only " + std::to_string(parameter.size()) + " parameter" + plural((int)parameter.size()) + " were provided.", Logger::WARNING, __LINE__);
      return false;
   }
   if (spec.max_extra >= 0) {
      size_t max = min + (size_t)spec.max_extra;
      if (parameter.size() > max) {
         logger.log("The " + std::string(spec.name) + " command accepts at most " + std::to_string(max) + " parameter"
            + plural((int)max) + ", but " + std::to_string(parameter.size()) + " were provided.", Logger::WARNING, __LINE__);
         return false;
      }
   }
   for (size_t i = 0; i < min; i++) {
      if (spec.required[i] == ULL) {
         if (!Saver::is_all_digits(parameter[i])) {
            logger.log("The " + ordinal((int)i + 1) + " parameter must be an integer. Check the input.", Logger::WARNING, __LINE__);
            return false;
         }
         if (parameter[i].size() > 18) {
            logger.log("The " + ordinal((int)i + 1) + " argument has a maximum of 18 digits. Check the input.", Logger::WARNING, __LINE__);
            return false;
         }
      }
   }
   return true;
}

bool Terminal::execute(unsigned long long pid, std::vector<std::string> parameter) {
   if (!check_parameters(pid, parameter)) return false;

   // 三个命令有各自的附加参数规则。
   if (pid == CMD_ADD_IDENTIFIER && !check_add_identifier_args(parameter)) return false;
   if (pid == CMD_LS && !check_ls_args(parameter)) return false;
   if (pid == CMD_CREATE_VERSION && !check_create_version_args(parameter)) return false;

   switch (pid) {
      case CMD_ADD_IDENTIFIER:
         if (!add_identifier(parameter[0], Saver::str_to_ull(parameter[1]))) return false;
         std::cout << "An identifier was successfully added for program " << Saver::str_to_ull(parameter[1]) << "." << '\n';
         break;

      case CMD_DELETE_IDENTIFIER:
         if (!delete_identifier(parameter[0])) return false;
         std::cout << "An identifier was successfully deleted." << '\n';
         break;

      case CMD_SWITCH_VERSION:
         if (!file_system.switch_version(Saver::str_to_ull(parameter[0]))) return false;
         std::cout << "Switched to version " + parameter[0] << '\n';
         break;

      case CMD_TOUCH:
         if (!file_system.make_file(parameter[0])) return false;
         break;

      case CMD_MKDIR:
         if (!file_system.make_dir(parameter[0])) return false;
         break;

      case CMD_CD:
         if (!file_system.change_directory(parameter[0])) return false;
         break;

      case CMD_RMF:
         for (auto &file_name : parameter) {
            if (!file_system.remove_file(file_name)) {
               std::cout << logger.information << '\n';
            } else {
               std::cout << "Removed " << file_name << '\n';
            }
         }
         break;

      case CMD_RMD:
         for (auto &file_name : parameter) {
            if (!file_system.remove_dir(file_name)) {
               std::cout << logger.information << '\n';
            } else {
               std::cout << "Removed " << file_name << '\n';
            }
         }
         break;

      case CMD_UPDATE_NAME:
         if (!file_system.update_name(parameter[0], parameter[1])) return false;
         break;

      case CMD_UPDATE_CONTENT:
         if (!file_system.update_content(parameter[0], parameter[1])) return false;
         break;

      case CMD_CAT:
      {
         std::string content;
         if (!file_system.get_content(parameter[0], content)) return false;
         std::cout << content;
         if (content.empty() || content.back() != '\n') std::cout << '\n';
      }
      break;

      case CMD_TREE:
      {
         std::string tree_content;
         if (!file_system.tree(tree_content)) return false;
         std::cout << tree_content;
         if (tree_content.empty() || tree_content.back() != '\n') std::cout << '\n';
      }
      break;

      case CMD_CDL:
         if (!file_system.goto_last_dir()) return false;
         break;

      case CMD_LS:
      {
         std::vector<std::string> ls_content;
         if (!file_system.list_directory_contents(ls_content)) return false;
         if (ls_content.empty()) {
            std::cout << "The folder is empty.  QAQ" << '\n';
            break;
         }
         std::sort(ls_content.begin(), ls_content.end());
         if (parameter.size() == 0 || parameter[0] != "-a") {
            for (size_t i = 0; i < ls_content.size(); i++) {
               if (i != 0 && i % 8 == 0) std::cout << '\n';
               std::cout << ls_content[i] << "\t";
            }
            std::cout << '\n';
         } else {
            std::cout << "type\t" << "create time\t\t" << "update time\t\t" << "name" << '\n';
            for (auto &content : ls_content) {
               treeNode::TYPE type;
               std::string create_time, update_time;
               if (!file_system.get_type(content, type)) return false;
               if (!file_system.get_create_time(content, create_time)) return false;
               if (!file_system.get_update_time(content, update_time)) return false;
               std::cout << (type == treeNode::FILE ? "file" : "dir") << '\t' << create_time << '\t' << update_time << '\t' << content << '\n';
            }
         }
      }
      break;

      case CMD_CREATE_VERSION:
         if (parameter.size() == 0) {
            if (!file_system.create_version()) return false;
         } else if (parameter.size() == 1) {
            if (Saver::is_all_digits(parameter[0])) {
               if (!file_system.create_version(Saver::str_to_ull(parameter[0]))) return false;
            } else {
               if (!file_system.create_version(NO_MODEL_VERSION, parameter[0])) return false;
            }
         } else {
            if (Saver::is_all_digits(parameter[0])) {
               if (!file_system.create_version(Saver::str_to_ull(parameter[0]), parameter[1])) return false;
            } else {
               if (!file_system.create_version(Saver::str_to_ull(parameter[1]), parameter[0])) return false;
            }
         }
         break;

      case CMD_VERSION:
      {
         std::vector<std::pair<unsigned long long, versionNode>> version_content;
         if (!file_system.version(version_content)) return false;
         std::cout << "version id" << '\t' << "information" << '\n';
         for (auto &it : version_content) {
            std::cout << it.first << "\t\t" << (it.second.info == "" ? "NULL" : it.second.info) << '\n';
         }
      }
      break;

      case CMD_GCV:
         std::cout << "The current version of the file system is " << file_system.get_current_version() << '\n';
         break;

      case CMD_INIT:
         if (!initialize()) return false;
         break;

      case CMD_CLEAR:
         // 直接输出 ANSI 清屏码，不依赖外部 clear 程序；管道输出保持干净。
         if (isatty(STDOUT_FILENO)) std::cout << "\033[2J\033[H";
         break;

      case CMD_VIM:
         if (!run_vim(parameter[0])) return false;
         break;

      case CMD_PWD:
      {
         std::vector<std::string> path;
         if (!file_system.get_current_path(path)) return false;
         print_slash_path(path);
      }
      break;

      case CMD_FIND:
      {
         std::vector<std::pair<std::string, std::vector<std::string>>> res;
         if (!file_system.Find(parameter[0], res)) return false;
         std::cout << "name\t" << "path" << '\n';
         for (auto &r : res) {
            std::cout << r.first << '\t';
            print_slash_path(r.second);
         }
      }
      break;
   }
   return true;
}

void Terminal::print_slash_path(const std::vector<std::string> &p) {
   std::cout << '/';
   for (auto &s : p) {
      std::cout << s << '/';
   }
   std::cout << '\n';
}

bool Terminal::run_vim(std::string name) {
   std::string file_name = "temp_file_vim_" + std::to_string(getpid());
   std::remove(file_name.c_str());
   std::ofstream touch_out(file_name);
   if (!touch_out.is_open()) {
      logger.log("Failed to create the temporary file for vim.", Logger::WARNING, __LINE__);
      return false;
   }
   touch_out.close();
   std::string original_content;
   bool existed = file_system.get_content(name, original_content);
   if (existed) {
      std::ofstream out(file_name, std::ios_base::trunc);
      if (!out.is_open()) {
         logger.log("Failed to write the temporary file for vim.", Logger::WARNING, __LINE__);
         std::remove(file_name.c_str());
         return false;
      }
      out << original_content;
      out.flush();
      if (!out.good()) {
         logger.log("Failed to write the temporary file for vim.", Logger::WARNING, __LINE__);
         std::remove(file_name.c_str());
         return false;
      }
      out.close();
   }
   std::string cmd = "vim " + file_name;
   if (system(cmd.c_str()) != 0) {
      logger.log("Failed to run vim.", Logger::WARNING, __LINE__);
      std::remove(file_name.c_str());
      return false;
   }
   std::ifstream in(file_name);
   if (!in.is_open()) {
      logger.log("Failed to read the temporary file after vim.", Logger::WARNING, __LINE__);
      std::remove(file_name.c_str());
      return false;
   }
   // 逐字节读取，保留文件原始内容（getline 会给末尾补 '\n'，导致未修改的文件也被判为改动）。
   std::string content;
   char ch;
   while (in.get(ch)) content.push_back(ch);
   in.close();
   std::remove(file_name.c_str());
   if (!existed && content.empty()) return true;
   if (existed && content == original_content) return true;
   if (!existed && !file_system.make_file(name)) return false;
   if (!file_system.update_content(name, content)) return false;
   return true;
}

bool Terminal::initialize() {
   clear_data();
   for (size_t i = 0; i < command_specs.size(); i++) {
      add_identifier(command_specs[i].name, i);
   }
   return true;
}

Terminal::Terminal() {
   if (FIRST_START) {
      initialize();
   }
}

int Terminal::run() {
   // 不用 signal()：macOS 上它默认 SA_RESTART，阻塞中的 read 不会被打断，
   // Ctrl+C 就永远等不到退出。sigaction 关掉 SA_RESTART 后 read 返回 EINTR，
   // getline 失败，主循环走正常退出路径（析构保存）。
   struct sigaction sa;
   sa.sa_handler = on_sigint;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = 0;
   sigaction(SIGINT, &sa, nullptr);

   // 只有交互式终端才打印提示符；管道输入时保持输出干净（和真实 shell 一致）。
   bool interactive = isatty(STDIN_FILENO);
   std::pair<unsigned long long, std::vector<std::string>> cmd;
   while (true) {
      if (g_interrupted) {
         // Ctrl+C：先换行再退出，避免 shell 提示符粘在 "# " 后面。
         if (interactive) std::cout << '\n' << std::flush;
         return 0;
      }
      if (interactive) std::cout << "# " << std::flush;
      cmd = get_command();
      if (g_interrupted) {
         if (interactive) std::cout << '\n' << std::flush;
         return 0;
      }
      if (cmd.first == NO_COMMAND) {
         // exit/quit 是保留字，get_command 里直接识别，不报 Command not found。
         if (!cmd.second.empty()) {
            if (cmd.second.front() == "exit" || cmd.second.front() == "quit") {
               return 0;
            }
            std::cout << logger.information << '\n';
         } else if (!std::cin.good()) {
            // 真正的 EOF（包括"最后一行没有换行符"时 getline 成功但置了 eofbit 的情况）；
            // 空行时 cin 仍然是 good 的，继续等待下一条命令。
            return 0;
         }
      } else {
         if (!execute(cmd.first, cmd.second)) {
            std::cout << logger.information << '\n';
         }
      }
   }
   return 0;
}

#endif
