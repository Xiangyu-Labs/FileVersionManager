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

class Terminal : private CommandInterpreter {
private:
   FileSystem file_system;
   Logger &logger = Logger::get_logger();

   enum PARA_TYPE {
      STR = 0, INT, ULL
   };

   /**
    * Add the requirements for function parameters to the constructor and 
    * implement the call in the execute function.
    * 
    * Default program number     nickname
    * 0: add_identifier          add_identifier
    * 1: delete_identifier       delete_identifier
    * 2: switch_version          switch_version
    * 3: make_file               touch
    * 4: make_dir                mkdir
    * 5: change_directory        cd
    * 6: remove_file             rmf
    * 7: remove_dir              rmd
    * 8: update_name             update_name
    * 9: update_content          update_content
    * 10: get_content            cat
    * 11: tree                   tree
    * 12: goto_last_dir          cdl
    * 13: list_directory_content ls
    * 14: create_version         create_version
    * 15: version                version
    * 16: get_current_version    gcv
    * 17: init                   init
    * 18: clear                  clear
    * 19: vim                    vim
    * 20: get_current_path       pwd
    * 21: find                   find
    */
   std::vector<std::vector<PARA_TYPE>> function_requirement;
   bool execute(unsigned long long pid, std::vector<std::string> parameter);
   bool initialize();

public:
   Terminal();
   int run();
};

