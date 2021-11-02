#pragma once

#include <sys/stat.h>  // TODO make utils.hpp

inline bool fileExists(const std::string& name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

namespace hidden {  //TODO move it

template <typename T>
inline void writeToFile(std::ofstream& fout, const T& x) {  //TODO move
    fout.write(reinterpret_cast<const char*>(&x), sizeof(x));
}

template <>
inline void writeToFile<std::string>(std::ofstream& fout, const std::string& s) {  //TODO move to utils
    writeToFile(fout, s.length());
    for (const char& c : s) {
        fout.write((const char*)&c, sizeof(unsigned char));
    }
}

template <>
inline void writeToFile<std::vector<bool>>(std::ofstream& fout, const std::vector<bool>& v) {  //TODO move to utils
    std::vector<bool>::size_type n = v.size();
    writeToFile(fout, n);

    for (std::vector<bool>::size_type i = 0; i < n;) {
        unsigned char aggr = 0;
        for (unsigned char mask = 1; mask > 0 && i < n; ++i, mask <<= 1)
            if (v.at(i))
                aggr |= mask;
        fout.write((const char*)&aggr, sizeof(unsigned char));
    }
}

template <typename T, typename... Args>
void writeToFile(std::ofstream& fout, T t, Args... args) {
    writeToFile(fout, t);
    writeToFile(fout, args...);
}

template <typename T>
inline T getFromFile(std::ifstream& fin) {  //TODO move
    T x;
    fin.read(reinterpret_cast<char*>(&x), sizeof(x));
    return x;
}

template <>
inline std::string getFromFile<std::string>(std::ifstream& fin) {  //TODO move
    const std::size_t& length = getFromFile<std::size_t>(fin);
    char ct;
    std::string s = "";
    for (std::size_t i = 0; i < length; i++) {
        fin.read((char*)&ct, sizeof(unsigned char));
        s += ct;
    }
    return s;
}

}  // namespace hidden

template <typename... Args>
void writeToFile(std::ofstream& fout, Args... args) {
    hidden::writeToFile(fout, args...);
}

template <typename T>
inline T getFromFile(std::ifstream& fin) {  //TODO move
    T x;
    fin.read(reinterpret_cast<char*>(&x), sizeof(x));
    return x;
}

template <>
inline std::string getFromFile<std::string>(std::ifstream& fin) {  //TODO move
    const std::size_t& length = getFromFile<std::size_t>(fin);
    char ct;
    std::string s = "";
    for (std::size_t i = 0; i < length; i++) {
        fin.read((char*)&ct, sizeof(unsigned char));
        s += ct;
    }
    return s;
}

// class FileWriter {
//    private:
//     const std::string _filename;

//    public:
//     FileWriter(const std::string filename) : _filename(filename) {}

//     template <typename T, typename... Args>
//     void write(T t, Args... args) {
//         writeToFile(T);
//         this->
//     }

//     ~FileWriter();
// };