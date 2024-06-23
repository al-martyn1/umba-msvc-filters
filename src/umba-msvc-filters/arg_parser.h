#pragma once

#include <stack>

//#include "app_config.h"
#include "umba/cmd_line.h"
#include "umba/cli_tool_helpers.h"


#if defined(WIN32) || defined(_WIN32)
    #include <shellapi.h>
#endif

// AppConfig    appConfig;

template<typename StringType>
struct ArgParser
{

std::stack<StringType> optFiles;


StringType getBasePath() const
{
    StringType basePath;
    if (optFiles.empty())
        basePath = umba::filesys::getCurrentDirectory<StringType>();
    else
        basePath = umba::filename::getPath(optFiles.top());

    return basePath;
}


StringType makeAbsPath( StringType p )
{
    return umba::filename::makeAbsPath( p, getBasePath() );
}



// 0 - ok, 1 normal stop, -1 - error
template<typename ArgsParser>
int operator()( const StringType                                &a           //!< строка - текущий аргумент
              , umba::command_line::CommandLineOption           &opt         //!< Объект-опция, содержит разобранный аргумент и умеет отвечать на некоторые вопросы
              , ArgsParser                                      &argsParser  //!< Класс, который нас вызывает, содержит некоторый контекст
              , umba::command_line::ICommandLineOptionCollector *pCol        //!< Коллектор опций - собирает инфу по всем опциям и готов вывести справку
              , bool fBuiltin
              , bool ignoreInfos
              )
{
    //using namespace marty::clang::helpers;

    std::string dppof = "Don't parse predefined options from ";

    if (opt.isOption())
    {
        std::string errMsg;
        std::string strVal;
        //int intVal;
        //unsigned uintVal;
        std::size_t szVal;
        bool boolVal;

        if (opt.name.empty())
        {
            LOG_ERR_OPT<<"invalid (empty) option name\n";
            return -1;
        }

       if (opt.isOption("quet") || opt.isOption('q') || opt.setDescription("Operate quetly"))  // . Short alias for '--verbose=quet'
        {
            argsParser.quet = true;
            //appConfig.setOptQuet(true);
        }

        #if defined(WIN32) || defined(_WIN32)
        else if (opt.isOption("home") || opt.setDescription("Open homepage"))
        {
            if (argsParser.hasHelpOption) return 0;
            ShellExecuteA( 0, "open", appHomeUrl, 0, 0, SW_SHOW );
            return 1;
        }
        #endif

        else if ( opt.isBuiltinsDisableOptionMain  () 
               || opt.setDescription( dppof + "main distribution options file '" + argsParser.getBuiltinsOptFileName(umba::program_location::BuiltinOptionsLocationFlag::appGlobal   ) + "'"))
        { } // simple skip - обработка уже сделана

        else if ( opt.isBuiltinsDisableOptionCustom() 
               || opt.setDescription( dppof + "custom global options file '"     + argsParser.getBuiltinsOptFileName(umba::program_location::BuiltinOptionsLocationFlag::customGlobal) + "'"))
        { } // simple skip - обработка уже сделана

        else if ( opt.isBuiltinsDisableOptionUser  () 
               || opt.setDescription( dppof + "user local options file '"        + argsParser.getBuiltinsOptFileName(umba::program_location::BuiltinOptionsLocationFlag::userLocal   ) + "'"))
        { } // simple skip - обработка уже сделана

        else if (opt.isOption("version") || opt.isOption('v') || opt.setDescription("Show version info"))
        {
            if (argsParser.hasHelpOption) return 0;

            if (!ignoreInfos)
            {
                umba::cli_tool_helpers::printOnlyVersion(std::cout);
                return 1;
            }
        }

        else if (opt.isOption("where") || opt.setDescription("Show where the executable file is"))
        {
            if (argsParser.hasHelpOption) return 0;

            LOG_MSG_OPT << argsParser.programLocationInfo.exeFullName << "\n";
            return 0;
        }

        else if (opt.setParam("CLR", 0, "no/none/file|" 
                                        "ansi/term|" 
                                        #if defined(WIN32) || defined(_WIN32)
                                        "win32/win/windows/cmd/console"
                                        #endif
                             )
              || opt.setInitial(-1) || opt.isOption("color") 
              || opt.setDescription("Force set console output coloring")
              /* ", can be:\nno, none, file - disable coloring\nansi, term - set ansi terminal coloring\nwin32, win, windows, cmd, console - windows console specific coloring method" */
              )
        {
            if (argsParser.hasHelpOption) return 0;

            umba::term::ConsoleType res;
            auto mapper = [](int i) -> umba::term::ConsoleType
                          {
                              switch(i)
                              {
                                  case 0 : return umba::term::ConsoleType::file;
                                  case 1 : return umba::term::ConsoleType::ansi_terminal;
                                  case 2 : return umba::term::ConsoleType::windows_console;
                                  default: return umba::term::ConsoleType::file;
                              };
                          };
            if (!opt.getParamValue( res, errMsg, mapper ) )
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }

            coutWriter.forceSetConsoleType(res);
            cerrWriter.forceSetConsoleType(res);

            return 0;
        }

        else if ( opt.setParam("?MODE",true)
               || opt.isOption("overwrite") || opt.isOption('Y') 
               // || opt.setParam("VAL",true)
               || opt.setDescription("Allow overwrite existing file."))
        {
            if (argsParser.hasHelpOption) return 0;

            if (!opt.getParamValue(boolVal,errMsg))
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }
            
            bOverwrite = boolVal;
            return 0;
        }

