/// @file options.h
/// @brief configuration options
/// @author Jeff Perry <jeffsp@gmail.com>
/// @date 2013-09-03

#ifndef OPTIONS_H
#define OPTIONS_H

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>

namespace soma
{

/// @brief configuration option
///
/// @tparam T option type
template<typename T>
struct option
{
    /// @brief constructor
    ///
    /// @param value option value
    /// @param name option name
    option (const T &value, const std::string &name)
        : value (value), name (name) { }
    T value;
    std::string name;
    /// @brief read an option
    ///
    /// @tparam S stream type
    /// @param s stream
    template<typename S>
    void parse (S &s)
    {
        //std::clog << "parsing " << name << " option" << std::endl;
        std::string tmp_name;
        T tmp_value;
        s >> tmp_name >> tmp_value;
        //std::clog << tmp_name << ' ' << tmp_value << std::endl;
        if (name != tmp_name)
            throw std::runtime_error ("warning: didn't get expected option");
        value = tmp_value;
    }
};

/// @brief configuration options
class options
{
    private:
    /// @brief version information
    option<int> major_revision;
    /// @brief version information
    option<int> minor_revision;
    /// @brief turn sound on/off
    option<bool> sound;
    public:
    /// @brief constructor
    options ()
        : major_revision (MAJOR_REVISION, "major_revision")
        , minor_revision (MINOR_REVISION, "minor_revision")
        , sound (true, "sound")
    {
    }
    /// @brief option access
    int get_major_revision () const
    {
        return major_revision.value;
    }
    /// @brief option access
    int get_minor_revision () const
    {
        return minor_revision.value;
    }
    /// @brief option access
    bool get_sound () const
    {
        return sound.value;
    }
    /// @brief option access
    void set_sound (bool f)
    {
        if (f == sound.value)
            return;
        sound.value = f;
    }
    /// @brief i/o helper
    friend std::ostream& operator<< (std::ostream &s, const options &opts)
    {
        s << opts.major_revision.name << " " << opts.major_revision.value << std::endl;
        s << opts.minor_revision.name << " " << opts.minor_revision.value << std::endl;
        s << opts.sound.name << " " << opts.sound.value << std::endl;
        return s;
    }
    /// @brief i/o helper
    friend std::istream& operator>> (std::istream &s, options &opts)
    {
        try
        {
            // get version information
            opts.major_revision.parse (s);
            opts.minor_revision.parse (s);
            if (opts.major_revision.value != MAJOR_REVISION)
                throw std::runtime_error ("warning: configuration file major revision is not the same as this programs's major revision number");
            if (opts.minor_revision.value > MINOR_REVISION)
                throw std::runtime_error ("warning: configuration file revision number is newer than this program's revision number");
            opts.sound.parse (s);
        }
        catch (const std::exception &e)
        {
            std::clog << e.what () << std::endl;
            std::clog << "cannot parse configuration file" << std::endl;
            std::clog << "setting options to their defaults" << std::endl;
            // set to default
            opts = options ();
        }
        return s;
    }
};

/// @brief helper
///
/// @param opts options
/// @param fn filename
void read (options &opts, const std::string &fn)
{
    std::clog << "reading configuration file " << fn << std::endl;
    std::ifstream ifs (fn.c_str ());
    if (!ifs)
        throw std::runtime_error ("could not open config file for reading");
    ifs >> opts;
}

/// @brief helper
///
/// @param opts options
/// @param fn filename
void write (const options &opts, const std::string &fn)
{
    std::clog << "writing configuration file " << fn << std::endl;
    std::ofstream ofs (fn.c_str ());
    if (!ofs)
        throw std::runtime_error ("could not open config file for writing");
    ofs << opts;
}

/// @brief get the directory of the configuration file, creating the directory if needed
///
/// @return name of the config directory
std::string get_config_dir ()
{
    std::string config_dir;
    if (getenv ("XDG_CONFIG_HOME"))
        config_dir = getenv ("XDG_CONFIG_HOME");
    else if (getenv ("HOME"))
        config_dir = getenv ("HOME") + std::string ("/.config");
    else
        config_dir = "~/.config";
    config_dir += "/soma";
    struct stat sb;
    if (stat (config_dir.c_str (), &sb) == -1)
    {
        std::clog << "creating config file directory " << config_dir << std::endl;
        mkdir (config_dir.c_str (), 0700);
    }
    if (stat (config_dir.c_str (), &sb) == -1)
        throw std::runtime_error ("could not create config file directory");
    return config_dir;
};

} // namespace soma

#endif
