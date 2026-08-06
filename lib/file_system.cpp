/**
   ___ _                 _                      
  / __| |__   __ _ _ __ | |_    /\/\   ___  ___ 
 / /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\/\ |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/    \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee 
*/

#ifndef FILE_SYSTEM_CPP
#define FILE_SYSTEM_CPP

#include "version_manager.cpp"
#include "bs_tree.cpp"
#include "node_manager.cpp"
#include "saver.cpp"
#include "logger.cpp"
#include <ctime>
#include <string>
#include <stack>
#include <vector>
#include <climits>

/**
 * @brief 
 * This part is the core part of the system (of course, no other part of the system can 
 * not run. QAQ)
 * This class mainly encapsulates the functions of other classes. Except for the Terminal 
 * class, this class is the class closest to the user. Users can use various functions of the 
 * file system by creating objects of this class, but it is not very convenient.
 */
class FileSystem : private BSTree {
private:
    VersionManager version_manager;
    Logger &logger = Logger::get_logger();
    NodeManager &node_manager = NodeManager::get_node_manager();
    int CURRENT_VERSION;

    /**
     * @brief 
     * Decrement the counter of the tree node corresponding to the pointer by one. 
     * If the counter decreases to 0 then the node will be deleted.
     * 
     * @param p 
     * Pointer of the node to decrease the counter by one.
     * 
     * @return true 
     * Successfully decrement the node's counter by one.
     * 
     * @return false 
     * If the check_node function, node_mamager.delete_node function returns an error, then 
     * this function will also return an error.
     */
    bool decrease_counter(treeNode *p);

    /**
     * @brief 
     * This function is used in conjunction with the remove_dir function.
     * 
     * @param p 
     * The root node of the tree to be deleted.
     * 
     * @param delete_brother 
     * If this variable is True, then the sibling node corresponding to the node will be 
     * deleted, otherwise it will not be deleted.
     * 
     * @return true 
     * The subtree was successfully deleted.
     * 
     * @return false 
     * A null pointer is encountered, but if this is not the function called by the user, it 
     * is normal to return False. Because this function will inevitably recurse to the 
     * boundary of the tree.
     */
    bool recursive_delete_nodes(treeNode *p, bool delete_brother=false);

    /**
     * @brief 
     * In theory, this function can only be used with the remove_file function.
     * 
     * @return true 
     * The node corresponding to path.back() is successfully deleted.
     * 
     * @return false 
     * If the check_path function, check_node function, rebuild_nodes function, and 
     * decrease_counter function return an error, then this function will also return an error.
     */
    bool delete_node();

    /**
     * @brief 
     * Super invincible core function!!!!!!!!
     * This function will look forward from the end of the path until it finds a node whose 
     * counter is 1 and rebuilds all the nodes along the way, decrementing the original node 
     * counter along the way by one.
     * 
     * @param p 
     * After the node is rebuilt, the next_brother of the last node of the path needs to be 
     * reset, and p is the value that needs to be reset.
     * 
     * @return true 
     * The nodes were successfully rebuilt.
     * 
     * @return false 
     * If the check_path function, check_node function, and decrease_counter function return 
     * an error, then this function will also return an error.
     */
    bool rebuild_nodes(treeNode *p);

    /**
     * @brief 
     * This function is used in conjunction with the tree function.
     * 
     * @param p 
     * The root node of the tree to be traversed.
     * 
     * @param tree_info 
     * The structure information of the tree will be stored in tree_info.
     * Note that tree_info is passed in the variable by reference.
     * 
     * @return true 
     * The subtree is successfully traversed, and the result of the traversal is stored in 
     * tree_info.
     * 
     * @return false 
     * A null pointer was encountered. QAQ
     * But don't worry, if the user does not call the function, it will return an error. This 
     * is normal, because this function will inevitably recurse to the boundary of the tree.
     */
    bool travel_tree(treeNode *p, std::string &tree_info);

    /**
     * @brief 
     * This function is used in conjunction with the find function.
     * 
     * @param name 
     * The name to search for.
     * 
     * @param res 
     * The results of the search are stored in this array.
     * 
     * @return true 
     * @return false 
     * This function currently does not think of where it will go wrong, so now it all 
     * returns True.
     */
    bool travel_find(std::string name, std::vector<std::pair<std::string, std::vector<std::string>>> &res);

