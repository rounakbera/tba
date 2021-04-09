CC  = clang-11
CXX = clang-11 --driver-mode=g++ -stdlib=libc++

INCLUDES = -I ../lib

CFLAGS   = -g -Wall $(INCLUDES)
CXXFLAGS = -g -Wall $(INCLUDES) -std=c++20 -O2 -fmodules -fprebuilt-module-path=.. -fmodule-file=$(MODULE)

LDFLAGS = -g -L ../lib
LDLIBS  = -ltba

EXECUTABLE = test_save_load
OBJECTS = test_save_load.o
MODULE = ../lib/tba.pcm

default: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

.PHONY: clean
clean:
	rm -f *~ a.out core $(OBJECTS) $(EXECUTABLE)

.PHONY: all
all: clean default