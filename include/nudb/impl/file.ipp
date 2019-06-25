//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//           (c) 2019 Mo Morsi (mo at devnull dot network)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_IMPL_FILE_IPP
#define NUDB_IMPL_FILE_IPP

#ifdef _MSC_VER
#include <Windows.h>
#include <direct.h>   // _mkdir
#else
#include <sys/stat.h>
#include <errno.h>
#endif

namespace nudb {

inline
bool path_exists(path_type const& path){
#ifdef _MSC_VER
  return GetFileAttributesA(path.c_str()) !=
      INVALID_FILE_ATTRIBUTES;

#else
  struct stat buf;
  return (stat(path.c_str(), &buf) == 0);
#endif
}

inline
bool is_dir(path_type const& path){
#ifdef _MSC_VER
  DWORD attrs = GetFileAttributesA(path.c_str());
  if (attrs == INVALID_FILE_ATTRIBUTES)
    return false;  //something is wrong with your path!

  if (attrs & FILE_ATTRIBUTE_DIRECTORY)
    return true;   // this is a directory!

  return false;

#else
  struct stat buf;
  if(stat(path.c_str(), &buf) != 0)
    return false;

  return buf.st_mode & S_IFDIR;
#endif
}

inline
path_type
path_cat(
    path_type const& base,
    path_type const& path)
{
    if(base.empty())
        return std::string(path);
    std::string result(base);
#ifdef BOOST_MSVC
    char constexpr path_separator = '\\';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    for(auto& c : result)
        if(c == '/')
            c = path_separator;
#else
    char constexpr path_separator = '/';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append("/");
    result.append(path.data(), path.size());
#endif
    return result;
}

inline
bool mkdir_p(path_type const& path)
{
#if _MSC_VER
    int ret = _mkdir(path.c_str());
#else
    mode_t mode = 0755;
    int ret = mkdir(path.c_str(), mode);
#endif
    if (ret == 0)
        return true;

    switch (errno)
    {
    case ENOENT:
        // create parent
        {
            int pos = path.find_last_of('/');
            if ((size_t)pos == std::string::npos)
#if _MSC_VER
                pos = path.find_last_of('\\');
            if ((size_t)pos == std::string::npos)
#endif
                return false;
            if (!mkdir_p( path.substr(0, pos) ))
                return false;
        }

        // create again
#if _MSC_VER
        return 0 == _mkdir(path.c_str());
#else
        return 0 == mkdir(path.c_str(), mode);
#endif

    case EEXIST:
        // done!
        return is_dir(path);

    default:
        return false;
    }
}

} // nudb

#endif