    /**
     * @brief 
     * Use with travel_find function.
     * 
     * @param str 
     * The target string.
     * 
     * @param tar 
     * The matching string.
     * 
     * @return true 
     * found it! ! Hahaha
     * 
     * @return false 
     * did not find. . . QAQ
     */
    bool kmp(std::string str, std::string tar);

public:
    FileSystem();

    /**
     * @brief 
     * Use this function to switch between different file versions.
     * 
     * @param version_id 
     * The number of the version you want to switch. You can get the current version through 
     * the get_current_version function, or you can get all the version information through 
     * the version function.
     * 
     * @return true 
     * The file version was successfully switched.
     * 
     * @return false 
     * If the version does not exist or version_manager returns an error, then False will be 
     * returned here. Please check the information of logger for specific reasons.
     */
    bool switch_version(int version_id);

    /**
     * @brief 
     * Use this function to create a new file in the current folder.
     * 
     * @param name 
     * The name of the file you want to create.
     * 
     * @return true 
     * Successfully create a new file in the current folder.
     * 
     * @return false 
     * If the name exists or the goto_tail and rebuild functions return an error, then an 
     * error will also be returned here.
     */
    bool make_file(std::string name);

    /**
     * @brief 
     * Create a new folder under the current folder.
     * 
     * @param name 
     * The name of the folder you want to create.
     * 
     * @return true 
     * Successfully create a new folder under the current folder.
     * 
     * @return false 
     * If the name exists or the goto_tail and rebuild functions return an error, then an 
     * error will also be returned here.
     */
    bool make_dir(std::string name);

    /**
     * @brief 
     * This function is equivalent to the cd command in the Linux system, which is to enter 
     * the folder specified by the user.
     * 
     * @param name 
     * The name of the folder you want to enter.
     * 
     * @return true 
     * Successfully enter the folder you want to enter.
     * 
     * @return false 
     * If the go_to function, check_node function returns an error, or the name does not 
     * correspond to a folder, then an error will be returned.
     */
    bool change_directory(std::string name);

    /**
     * @brief 
     * Delete the target file from the current folder.
     * Notice! ! ! If you want to delete a folder, you need to use the remove_dir function. 
     * Because deleting the folder then needs to delete all the files in the folder 
     * recursively, and the file only needs to delete itself! ! !
     * 
     * @param name 
     * The name of the file you want to delete.
     * 
     * @return true 
     * The file was successfully deleted from the folder.
     * 
     * @return false 
     * If the go_to function, check_path function, rebuild_nodes function, reduce_counter 
     * function returns an error or the name does not correspond to a file, then an error 
     * will be returned.
     */
    bool remove_file(std::string name);

    /**
     * @brief 
     * Delete the folder corresponding to the name from the folder. 
     * This function will be used in conjunction with the recursive_delete_nodes function. 
     * In other words, this function will first remove the node corresponding to the folder 
     * from the folder, and then use the recursive_delete_nodes function to delete the tree 
     * recursively using this node as the root.
     * 
     * @param name 
     * The name of the folder you want to delete.
     * 
     * @return true 
     * Successfully delete the target folder from the current folder.
     * 
     * @return false 
     * If the go_to function, check_path function, rebuild_nodes function, 
     * recursive_delete_nodes function returns an error or the name does not correspond to a 
     * folder, the function will also return an error.
     */
    bool remove_dir(std::string name);

    /**
     * @brief 
     * Modify the name of the file or folder.
     * 
     * @param fr_name 
     * The name of the file or folder to be modified.
     * 
     * @param to_name 
     * Change the name of the file or folder corresponding to fr_name to to_name.
     * 
     * @return true 
     * The name of the file or folder was successfully modified.
     * 
     * @return false 
     * If the go_to function, check_path function, rebuild_nodes function, reduce_counter 
     * function return an error or the name does not exist, the function will return an error.
     */
    bool update_name(std::string fr_name, std::string to_name);

    /**
     * @brief 
     * Use this function to modify the content of the file.
     * 
     * @param name 
     * The name of the file you want to modify.
     * 
     * @param content 
     * This is what you want to modify the file into.
     * 
     * @return true 
     * The content in the file was successfully modified.
     * 
     * @return false 
     * If the go_to function, check_path function, rebuild_nodes function, decrea_counter 
     * function returns an error or the name does not correspond to a file, then this 
     * function will also return an error.
     */
    bool update_content(std::string name, std::string content);

