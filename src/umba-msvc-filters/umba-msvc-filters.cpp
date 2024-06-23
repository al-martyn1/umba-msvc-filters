/*! \file
    \brief Утилита umba-msvc-filters - генерим .filters файлы для MSVC
 */

// Должна быть первой
#include "umba/umba.h"
//---

//#-sort
#include "umba/simple_formatter.h"
#include "umba/char_writers.h"
//#+sort

#include "umba/debug_helpers.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
// #include <filesystem>

#include "umba/debug_helpers.h"
#include "umba/string_plus.h"
#include "umba/program_location.h"
#include "umba/scope_exec.h"
#include "umba/macro_helpers.h"
#include "umba/macros.h"

#include "encoding/encoding.h"
#include "umba/cli_tool_helpers.h"
#include "umba/time_service.h"

#include "umba/filesys.h"
#include "umba/filename.h"
#include "umba/format_message.h"
#include "umba/id_gen.h"
#include "umba/utf8.h"
//

#include "encoding/encoding.h"
#include "utils.h"

//
#include "msvc_sln.h"



#if defined(WIN32) || defined(_WIN32)

    #define HAS_CLIPBOARD_SUPPORT 1
    #include "umba/clipboard_win32.h"

#endif


#include "app_config.h"


umba::StdStreamCharWriter coutWriter(std::cout);
umba::StdStreamCharWriter cerrWriter(std::cerr);
umba::NulCharWriter       nulWriter;

umba::SimpleFormatter umbaLogStreamErr(&cerrWriter);
umba::SimpleFormatter umbaLogStreamMsg(&cerrWriter);
umba::SimpleFormatter umbaLogStreamNul(&nulWriter);

bool umbaLogGccFormat   = false; // true;
bool umbaLogSourceInfo  = false;

bool bOverwrite         = false;



std::string               inputFilename;
//std::string               outputFilename;

#include "log.h"
//
#include "umba/cmd_line.h"
//

AppConfig<std::string> appConfig;

// Конфиг версии
#include "app_ver_config.h"
// Парсер параметров командной строки
#include "arg_parser.h"

//
std::string curFile;
unsigned lineNo = 0;


int safe_main(int argc, char* argv[]);

int main(int argc, char* argv[])
   {
       try
       {
           int res = safe_main(argc, argv);
           //if (res!=0 && rbcOpts.pauseOnError)
           {
               //std::cout << "Press any key to exit" << std::endl;
               // while(!_kbhit()) {}
               // _getch();
           }

           return res;
       }
       catch(const std::exception &e)
       {
           LOG_ERR_OPT<<"fatal error: "<<e.what()<<umba::omanip::endl;
       }
       catch(...)
       {
           LOG_ERR_OPT<<"fatal error: unknown error"<<umba::omanip::endl;
       }

       return 100;
   }


int safe_main(int argc, char* argv[])

