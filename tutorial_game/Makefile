CC  = clang-11
CXX = clang-11 --driver-mode=g++ -stdlib=libc++

INCLUDES = -I ../lib -I ../rapidxml/

CFLAGS   = -g -Wall $(INCLUDES)
CXXFLAGS = -g -Wall $(INCLUDES) -std=c++20 -O2 -fmodules -fprebuilt-module-path=.. -fmodule-file=$(MODULE)

LDFLAGS = -g -L ../lib -L ../rapidxml/
LDLIBS  = -ltba

EXECUTABLE = main
OBJECTS = main.o
MODULE = ../lib/tba.pcm

default: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

.PHONY: clean
clean:
	rm -f *~ a.out core $(OBJECTS) $(EXECUTABLE)

.PHONY: all
all: clean default