bool Terminal::execute(unsigned long long pid, std::vector<std::string> parameter) {
   if (pid >= function_requirement.size()) {
      logger.log("There is no program numbered " + std::to_string(pid) + ". Please check whether the configuration is correct.", Logger::WARNING, __LINE__);
      return false;
   }
   std::vector<PARA_TYPE> &fr = function_requirement[pid];
   if (parameter.size() < fr.size()) {
      logger.log("Parameters are insufficient. " + std::to_string(fr.size()) + " parameters were required but only " + std::to_string(parameter.size()) + " were provided.", Logger::WARNING, __LINE__);
      return false;
   }
   bool extra_parameter_allowed = (pid == 6 || pid == 7 || pid == 13 || pid == 14);
   if (!extra_parameter_allowed && parameter.size() > fr.size()) {
      logger.log("Too many parameters. " + std::to_string(fr.size()) + " parameters were required but " + std::to_string(parameter.size()) + " were provided.", Logger::WARNING, __LINE__);
      return false;
   }
   if (pid == 13 && parameter.size() > 1) {
      logger.log("The ls command accepts at most 1 parameter, but " + std::to_string(parameter.size()) + " were provided.", Logger::WARNING, __LINE__);
      return false;
   }
   if (pid == 13 && parameter.size() == 1 && parameter[0] != "-a") {
      logger.log("Invalid parameter for ls: " + parameter[0] + ". Only \"-a\" is accepted.", Logger::WARNING, __LINE__);
      return false;
   }
   if (pid == 14 && parameter.size() > 2) {
      logger.log("The create_version command accepts at most 2 parameters, but " + std::to_string(parameter.size()) + " were provided.", Logger::WARNING, __LINE__);
      return false;
   }
   for (size_t i = 0; i < fr.size(); i++) {
      if (fr[i] == INT || fr[i] == ULL) {
         if (!Saver::is_all_digits(parameter[i])) {
            logger.log("The " + std::to_string(i) + "th parameter must be an integer. Check the input.", Logger::WARNING, __LINE__);
            return false;
         }
         if (fr[i] == INT && parameter[i].size() > 9) {
            logger.log("The " + std::to_string(i) + "th argument has a maximum of 9 digits. Check the input.", Logger::WARNING, __LINE__);
            return false;
         }

         if (fr[i] == ULL && parameter[i].size() > 18) {
            logger.log("The " + std::to_string(i) + "th argument has a maximum of 18 digits. Check the input.", Logger::WARNING, __LINE__);
            return false;
         }
      }
   }


   std::string get_content_content;       // case 10
   std::string tree_content;              // case 11
   std::vector<std::string> ls_content;   // case 13
   std::vector<std::pair<unsigned long long, versionNode>> version_content; // case 15
   std::string file_name;                 // case 19
   std::string cmd;                       // case 19
   std::string content;                   // case 19
   std::string tmp;                       // case 19
   std::ifstream in;                      // case 19
   std::vector<std::string> path;         // case 20
   std::vector<std::pair<std::string, std::vector<std::string>>> res;      // case 21


   switch (pid) {
      case 0: 
      if (Saver::str_to_ull(parameter[1]) >= function_requirement.size()) {
         logger.log("There is no program numbered " + parameter[1] + ". Please check whether the configuration is correct.", Logger::WARNING, __LINE__);
         return false;
      }
      if (!add_identifier(parameter[0], Saver::str_to_ull(parameter[1]))) return false;
      else std::cout << "An identifier was successfully added for program " << Saver::str_to_ull(parameter[1]) << "." << '\n';
      break;
      case 1:
      if (!delete_identifier(parameter[0])) return false;
      else std::cout << "An identifier was successfully deleted." << '\n';
      break;

      case 2:
      if (!file_system.switch_version(Saver::str_to_ull(parameter[0]))) return false;
      else std::cout << "Switched to version " + parameter[0] << '\n';
      break;

      case 3:
      if (!file_system.make_file(parameter[0])) return false;
      break;

      case 4:
      if (!file_system.make_dir(parameter[0])) return false;
      break;

      case 5:
      if (!file_system.change_directory(parameter[0])) return false;
      break;

      case 6:
      for (auto &file_name : parameter) {
         if (!file_system.remove_file(file_name)) {
            std::cout << logger.information << '\n';
         } else {
            std::cout << "Removed " << file_name << '\n';
         }
      }
      break;

      case 7:
      for (auto &file_name : parameter) {
         if (!file_system.remove_dir(file_name)) {
            std::cout << logger.information << '\n';
         } else {
            std::cout << "Removed " << file_name << '\n';
         }
      }
      break;

      case 8:
      if (!file_system.update_name(parameter[0], parameter[1])) return false;
      break;

      case 9:
      if (!file_system.update_content(parameter[0], parameter[1])) return false;
      break;

      case 10:
      if (!file_system.get_content(parameter[0], get_content_content)) return false;
      else {
         std::cout << get_content_content;
         if (get_content_content.empty() || get_content_content.back() != '\n') std::cout << '\n';
      }
      break;

      case 11:
      if (!file_system.tree(tree_content)) return false;
      std::cout << tree_content;
      if (tree_content.empty() || tree_content.back() != '\n') std::cout << '\n';
      break;

      case 12:
      if (!file_system.goto_last_dir()) return false;
      break;

      case 13:
      if (!file_system.list_directory_contents(ls_content)) return false;
      if (ls_content.empty()) {
         std::cout << "The folder is empty.  QAQ" << '\n';
         break;
      }

      if (parameter.size() == 0 || parameter[0] != "-a") {
         std::sort(ls_content.begin(), ls_content.end());
         for (size_t i = 0; i < ls_content.size(); i++) {
            if (i != 0 && i % 8 == 0) std::cout << '\n';
            std::cout << ls_content[i] << "\t";
         }
         std::cout << '\n';
      } else {
         std::sort(ls_content.begin(), ls_content.end());
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
      break;

      case 14:
      if (parameter.size() >= 1 && Saver::is_all_digits(parameter[0]) && parameter[0].size() > 18) {
         logger.log("The 0th argument has a maximum of 18 digits. Check the input.", Logger::WARNING, __LINE__);
         return false;
      }
      if (parameter.size() == 2 && !Saver::is_all_digits(parameter[0]) && Saver::is_all_digits(parameter[1]) && parameter[1].size() > 18) {
         logger.log("The 1th argument has a maximum of 18 digits. Check the input.", Logger::WARNING, __LINE__);
         return false;
      }
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
         } else if (Saver::is_all_digits(parameter[1])) {
            if (!file_system.create_version(Saver::str_to_ull(parameter[1]), parameter[0])) return false;
         } else {
            logger.log("At least one of the two parameters of create_version must be an integer version number.", Logger::WARNING, __LINE__);
            return false;
         }
      }
      break;

      case 15:
      if (!file_system.version(version_content)) return false;
      std::cout << "version id" << '\t' << "information" << '\n';
      for (auto &it : version_content) {
         std::cout << it.first << "\t\t" << (it.second.info == "" ? "NULL" : it.second.info) << '\n';
      }
      break;

      case 16:
      std::cout << "The current version of the file system is " << file_system.get_current_version() << '\n';
      break;

      case 17:
      if (!initialize()) return false;
      break;

      case 18:
      system("clear");
      break;

      case 19:
      {
         file_name = "temp_file_vim_" + std::to_string(getpid());
         std::remove(file_name.c_str());
         std::ofstream touch_out(file_name);
         if (!touch_out.is_open()) {
            logger.log("Failed to create the temporary file for vim.", Logger::WARNING, __LINE__);
            return false;
         }
         touch_out.close();
         std::string original_content;
         bool existed = file_system.get_content(parameter[0], original_content);
         if (existed) {
            std::ofstream out(file_name, std::ios_base::trunc);
            out << original_content;
            out.flush();
            if (!out.good()) {
               logger.log("Failed to write the temporary file for vim.", Logger::WARNING, __LINE__);
               std::remove(file_name.c_str());
               return false;
            }
            out.close();
         }
         cmd = "vim " + file_name;
         if (system(cmd.c_str()) != 0) {
            logger.log("Failed to run vim.", Logger::WARNING, __LINE__);
            std::remove(file_name.c_str());
            return false;
         }
         in.open(file_name);
         if (!in.is_open()) {
            logger.log("Failed to read the temporary file after vim.", Logger::WARNING, __LINE__);
            std::remove(file_name.c_str());
            return false;
         }
         content.clear();
         while (std::getline(in, tmp)) {
            content += tmp;
            content.push_back('\n');
         }
         in.close();
         std::remove(file_name.c_str());
         if (!existed && content.empty()) return true;
         if (existed && content == original_content) return true;
         if (!existed && !file_system.make_file(parameter[0])) return false;
         if (!file_system.update_content(parameter[0], content)) return false;
      }
      break;

      case 20:
      if (!file_system.get_current_path(path)) return false;
      std::cout << '/';
      for (auto &p : path) {
         std::cout << p << '/';
      }
      std::cout << '\n';
      break;

      case 21:
      if (!file_system.Find(parameter[0], res)) return false;
      std::cout << "name\t" << "path" << '\n';
      for (auto &r : res) {
         std::cout << r.first << '\t';
         std::cout << '/';
         for (auto &s : r.second) {
            std::cout << s << '/';
         }
         std::cout << '\n';
      }
      break;
   }
   return true;
}

