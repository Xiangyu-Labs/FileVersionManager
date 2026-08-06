/**
   ___ _                 _                      
  / __| |__   __ _ _ __ | |_    /\/\   ___  ___ 
 / /  | '_ \ / _` | '_ \| __|  /    \ / _ \/ _ \
/ /___| | | | (_| | | | | |_  / /\/\ |  __|  __/
\____/|_| |_|\__,_|_| |_|\__| \/    \/\___|\___|

@ Author: Mu Xiangyu, Chant Mee 
*/

#ifndef SAVER_CPP
#define SAVER_CPP

#include "logger.cpp"
#include "encryptor.cpp"
#include <cctype>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <climits>
#include <cstdio>
#include <iomanip>

typedef std::vector<std::vector<std::string>> vvs;

/**
 * @brief 
 * The structure encapsulates the data, including the name of the data, the hash value 
 * of the data, the length of the encrypted data, and the encrypted data.
 * 
 * It should be noted that the len here is not the actual length of the data, but the 
 * actual length/N, where N is the length of a data block. (This concept is proposed in 
 * the encryptor class)
 */
struct dataNode {
    unsigned long long name_hash, data_hash;
    int len;
    std::vector<std::pair<double, double>> data;

    dataNode();
    dataNode(unsigned long long name_hash, unsigned long long data_hash, const std::vector<std::pair<double, double>> &data);
};

/**
 * @brief 
 * This class realizes the preservation of data.
 * This class inherits the Encryptor class and re-encapsulates the functions in it.
 * 
 * There is a data type "vvs" in this class, you can simply think of it as a 
 * two-dimensional array storing strings, but this array is implemented with a vector.
 * 
 * The prototype of "vvs" is std::vector<std::vector<std::string>>. I believe that 
 * most of the time this data structure can store any type of data, even if it cannot 
 * be directly stored in this structure, it can be stored through a certain deformation.
 * 
 * The functional design of this class is very interesting. It achieves such a function. 
 * The user provides a vvs where he has stored the data and the name of the data. After 
 * that, if the user wants this set of data, he only needs to provide the name of the 
 * data, and the user can get a copy that is exactly the same as when he stored it.
 */
class Saver : private Encryptor {
private:
    /**
     * @brief 
     * The file name of the stored data is set here.
     */
    std::string data_file = "data.chm";

    /**
     * @brief 
     *  Here, the hash value of the data name provided by the user is used as the primary 
     *  key to map a data structure.
     */
    std::map<unsigned long long, dataNode> mp;

    /**
     * @brief 
     * Set to false when data.chm exists but is damaged. In that case save() always
     * fails and the destructor will not overwrite the damaged file.
     */
    bool writable = true;

    Logger logger = Logger::get_logger();

    /**
     * @brief Get the hash object
     * This is a generic function, it is a generalized hash function.
     * 
     * @param s 
     * You want to take the data array of the hash value.
     * 
     * @return unsigned long long 
     * The calculated hash value.
     */
    template <class T>
    unsigned long long get_hash(T &s);

    /**
     * @brief 
     * Read the previously stored data from the file.
     * 
     * @return true 
     * The data was successfully read from the file.
     * 
     * @return false 
     * The file does not exist or the file is damaged.
     */
    bool load_file();

    /**
     * @brief 
     * Store the processed data through this function.
     * The things that need to be processed are the hash value of the data name, the 
     * hash value of the data, and the encrypted data.
     * 
     * @param name_hash 
     * The hash value of the data name.
     * 
     * @param data_hash 
     * The hash value of the data.
     * 
     * @param data 
     * The encrypted data array.
     */
    void save_data(unsigned long long name_hash, unsigned long long data_hash, const std::vector<std::pair<double, double>> &data);

    /**
     * @brief 
     * This function is used to assist the load function.
     * 
     * This function takes the first number out of the string and removes the number 
     * part from the string.
     */
    bool read(std::string &s, int &res);

public:
    Saver();
    
    /**
     * 保存的格式：
     * name_hash data_hash len 后面跟len对浮点数
    */
    ~Saver();

    /**
     * data字符串的格式:
     * 数据块的个数 
     * 每个数据块中数据的个数
     * 数据的长度
     * 数据的字符表示
     * 每个单元之间都用空格隔开
    */
    bool save(std::string name, std::vector<std::vector<std::string>> &content);
    bool load(std::string name, std::vector<std::vector<std::string>> &content, bool mandatory_access = false);
    bool exists(std::string name);
    static bool is_all_digits(std::string &s);
    static unsigned long long str_to_ull(std::string &s);
    static Saver& get_saver();
};



                        /* ======= struct dataNode ======= */
