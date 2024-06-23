#pragma once


#include "encoding/encoding.h"
//
#include "umba/program_location.h"
#include "umba/enum_helpers.h"
#include "umba/flag_helpers.h"
#include "umba/string_plus.h"
#include "umba/id_gen.h"
//
//#include "umba/regex_helpers.h"
//
#include "umba/filename.h"
#include "umba/filesys.h"

//
#include "umba/regex_helpers.h"

//
#include "marty_cpp/marty_cpp.h"
#include "marty_cpp/src_normalization.h"

//
#include <string>
#include <vector>
#include <map>
#include <unordered_map>



//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
template<typename FilenameStringType>
struct AppConfig
{
    std::unordered_map<std::string, std::string>          vars;
    std::vector<std::string>                              includeFilesMaskList;
    std::vector<std::string>                              excludeFilesMaskList;

    /*
    AppConfig getAdjustedConfig( const umba::program_location::ProgramLocation<std::string> &programLocation ) const
    {
        AppConfig appConfig = *this;

        for(auto includeFileMask: includeFilesMaskList)
        {
            if (umba::string_plus::starts_with(includeFileMask,umba::regex_helpers::getRawEcmaRegexPrefix<std::string>()))
                appConfig.includeFilesMaskList.push_back(includeFileMask); // keep regex as is
            else
                appConfig.includeFilesMaskList.push_back( umba::filename::normalizePathSeparators(includeFileMask,'/') );
        }

        for(auto excludeFileMask: excludeFilesMaskList)
        {
            if (umba::string_plus::starts_with(excludeFileMask,umba::regex_helpers::getRawEcmaRegexPrefix<std::string>()))
                appConfig.excludeFilesMaskList.push_back(excludeFileMask); // keep regex as is
            else
                appConfig.excludeFilesMaskList.push_back( umba::filename::normalizePathSeparators(excludeFileMask,'/') );
        }

        return appConfig;
    }
    */


    bool addVar(std::string name, std::string value)
    {
        umba::string_plus::trim(name);
        umba::string_plus::trim(value);

        if (name.empty())
            return false;

        vars[name] = value;
        return true;
    }

    bool addVar(std::string nameValue)
    {
        umba::string_plus::trim(nameValue);
        
        std::vector<std::string> nameValuePair = marty_cpp::splitToLinesSimple(nameValue, false, ':');
        if (nameValuePair.empty())
            return false;
        if (nameValuePair.size()<2)
            return addVar(nameValue,std::string());
        else
            return addVar(nameValuePair[0],nameValuePair[1]);
    }

    #if 0
    bool addSamplesPaths(const std::vector<FilenameStringType> &pl, const FilenameStringType &basePath)
    {
        //std::vector<FilenameStringType> &dirs = m_lookupMap[lvl].lookupDirs;

        std::vector<FilenameStringType> absPaths; absPaths.reserve(pl.size());
        for(auto p: pl)
        {
            p = umba::filename::makeAbsPath(p, basePath);
            absPaths.emplace_back(umba::filename::makeCanonical(p));
        }
        
        samplesPaths.insert( samplesPaths.end(), absPaths.begin(), absPaths.end() );
        return true;
    }

    //! Добавляет пути поиска примеров
    /*! Разделителем является спец символ:
        - Win32 - ';' (точка с запятой, semicolon)
        - Linux - ':' (двоеточие, colon)
     */
    bool addSamplesPaths(const FilenameStringType &pl, const FilenameStringType &basePath)
    {
        return addSamplesPaths(umba::filename::splitPathList( pl ), basePath);
    }
    #endif

    static
    std::string autoEncodeToUtf(const std::string& text)
    {
        size_t bomSize = 0;
        //const charDataPtr = 
        encoding::EncodingsApi* pEncodingsApi = encoding::getEncodingsApi();
        std::string detectRes = pEncodingsApi->detect(text.data(), text.size(), bomSize);
        auto cpId = pEncodingsApi->getCodePageByName(detectRes);
        std::string utfText = pEncodingsApi->convert(text.data() + bomSize, text.size() - bomSize, cpId, encoding::EncodingsApi::cpid_UTF8);
        return utfText;
    }