    /**
     * @brief
     * Get the content stored in the file.
     * 
     * @param name 
     * The name of the file you want to view.
     * 
     * @param content 
     * If the function returns True, which means that the content of the file is successfully 
     * obtained, then the content of the file will be stored in content.
     * Note that the content here is passed in by reference!
     * 
     * @return true 
     * It shows that the content in the file has been successfully obtained. The content of 
     * the file is stored in content.
     * 
     * @return false 
     * If the go_to function, check_path function returns an error or the name does not 
     * correspond to a file, the function will return an error.
     */
    bool get_content(std::string name, std::string &content);

    /**
     * @brief 
     * This function will be used in conjunction with the travel_tree function.
     * Use this function to get the tree structure of the tree rooted at the root of the 
     * current file version.
     * 
     * @param tree_info 
     * If the function returns true, that is to say, after the tree structure is successfully 
     * obtained, the tree structure information will be stored in tree_info.
     * Note that tree_info is passed into the function by reference.
     * 
     * @return true 
     * The tree structure is successfully obtained, and the tree structure is stored in 
     * tree_info.
     * 
     * @return false 
     * If check_path or travel_tree returns an error, then this function will also return an error.
     * It should be noted that the travel_tree function will return an error under normal 
     * circumstances, because it will inevitably recurse to the boundary of the tree, 
     * encounter a null pointer and then return an error. But if you just call this function 
     * and return an error, this is abnormal, because it means that the path.front() pointer 
     * we passed in is a null pointer! !
     */
    bool tree(std::string &tree_info);

    /**
     * @brief 
     * 
     * Use this function to adjust the path to the upper level folder. 
     * If you are already in the root directory, calling this function will not return an 
     * error, but at the same time it will not adjust the current directory.
     * 
     * @return true 
     * There is no error in the process of returning to the upper directory.
     * 
     * @return false 
     * Um, take a look in BSTree. QAQ
     */
    bool goto_last_dir();

    /**
     * @brief 
     * This function is implemented in BSTree. If you want to see specific information, you 
     * can go
     */
    bool list_directory_contents(std::vector<std::string> &content);

    /**
     * @brief
     * Create a new version through this function.
     * 
     * @param model_version 
     * The template version of the version you want to create. 
     * In other words, create a new version based on the template version. The new version is 
     * exactly the same as the template version, but they do not affect each other.
     * 
     * @param info 
     * The comment of the new version created. It is not required and can be empty.
     * 
     * @return true 
     * A new version was successfully created. And automatically switch to the latest version.
     * 
     * @return false 
     * If the version_manager.create_version function, get_latest_version function, 
     * switch_version function returns an error, then this function will also return an error.
     */
    bool create_version(unsigned long long model_version=NO_MODEL_VERSION, std::string info="");
    bool create_version(std::string info = "", unsigned long long model_version=NO_MODEL_VERSION);

    /**
     * @brief 
     * This function is implemented in VersionManager.
     * Get version information of all versions.
     * 
     * @param version_log 
     * The obtained version information will be saved in version_log.
     * Note that version_log is passed into the function by reference.
     * 
     * @return true 
     * The version information of all versions is successfully obtained, and the version 
     * information is stored in version_log.
     * 
     * @return false 
     * emm.. Take a look at the get_version_log function in version_manager.
     */
    bool version(std::vector<std::pair<unsigned long long, versionNode>> &version_log);

    /**
     * @brief
     * Most of this function is implemented in NodeManager.
     * Get the update time of a file or folder.
     * For a folder, modifying its name is considered to have been modified; for a file, if 
     * you want to modify its content or modify the file name, it is considered to have been 
     * modified.
     * 
     * @param name 
     * The name of the file or folder for which you want to obtain information.
     * 
     * @param update_time 
     * If the acquisition is successful, the acquired update time is stored in update_time. 
     * Note that update_time is passed into the function as a reference.
     * 
     * @return true 
     * Get the update time successfully, and save the update time in update_time.
     * 
     * @return false 
     * If the go_to function or node_manager.get_update_time returns an error, then this 
     * function will also return an error.
     * Details can be obtained in the logger's information.
     */
    bool get_update_time(std::string name, std::string &update_time);

