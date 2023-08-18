#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <iostream>
#include <fstream>
#include <string>

#include <iostream>
#include <fstream>
#include <cstdint>

class BinaryFileHandler {
public:
    static int CreateBinaryFile(const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        if (!file) {
            std::cout << "无法创建文件：" << filename << std::endl;
            return -1;
        }

        file.close();
        std::cout << "文件创建成功：" << filename << std::endl;
        return 0;
    }

    static int WriteBinaryFile(const std::string& filename, const uint8_t* data, uint32_t size) {
        std::ofstream file(filename, std::ios::binary);
        if (!file) {
            std::cout << "无法打开文件：" << filename << std::endl;
            return -1;
        }

        file.write(reinterpret_cast<const char*>(data), size);
        file.close();

        std::cout << "数据已成功写入到文件：" << filename << std::endl;
        return 0;
    }

    static int ReadBinaryFile(const std::string& filename, uint8_t* data, uint32_t size) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            std::cout << "无法打开文件：" << filename << std::endl;
            return -1;
        }

        file.read(reinterpret_cast<char*>(data), size);
        file.close();

        std::cout << "数据已成功从文件中读取：" << filename << std::endl;
        return 0;
    }

    static bool DeleteFile(const std::string& filename) {
        if (std::remove(filename.c_str()) != 0) {
            std::cout << "无法删除文件：" << filename << std::endl;
            return false;
        }

        std::cout << "文件删除成功：" << filename << std::endl;
        return true;
    }

    static int GetFileSize(const std::string& filename, uint32_t& size) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file) {
            std::cout << "无法打开文件：" << filename << std::endl;
            return -1;
        }

        size = static_cast<uint32_t>(file.tellg());
        file.close();

        std::cout << "文件大小为：" << size << " 字节" << std::endl;
        return 0;
    }
};




#endif // FILE_HANDLER_H
