/**
   ___ _                 _                      
  / __| |__   __ _ _ __ | |_    /\/\   ___  ___ 
 / /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\/\ |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/    \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee 
*/

#ifndef BS_TREE_CPP
#define BS_TREE_CPP

#include "node_manager.cpp"
#include "logger.cpp"
#include <algorithm>
#include <vector>

/**
 * @brief 
 * This class implements the basic functions of the file system tree structure.
 * This class only implements the most basic operations on the tree, and other 
 * specific operations such as file addition, deletion, checking and modification 
 * are implemented in the FileSystem class.
 */
struct treeNode {
    /**
     * @brief 
     * List the types of tree nodes. It may be files, folders, head nodes.
     */
    enum TYPE {
        FILE = 0, DIR, HEAD_NODE
    };

    TYPE type;
    int cnt;
    unsigned long long link;
    treeNode *next_brother, *first_son;

    treeNode();
    treeNode(TYPE type);
};

class BSTree {
private:
    Logger &logger = Logger::get_logger();
    NodeManager &node_manager = NodeManager::get_node_manager();
protected:
    /**
     * @brief 
     * The path vector plays a very important role in the entire tree and file system. 
     * This variable holds the simple path from the root node to the node to be operated.
     * 
     * When the user needs to enter a folder, first change the content of the path to 
     * make it point to the target folder, and then add first_son to the path variable
     * through other functions, and then enter the folder; 
     * When the user wants to delete the file, It also points to the file through the 
     * same operation, and then deletes it through other functions; 
     * There is a very important function in the FileSystem class called rebuild_tree, 
     * which also uses path to find the node it needs and rebuild nodes. 
     * And many more.. All in all, this is a very very important variable.
     */
    std::vector<treeNode*> path;

    /**
     * @brief
     * Use this function to check whether the nodes in the path are all legal.
     * What it checks are:
     * 1. Whether path is empty. If it is empty, this is abnormal, because there should 
     * be at least one root directory in the path.
     * 2. Check whether there is a null pointer in the path.
     * 
     * @return true 
     * No problem with the path.
     * 
     * @return false 
     * The path is empty or there is a null pointer.
     */
    bool check_path();

    /**
     * @brief
     * Use this function to check whether a node in the tree is legal.
     * What it checks are:
     * 1. Whether the node is a null pointer.
     * 2. Check whether the counter of the node is less than or equal to 0. 
     * If it is less than or equal to 0, then the node should be deleted, but it has 
     * not been deleted now, which is obviously abnormal.
     * 
     * @param p
     * The node you want to check.
     * 
     * @param line 
     * The line number where you are calling the function. 
     * This is mainly used to help you locate the wrong location. 
     * (You can't let the wrong position in this function. QAQ)
     * 
     * @return true 
     * This means that there is no problem with the node.
     * 
     * @return false 
     * This means that the node check failed, and the specific reason can be 
     * obtained through the information in the logger.
     */
    bool check_node(treeNode *p, int line);

    /**
     * @brief 
     * Check if the last element in path is a child node.
     * The principle of inspection is to determine whether the node type of the 
     * tree is head_node.
     * 
     * @return true 
     * This means that the last node in the path is a child node.
     * 
     * @return false 
     * This means that the node is not a child node or the node's node test failed.
     */
    bool is_son();

    /**
     * @brief 
     * Adjust the path to the last node in the folder.
     * It is by constantly accessing next_brother until it gets the null pointer position.
     * 
     * @return true 
     * The path is successfully adjusted to the last node in the folder.
     * 
     * @return false 
     * The path check before or after adjustment failed.
     */
    bool goto_tail();

    /**
     * @brief 
     * Adjust the path to the head node in the folder.
     * It is by constantly checking whether the last element in the path is a child node, 
     * and if it is not, the last element is continuously popped up until the child node 
     * is found.
     * 
     * @return true 
     * The path is successfully adjusted to the head node.
     * 
     * @return false 
     * The path check before or after adjustment failed.
     */
    bool goto_head();

    /**
     * @brief 
     * Check if the name exists in the current folder.
     * 
     * @param name 
     * The name you want to check.
     * 
     * @return true 
     * The name exists in this folder.
     * 
     * @return false 
     * The name does not exist in the folder or the list_directory_contents 
     * function returns an error.
     */
    bool name_exist(std::string name);

    /**
     * @brief 
     * Adjust the path to the node named name.
     * 
     * @param name 
     * Adjust the path to the node named name.
     * 
     * @return true 
     * The path is successfully adjusted to the target node.
     * 
     * @return false 
     * The target node does not exist or the goto_head function returns an error.
     */
    bool go_to(std::string name);

    /**
     * @brief 
     * Adjust the path to the upper level directory.
     * It should be noted that this function will reserve two nodes in 
     * the path, one is the root directory, and the other is the head node corresponding 
     * to root's first_son.
     * 
     * @return true 
     * The path is successfully adjusted to the upper level directory.
     * 
     * @return false 
     * The goto_head or check_path function returns an error. 
     */
    bool goto_last_dir();