    /**
     * @brief
     * Most of this function is implemented in NodeManager.
     * Get the creation time of a file or folder through this function.
     * 
     * @param name 
     * The name of the file or folder for which you want to obtain information.
     * 
     * @param create_time 
     * If the creation time is successfully obtained, the storage time will be stored in 
     * create_time.
     * Note that create_time is passed into the function by reference.
     * 
     * @return true 
     * Get the update time successfully, and save the create time in create_time.
     * 
     * @return false 
     * If the go_to function or node_manager.get_create_time returns an error, then this 
     * function will also return an error.
     * Details can be obtained in the logger's information.
     */
    bool get_create_time(std::string name, std::string &create_time);

    /**
     * @brief
     * Know whether the name corresponds to a file or a folder.
     * 
     * @param name 
     * The name of the type of node that you want to know.
     * 
     * @param type 
     * If the node type is successfully obtained, the node type will be stored in type.
     * Note that type is passed into the function by reference.
     * 
     * @return true 
     * The type is successfully obtained, and the node type has been stored in type.
     * 
     * @return false 
     * If the go_to function returns an error, then the input will also return an error.
     */
    bool get_type(std::string name, treeNode::TYPE &type);

    /**
     * @brief
     * This function is implemented in BSTree.
     * Get the path from the root directory to the current directory through this function.
     * 
     * @param p 
     * If the current directory path is successfully obtained then the path will be stored in p.
     * Note that p is passed into the function by reference.
     * 
     * @return true 
     * @return false 
     */
    bool get_current_path(std::vector<std::string> &p);

    /**
     * @brief 
     * Search for files or folders whose names contain a certain character string in the 
     * current version.
     * 
     * @param name 
     * If the name of the file or folder contains the string, it will be searched.
     * 
     * @param res 
     * The results of the search are stored in res.
     * Note that res is passed into the function by reference.
     * 
     * @return true 
     * @return false 
     * This function should not go wrong at the moment, and mistakes are fatal errors. . QAQ
     */
    bool Find(std::string name, std::vector<std::pair<std::string, std::vector<std::string>>> &res);

    /**
     * @brief Get the current version object
     * Get the version number of the current version.
     * 
     * @return int 
     * The version number of the current version.
     */
    int get_current_version();
};






                        /* ======= class FileSystem ======= */

FileSystem::FileSystem() {
    if (version_manager.empty()) {
        version_manager.create_version();
    }
    unsigned long long latest_version_id;
    if (!version_manager.get_latest_version(latest_version_id)) return;
    switch_version(latest_version_id);
}

bool FileSystem::decrease_counter(treeNode *p) {
    if (!check_node(p, __LINE__)) return false;
    if (--p->cnt == 0) {
        logger.log("Node " + node_manager.get_name(p->link) + " will be deleted...");
        node_manager.delete_node(p->link);
        delete p;
        logger.log("Deleting completed.");
    }
    return true;
}

bool FileSystem::recursive_delete_nodes(treeNode *p, bool delete_brother) {
    if (p == nullptr) {
        logger.log("Get a null pointer in line " + std::to_string(__LINE__));
        return false;
    }
    recursive_delete_nodes(p->first_son, true);
    if (delete_brother) recursive_delete_nodes(p->next_brother, true);
    decrease_counter(p);
    return true;
}

bool FileSystem::delete_node() {
    if (!check_path()) return false;
    treeNode *t = path.back();
    if (!check_node(t, __LINE__)) return false;
    path.pop_back();
    if (!rebuild_nodes(t->next_brother)) return false;
    if (!decrease_counter(t)) return false;
    return true;
}

bool FileSystem::rebuild_nodes(treeNode *p) {
    if (!check_path()) return false;
    int relation = 0;   // 0 next_brother 1 first_son
    std::stack<treeNode*> stk;
    stk.push(p);
    for (; check_node(path.back(), __LINE__) && path.back()->cnt > 1; path.pop_back()) {
        treeNode *t = new treeNode();
        (*t) = (*path.back());
        node_manager.increase_counter(t->link);
        if (relation == 1) t->first_son = stk.top();
        else t->next_brother = stk.top();
        relation = is_son();
        stk.push(t);
        if (!decrease_counter(path.back())) return false;
    }
    if (!check_node(path.back(), __LINE__)) return false;
    (relation ? path.back()->first_son : path.back()->next_brother) = stk.top();
    for (; !stk.empty(); stk.pop()) {
        path.push_back(stk.top());
    }
    if (path.back() == nullptr) path.pop_back();
    if (!check_path()) return false;
    return true;
}

