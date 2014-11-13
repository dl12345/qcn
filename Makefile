ifeq ($(OS),Windows_NT)
	RMOPTS=2> nul
	RM=@del
	INCLUDE=-ID:\boost_1_57_0
	LIBSUFFIX=-mgw49-mt-1_57
	LDOPTS=-static
	EXT=.exe
	ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)		
		ARCH=64
	endif
	ifeq ($(PROCESSOR_ARCHITECTURE),x86)
		ARCH=32
	endif
	ifeq ($(ARCH),64)
		LIBPATH=-LD:\boost_1_57_0\x64\lib
	endif
	ifeq ($(ARCH),32)
		LIBPATH=-LD:\boost_1_57_0\x86\lib
	endif
else
	RMOPTS=2> /dev/null || true
	RM=@rm -f 
	LIBSUFFIX=
	EXT=
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		UNAME_P := $(shell uname -p)
		ifeq ($(UNAME_P),x86_64)
			ARCH=64
		endif
		ifneq ($(filter %86,$(UNAME_P)),)
			ARCH=32
		endif
	endif
endif

STANDARD=-std=c++11
CPPFLAGS=-m$(ARCH) $(STANDARD) $(INCLUDE) -c 
LDFLAGS=-m$(ARCH) $(LIBPATH) $(LDOPTS)
LIBS=-lboost_program_options$(LIBSUFFIX) -lboost_filesystem$(LIBSUFFIX) -lboost_system$(LIBSUFFIX)
EXENAME=qcndiff
CLEANFILES=*.o $(EXENAME)$(ARCH)$(EXE)

$(EXENAME)$(ARCH): qcn$(ARCH).o main$(ARCH).o
	g++ $(LDFLAGS) -o $(EXENAME)$(ARCH) qcn$(ARCH).o main$(ARCH).o $(LIBS)
#	strip $(EXENAME)$(ARCH)$(EXT)

qcn$(ARCH).o: qcn.cpp qcn.hpp
	g++ $(CPPFLAGS) -o qcn$(ARCH).o qcn.cpp

main$(ARCH).o: qcn.cpp qcn.hpp main.cpp
	g++ $(CPPFLAGS) -o main$(ARCH).o main.cpp

clean:
	$(RM) $(CLEANFILES) $(RMOPTS) 

target:
	echo Processor architecture is $(PROCESSOR_ARCHITECTURE)
