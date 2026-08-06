/**
   ___ _                 _                      
  / __| |__   __ _ _ __ | |_    /\/\   ___  ___ 
 / /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\/\ |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/    \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee 
*/

#ifndef VERSION_MANAGER_CPP
#define VERSION_MANAGER_CPP

#include "bs_tree.cpp"
#include "node_manager.cpp"
#include "logger.cpp"
#include "saver.cpp"
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <set>

#define NO_MODEL_VERSION 0x3f3f3f3f

struct versionNode {
    std::string info;
    treeNode *p;

    versionNode() = default;
    versionNode(std::string info, treeNode *p) : info(info), p(p) {}
};

class VersionManager {
private:
    std::map<unsigned long long, versionNode> version;
    NodeManager &node_manager = NodeManager::get_node_manager();
    Logger &logger = Logger::get_logger();
    Saver &saver = Saver::get_saver();
    const unsigned long long NULL_NODE = 0x3f3f3f3f3f3fULL;
    bool loaded_ok = false;
    std::string DATA_TREENODE_INFO = "VersionManager::DATA_TREENODE_INFO";
    std::string DATA_VERSION_INFO = "VersionManager::DATA_VERSION_INFO";

    bool load();
    bool save();
    void dfs(treeNode *cur, std::map<treeNode *, unsigned long long> &label);
    bool recursive_increase_counter(treeNode *p, bool modify_brother=false);
public:
    VersionManager();
    ~VersionManager();
    bool init_version(treeNode *p, treeNode *vp);
    bool create_version(unsigned long long model_version=NO_MODEL_VERSION, std::string info="");
    bool version_exist(unsigned long long id);
    bool get_version_pointer(unsigned long long id, treeNode *&p);
    bool get_latest_version(unsigned long long &id);
    bool get_version_log(std::vector<std::pair<unsigned long long, versionNode>> &version_log);
    bool empty();
};




                        /* ====== VersionManager ====== */
