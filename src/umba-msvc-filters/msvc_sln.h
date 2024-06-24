#pragma once

#include "umba/umba.h"
//
#include "umba/string_plus.h"
//
#include "marty_cpp/marty_cpp.h"

//
#include <exception>
#include <stdexcept>



// umba::msvc::
namespace umba {
namespace msvc {


struct ProjectDependency
{
    std::string    dependecyName; // По факту - тот же GUID, что и dependecyGuid
    std::string    dependecyGuid;

    // {A11252C4-F50A-4DA7-A059-FBD74024996A} = {A11252C4-F50A-4DA7-A059-FBD74024996A}
    bool parseDependencyLine(std::string line)
    {
        auto pos = line.find(" = ");
        if (pos==line.npos)
            return false;

        dependecyName.assign(line, 0, pos);
        dependecyGuid.assign(line, pos+3, line.npos);
        umba::string_plus::trim(dependecyName, umba::string_plus::is_one_of<char>("{}"));
        umba::string_plus::trim(dependecyGuid, umba::string_plus::is_one_of<char>("{}"));
        return true;
    }

}; // struct ProjectDependency



struct SolutionProjectInfo
{
    std::string                           entryGuid   ; // Похоже, это GUID самого солюшена, он в проекте для всех элементов одинаковый
    std::string                           entryName   ;
    std::string                           projectFile ;
    std::string                           projectGuid ;
    std::vector<ProjectDependency>        dependencies;

    // Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "_all", ".msvc2019\_all.vcxproj", "{D66975DB-7E59-4BE8-AD8D-C13E9B858F44}"
    // _all.vcxproj - <ProjectGuid>{D66975DB-7E59-4BE8-AD8D-C13E9B858F44}</ProjectGuid>
    bool parseDefinitionLine(std::string line)
    {
        if (!umba::string_plus::starts_with_and_strip(line, "Project(\""))
            return false;

        auto pos = line.find("\") = ");
        if (pos==line.npos)
            return false;

        entryGuid = std::string(line, 0, pos);
        line.erase(0, pos+5);
        umba::string_plus::trim(entryGuid, umba::string_plus::is_one_of<char>(" \"{}"));

        std::vector<std::string> vals;
        std::string::const_iterator it = line.begin();
        std::string::const_iterator startPosIt = it; // line.end();

        bool inQuot = false;
        for(; it!=line.end(); ++it)
        {
            if (*it=='\"')
            {
                inQuot = !inQuot;
                continue;
            }

            if (*it==',')
            {
                if (inQuot)
                    continue;
                vals.emplace_back(startPosIt, it);
                startPosIt = it;
                ++startPosIt;
                continue;
            }
        }

        if (startPosIt!=line.end())
        {
            vals.emplace_back(startPosIt, it);
        }

        if (vals.size()<3)
            return false;

        for(auto &v : vals)
        {
            umba::string_plus::trim(v, umba::string_plus::is_one_of<char>(" \"{}"));
        }

        entryName  .assign(vals[0]);
        projectFile.assign(vals[1]);
        projectGuid.assign(vals[2]);
        
        // if (umba::string_plus::starts_with(*lit, "Project"))
        // {
        //     lit = parseProjectEntry(lit, textLines.end());
        //     continue;
        // }    
        return true;
    }

    // Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "_all", ".msvc2019\_all.vcxproj", "{D66975DB-7E59-4BE8-AD8D-C13E9B858F44}"
    // 	ProjectSection(ProjectDependencies) = postProject
    // 		{A11252C4-F50A-4DA7-A059-FBD74024996A} = {A11252C4-F50A-4DA7-A059-FBD74024996A}
    // 	EndProjectSection
    // EndProject
    // Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "umba-msvc-filters", ".msvc2019\umba-msvc-filters.vcxproj", "{A11252C4-F50A-4DA7-A059-FBD74024996A}"
    // EndProject

    std::vector<std::string>::iterator parseProjectEntryLines(std::vector<std::string>::iterator b, std::vector<std::string>::iterator e)
    {
        if (!parseDefinitionLine(*b))
            throw std::runtime_error("Invalid sln format");

        ++b;
        if (b==e)
            throw std::runtime_error("Invalid sln format");

        for(; b!=e; ++b)
        {
            umba::string_plus::trim(*b);

            if (umba::string_plus::starts_with_and_strip(*b, "ProjectSection(ProjectDependencies) ="))
            {
                // read dependencies here
                ++b;

                for(; b!=e; ++b)
                {
                    umba::string_plus::trim(*b);
                    if (umba::string_plus::starts_with_and_strip(*b, "EndProjectSection"))
                        break;
                    ProjectDependency dep;
                    if (!dep.parseDependencyLine(*b))
                        throw std::runtime_error("Invalid sln format");
                    dependencies.emplace_back(dep);
                }
                continue;
            }
            else if (umba::string_plus::starts_with_and_strip(*b, "EndProject"))
            {
                return b; 
            }
        
            break;
        }

        throw std::runtime_error("Invalid sln format");
    }

}; // struct SolutionProjectInfo


// umba::string_plus::
// StringType toupper_copy( const StringType &str )
// inline bool starts_with( const StringType &str, const StringType &prefix )
// bool starts_with_and_strip( StringType &str, const StringType &prefix )


struct Solution
{
    std::string                          solutionVersionString ; // MSVC2019 - 12.00 - Microsoft Visual Studio Solution File, Format Version 12.00
    std::string                          vsVersionString       ; // MSVC2019 - VisualStudioVersion = 16.0.31205.134
    std::string                          minimimVsVersionString; // MSVC2019 - 10.0.40219.1 - MinimumVisualStudioVersion = 10.0.40219.1

