#pragma once

/*! \file
    \brief Разные утилитки
 */

#include "umba/filename.h"
#include "umba/filesys.h"
#include "umba/string_plus.h"
#include "umba/cli_tool_helpers.h"

//
#include <string>


inline
bool findProjectOptionsFile(const std::string &prjFile, std::string &foundOptionsFile)
{
    return umba::cli_tool_helpers::findProjectOptionsFile(prjFile, foundOptionsFile, std::vector<std::string>{".msvc-filters.options", ".umba-msvc-filters.options"});
}

inline
bool findProjectOptionsFile(const std::wstring &prjFile, std::wstring &foundOptionsFile)
{
    return umba::cli_tool_helpers::findProjectOptionsFile(prjFile, foundOptionsFile, std::vector<std::wstring>{L".msvc-filters.options", L".umba-msvc-filters.options"});
}
