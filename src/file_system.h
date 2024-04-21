//
// Created by Alether on 4/16/2024.
//

#ifndef ALETHERENGINE_FILE_SYSTEM_H
#define ALETHERENGINE_FILE_SYSTEM_H

#include"config.h"
#include<string>

using namespace std;

namespace ale {

class FileSystem {
public:
    static string root(const string& path);
};

} // ale

#endif //ALETHERENGINE_FILE_SYSTEM_H