        else if ( opt.setParam("MASK,...") || opt.isOption("exclude-files") || opt.isOption('X')
               || opt.setDescription("Exclude files from parsing. The 'MASK' parameter is a simple file mask, where '*' "
                                     "means any number of any chars, and '?' means exact one of any char. In addition, "
                                     "symbol '^' in front and/or back of the mask means that the mask will be bound to beginning/ending "
                                     "of the tested file name.\n"
                                     "Also, regular expresion syntax allowed in form '" + 
                                     umba::regex_helpers::getRawEcmaRegexPrefix<std::string>() + "YOURREGEX'. The regular expresions supports\n"
                                     "See also: C++ Modified ECMA Script regular expression grammar - https://en.cppreference.com/w/cpp/regex/ecmascript"
                                    )
                )
        {
            if (argsParser.hasHelpOption) return 0;
            
            if (!opt.hasArg())
            {
                LOG_ERR_OPT<<"exclude files mask not taken (--exclude-files)\n";
                return -1;
            }

            std::vector< std::string > lst = umba::string_plus::split(opt.optArg, ',');
            appConfig.excludeFilesMaskList.insert(appConfig.excludeFilesMaskList.end(), lst.begin(), lst.end());

            return 0;
        }
        /*
        else if ( opt.setParam("LINEFEED",umba::command_line::OptionType::optString)
               || opt.isOption("linefeed") || opt.isOption("LF") || opt.isOption('L')
               // || opt.setParam("VAL",true)
               || opt.setDescription("Output linefeed. LINEFEED is one of: CR/LF/CRLF/LFCR/DETECT. "
                                     #if defined(WIN32) || defined(_WIN32)
                                     "Default is CRLF."
                                     #else
                                     "Default is LF."
                                     #endif
                                    )
                )
        {
            if (argsParser.hasHelpOption) return 0;

            if (!opt.getParamValue(strVal,errMsg))
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }
            
            marty_cpp::ELinefeedType tmp = marty_cpp::enum_deserialize( strVal, marty_cpp::ELinefeedType::invalid );
            if (tmp== marty_cpp::ELinefeedType::invalid)
            {
                LOG_ERR_OPT<<"Invalid linefeed option value: "<<strVal<<"\n";
                return -1;
            }

            appConfig.outputLinefeed = tmp;

            return 0;
        }
        */

        else if ( opt.setParam("VAR:VAL")
               || opt.isOption("set-var") || opt.isOption('C')
               || opt.setDescription("Set variable"))
        {
            if (argsParser.hasHelpOption) return 0;
            
            if (!opt.hasArg())
            {
                LOG_ERR_OPT<<"Setting variable requires argument (--set-var)\n";
                return -1;
            }

            auto optArg = opt.optArg;
            if (!appConfig.addVar(optArg))
            {
                LOG_ERR_OPT<<"Setting variable failed, invalid argument: '" << optArg << "'\n";
                return -1;
            }

            return 0;
        }

        // param = "md,md_,_md,markdown,_markdown,markdown_";


        //LOG_MSG_OPT << argsParser.programLocationInfo.exeFullName << "\n";