bool FileSystem::travel_tree(treeNode *p,std::string &tree_info) {
    if (p == nullptr) {
        logger.log("Get a null pointer in line " + std::to_string(__LINE__));
        return false;
    }
    static int tab_cnt = 1;
    if (p->type == 2) {
        travel_tree(p->next_brother, tree_info);
        return true;
    }
    for (unsigned int i = 0; i < tab_cnt; i++) {
        if (i < tab_cnt - 1) {
            tree_info += "    ";
        } else if (p->next_brother != nullptr) {
            tree_info += "├── ";
        } else {
            tree_info += "└── ";
        }
    }
    tree_info += node_manager.get_name(p->link) + '\n';
    tab_cnt++;
    travel_tree(p->first_son, tree_info);
    tab_cnt--;
    travel_tree(p->next_brother, tree_info);
    return true;
}

bool FileSystem::kmp(std::string str, std::string tar) {
    static const int MAX_NAME_LEN = 1000;
    if (str.size() > MAX_NAME_LEN || tar.size() > MAX_NAME_LEN) return false;
    int next[1000];
    memset(next, 0, sizeof next);
    for (int i = 0, j = -1; i < tar.size(); i++) {
        while (j >= 0 && tar[j + 1] != tar[i]) {
            j = next[j];
        }
        if (tar[j + 1] == tar[i]) {
            j++;
        }
        next[i] = j;
    }

    for (int i = 0, j = -1; i < str.size(); i++) {
        while (j >= 0 && tar[j + 1] != str[i]) {
            j = next[j];
        }
        if (tar[j + 1] == str[i]) {
            j++;
        }
        if (j == tar.size() - 1) {
            return true;
        }
    }
    return false;
}

bool FileSystem::travel_find(std::string name, std::vector<std::pair<std::string, std::vector<std::string>>> &res) {
    if (kmp(node_manager.get_name(path.back()->link), name)) {
        std::vector<std::string> p;
        if (!get_current_path(p)) return false;
        res.push_back(std::make_pair(node_manager.get_name(path.back()->link), p));
    }
    if (path.back()->next_brother != nullptr) {
        path.push_back(path.back()->next_brother);
        travel_find(name, res);
        path.pop_back();
    }
    if (path.back()->first_son != nullptr) {
        path.push_back(path.back()->first_son);
        travel_find(name, res);
        path.pop_back();
    }
    return true;
}

bool FileSystem::switch_version(int version_id) {
    if (!version_manager.version_exist(version_id)) {
        logger.log("This version is not in the system.");
        return false;
    }
    CURRENT_VERSION = version_id;
    treeNode *p;
    if (!version_manager.get_version_pointer(version_id, p)) {
        return false;
    }
    path.clear();
    path.push_back(p);
    if (p->first_son == nullptr) {
        logger.log("The root directory does not have a \"first son\" folder, which is abnormal. Please check that the procedure is correct.", Logger::FATAL, __LINE__);
        return false;
    }
    path.push_back(path.back()->first_son);
    return true;
}

bool FileSystem::make_file(std::string name) {
    if (name_exist(name)) {
        logger.log(name + ": Name exist.");
        return false;
    }
    if (!goto_tail()) return false;
    treeNode *t = new treeNode(treeNode::FILE);
    t->link = node_manager.get_new_node(name);
    if (!rebuild_nodes(t)) return false;
    return true;
}

bool FileSystem::make_dir(std::string name) {
    if (name_exist(name)) {
        logger.log(name + ": Name exist.");
        return false;
    }
    if (!goto_tail()) return false;
    treeNode *t = new treeNode(treeNode::DIR);
    if (t == nullptr) {
        logger.log("The system did not allocate memory for this operation.", Logger::FATAL, __LINE__);
        return false;
    }
    t->link = node_manager.get_new_node(name);
    if (!rebuild_nodes(t)) return false;
    return true;
}

bool FileSystem::change_directory(std::string name) {
    if (!go_to(name)) return false;
    if (path.back()->type != 1) {
        logger.log(name + ": Not a directory.");
        return false;
    }
    if (!check_node(path.back()->first_son, __LINE__)) {
        return false;
    }
    path.push_back(path.back()->first_son);
    return true;
}

bool FileSystem::remove_file(std::string name) {
    if (!go_to(name)) return false;
    if (path.back()->type != 0) {
        logger.log(name + ": Not a file.");
        return false;
    }
    treeNode *t = path.back();
    path.pop_back();
    if (!check_path()) return false;
    if (!rebuild_nodes(t->next_brother)) return false;
    if (!decrease_counter(t)) return false;
    return true;
}