bool VersionManager::load() {
    vvs node_information;
    if (!saver.load(DATA_TREENODE_INFO, node_information)) return false;
    vvs version_information;
    if (!saver.load(DATA_VERSION_INFO, version_information)) return false;

    std::vector<treeNode*> allocated;
    std::map<unsigned long long, treeNode*> label_to_ptr;
    std::string s_label, s_type, s_cnt, s_link, s_next_brother, s_first_son;
    std::string s_version_id, version_info, s_version_head_label;
    for (auto &node : node_information) {
        if (node.size() != 6) {
            logger.log("VersionManager: File is corrupted and cannot be read.", Logger::WARNING, __LINE__);
            goto fail;
        }
        s_label = node[0];
        s_type = node[1];
        s_cnt = node[2];
        s_link = node[3];
        s_next_brother = node[4];
        s_first_son = node[5];
        if (!saver.is_all_digits(s_label) || !saver.is_all_digits(s_type) || !saver.is_all_digits(s_cnt) || !saver.is_all_digits(s_link) || !saver.is_all_digits(s_next_brother) || !saver.is_all_digits(s_first_son) ||
            s_label.size() > 20 || s_type.size() > 20 || s_cnt.size() > 20 || s_link.size() > 20 || s_next_brother.size() > 20 || s_first_son.size() > 20) {
            logger.log("VersionManager: File is corrupted and cannot be read.", Logger::WARNING, __LINE__);
            goto fail;
        }

        unsigned long long label, type, cnt, link;
        label = saver.str_to_ull(s_label);
        type = saver.str_to_ull(s_type);
        cnt = saver.str_to_ull(s_cnt);
        link = saver.str_to_ull(s_link);

        if (cnt == 0 || cnt > 0x7fffffffULL) {
            logger.log("VersionManager: File is corrupted and cannot be read.", Logger::WARNING, __LINE__);
            goto fail;
        }
        
        if (type >= 3) {
            logger.log("VersionManager: File is corrupted and cannot be read.", Logger::WARNING, __LINE__);
            goto fail;
        }
        if (label_to_ptr.count(label)) {
            logger.log("VersionManager: File is corrupted and cannot be read.", Logger::WARNING, __LINE__);
            goto fail;
        }

        treeNode *t = new treeNode();
        allocated.push_back(t);
        if (type == 0) t->type = treeNode::FILE;
        else if (type == 1) t->type = treeNode::DIR;
        else t->type = treeNode::HEAD_NODE;
        t->cnt = (int)cnt;
        t->link = link;

        label_to_ptr[label] = t;
    }

    for (auto &node : node_information) {
        s_label = node[0];
        s_next_brother = node[4];
        s_first_son = node[5];

        unsigned long long label, next_brother, first_son;
        label = saver.str_to_ull(s_label);
        next_brother = saver.str_to_ull(s_next_brother);
        first_son = saver.str_to_ull(s_first_son);
        
        if (next_brother != NULL_NODE && !label_to_ptr.count(next_brother)) {
            logger.log("VersionManager: File is corrupted and cannot be read.", Logger::WARNING, __LINE__);
            goto fail;
        }
        if (first_son != NULL_NODE && !label_to_ptr.count(first_son)) {
            logger.log("VersionManager: File is corrupted and cannot be read.", Logger::WARNING, __LINE__);
            goto fail;
        }

        treeNode *t = label_to_ptr[label];
        t->next_brother = next_brother == NULL_NODE ? nullptr : label_to_ptr[next_brother];
        t->first_son = first_son == NULL_NODE ? nullptr : label_to_ptr[first_son];
    }

    for (auto &ver : version_information) {
        if (ver.size() != 3) {
            logger.log("VersionManager: File is corrupted and cannot be read.", Logger::WARNING, __LINE__);
            goto fail;
        }
        s_version_id = ver[0];
        version_info = ver[1];
        s_version_head_label = ver[2];
        if (!saver.is_all_digits(s_version_id) || !saver.is_all_digits(s_version_head_label) ||
            s_version_id.size() > 20 || s_version_head_label.size() > 20) {
            logger.log("VersionManager: File is corrupted and cannot be read.", Logger::WARNING, __LINE__);
            goto fail;
        }
        unsigned long long version_id, version_head_label;
        version_id = saver.str_to_ull(s_version_id);
        version_head_label = saver.str_to_ull(s_version_head_label);

        if (!label_to_ptr.count(version_head_label) || label_to_ptr[version_head_label]->type != treeNode::DIR ||
            label_to_ptr[version_head_label]->first_son == nullptr ||
            label_to_ptr[version_head_label]->first_son->type != treeNode::HEAD_NODE) {
            logger.log("VersionManager: File is corrupted and cannot be read.", Logger::WARNING, __LINE__);
            goto fail;
        }
        if (version.count(version_id)) {
            logger.log("VersionManager: File is corrupted and cannot be read.", Logger::WARNING, __LINE__);
            goto fail;
        }

        auto t = versionNode();
        t.info = version_info;
        t.p = label_to_ptr[version_head_label];
        
        version[version_id] = t;
    }

    // 从每个版本根做 DFS，统计每个节点被引用的次数，并与文件中的 cnt 核对；in_path 顺带检测环。
    {
        std::map<treeNode *, unsigned long long> ref_count;
        for (auto &ver : version) {
            std::set<treeNode *> in_path;
            std::stack<std::pair<treeNode *, int>> stk;   // (node, 0=进入 1=离开)
            stk.push(std::make_pair(ver.second.p, 0));
            while (!stk.empty()) {
                std::pair<treeNode *, int> cur = stk.top();
                stk.pop();
                if (cur.first == nullptr) continue;
                if (cur.second == 0) {
                    if (in_path.count(cur.first)) {
                        logger.log("VersionManager: The tree contains a cycle. File is corrupted.", Logger::WARNING, __LINE__);
                        goto fail;
                    }
                    in_path.insert(cur.first);
                    ref_count[cur.first] ++;
                    stk.push(std::make_pair(cur.first, 1));
                    if (cur.first->next_brother != nullptr) stk.push(std::make_pair(cur.first->next_brother, 0));
                    if (cur.first->first_son != nullptr) stk.push(std::make_pair(cur.first->first_son, 0));
                } else {
                    in_path.erase(cur.first);
                }
            }
        }
        for (auto *t : allocated) {
            if (!ref_count.count(t) || ref_count[t] != (unsigned long long)t->cnt) {
                logger.log("VersionManager: The reference count of a node does not match the file. File is corrupted.", Logger::WARNING, __LINE__);
                goto fail;
            }
        }
    }

    return true;

fail:
    for (auto *t : allocated) delete t;
    version.clear();
    return false;
}