        // else if ( opt.isOption("all")
        //        || opt.setDescription("In scan mode, if no --exclude-files nor --include-files mask are taken, --all option required to confirm processing all files")
        //         )
        // {
        //     if (argsParser.hasHelpOption) return 0;
        //     
        //     appConfig.allFiles = true;
        //  
        //     return 0;
        // }
        //  
        else if ( opt.isOption("gcc")
               || opt.setDescription("GCC messages format instead of MSVC format")
                )
        {
            if (argsParser.hasHelpOption) return 0;
            
            umbaLogGccFormat = true;

            return 0;
        }

        else if ( opt.isOption("autocomplete-install") 
               || opt.setDescription("Install autocompletion to bash"
                                     #if defined(WIN32) || defined(_WIN32)
                                         "/clink(cmd)"
                                     #endif
                                    )
               )
        {
            if (argsParser.hasHelpOption) return 0;

            //return autocomplete(opt, true);
            return umba::command_line::autocompletionInstaller( pCol, opt, pCol->getPrintHelpStyle(), true, [&]( bool bErr ) -> decltype(auto) { return bErr ? LOG_ERR_OPT : LOG_MSG_OPT; } );
        }
        else if ( opt.isOption("autocomplete-uninstall") 
               || opt.setDescription("Remove autocompletion from bash"
                                     #if defined(WIN32) || defined(_WIN32)
                                         "/clink(cmd)"
                                     #endif
                                    )
                )
        {
            if (argsParser.hasHelpOption) return 0;

            //return autocomplete(opt, false);
            return umba::command_line::autocompletionInstaller( pCol, opt, pCol->getPrintHelpStyle(), false, [&]( bool bErr ) -> decltype(auto) { return bErr ? LOG_ERR_OPT : LOG_MSG_OPT; } );
        }

        else if (opt.isHelpStyleOption())
        {
            // Job is done in isHelpStyleOption
        }
        else if (opt.isHelpOption()) // if (opt.infoIgnore() || opt.isOption("help") || opt.isOption('h') || opt.isOption('?') || opt.setDescription(""))
        {
            if (!ignoreInfos)
            {
                if (pCol && !pCol->isNormalPrintHelpStyle())
                    argsParser.quet = true;
                //printNameVersion();
                if (!argsParser.quet)
                {
                    umba::cli_tool_helpers::printNameVersion(std::cout);
                    //printBuildDateTime();
                    umba::cli_tool_helpers::printCommitHash(std::cout);
                    std::cout<<"\n";
                //printHelp();
                }

                if (pCol && pCol->isNormalPrintHelpStyle() && argsParser.argsNeedHelp.empty())
                {
                    auto helpText = opt.getHelpOptionsString();
                    std::cout << "Usage: " << umba::toUtf8(argsParser.programLocationInfo.exeName)
                              << " [OPTIONS] input_file [output_file]\n"
                              << "  If output_file not taken, STDOUT used\n"
                              << "\nOptions:\n\n"
                              << helpText;
                              //<< " [OPTIONS] input_file [output_file]\n\nOptions:\n\n"<<helpText;

                }
                
                if (pCol) // argsNeedHelp
                    std::cout<<pCol->makeText( 78, &argsParser.argsNeedHelp );

                return 1;

            }

            return 0; // simple skip then parse builtins
        }
        else
        {
            LOG_ERR_OPT<<"unknown option: "<<opt.argOrg<<"\n";
            return -1;
        }

        return 0;

    } // if (opt.isOption())
    else if (opt.isResponseFile())
    {
        //std::string

        StringType optName;
        umba::utfToStringTypeHelper(optName, opt.name);
        auto optFileName = makeAbsPath(optName);

        if (!argsParser.quet)
        {
            std::cout << "Processing options file: " << optFileName << "\n";
        }

        optFiles.push(optFileName);

        auto parseRes = argsParser.parseOptionsFile( optFileName );

        optFiles.pop();

        if (!parseRes)
            return -1;

        if (argsParser.mustExit)
            return 1;

        return 0;
    
    }

    //if (inputFilename.empty())
    {
        inputFilename = makeAbsPath(a);
    }
    // else
    // {
    //     outputFilename = makeAbsPath(a);
    // }
    

    return 0;

}

}; // struct ArgParser



class CommandLineOptionCollector : public umba::command_line::CommandLineOptionCollectorImplBase
{
protected:
    virtual void onOptionDup( const std::string &opt ) override
    {
        LOG_ERR_OPT<<"Duplicated option key - '"<<opt<<"'\n";
        throw std::runtime_error("Duplicated option key");
    }

};