bool FileSystem::remove_dir(std::string name) {
    if (!go_to(name)) return false;
    if (path.back()->type != 1) {
        logger.log(name + ": Not a directory.");
        return false;
    }
    if (!check_path()) return false;
    treeNode *t = path.back();
    path.pop_back();        // 20211023
    if (!rebuild_nodes(t->next_brother)) return false;
    if (!recursive_delete_nodes(t)) return false;
    return true;
}

bool FileSystem::update_name(std::string fr_name, std::string to_name) {
    if (!go_to(fr_name)) return false;
    if (name_exist(to_name)) {
        logger.log(to_name + ": Name exists.", Logger::WARNING, __LINE__);
        return false;
    }
    treeNode *t = new treeNode();
    if (t == nullptr) {
        logger.log("The system did not allocate memory for this operation.", Logger::FATAL, __LINE__);
        return false;
    }
    if (!check_path()) return false;
    treeNode *back = path.back();
    *t = *back;
    t->cnt = 1;
    t->link = node_manager.update_name(t->link, to_name);
    if (t->link == ULLONG_MAX) {
        delete t;
        return false;
    }
    path.pop_back();
    if (!rebuild_nodes(t)) return false;
    if (!decrease_counter(back)) return false; 
    // 这里必须最后decrease，如果提前回导致节点丢失
    return true;
}

bool FileSystem::update_content(std::string name, std::string content) {
    if (!go_to(name)) return false;
    if (!check_path()) return false;
    if (path.back()->type != 0) {
        logger.log(name + ": Not a file.");
        return false;
    }
    treeNode *back = path.back();
    treeNode *t = new treeNode();
    if (t == nullptr) {
        logger.log("The system did not allocate memory for this operation.", Logger::FATAL, __LINE__);
        return false;
    }
    *t = *back;
    t->cnt = 1;
    t->link = node_manager.update_content(t->link, content);
    if (t->link == ULLONG_MAX) {
        delete t;
        return false;
    }
    path.pop_back();
    if (!rebuild_nodes(t)) return false;
    if (!decrease_counter(back)) return false;
    return true;
}

bool FileSystem::get_content(std::string name, std::string &content) {
    if (!go_to(name)) return false;
    if (!check_path()) return false;
    if (path.back()->type != 0) {
        logger.log(name + ": Not a file.");
        return false;
    }
    content = node_manager.get_content(path.back()->link);
    return true;
}

bool FileSystem::tree(std::string &tree_info) {
    if (!check_path()) return false;
    if (!travel_tree(path.front(), tree_info)) return false;
    return true;
}

bool FileSystem::goto_last_dir() {
    return BSTree::goto_last_dir();
}
bool FileSystem::list_directory_contents(std::vector<std::string> &content) {
    return BSTree::list_directory_contents(content);
}

bool FileSystem::create_version(unsigned long long model_version, std::string info) {
    if (!version_manager.create_version(model_version, info)) return false;
    unsigned long long latest_version;
    if (!version_manager.get_latest_version(latest_version)) {
        return false;
    }
    if (!switch_version(latest_version)) return false;
    return true;
}

bool FileSystem::create_version(std::string info, unsigned long long model_version) {
    return create_version(model_version, info);
}

bool FileSystem::version(std::vector<std::pair<unsigned long long, versionNode>> &version_log) {
    return version_manager.get_version_log(version_log);
}

bool FileSystem::get_update_time(std::string name, std::string &update_time) {
    if (!go_to(name)) return false;
    update_time = node_manager.get_update_time(path.back()->link);
    return true;
}

bool FileSystem::get_create_time(std::string name, std::string &create_time) {
    if (!go_to(name)) return false;
    create_time = node_manager.get_create_time(path.back()->link);
    return true;
}

bool FileSystem::get_type(std::string name, treeNode::TYPE &type) {
    if (!go_to(name)) return false;
    type = path.back()->type;
    return true;
}

bool FileSystem::get_current_path(std::vector<std::string> &p) {
    if (!BSTree::get_current_path(p)) return false;
    return true;
}

bool FileSystem::Find(std::string name, std::vector<std::pair<std::string, std::vector<std::string>>> &res) {
    auto path_backup = path;
    path.erase(path.begin() + 2, path.end());
    travel_find(name, res);
    path = path_backup;
    return true;
}

int FileSystem::get_current_version() {
    return CURRENT_VERSION;
}

#endif
