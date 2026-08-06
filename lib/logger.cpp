/**
   ___ _                 _                      
  / __| |__   __ _ _ __ | |_    /\/\   ___  ___ 
 / /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\/\ |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/    \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee 
*/

#ifndef LOGGER_CPP
#define LOGGER_CPP

#include <ctime>
#include <string>
#include <fstream>
#include <iostream>

#define ll Logger::get_logger().log("===== 11111 =====", Logger::DEBUG, __LINE__);
#define rr Logger::get_logger().log("===== 22222 =====", Logger::DEBUG, __LINE__);
#define wa Logger::get_logger().log("===== WARN =====", Logger::DEBUG, __LINE__);

class Logger {
private:
    // Set the log file name here
    std::string log_file = "log.chm";

    // Gets the current time as a string
    std::string get_time();

public:
    Logger();

    /**
     * Unless there is no other processing method, the User-oriented and 
     * developer-oriented functions in the program only returns true or false 
     * to indicate whether the requested operation is successfully executed.
     * 
     * If the execution is successful and the function has a return value, 
     * then the return value will be stored in this variable; if it is not 
     * successfully executed, the reason for not being successfully executed 
     * will be stored in this variable.
    */
    std::string *information;

    /**
     * This function can be used in multiple places to obtain a logger.
     * Log management is implemented by the Logger class. 
     * You only need to correctly mark the log level of the information that 
     * needs to be recorded and the line number where the information is 
     * located when necessary, and output it through the log function.
    */
    static Logger& get_logger();

    /**
    * An explanation of the four types of log registration:
    * INFO: 
    * Normal logs are used to record what operations the program has performed.
    * At this level, logs are only printed to log files and not to the console.
    * 
    * DEBUG: 
    * This level is mainly used when debugging programs.
    * Using this level will output information to the console using cerr.
    * 
    * WARNING:
    * This level is used when the user does something wrong.
    * For example, if the user tries to access a folder that does not exist, the
    * function will only return false, because it is printed this way.
    * To find out exactly what happened, you can use the logger's "information" variable.
    * 
    * FATAL:
    * This level records information that may cause the program to crash.
    * At this level, information will be forced to be output to the console 
    * through cerr and the log level will be marked.
    * Due to the limited level of the author, errors of this level are 
    * almost never caught. QAQ
    */
    enum LOG_LEVEL {
        INFO = 0, DEBUG, WARNING, FATAL
    };

    /**
     * Use this function to record logs. 
     * "content" is the content to be recorded, and "level" is the log level. 
     * For a detailed introduction to "level", please see above.
     * If you use a log level other than INFO, you must provide the line 
     * number where the log function is called, that is, __LINE__.
    */
    void log(std::string content, LOG_LEVEL level=INFO, int line=__LINE__);
};



                        /* ======= class Logger ======= */

std::string Logger::get_time() {
    static char t[100];
    time_t timep;
    time(&timep);
    struct tm* p = localtime(&timep);
    sprintf(t, "%d-%02d-%02d %02d:%02d:%02d", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
    return std::string(t);
}

Logger::Logger() {
    information = new std::string();
}

Logger& Logger::get_logger() {
    static Logger logger;
    return logger;
}

void Logger::log(std::string content, LOG_LEVEL level, int line) {
    static std::ofstream out(log_file, std::ios_base::app);
    *information = std::string(' ' + content);
    std::string app_tm = "(" + get_time() + ")" + *information;
    if (level == INFO) {
        out << "level: INFO " << '\n' << app_tm << std::endl;
    } else if (level == DEBUG) {
        out << "level: DEBUG " << '\n' << "line: " << line << ' ' << app_tm << std::endl;
        std::cerr << "line: " << line << ' ' << app_tm << std::endl;
    } else if (level == WARNING) {
        out << "level: WARNING " << '\n' << "line: " << line << ' ' << app_tm << std::endl;
    } else {
        out << "level: FATAL " << '\n' << "line: " << line << ' ' << app_tm << std::endl;
        std::cerr << "level: FATAL " << '\n' << "line: " << line << ' ' << app_tm << std::endl;
    }
}

#endif