    std::vector<SolutionProjectInfo>     projects;

    bool isTrimmedLineComment(const std::string &line)
    {
        if (line.empty())
            return false;
        if (line[0]=='#')
            return true;
        return false;
    }

    bool isTrimmedLineCommentOrEmpty(const std::string &line)
    {
        if (line.empty())
            return true;
        return isTrimmedLineComment(line);
    }


    bool parse(const std::string &text)
    {
        return parse(marty_cpp::simple_string_split(text, std::string("\n")));
    }

    bool parse(std::vector<std::string> textLines)
    {
        std::vector<std::string>::iterator lit = textLines.begin();

        // First step - look for SLN file format
        for(; lit!=textLines.end(); ++lit)
        {
            umba::string_plus::trim(*lit);

            if (isTrimmedLineCommentOrEmpty(*lit))
                continue;

            //if (!umba::string_plus::starts_with_and_strip(*lit, std::string("Microsoft Visual Studio Solution File, Format Version")))
            if (!umba::string_plus::starts_with_and_strip(*lit, "Microsoft Visual Studio Solution File, Format Version"))
                return false; // формат солюшена должен идти первой значимой строкой, если нет - что-то пошло не так

            umba::string_plus::trim(*lit);

            solutionVersionString = *lit;

            break;
        }

        if (lit==textLines.end())
            return false;

        ++lit;

        // Second step - look for VisualStudioVersion
        for(; lit!=textLines.end(); ++lit)
        {
            umba::string_plus::trim(*lit);

            if (isTrimmedLineCommentOrEmpty(*lit))
                continue;

            if (!umba::string_plus::starts_with_and_strip(*lit, "VisualStudioVersion ="))
                return false;

            umba::string_plus::trim(*lit);

            vsVersionString = *lit;

            break;
        }

        if (lit==textLines.end())
            return false;

        ++lit;

        // Second step - look for VisualStudioVersion
        for(; lit!=textLines.end(); ++lit)
        {
            umba::string_plus::trim(*lit);

            if (isTrimmedLineCommentOrEmpty(*lit))
                continue;

            if (!umba::string_plus::starts_with_and_strip(*lit, "MinimumVisualStudioVersion ="))
                return false;

            umba::string_plus::trim(*lit);

            minimimVsVersionString = *lit;

            break;
        }

        if (lit==textLines.end())
            return false;

        ++lit;

        try
        {
            for(; lit!=textLines.end(); ++lit)
            {
                umba::string_plus::trim(*lit);

                if (isTrimmedLineCommentOrEmpty(*lit))
                    continue;
    
                if (umba::string_plus::starts_with(*lit, "Project"))
                {
                    lit = parseProjectEntry(lit, textLines.end());
                    continue;
                }    
                if (umba::string_plus::starts_with(*lit, "Global"))
                {
                    lit = parseGlobalEntry(lit, textLines.end());
                    continue;
                }    

                return false;
            }

        }
        catch(...)
        {
            return false;
        }

        return true;
    }

    std::vector<std::string>::iterator parseProjectEntry(std::vector<std::string>::iterator b, std::vector<std::string>::iterator e)
    {
        SolutionProjectInfo prjInfo;
        auto resIt = prjInfo.parseProjectEntryLines(b, e);
        projects.emplace_back(prjInfo);
        return resIt;
    }

    std::vector<std::string>::iterator parseGlobalEntry(std::vector<std::string>::iterator b, std::vector<std::string>::iterator e)
    {
        for(; b!=e; ++b)
        {
            umba::string_plus::trim(*b);

            if (umba::string_plus::starts_with_and_strip(*b, "EndGlobalSection"))
            {
            }
            else if (umba::string_plus::starts_with_and_strip(*b, "EndGlobal"))
            {
                return b; 
            }
        
            //break;
        }

        throw std::runtime_error("Invalid sln format");
    }

