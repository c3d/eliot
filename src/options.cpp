// ****************************************************************************
//   Christophe de Dinechin                                      ELFE PROJECT
//   options.cpp
// ****************************************************************************
//
//   File Description:
//
//     Processing of ELFE compiler options
//
//
//
//
//
//
//
//
// ****************************************************************************
// This document is distributed under the GNU General Public License
// See the enclosed COPYING file or http://www.gnu.org for information
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include "options.h"
#include "errors.h"
#include "renderer.h"
#include "recorder.h"


ELFE_BEGIN

/* ========================================================================= */
/*                                                                           */
/*   The compiler options parsing                                            */
/*                                                                           */
/* ========================================================================= */

RECORDER_DEFINE(options, 64, "Compiler options");

Options *Options::options = NULL;

Options::Options(int argc, char **argv):
/*---------------------------------------------------------------------------*/
/*  Set the default values for all options                                   */
/*---------------------------------------------------------------------------*/
#define OPTVAR(name, type, value)       name(value),
#define OPTION(name, descr, code)
#include "options.tbl"
    arg(1), args()
{
    // Store name of the program
    args.push_back(argv[0]);

    // Record singleton
    options = this;

    // Check if some options are given from environment
    if (kstring envopt = getenv("ELFE_OPT"))
    {
        record(options, "Environment variable ELFE_OPT='%s'", envopt);

        // Split space-separated input options and prepend them to args[]
        text envtext = envopt;
        std::istringstream input(envtext);
        std::copy(std::istream_iterator<text> (input),
                  std::istream_iterator<text> (),
                  std::back_inserter< std::vector<text> > (args));
    }
    else
    {
        record(options, "Environment variable ELFE_OPT is not set");
    }

    // Add options from the command-line
    record(options, "Command line has %d options", argc-1);
    for (int a = 1; a < argc; a++)
    {
        record(options, "  Option #%d is '%s'", a, argv[a]);
        args.push_back(argv[a]);
    }

    // Process arguments and extract options vs. files
    Process();
}


static void Usage(kstring appName)
// ----------------------------------------------------------------------------
//    Display usage information when an invalid name is given
// ----------------------------------------------------------------------------
{
    std::cerr << "Usage:\n";
    std::cerr << appName << " <options> <source_file>\n";

#define OPTVAR(name, type, value)
#define OPTION(name, descr, code)                                       \
    std::cerr << "\t-" << #name ": " descr "\n";
#include "options.tbl"
#if ELFE_DEBUG
    std::set<std::string> names = ELFE::Traces::names();
    if (names.size())
    {
        std::cerr << "\t-t<name>: Enable trace <name>. ";
        std::cerr << "Valid trace names are:\n";
        std::cerr << "\t          ";
        std::set<std::string>::iterator it;
        for (it = names.begin(); it != names.end(); it++)
            std::cerr << (*it) << " ";
        std::cerr << "\n";
    }
#endif
}


static bool OptionMatches(kstring &command_line, kstring optdescr)
// ----------------------------------------------------------------------------
//   Check if a given option matches the command line
// ----------------------------------------------------------------------------
// Single character options may accept argument as same or next parameter
{
    size_t len = strlen(optdescr);
    if (strncmp(command_line, optdescr, len) == 0)
    {
        command_line += len;
        return true;
    }
    return false;
}


static kstring OptionString(kstring &command_line, Options &opt)
// ----------------------------------------------------------------------------
//   Check if we find an integer between low and high on the command line
// ----------------------------------------------------------------------------
{
    if (*command_line)
    {
        kstring result = command_line;
        command_line = "";
        return result;
    }
    opt.arg += 1;
    if (opt.arg  < opt.args.size())
    {
        command_line = "";
        return opt.args[opt.arg].c_str();
    }
    Ooops("Option #$1 does not exist", Tree::COMMAND_LINE)
        .Arg(opt.arg);
    return "";
}


static ulong OptionInteger(kstring &command_line, Options &opt,
                           ulong low, ulong high)
// ----------------------------------------------------------------------------
//   Check if we find an integer between low and high on the command line
// ----------------------------------------------------------------------------
{
    uint result = low;
    kstring old = command_line;
    if (*command_line)
    {
        if (isdigit(*command_line))
            result = strtol(command_line, (char**) &command_line, 10);
        else
            Ooops("Option $1 is not an integer value", Tree::COMMAND_LINE)
                .Arg(command_line);
    }
    else
    {
        opt.arg += 1;
        if (opt.arg  < opt.args.size() && isdigit(opt.args[opt.arg][0]))
            result = strtol(old = opt.args[opt.arg].c_str(),
                            (char **) &command_line, 10);
        else
            Ooops("Option $1 is not an integer value", Tree::COMMAND_LINE)
                .Arg(old);
    }
    if (result < low || result > high)
    {
        char lowstr[15], highstr[15];
        sprintf(lowstr, "%lu", low);
        sprintf(highstr, "%lu", high);
        Ooops("Option $1 is out of range $2..$3", Tree::COMMAND_LINE)
            .Arg(result).Arg(low).Arg(high);
        if (result < low)
            result = low;
        else
            result = high;
    }
    return result;
}


static void PassOptionToLLVM(kstring &command_line)
// ----------------------------------------------------------------------------
//   An option beginning with -llvm is passed to llvm as is
// ----------------------------------------------------------------------------
{
    while (*command_line++) /* Skip */;
}


void Options::Process()
// ----------------------------------------------------------------------------
//   Looks for options and files on the command line
// ----------------------------------------------------------------------------
{
    while (arg < args.size())
    {
        if(args[arg].length() > 1 && args[arg][0] == '-')
        {
            kstring option = args[arg].c_str() + 1;
            kstring argval = option;

            record(options, "Parse option #%d, '%s'", arg, option);

#if ELFE_DEBUG
            if (argval[0] == 't')
            {
                kstring trace_name = argval + 1;
                Traces::enable(trace_name);
            }
            else
#endif

#define OPTVAR(name, type, value)
#define OPTION(name, descr, code)                                       \
            if (OptionMatches(argval, #name))                           \
            {                                                           \
                code;                                                   \
                                                                        \
                if (*argval)                                            \
                    Ooops("Garbage found after option -$1 : $2",        \
                          Tree::COMMAND_LINE).Arg(#name).Arg(option);   \
            }                                                           \
            else
#define INTEGER(n, m)           OptionInteger(argval, *this, n, m)
#define STRING                  OptionString(argval, *this)
#include "options.tbl"
            {
                // Default: Output usage
                Ooops("Unknown option $1 ignored", Tree::COMMAND_LINE)
                    .Arg(argval);
                Usage(args[0].c_str());
            }
            arg++;
        }
        else
        {
            text fileName = args[arg++];
            files.push_back(fileName);
        }
    }

    // Add builtins file at the beginning if 'nobuiltins' option was not set
    if (!builtins.empty())
        files.insert(files.begin(), builtins);
}

ELFE_END
ulong elfe_traces = 0;
// ----------------------------------------------------------------------------
//   Bits for each trace
// ----------------------------------------------------------------------------