dataNode::dataNode() : name_hash(0), data_hash(0), len(0) {}

dataNode::dataNode(     unsigned long long name_hash, 
                        unsigned long long data_hash, 
                        const std::vector<std::pair<double, double>> &data) 
{
    this->name_hash = name_hash;
    this->data_hash = data_hash;
    this->len = data.size() / Encryptor::N;
    this->data = data;
}


                        /* ======= class Saver ======= */
template <class T>
unsigned long long Saver::get_hash(T &s) {
    unsigned long long seed = 13331, hash = 0;
    for (auto &ch : s) {
        hash = hash * seed + ch;
    }
    return hash;
}

bool Saver::load_file() {
    std::ifstream in(data_file);
    if (!in.good()) {
        logger.log("load_file: No data file.", Logger::WARNING, __LINE__);
        return false;
    }
    std::map<unsigned long long, dataNode> tmp;
    unsigned long long name_hash, data_hash, len;
    std::vector<std::pair<double, double>> data;
    while (in >> name_hash) {
        data.clear();
        if (!(in >> data_hash >> len)) {
            logger.log("Read interrupted, please check data integrity.", Logger::WARNING, __LINE__);
            writable = false;
            return false;
        }
        if (len > (unsigned long long)INT_MAX / N) {
            logger.log("Read interrupted, the declared data length is abnormal. Please check data integrity.", Logger::WARNING, __LINE__);
            writable = false;
            return false;
        }
        for (unsigned long long i = 0; i < len * N; i++) {
            double a, b;
            if (!(in >> a >> b)) {
                logger.log("Read interrupted, please check data integrity.", Logger::WARNING, __LINE__);
                writable = false;
                return false;
            }
            data.push_back(std::make_pair(a, b));
        }
        if (tmp.count(name_hash)) {
            logger.log("There are multiple data blocks with the same name hash. Please check data integrity.", Logger::WARNING, __LINE__);
            writable = false;
            return false;
        }
        tmp[name_hash] = dataNode(name_hash, data_hash, data);
    }
    if (!in.eof()) {
        logger.log("Read interrupted, please check data integrity.", Logger::WARNING, __LINE__);
        writable = false;
        return false;
    }
    mp.swap(tmp);
    return true;
}

/**
 * 在map中的存储方式
 * 以name_hash作为主键，会检索出一个dataNode，里面包括了name_hash, data_hash, len, data.
 * 其中 len为data的pair的对数 / N
*/
void Saver::save_data(unsigned long long name_hash, unsigned long long data_hash, const std::vector<std::pair<double, double>> &data) {
    if (mp.count(name_hash)) {
        mp.erase(mp.find(name_hash));
    }
    mp[name_hash] = dataNode(name_hash, data_hash, data);
}

bool Saver::read(std::string &s, int &res) {
    res = 0;
    if (s.empty()) return false;
    int cur = 0;
    while (cur < (int)s.size() && !isdigit((unsigned char)s[cur])) cur++;
    if (cur == (int)s.size()) return false;
    long long d = 0;
    while (cur < (int)s.size() && isdigit((unsigned char)s[cur])) {
        d = d * 10 + s[cur] - '0';
        if (d > (long long)INT_MAX) return false;
        cur++;
    }
    s.erase(s.begin(), s.begin() + cur);
    if (!s.empty()) s.erase(s.begin());
    res = (int)d;
    return true;
}

Saver::Saver() {
    load_file();
}

/**
 * 保存的格式：
 * name_hash data_hash len 后面跟len对浮点数
*/
Saver::~Saver() {
    if (!writable) return;
    std::string tmp_file = data_file + ".tmp";
    std::ofstream out(tmp_file);
    out << std::setprecision(15);
    for (auto &data : mp) {
        dataNode &dn = data.second;
        out << data.first << ' ' << dn.data_hash << ' ' << dn.len;
        for (auto &pr : dn.data) {
            out << ' ' << pr.first << ' ' << pr.second;
        }
        out << '\n';
    }
    out.flush();
    if (!out.good()) {
        logger.log("Failed to write the data file. The original file is kept.", Logger::WARNING, __LINE__);
        writable = false;
        out.close();
        std::remove(tmp_file.c_str());
        return;
    }
    out.close();
    if (rename(tmp_file.c_str(), data_file.c_str()) != 0) {
        logger.log("Failed to replace the data file. The original file is kept.", Logger::WARNING, __LINE__);
        writable = false;
        std::remove(tmp_file.c_str());
        return;
    }
}