bool Terminal::initialize() {
   clear_data();
   add_identifier("add_identifier", 0);
   add_identifier("delete_identifier", 1);
   add_identifier("switch_version", 2);
   add_identifier("touch", 3);
   add_identifier("mkdir", 4);
   add_identifier("cd", 5);
   add_identifier("rmf", 6);
   add_identifier("rmd", 7);
   add_identifier("update_name", 8);
   add_identifier("update_content", 9);
   add_identifier("cat", 10);
   add_identifier("tree", 11);
   add_identifier("cdl", 12);
   add_identifier("ls", 13);
   add_identifier("create_version", 14);
   add_identifier("version", 15);
   add_identifier("gcv", 16);
   add_identifier("init", 17);
   add_identifier("clear", 18);
   add_identifier("vim", 19);
   add_identifier("pwd", 20);
   add_identifier("find", 21);
   return true;
}

Terminal::Terminal() {
   // add_identifier
   function_requirement.push_back(std::vector<PARA_TYPE>({STR, ULL}));
   // delete_identifier
   function_requirement.push_back(std::vector<PARA_TYPE>({STR}));
   // switch_version
   function_requirement.push_back(std::vector<PARA_TYPE>({ULL}));
   // make_file
   function_requirement.push_back(std::vector<PARA_TYPE>({STR}));
   // make_dir
   function_requirement.push_back(std::vector<PARA_TYPE>({STR}));
   // change_directory
   function_requirement.push_back(std::vector<PARA_TYPE>({STR}));
   // remove_file
   function_requirement.push_back(std::vector<PARA_TYPE>({STR}));
   // remove_dir
   function_requirement.push_back(std::vector<PARA_TYPE>({STR}));
   // update_name
   function_requirement.push_back(std::vector<PARA_TYPE>({STR, STR}));
   // update_content
   function_requirement.push_back(std::vector<PARA_TYPE>({STR, STR}));
   // get_content
   function_requirement.push_back(std::vector<PARA_TYPE>({STR}));
   // tree
   function_requirement.push_back(std::vector<PARA_TYPE>());
   // goto_last_dir
   function_requirement.push_back(std::vector<PARA_TYPE>());
   // list_directory_content
   function_requirement.push_back(std::vector<PARA_TYPE>());
   // create_version
   function_requirement.push_back(std::vector<PARA_TYPE>());
   // version
   function_requirement.push_back(std::vector<PARA_TYPE>());
   // get_current_version
   function_requirement.push_back(std::vector<PARA_TYPE>());
   // init
   function_requirement.push_back(std::vector<PARA_TYPE>());
   // clear
   function_requirement.push_back(std::vector<PARA_TYPE>());
   // vim
   function_requirement.push_back(std::vector<PARA_TYPE>({STR}));
   // pwd
   function_requirement.push_back(std::vector<PARA_TYPE>());
   // find
   function_requirement.push_back(std::vector<PARA_TYPE>({STR}));

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
   std::pair<unsigned long long, std::vector<std::string>> cmd;
   while (true) {
      if (g_interrupted || !std::cin.good()) return 0;
      std::cout << "# ";
      cmd = get_command();
      if (g_interrupted || !std::cin.good()) return 0;
      if (cmd.first == NO_COMMAND) {
         // exit/quit 是保留字，add_identifier 拒绝注册，所以它们永远走 NO_COMMAND 分支。
         if (!cmd.second.empty() && (cmd.second.front() == "exit" || cmd.second.front() == "quit")) {
            return 0;
         }
         if (!cmd.second.empty()) {
            std::cout << logger.information << '\n';
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
