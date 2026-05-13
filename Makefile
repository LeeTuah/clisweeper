CXX = g++
CXX_FLAGS = -Wall -std=c++17

ifeq ($(OS),Windows_NT)
    EXT = .exe
    LDFLAGS = -lws2_32
    CLEAN_CMD = del /Q /F
    CLEAN_TARGETS = src\server\server$(EXT) src\client\client$(EXT)
else
    EXT =
    LDFLAGS = -pthread
    CLEAN_CMD = rm -f
    CLEAN_TARGETS = src/server/server$(EXT) src/client/client$(EXT)
endif

all: src/server/server$(EXT) src/client/client$(EXT)

src/server/server$(EXT): src/server/main.cpp
	$(CXX) $(CXX_FLAGS) src/server/main.cpp -o src/server/server$(EXT) $(LDFLAGS)

src/client/client$(EXT): src/client/main.cpp
	$(CXX) $(CXX_FLAGS) src/client/main.cpp -o src/client/client$(EXT) $(LDFLAGS)

clean:
	$(CLEAN_CMD) $(CLEAN_TARGETS)
