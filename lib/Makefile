CC  = clang-11
CXX = clang-11 --driver-mode=g++

INCLUDES =

CFLAGS   = -g -Wall $(INCLUDES)
CXXFLAGS = -g -Wall $(INCLUDES) -std=c++20 -stdlib=libc++ -O2 -fmodules -fprebuilt-module-path=. -fmodule-file=$(MODULE)
CPPMFLAGS = -g -Wall $(INCLUDES) -std=c++20 -stdlib=libc++ -O2 -fmodules

LDFLAGS = -g
LDLIBS  =

LIBRARY = libtba.a
OBJECTS = game_talker.o
MOBJECTS = tba.o
MODULE = tba.pcm

default: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	ar rcs $@ $^

$(OBJECTS): $(MOBJECTS)

$(MOBJECTS): $(MODULE)
	$(CXX) -fmodules -c $^ -o $@

$(MODULE): tba.cppm
	$(CXX) $(CPPMFLAGS) --precompile $^ -o $@

.PHONY: clean
clean:
	rm -f *~ a.out core $(OBJECTS) $(MOBJECTS) $(LIBRARY) $(MODULE)

.PHONY: all
all: clean default