{

    using namespace umba::omanip;


    auto argsParser = umba::command_line::makeArgsParser( ArgParser<std::string>()
                                                        , CommandLineOptionCollector()
                                                        , argc, argv
                                                        , umba::program_location::getProgramLocation
                                                            ( argc, argv
                                                            , false // useUserFolder = false
                                                            //, "" // overrideExeName
                                                            )
                                                        );

    //programLocationInfo = argsParser.programLocationInfo;

    // Force set CLI arguments while running under debugger
    if (umba::isDebuggerPresent())
    {
        std::string cwd = umba::filesys::getCurrentDirectory<std::string>();
        //if (getCurrentDirectory(cwd))
        {
            std::cout << "Working Dir: " << cwd << "\n";
        }

        #if (defined(WIN32) || defined(_WIN32))

            #if defined(__GNUC__)

                std::string rootPath = "..\\..\\..\\..\\..\\";

            #else // if

                std::string rootPath = "..\\";

            #endif

        #endif

        argsParser.args.clear();

        // argsParser.args.push_back("@" + rootPath + "_distr_conf/conf/umba-md-pp.options");
        //  
        // argsParser.args.push_back("--overwrite");
        // argsParser.args.push_back("--add-examples-path="+rootPath+"/tests/snippets");
        // argsParser.args.push_back("--add-examples-path="+rootPath+"/tests/..");
        // argsParser.args.push_back(rootPath + "tests\\test03.md_");
        // // F:\_github\umba-tools\umba-md-pp

        //argsParser.args.push_back("--register-view-handler");

        //argsParser.args.push_back("C:\\work\\github\\umba-tools\\umba-roboconf\\README.md_");

        argsParser.args.push_back(rootPath + "/umba-msvc-filters-msvc2019.sln");

    }


        // Job completed - may be, --where option found
        if (argsParser.mustExit)
            return 0;
       
        if (!argsParser.parseStdBuiltins())
            return 1;
        if (argsParser.mustExit)
            return 0;
       
        if (!argsParser.parse())
            return 1;
        if (argsParser.mustExit)
            return 0;


    std::vector<std::string> inputFiles;
    std::string inputFileExt  = umba::filename::getExt(inputFilename);
    std::string inputFileType = umba::string_plus::toupper_copy(inputFileExt);


    if (inputFileType=="VCXPROJ")
    {
        inputFiles.emplace_back(inputFilename);

        std::string projectOptionsFile;
        if (findProjectOptionsFile(inputFilename, projectOptionsFile))
        {
            argsParser.parseOptionsFile(projectOptionsFile);
        }

    }

    else if (inputFileType=="SLN")
    {
        // парсим .sln и заполняем inputFiles

        std::string slnFileText;
        //if (!umba::filesys::readFile(inputFilename, inputFileText))
        if (!AppConfig<std::string>::readInputFile(inputFilename, slnFileText))
        {
            LOG_ERR_OPT << umba::formatMessage("failed to read input file: '$(fileName)'")
                                              .arg("fileName",umba::toUtf8(inputFilename))
                                              .toString()
                        << "\n";
            return 1;
        }

        umba::msvc::Solution sln;
        if (!sln.parse(slnFileText))
        {
            LOG_ERR_OPT << umba::formatMessage("failed to parse solution file: '$(fileName)'")
                                              .arg("fileName",umba::toUtf8(inputFilename))
                                              .toString()
                        << "\n";
            return 1;
        
        }

        std::string projectOptionsFile;
        if (findProjectOptionsFile(inputFilename, projectOptionsFile))
        {
            argsParser.parseOptionsFile(projectOptionsFile);
        }


        auto slnPath = umba::filename::getPath(inputFilename);
        std::map<std::string,std::regex>  excludeRegexes;
        for(auto excludeFileMask : appConfig.excludeFilesMaskList)
        {
            auto regexStr                  = umba::regex_helpers::expandSimpleMaskToEcmaRegex( excludeFileMask, true /* useAnchoring */, true /* allowRawRegexes */ );
            excludeRegexes      [regexStr] = std::regex(regexStr);
            //excludeOriginalMasks[regexStr] = excludeFileMask;
        }


        for(const auto prjInfo : sln.projects)
        {
            // auto normalizedEntryName = umba::filename::normalizePathSeparators(entryName,'/');
            //  
            // //TODO: !!! Нужно что-то решать с отсутствующим расширением
            //  
            // bool addThisFile = false;
            // bool excludedByIncludeMask = false;
            //  
            // std::string includeRegexStr;
            // std::string excludeRegexStr;
            //  
            // bool matchInclude = true;
            // if (!includeRegexes.empty()) // матчим только если не пусто
            // {
            //     matchInclude = umba::regex_helpers::regexMatch(normalizedEntryName,includeRegexes,&includeRegexStr);

            std::string projectFileName = umba::filename::isAbsPath(prjInfo.projectFile)
                                        ? prjInfo.projectFile
                                        : umba::filename::appendPath(slnPath, prjInfo.projectFile)
                                        ;
            std::string canonicalProjectFileName = umba::filename::makeCanonical(projectFileName);
            auto normalizedProjectFileName = umba::filename::normalizePathSeparators(canonicalProjectFileName,'/');
            auto lowerNormalizedProjectFileName = umba::string_plus::tolower_copy(normalizedProjectFileName);
            //auto lowerNormalizedProjectFileName = 

            std::string matchedRegex;
            if (!umba::regex_helpers::regexMatch(lowerNormalizedProjectFileName, excludeRegexes, &matchedRegex))
            {
                inputFiles.emplace_back(canonicalProjectFileName);
            }
            
        }
        //bool isAbsPath( StringType p, typename StringType::value_type pathSep = getNativePathSep<typename StringType::value_type>() )


    }
    else
    {
        LOG_ERR_OPT << umba::formatMessage("unknown file type: '$(fileName)'")
                                          .arg("fileName",umba::toUtf8(inputFilename))
                                          .toString()
                    << "\n";
        return 1;
    }


    // std::string projectOptionsFile;
    // if (findProjectOptionsFile(inputFilename, projectOptionsFile))
    // {
    //     appConfig.setStrictPathFromFilename(projectOptionsFile);
    //     argsParser.parseOptionsFile(projectOptionsFile);
    // }

    // std::string projectOptionsFile;
    // if (findProjectOptionsFile(inputFilename, projectOptionsFile))
    // {
    //     appConfig.setStrictPathFromFilename(projectOptionsFile);
    //     argsParser.parseOptionsFile(projectOptionsFile);
    // }

    //auto appConfigBackup = appConfig;


    for(auto inputProjectFile: inputFiles)
    {
        if (!argsParser.quet)
        {
            LOG_MSG_OPT << "Processing '" << inputProjectFile << "'\n";
        }

        //appConfig = appConfigBackup;

        // std::string projectOptionsFile;
        // if (findProjectOptionsFile(inputFile, projectOptionsFile))
        // {
        //     argsParser.parseOptionsFile(projectOptionsFile);
        // }

        std::string prjFileText;
        //if (!umba::filesys::readFile(inputFilename, inputFileText))
        if (!AppConfig<std::string>::readInputFile(inputProjectFile, prjFileText))
        {
            LOG_ERR_OPT << umba::formatMessage("failed to read input file: '$(fileName)'")
                                              .arg("fileName",umba::toUtf8(prjFileText))
                                              .toString()
                        << "\n";
            return 1;
        }

    }



    try
    {

    } // try
    catch(const std::runtime_error &e)
    {
        LOG_ERR_OPT << e.what() << "\n";
        return 1;
    }

    return 0;
}




