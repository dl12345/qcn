<h1>QCNDIFF</h1>

This is a utility that parses and displays differences between two text format .qcn files produced by Qualcomm's QPST (Qualcomm Product Support Tools). Examples for Linux usage and compilation are given for a RHEL / Centos / Fedora distribution. Adapt as necessary to your own distribution.

<h2>HOW TO USE</h2>

There are pre-built binaries for 64 and 32 bit Windows as well as pre-built binaries for 32 and 64 bit Linux. The Linux binaries were built on Fedora 20 and the Windows binaries were built with MinGW and statically linked. For Linux you will need to install the boost shared libraries
````
yum install boost
````

Program usage is as follows

````
Usage: qcndiff64 [options] file file

  -h [ --help ]            show help message
                           
  -t [ --type ] arg (=p)   show differences
                               p for items present in both files
                               m for items missing in either file
                               b for both present and missing items
                           
  -f [ --format ] arg (=i) output format
                               i for interleaved output
                               s for sequential output
                               c to suppress item data and print only count
````

Interleaved output shows the nvitem that is different for both files before displaying the next one. Sequential output displays all the differing items in the first file before proceeding to display the second file

<h2>HOW TO COMPILE</h2>

<h3>Linux</h3>

````
yum install boost boost-devel
make
````

If you wish to compile for a different architecture, for example you run x86_64 and you wish to compile a 32 bit variant, then first make sure you have the 32 bit libraries and then override the target with the ARCH commandline option

````
yum install boost.i686 boost-devel.i686
make ARCH=32
````

<h3>Windows</h3>

You will need to install MinGW. You need separate toolchains for the 64 bit versions and the 32 bit versions. You will also need to compile the boost libraries.

Once Boost is compiled, edit the Makefile to change the paths to the libraries and include files to conform to where you have them installed. Take note that if the default --layout==versioning option is used to compile Boost then you may also need to change the suffix option in the Makefile

<h3>DOWNLOAD LINKS</h3>

MinGW-32 binary distribution: http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/4.9.2/threads-posix/dwarf/i686-4.9.2-release-posix-dwarf-rt_v3-rev0.7z/download

MinGW-64 binary distribution: http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/4.9.2/threads-posix/seh/x86_64-4.9.2-release-posix-seh-rt_v3-rev0.7z/download

Boost 1.57 source: http://sourceforge.net/projects/boost/files/boost/1.57.0/

gmake for Windows</url>: http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/4.9.2/threads-posix/dwarf/i686-4.9.2-release-posix-dwarf-rt_v3-rev0.7z/download