void VersionManager::dfs(treeNode *cur, std::map<treeNode *, unsigned long long> &label) {
    if (cur == nullptr) return;
    // 显式栈后序遍历：先访问 next_brother 链与 first_son 子树，再给当前节点编号。
    std::stack<std::pair<treeNode *, bool>> stk;   // (node, expanded)
    stk.push(std::make_pair(cur, false));
    while (!stk.empty()) {
        std::pair<treeNode *, bool> top = stk.top();
        stk.pop();
        if (top.first == nullptr || label.count(top.first)) continue;
        if (!top.second) {
            stk.push(std::make_pair(top.first, true));
            if (top.first->next_brother != nullptr) stk.push(std::make_pair(top.first->next_brother, false));
            if (top.first->first_son != nullptr) stk.push(std::make_pair(top.first->first_son, false));
        } else {
            label[top.first] = label.size();
        }
    }
}

bool VersionManager::save() {
    // label each node.
    std::map<treeNode*, unsigned long long> label;
    // std::vector<std::pair<unsigned, std::pair<unsigned long long, unsigned long long>>> relation;
    for (auto &ver : version) {
        dfs(ver.second.p, label);
    }
    vvs node_information;
    for (auto &node : label) {
        node_information.push_back(std::vector<std::string>());
        std::vector<std::string> &noif = node_information.back();
        noif.push_back(std::to_string(node.second));
        // std::cout << "id: " << noif.back() << '\n';
        treeNode *tn = node.first;
        if (tn->type == treeNode::FILE) {
            noif.push_back("0");
        } else if (tn->type == treeNode::DIR) {
            noif.push_back("1");
        } else if (tn->type == treeNode::HEAD_NODE) {
            noif.push_back("2");
        }
        // std::cout << "name: " << node_manager.get_name(tn->link) << '\n';
        // std::cout << "type: " << noif.back() << '\n';
        noif.push_back(std::to_string(tn->cnt));
        // std::cout << "cnt: " << noif.back() << '\n';
        noif.push_back(std::to_string(tn->link));
        // std::cout << "link: " << noif.back() << '\n';
        if (tn->next_brother == nullptr) {
            noif.push_back(std::to_string(NULL_NODE));
        } else {
            if (!label.count(tn->next_brother)) {
                logger.log("VersionManager: Failed to save. A node link is missing.", Logger::WARNING, __LINE__);
                return false;
            }
            noif.push_back(std::to_string(label[tn->next_brother]));
        }
        // std::cout << "next_b: " << noif.back() << '\n';
        if (tn->first_son == nullptr) {
            noif.push_back(std::to_string(NULL_NODE));
        } else {
            if (!label.count(tn->first_son)) {
                logger.log("VersionManager: Failed to save. A node link is missing.", Logger::WARNING, __LINE__);
                return false;
            }
            noif.push_back(std::to_string(label[tn->first_son]));
        }
        // std::cout << "first_s: " << noif.back() << '\n';
        // std::cout << '\n';
        // std::cout << noif.size() << '\n';
        // for (auto it : noif) std::cout << it << '\n';
    }
    if (!saver.save(DATA_TREENODE_INFO, node_information)) {
        return false;
    }
    vvs version_information;
    for (auto &it : version) {
        version_information.push_back(std::vector<std::string>());
        std::vector<std::string> &veif = version_information.back();
        veif.push_back(std::to_string(it.first));
        // std::cout << "version_id: " << veif.back() << '\n';
        veif.push_back(it.second.info);
        // std::cout << "version_info: " << veif.back() << '\n';
        if (!label.count(it.second.p)) {
            logger.log("VersionManager: Failed to save. A version root is missing.", Logger::WARNING, __LINE__);
            return false;
        }
        veif.push_back(std::to_string(label[it.second.p]));
        // std::cout << "version_head_node_label: " << veif.back() << '\n';
    }
    if (!saver.save(DATA_VERSION_INFO, version_information)) {
        return false;
    }
    return true;
}