    static
    bool readInputFile(const std::string &inputFilename, std::string &inputFileText)
    {
        std::string inputFileTextOrg;

        if (!umba::filesys::readFile(inputFilename, inputFileTextOrg))
        {
            return false;
        }

        inputFileText = autoEncodeToUtf(inputFileTextOrg);
        inputFileText = marty_cpp::normalizeCrLfToLf(inputFileText);

        return true;
    }

    static
    bool readInputFile(const std::wstring &inputFilename, std::string &inputFileText)
    {
        std::string inputFileTextOrg;

        if (!umba::filesys::readFile(inputFilename, inputFileTextOrg))
        {
            return false;
        }

        inputFileText = autoEncodeToUtf(inputFileTextOrg);
        inputFileText = marty_cpp::normalizeCrLfToLf(inputFileText);

        return true;
    }

    #if 0
    static
    bool findSamplesFileImpl(const std::vector<FilenameStringType> &samplesPathsVec, FilenameStringType lookFor, FilenameStringType &foundFullFilename, std::string &foundFileText)
    {
         lookFor = umba::filename::makeCanonical(lookFor);

         for(auto path: samplesPathsVec)
         {
             auto fullName = umba::filename::appendPath(path, lookFor);
             if (readInputFile(fullName, foundFileText))
             {
                  foundFullFilename = fullName;
                  return true;
             }
             // std::string orgText;
             // if (umba::filesys::readFile(fullName, orgText))
             // {
             //     orgText           = autoEncodeToUtf(orgText);
             //     foundFileText     = marty_cpp::normalizeCrLfToLf(orgText);
             //     foundFullFilename = fullName;
             //     return true;
             // }
         }

         return false;
    }

    bool findSamplesFile(const FilenameStringType &lookFor, FilenameStringType &foundFullFilename, std::string &foundFileText) const
    {
        return findSamplesFileImpl(samplesPaths, lookFor, foundFullFilename, foundFileText);
    }

    bool findSamplesFileUseExtraPath(const FilenameStringType &lookFor, FilenameStringType &foundFullFilename, FilenameStringType &foundFileText, const FilenameStringType &extraPath) const
    {
        auto p = samplesPaths;
        p.emplace_back(extraPath);
        return findSamplesFileImpl(p, lookFor, foundFullFilename, foundFileText);
    }

    bool findSamplesFileUseExtraPathFromFilename(const FilenameStringType &lookFor, FilenameStringType &foundFullFilename, std::string &foundFileText, const FilenameStringType &fileInsertedFrom) const
    {
        return findSamplesFileUseExtraPath(lookFor, foundFullFilename, foundFileText, umba::filename::getPath(fileInsertedFrom));
    }

    static
    bool findDocFileByPath(const FilenameStringType &lookFor, std::string &foundFullFilename, std::string &foundFileText, const std::string &includedFromPath)
    {
        std::vector<FilenameStringType> docPaths;
        docPaths.emplace_back(includedFromPath);
        return findSamplesFileImpl(docPaths, lookFor, foundFullFilename, foundFileText);
    }

    static
    bool findDocFileByIncludedFromFilename(const FilenameStringType &lookFor, FilenameStringType &foundFullFilename, std::string &foundFileText, const FilenameStringType &includedFromFile)
    {
        return findDocFileByPath(lookFor, foundFullFilename, foundFileText, umba::filename::getPath(includedFromFile));
    }

    static
    FilenameStringType normalizeExt(FilenameStringType ext)
    {
        while(!ext.empty() && ext.front()==(typename FilenameStringType::value_type)'.')
        {
            ext.erase(0,1);
        }

        return marty_cpp::toLower(ext);
    }
    #endif



}; // struct AppConfig




