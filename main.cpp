/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Copyright 2014 dl12345@xda-developers forum

*/

#include <string>
#include <iostream>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/filesystem.hpp>
#include "qcn.hpp"

using qcn::Qcn;
namespace fs = boost::filesystem;
typedef enum {interleave, sequential, count} printformat;

bool const ProcessCommandLine(
    int ac, 
    char *av[], 
    Qcn::cmp& cmp,
    printformat& format, 
    std::string& fileone,
    std::string& filetwo,
    std::string& filedict
)
{
    namespace po = boost::program_options;
    typedef std::vector<std::string> files_type;

    char t, pf;
    files_type f;
    
    std::string prog(fs::path(av[0]).filename().string());
    std::string usage = "Usage: " + prog + " [options] file file";

    po::options_description visible;
    visible.add_options()
        ("help,h", "show help message\n")
        ("type,t", po::value<char>(&t)->default_value('p'),
                        "show differences\n"
                        "    p for items present in both files\n"
                        "    m for items missing in either file\n"
                        "    b for both present and missing items\n")   
        ("format,f", po::value<char>(&pf)->default_value('i'),
                        "output format\n"
                        "    i for interleaved output\n"
                        "    s for sequential output\n"
                        "    c to suppress item data and print only count\n")
        ("lookup,l", po::value<std::string>(&filedict)->default_value("nv.txt"),
                        "nv item descriptions")
    ;

    po::options_description hidden("hidden options");
    hidden.add_options()("input,i", po::value<files_type>(&f), "input file");
    
    po::positional_options_description p;
    p.add("input", -1);

    po::options_description all;
    all.add(visible).add(hidden);

    po::variables_map vm;
    po::store(
        po::command_line_parser(ac, av).options(all).positional(p).run(), vm
    );
    po::notify(vm);

    if (vm.count("help"))
    {
        std::cout << std::endl << usage << std::endl << std::endl;
        std::cout << visible;
        return false;
    }

    // process and set input files

    if (vm.count("input") && f.size() == 2)
    {
        fileone = f[0];
        filetwo = f[1];

        if (!fs::exists(fileone))
        {
            std::cout << prog << ": " << fileone << " not found" << std::endl;
            return false;
        }

        if (!fs::exists(filetwo))
        {
            std::cout << prog << ": " << filetwo << " not found" << std::endl;
            return false;
        }
    }
    else
    {
        std::cout << std::endl << usage << std::endl << std::endl;
        std::cout << visible;
        return false;
    }

    // process and set comparison type

    if (vm.count("type"))
    {
        switch(t)
        {
            case 'p':
                cmp = Qcn::cmp::present;
                break;
            case 'm':
                cmp = Qcn::cmp::missing;
                break;
            case 'b':
                cmp = Qcn::cmp::both;
                break;
            default:
                std::cout << std::endl << usage << std::endl << std::endl;
                std::cout << visible;
                return false;
        }
    }

    // process print output format

    if (vm.count("format"))
    {
        switch(pf)
        {
            case 'i':
                format = interleave;
                break;
            case 's':
                format = sequential;
                break;
            case 'c':
                format = count;
                break;
            default:
                std::cout << std::endl << usage << std::endl << std::endl;
                std::cout << visible;
                return false;
        }
    }

    return true;
}

void PrintOutput(
    qcn::diff_type const& d, 
    std::string const& fileone,
    std::string const& filetwo,
    std::string const& filedict,
    printformat p = interleave
)
{
    std::cout << std::endl;
    std::cout << "Found " << d.size() << " non matching items";
    std::cout << std::endl << std::endl;
    
    qcn::Dictionary dict(filedict);
    bool printinfo = false;      
    if (dict.Open()) printinfo = true;

    switch(p)
    {
        case interleave:

            for (auto i = d.begin(); i != d.end(); ++i)
            {
                auto p = *i;
                
                if (printinfo) 
                {
                    auto j = dict.Find(p.first.code);                    
                    
                    if (j != dict.end())
                    {
                        auto q = *j;
                        
                        p.first.description = q.description;
                        p.second.description = q.description;
                        
                        p.first.category = q.category;
                        p.second.category = q.category;
                    }
                }

                std::cout << '[' << fileone << "]: ";
                std::cout << p.first << std::endl;

                std::cout << '[' << filetwo << "]: ";
                std::cout << p.second << std::endl;
            }
            break;

        case sequential:

            std::cout << '[' << fileone << "]: " << std::endl << std::endl; 
            for (auto i = d.begin(); i != d.end(); ++i)
            {
                auto p = *i;
                std::cout << p.first << std::endl;
            }

            std::cout << '[' << filetwo << "]: " << std::endl << std::endl;
            for (auto i = d.begin(); i != d.end(); ++i)
            {
                auto p = *i;
                std::cout << p.second << std::endl;
            }
            break;
         
        default:
            break;
    }
}

int main(int argc, char *argv[])
{
    Qcn::cmp cmp;
    std::string nameone, nametwo, nameinfo;
    printformat pf;
    
    if (ProcessCommandLine(argc, argv, cmp, pf, nameone, nametwo, nameinfo))
    {
        qcn::Qcn fileone(nameone), filetwo(nametwo);        
        auto n1 = fs::path(nameone).filename().string();
        auto n2 = fs::path(nametwo).filename().string();

        if (fileone.Open() && filetwo.Open())
        {
            
            auto d = qcn::Compare(fileone, filetwo, cmp);
            PrintOutput(d, n1, n2, nameinfo, pf);    
        }
        else 
        {
            if (!fileone.IsOpen())
            {
                std::cout << n1 << ": ";
                std::cout << fileone.ErrorMessage() << std::endl;
            }
            if (!filetwo.IsOpen())
            {
                std::cout << n2 << ": ";
                std::cout << filetwo.ErrorMessage() << std::endl;
            }
            return 1;
        }
    }
    return 0;
}