VersionManager::VersionManager() {
    loaded_ok = (saver.exists(DATA_TREENODE_INFO) || saver.exists(DATA_VERSION_INFO)) ? load() : true;
}

VersionManager::~VersionManager() {
    if (loaded_ok && !save()) {
        logger.log("Failed to save the version data on exit.", Logger::WARNING, __LINE__);
    }
}

bool VersionManager::recursive_increase_counter(treeNode *p, bool modify_brother) {
    if (p == nullptr) return true;
    std::stack<treeNode*> stk;
    stk.push(p);
    while (!stk.empty()) {
        treeNode *cur = stk.top();
        stk.pop();
        if (cur == nullptr) continue;
        cur->cnt ++;
        node_manager.increase_counter(cur->link);
        if (cur->first_son != nullptr) stk.push(cur->first_son);
        if (modify_brother && cur->next_brother != nullptr) stk.push(cur->next_brother);
    }
    return true;
}

bool VersionManager::init_version(treeNode *p, treeNode *vp) {
    if (p == nullptr || vp == nullptr) {
        logger.log("Get a null pointer in line " + std::to_string(__LINE__), Logger::FATAL, __LINE__);
        return false;
    }
    p->first_son = vp->first_son;
    p->cnt = 1;
    if (vp == p) return true;
    if (!recursive_increase_counter(p->first_son, true)) return false;
    return true;
}

bool VersionManager::create_version(unsigned long long model_version, std::string version_info) {
    if (model_version != NO_MODEL_VERSION && !version_exist(model_version)) {
        logger.log("The version number does not exist in the system.", Logger::WARNING, __LINE__);
        return false;
    }
    treeNode *new_version = new treeNode(treeNode::DIR);
    if (new_version == nullptr) {
        logger.log("The system did not allocate memory for this operation.", Logger::FATAL, __LINE__);
        return false;
    }
    new_version->link = node_manager.get_new_node("root");
    if (model_version != NO_MODEL_VERSION) delete new_version->first_son;
    treeNode *model = model_version == NO_MODEL_VERSION ? new_version : version[model_version].p;
    if (!init_version(new_version, model)) {
        if (model == new_version && new_version->first_son != nullptr) {
            delete new_version->first_son;
        }
        node_manager.delete_node(new_version->link);
        delete new_version;
        return false;
    }
    unsigned long long id = version.empty() ? 1001 : (*version.rbegin()).first + 1;
    if (id == 0 || version.count(id)) {
        logger.log("Failed to create the version. The version id overflows or already exists.", Logger::WARNING, __LINE__);
        if (model == new_version && new_version->first_son != nullptr) {
            delete new_version->first_son;
        }
        node_manager.delete_node(new_version->link);
        delete new_version;
        return false;
    }
    // Note: version_info is a comment rather than a unique key, so duplicate
    // comments are allowed; the version id is the only identifier.
    version[id] = versionNode(version_info, new_version);
    return true;
}

bool VersionManager::version_exist(unsigned long long id) {
    return version.count(id);
}

bool VersionManager::get_version_pointer(unsigned long long id, treeNode *&p) {
    if (!version_exist(id)) {
        logger.log("Version " + std::to_string(id) + " does not exist.", Logger::WARNING, __LINE__);
        return false;
    }
    p = version[id].p;
    return true;
}

bool VersionManager::get_latest_version(unsigned long long &id) {
    if (version.empty()) {
        logger.log("No version exists in the system. Please create a new version to use.", Logger::WARNING, __LINE__);
        return false;
    }
    id = version.rbegin()->first;
    return true;
}

bool VersionManager::get_version_log(std::vector<std::pair<unsigned long long, versionNode>> &version_log) {
    for (auto &it : version) {
        version_log.push_back(it);
        version_log.back().second.p = nullptr;
    }
    return true;
}

bool VersionManager::empty() {
    return version.empty();
}

#endif
