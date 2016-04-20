#pragma once

#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <map>

class Config
{
public:
    static Config* GetInstance()
    {
        if(instance == 0)
        {
            instance = new Config;
        }
        return instance;
    }

    std::vector<std::string> GetLibDirs()
    {
        std::vector<std::string> dirs;
        std::string cfgvalue = m_configs["libdirs"];
        cfgvalue.erase(cfgvalue.begin());
        cfgvalue.erase(cfgvalue.end()-1);
        cfgvalue = this->Trim(cfgvalue);
        std::istringstream iss(cfgvalue);
        std::string dir;
        while( std::getline(iss, dir, ',') )
        {
            dir = this->Trim(dir);
            dir.erase(dir.begin());
            dir.erase(dir.end()-1);
            dir = this->Trim(dir);
            dirs.push_back(dir);
        }
        return dirs;
    }

    // NOTE: default behavior is false
    bool GetRepeat()
    {
        return m_configs["repeat"] == "true";
    }

    // NOTE: default behavior is false
    bool GetShuffle()
    {
        return m_configs["shuffle"] == "true";
    }
private:
    Config()
    {
        struct passwd *pw = getpwuid(getuid());
        std::string configpath = std::string(pw->pw_dir) + "/.mmconfig";
        if(std::ifstream(configpath))
        {
            // std::cout << "found config file" << std::endl;
        }
        else
        {
            // std::cout << "config file not found, creating one" << std::endl;
            std::ofstream configfile(configpath);
            configfile << "# configuration file for music-magister" << std::endl;
            configfile << std::endl;
            configfile << "repeat = false" << std::endl;
            configfile << "shuffle = true" << std::endl;
            configfile << "libdirs = ['" << pw->pw_dir << "/Music']" << std::endl;
        }
        this->Process(configpath);
    }

    void Process(std::string filename)
    {
        std::ifstream infile(filename);
        std::string line;
        while (std::getline(infile, line))
        {
            this->ParseLine(line);
        }
    }

    std::string Trim(std::string s)
    {
        std::string::const_iterator it = s.begin();
        while (it != s.end() && isspace(*it))
            it++;
        std::string::const_reverse_iterator rit = s.rbegin();
        while (rit.base() != it && isspace(*rit))
            rit++;
        s = std::string(it, rit.base());
        return s;
    }

    void ParseLine(std::string line)
    {
        line = this->Trim(line);
        std::istringstream iss(line);
        std::string key, value;
        if(line.length() > 0 && line.at(0) != '#')
        {
            if( std::getline(iss, key, '=') )
            {
                std::getline(iss, value);
                m_configs[this->Trim(key)] = this->Trim(value);
                //std::cout << "key: " << key << ", value: " << value << std::endl;
            }
        }
    }


    std::map<std::string, std::string> m_configs;
    static Config* instance;
};

Config* Config::instance = 0;
