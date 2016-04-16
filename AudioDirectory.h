#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

class AudioDirectory
{
public:
    void SetPath(std::string path)
    {
        m_path = path;
        this->Parse();
    }
    std::vector<std::string> GetFiles()
    {
        return m_files;
    }
private:
    void Parse()
    {
        DIR* dp;
        struct dirent *dirp;
        std::vector<std::string> dirs;
        struct stat st;

        dirs.push_back(m_path);
        while(dirs.size() > 0)
        {
            if((dp = opendir(dirs[0].c_str())) != NULL)
            {
                while((dirp = readdir(dp)) != NULL)
                {
                    if(std::string(".").compare(dirp->d_name) == 0
                        || std::string("..").compare(dirp->d_name) == 0)
                    {
                        continue;
                    }
                    std::string filename = dirs[0] + "/" + dirp->d_name;
                    stat(filename.c_str(), &st);
                    if(S_ISDIR(st.st_mode))
                    {
                        dirs.push_back(filename);
                    }
                    else
                    {
                        if(this->string_ends_with(filename, ".mp3"))
                        {
                            m_files.push_back(filename);
                        }
                    }
                }
                closedir(dp);
            }
            dirs.erase(dirs.begin());
        }
    }

    // no camel case here, don't know why
    bool string_ends_with(std::string const & value, std::string const & ending)
    {
        if (ending.size() > value.size()) return false;
        return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
    }

    std::string m_path;
    std::vector<std::string> m_files;
};
