/**
   ___ _                 _                      
  / __| |__   __ _ _ __ | |_    /\/\   ___  ___ 
 / /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\/\ |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/    \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee 
*/

#ifndef FILE_MANAGER_CPP
#define FILE_MANAGER_CPP

#include "logger.cpp"
#include "saver.cpp"
#include <cctype>
#include <string>
#include <map>
#include <climits>
#include <cstdlib>

struct fileNode {
    std::string content;
    unsigned long long cnt;

    fileNode() : content(""), cnt(0) {}
    fileNode(std::string content) : content(content), cnt(1) {}
};

class FileManager {
private:
    std::string DATA_STORAGE_NAME = "FileManager::map_relation";
    std::map<unsigned long long, fileNode> mp;
    // content -> fid, used to ensure files with the same content are stored only once.
    std::map<std::string, unsigned long long> content_map;
    Saver &saver = Saver::get_saver();
    Logger &logger = Logger::get_logger();
    bool loaded_ok = false;

    unsigned long long get_new_id();
    bool file_exist(unsigned long long fid);
    bool check_file(unsigned long long fid);
    bool save();
    bool load();

public:
    FileManager();
    ~FileManager();
    static FileManager& get_file_manager();
    unsigned long long create_file(std::string content="");
    bool increase_counter(unsigned long long fid);
    bool decrease_counter(unsigned long long fid);
    bool update_content(unsigned long long fid, unsigned long long &new_id, std::string content);
    bool get_content(unsigned long long fid, std::string &content);
    bool has_file(unsigned long long fid);
};



                        /* ====== FileManager ====== */
unsigned long long FileManager::get_new_id() {
    unsigned long long id;
    int tries = 0;
    do {
        id = 1ULL * rand() * rand() * rand();
        tries++;
        if (tries > 1000000) {
            logger.log("Failed to allocate a new file id after 1000000 attempts.", Logger::FATAL, __LINE__);
            return 0xffffffffffffffffULL;
        }
    } while (mp.count(id));
    return id;
}

bool FileManager::file_exist(unsigned long long fid) {
    if (!mp.count(fid)) {
        logger.log("File id " + std::to_string(fid) + " does not exists. This is not normal. Please check if the procedure is correct.", Logger::FATAL, __LINE__);
        return false;
    }
    return true;
}

bool FileManager::check_file(unsigned long long fid) {
    if (!file_exist(fid)) return false;
    if (mp[fid].cnt <= 0 ) {
        logger.log("File ID is " + std::to_string(fid) + " corresponding to the file, its counter is less than or equal to 0, this is abnormal, please check whether the program is correct.", Logger::FATAL, __LINE__);
        return false;
    }
    return true;
}

bool FileManager::has_file(unsigned long long fid) {
    return mp.count(fid) > 0;
}

bool FileManager::save() {
    vvs data;
    for (auto &it : mp) {
        data.push_back(std::vector<std::string>());
        data.back().push_back(std::to_string(it.first));
        data.back().push_back(it.second.content);
        data.back().push_back(std::to_string(it.second.cnt));
    }
    if (!saver.save(DATA_STORAGE_NAME, data)) return false;
    return true;
}

bool FileManager::load() {
    vvs data;
    if (!saver.load(DATA_STORAGE_NAME, data)) return false;
    mp.clear();
    content_map.clear();
    for (auto &it : data) {
        if (it.size() != 3) {
            logger.log("FileSystem: File is corrupted and cannot be read.", Logger::WARNING, __LINE__);
            mp.clear();
            return false;
        }
        if (!saver.is_all_digits(it[0]) || !saver.is_all_digits(it[2])) {
            logger.log("FileSystem: File is corrupted and cannot be read.", Logger::WARNING, __LINE__);
            mp.clear();
            return false;
        }
        if (it[0].size() > 20 || it[2].size() > 20) {
            logger.log("FileSystem: File is corrupted and cannot be read.", Logger::WARNING, __LINE__);
            mp.clear();
            return false;
        }
        unsigned long long key = saver.str_to_ull(it[0]);
        unsigned long long cnt_value = saver.str_to_ull(it[2]);
        if (cnt_value == 0) {
            logger.log("FileSystem: File is corrupted and cannot be read.", Logger::WARNING, __LINE__);
            mp.clear();
            return false;
        }
        if (mp.count(key)) {
            logger.log("FileSystem: File is corrupted and cannot be read.", Logger::WARNING, __LINE__);
            mp.clear();
            return false;
        }
        std::string &content = it[1];
        auto t = std::make_pair(key, fileNode(content));
        t.second.cnt = cnt_value;
        mp.insert(t);
        if (!content_map.count(content)) content_map[content] = key;
    }
    return true;
}

FileManager::FileManager() {
    loaded_ok = saver.exists(DATA_STORAGE_NAME) ? load() : true;
}

FileManager::~FileManager() {
    if (loaded_ok && !save()) {
        logger.log("Failed to save the file table on exit.", Logger::WARNING, __LINE__);
    }
}

FileManager& FileManager::get_file_manager() {
    static FileManager file_manager;
    return file_manager;
}

unsigned long long FileManager::create_file(std::string content) {
    if (content.size() > (size_t)INT_MAX / 4) {
        logger.log("The file content is too large to store.", Logger::WARNING, __LINE__);
        return 0xffffffffffffffffULL;
    }
    if (content_map.count(content)) {
        unsigned long long exist_id = content_map[content];
        if (!increase_counter(exist_id)) return 0xffffffffffffffffULL;
        return exist_id;
    }
    unsigned long long id = get_new_id();
    mp[id] = fileNode(content);
    content_map[content] = id;
    return id;
}

bool FileManager::increase_counter(unsigned long long fid) {
    if (!mp.count(fid)) {
        logger.log("File id does not exists. Please check if the procedure is correct.", Logger::FATAL, __LINE__);
        return false;
    }
    if (!check_file(fid)) return false;
    mp[fid].cnt ++;
    return true;
}

bool FileManager::decrease_counter(unsigned long long fid) {
    if (!mp.count(fid)) {
        logger.log("File id does not exists. Please check if the procedure is correct.", Logger::FATAL, __LINE__);
        return false;
    }
    if (!check_file(fid)) return false;
    if (--mp[fid].cnt <= 0) {
        std::string c = mp[fid].content;
        mp.erase(mp.find(fid));
        if (content_map.count(c) && content_map[c] == fid) content_map.erase(c);
    }
    return true;
}

bool FileManager::update_content(unsigned long long fid, unsigned long long &new_id, std::string content) {
    if (!file_exist(fid)) return false;
    if (content.size() > (size_t)INT_MAX / 4) {
        logger.log("The file content is too large to store.", Logger::WARNING, __LINE__);
        return false;
    }
    bool reused = false;
    if (content_map.count(content)) {
        new_id = content_map[content];
        reused = true;
    } else {
        new_id = get_new_id();
        mp[new_id].content = content;
        mp[new_id].cnt = 1;
        content_map[content] = new_id;
    }
    if (new_id == fid) return true;
    if (reused && !increase_counter(new_id)) {
        logger.log("Failed to increase the counter of the reused file.", Logger::WARNING, __LINE__);
        return false;
    }
    if (!decrease_counter(fid)) {
        if (reused) {
            decrease_counter(new_id);
        } else {
            mp.erase(new_id);
            if (content_map.count(content) && content_map[content] == new_id) content_map.erase(content);
        }
        return false;
    }
    return true;
}

bool FileManager::get_content(unsigned long long fid, std::string &content) {
    if (!file_exist(fid)) return false;
    content = mp[fid].content;
    return true;
}

#endif