    // std::vector<std::string>::iterator parseGlobalSectionEntry(std::vector<std::string>::iterator b, std::vector<std::string>::iterator e)
    // {
    //     return b;
    // }

}; // struct Solution




/*


Microsoft Visual Studio Solution File, Format Version 12.00
# Visual Studio Version 16
VisualStudioVersion = 16.0.31205.134
MinimumVisualStudioVersion = 10.0.40219.1
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "_all", ".msvc2019\_all.vcxproj", "{D66975DB-7E59-4BE8-AD8D-C13E9B858F44}"
	ProjectSection(ProjectDependencies) = postProject
		{A11252C4-F50A-4DA7-A059-FBD74024996A} = {A11252C4-F50A-4DA7-A059-FBD74024996A}
	EndProjectSection
EndProject
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "umba-msvc-filters", ".msvc2019\umba-msvc-filters.vcxproj", "{A11252C4-F50A-4DA7-A059-FBD74024996A}"
EndProject
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|x64 = Debug|x64
		Debug|x86 = Debug|x86
		MinSizeRel|x64 = MinSizeRel|x64
		MinSizeRel|x86 = MinSizeRel|x86
		Release|x64 = Release|x64
		Release|x86 = Release|x86
		RelWithDebInfo|x64 = RelWithDebInfo|x64
		RelWithDebInfo|x86 = RelWithDebInfo|x86
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution
		{D66975DB-7E59-4BE8-AD8D-C13E9B858F44}.Debug|x64.ActiveCfg = Debug|x64
		{D66975DB-7E59-4BE8-AD8D-C13E9B858F44}.Debug|x64.Build.0 = Debug|x64
		{D66975DB-7E59-4BE8-AD8D-C13E9B858F44}.Debug|x86.ActiveCfg = Debug|Win32
		{D66975DB-7E59-4BE8-AD8D-C13E9B858F44}.Debug|x86.Build.0 = Debug|Win32
		{D66975DB-7E59-4BE8-AD8D-C13E9B858F44}.MinSizeRel|x64.ActiveCfg = MinSizeRel|x64
		{D66975DB-7E59-4BE8-AD8D-C13E9B858F44}.MinSizeRel|x64.Build.0 = MinSizeRel|x64
		{D66975DB-7E59-4BE8-AD8D-C13E9B858F44}.MinSizeRel|x86.ActiveCfg = MinSizeRel|x64
		{D66975DB-7E59-4BE8-AD8D-C13E9B858F44}.Release|x64.ActiveCfg = Release|x64
		{D66975DB-7E59-4BE8-AD8D-C13E9B858F44}.Release|x64.Build.0 = Release|x64
		{D66975DB-7E59-4BE8-AD8D-C13E9B858F44}.Release|x86.ActiveCfg = Release|Win32
		{D66975DB-7E59-4BE8-AD8D-C13E9B858F44}.Release|x86.Build.0 = Release|Win32
		{D66975DB-7E59-4BE8-AD8D-C13E9B858F44}.RelWithDebInfo|x64.ActiveCfg = RelWithDebInfo|Win32
		{D66975DB-7E59-4BE8-AD8D-C13E9B858F44}.RelWithDebInfo|x86.ActiveCfg = RelWithDebInfo|Win32
		{D66975DB-7E59-4BE8-AD8D-C13E9B858F44}.RelWithDebInfo|x86.Build.0 = RelWithDebInfo|Win32
		{A11252C4-F50A-4DA7-A059-FBD74024996A}.Debug|x64.ActiveCfg = Debug|x64
		{A11252C4-F50A-4DA7-A059-FBD74024996A}.Debug|x64.Build.0 = Debug|x64
		{A11252C4-F50A-4DA7-A059-FBD74024996A}.Debug|x86.ActiveCfg = Debug|Win32
		{A11252C4-F50A-4DA7-A059-FBD74024996A}.Debug|x86.Build.0 = Debug|Win32
		{A11252C4-F50A-4DA7-A059-FBD74024996A}.MinSizeRel|x64.ActiveCfg = MinSizeRel|x64
		{A11252C4-F50A-4DA7-A059-FBD74024996A}.MinSizeRel|x64.Build.0 = MinSizeRel|x64
		{A11252C4-F50A-4DA7-A059-FBD74024996A}.MinSizeRel|x86.ActiveCfg = MinSizeRel|x64
		{A11252C4-F50A-4DA7-A059-FBD74024996A}.Release|x64.ActiveCfg = Release|x64
		{A11252C4-F50A-4DA7-A059-FBD74024996A}.Release|x64.Build.0 = Release|x64
		{A11252C4-F50A-4DA7-A059-FBD74024996A}.Release|x86.ActiveCfg = Release|Win32
		{A11252C4-F50A-4DA7-A059-FBD74024996A}.Release|x86.Build.0 = Release|Win32
		{A11252C4-F50A-4DA7-A059-FBD74024996A}.RelWithDebInfo|x64.ActiveCfg = RelWithDebInfo|Win32
		{A11252C4-F50A-4DA7-A059-FBD74024996A}.RelWithDebInfo|x86.ActiveCfg = RelWithDebInfo|Win32
		{A11252C4-F50A-4DA7-A059-FBD74024996A}.RelWithDebInfo|x86.Build.0 = RelWithDebInfo|Win32
	EndGlobalSection
	GlobalSection(SolutionProperties) = preSolution
		HideSolutionNode = FALSE
	EndGlobalSection
	GlobalSection(ExtensibilityGlobals) = postSolution
		SolutionGuid = {85E12D6E-79C6-4E78-A1DC-62B12FF9F244}
	EndGlobalSection
EndGlobal



*/




} // namespace msvc
} // namespace umba