/**
 * data字符串的格式:
 * 数据块的个数 
 * 每个数据块中数据的个数
 * 数据的长度
 * 数据的字符表示
 * 每个单元之间都用空格隔开
*/
bool Saver::save(std::string name, std::vector<std::vector<std::string>> &content) {
    if (!writable) {
        logger.log("The data file is damaged and will not be overwritten.", Logger::WARNING, __LINE__);
        return false;
    }
    std::string data;
    data += std::to_string(content.size());
    for (auto &data_block : content) {
        data += " " + std::to_string(data_block.size());
        for (auto &dt : data_block) {
            data += " " + std::to_string(dt.size()) + " " + dt;
        }
    }
    std::vector<int> sequence;
    for (auto &it : data) {
        sequence.push_back((int)it);
    }
    std::vector<std::pair<double, double>> res;
    encrypt_sequence(sequence, res);
    unsigned long long name_hash = get_hash(name);
    unsigned long long data_hash = get_hash(data);
    save_data(name_hash, data_hash, res);
    return true;
}

bool Saver::load(std::string name, std::vector<std::vector<std::string>> &content, bool mandatory_access) {
    unsigned long long name_hash = get_hash(name);
    if (!mp.count(name_hash)) {
        logger.log("Failed to load data. No data named \"" + name + "\" exists.", Logger::WARNING, __LINE__);
        return false;
    }
    dataNode &data = mp[name_hash];
    std::vector<int> sequence;
    if (!decrypt_sequence(data.data, sequence)) {
        content.clear();
        logger.log("Failed to load data. The data may be damaged.", Logger::WARNING, __LINE__);
        return false;
    }
    if (get_hash(sequence) != data.data_hash) {
        logger.log("Data failed to pass integrity verification.", Logger::WARNING, __LINE__);
        if (!mandatory_access) {
            content.clear();
            return false;
        }
    }
    std::string str;
    for (auto &it : sequence) {
        str.push_back(it);
    }
    /**
     * data字符串的格式:
     * 数据块的个数 
     * 每个数据块中数据的个数
     * 数据的长度
     * 数据字符串
    */
    content.clear();
    int block_num, data_num, data_len;
    if (!read(str, block_num)) {
        content.clear();
        logger.log("Failed to load data. The data may be damaged.", Logger::WARNING, __LINE__);
        return false;
    }
    for (int i = 0; i < block_num; i++) {
        content.push_back(std::vector<std::string>());
        if (!read(str, data_num)) {
            content.clear();
            logger.log("Failed to load data. The data may be damaged.", Logger::WARNING, __LINE__);
            return false;
        }
        for (int j = 0; j < data_num; j++) {
            if (!read(str, data_len)) {
                content.clear();
                logger.log("Failed to load data. The data may be damaged.", Logger::WARNING, __LINE__);
                return false;
            }
            if (data_len > (int)str.size()) {
                content.clear();
                logger.log("Failed to load data. The declared data length exceeds the remaining data. Please check data integrity.", Logger::WARNING, __LINE__);
                return false;
            }
            content.back().push_back(std::string(str.begin(), str.begin() + data_len));
            str.erase(str.begin(), str.begin() + data_len);
        }
    }
    if (!str.empty()) {
        content.clear();
        logger.log("Failed to load data. There are extra bytes after the data. Please check data integrity.", Logger::WARNING, __LINE__);
        return false;
    }
    return true;
}

bool Saver::exists(std::string name) {
    return mp.count(get_hash(name)) > 0;
}

bool Saver::is_all_digits(std::string &s) {
    if (s.empty()) return false;
    for (auto &ch : s) {
        if (!isdigit((unsigned char)ch)) return false;
    }
    return true;
}

unsigned long long Saver::str_to_ull(std::string &s) {
    unsigned long long res = 0;
    for (auto &ch : s) {
        res = res * 10 + ch - '0';
    }
    return res;
}

Saver& Saver::get_saver() {
    static Saver saver;
    return saver;
}

#endif