    /**
     * @brief 
     * List all file folders in the current directory. 
     * The listed list is arranged in the order of creation. 
     * The list only contains the names of files or folders, not their types, creation 
     * time, and modification time. This information can be obtained through the
     * get_update_time, get_create_time, and get_type functions in the FileSystem class.
     * 
     * @param content 
     * The names of files and folders in this folder are stored in this variable. 
     * Note that this variable is a reference.
     * 
     * @return true 
     * The names of the files and folders in the folder have been saved to the 
     * content variable.
     * 
     * @return false 
     * The goto_head function or check_path function returns an error.
     */
    bool list_directory_contents(std::vector<std::string> &content);

    /**
     * @brief 
     * Get the directory of the current folder.
     * 
     * @param path 
     * The path of the current folder is stored in this variable.
     * The principle of this function is:
     * Save the original path array first. Then keep goto_head, record the name of 
     * the penultimate node of the path, and goto_last_dir until the number of nodes 
     * in the path becomes two. (Only root and root's first_son —— head_node)
     * 
     * @return true 
     * Successfully get the path of the current folder and store the path in the path array.
     * 
     * @return false 
     * The goto_head function or the goto_last_dir function returns an error.
     */
    bool get_current_path(std::vector<std::string> &path);
};



                        /* ======= struct treeNode ======= */
treeNode::treeNode() : type(FILE), cnt(1), link(0xffffffffffffffffULL), next_brother(nullptr), first_son(nullptr) {}

treeNode::treeNode(TYPE type) {
    this->type = type;
    this->cnt = 1;
    this->next_brother = nullptr;
    // 0xffffffffffffffffULL means "no link" (the same value as -1 converted to unsigned).
    this->link = 0xffffffffffffffffULL;
    if (type == FILE || type == HEAD_NODE) this->first_son = nullptr;
    else if (type == DIR) this->first_son = new treeNode(HEAD_NODE);
}


                        /* ======= class BSTree ======= */

bool BSTree::check_path() {
    if (path.empty()) {
        logger.log("Path is empty. This not normal.", Logger::FATAL, __LINE__);
        return false;
    }
    for (auto &it : path) {
        if (it == nullptr) {
            logger.log("Null pointer exists in path. This not normal.", Logger::FATAL, __LINE__);
            return false;
        }
    }
    return true;
}

bool BSTree::check_node(treeNode *p, int line) {
    if (p == nullptr) {
        logger.log("The pointer is empty, please check whether the program is correct.", Logger::FATAL, line);
        return false;
    }
    if (p->cnt <= 0) {
        logger.log("The node counter is already less than or equal to 0, please check the program!", Logger::FATAL, line);
        return false;
    }
    return true;
}

bool BSTree::is_son() {
    if (!check_path()) return false;
    return path.back()->type == treeNode::HEAD_NODE;
}

bool BSTree::goto_tail() {
    if (!check_path()) return false;
    while (path.back()->next_brother != nullptr) {
        path.push_back(path.back()->next_brother);
    }
    if (!check_path()) return false;
    return true;
}

bool BSTree::goto_head() {
    if (!check_path()) return false;
    for (; !path.empty() && !is_son(); path.pop_back());
    if (!check_path()) return false;
    return true;
}

bool BSTree::name_exist(std::string name) {
    std::vector<std::string> dir_content;
    if (!list_directory_contents(dir_content)) return false;
    for (auto &nm : dir_content) {
        if (nm == name) return true;
    }
    return false;
}

bool BSTree::go_to(std::string name) {
    if (!name_exist(name)) {
        logger.log("no file or directory named " + name, Logger::WARNING, __LINE__);
        return false;
    }
    if (!goto_head()) return false;
    while (path.back()->type == treeNode::HEAD_NODE ||
           node_manager.get_name(path.back()->link) != name) {
        if (path.back()->next_brother == nullptr) {
            return false;
        }
        path.push_back(path.back()->next_brother);
    }
    return true;
}

bool BSTree::goto_last_dir() {
    if (!goto_head()) return false;
    if (path.size() > 2) {
        path.pop_back();
    }
    if (!check_path()) return false;
    return true;
}

bool BSTree::list_directory_contents(std::vector<std::string> &content) {
    content.clear();
    if (!goto_head()) return false;
    if (!check_path()) return false;
    while (path.back()->next_brother != nullptr) {
        content.push_back(node_manager.get_name(path.back()->next_brother->link));
        path.push_back(path.back()->next_brother);
    }
    return true;
}

bool BSTree::get_current_path(std::vector<std::string> &p) {
    auto path_backup = path;
    p.clear();
    if (!goto_head()) {
        path = path_backup;
        return false;
    }
    while (path.size() > 2) {
        p.push_back(node_manager.get_name(path[path.size() - 2]->link));
        if (!goto_last_dir()) {
            path = path_backup;
            return false;
        }
        if (!goto_head()) {
            path = path_backup;
            return false;
        }
    }
    p.push_back(node_manager.get_name(path.front()->link));
    path = path_backup;
    std::reverse(p.begin(), p.end());
    return true;
}

#endif
