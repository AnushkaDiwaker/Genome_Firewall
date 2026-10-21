CXX      = g++
# C++14 is used for compatibility with MinGW.org GCC 6.3 and modern GCC/Clang.
# -O3 enables aggressive loop unrolling and SIMD vectorization.
CXXFLAGS = -std=c++14 -O3 -pthread \
           -Wall -Wextra \
           -Wno-unused-result -Wno-unused-parameter \
           -Wno-missing-field-initializers \
           -Wno-shift-count-overflow \
           -Iinclude

# Statically link libgcc and libstdc++ so the executable runs standalone on any Windows machine
LDFLAGS  = -pthread -static -static-libgcc -static-libstdc++

SRCDIR   = src
INCDIR   = include
OBJDIR   = obj
BINDIR   = .

SOURCES  = $(SRCDIR)/AMRDatabase.cpp \
           $(SRCDIR)/FastaMmapParser.cpp \
           $(SRCDIR)/AMRBloomFilter.cpp \
           $(SRCDIR)/TrieArena.cpp \
           $(SRCDIR)/AMRTrie.cpp \
           $(SRCDIR)/LogisticRegressor.cpp \
           $(SRCDIR)/PredictorEngine.cpp \
           $(SRCDIR)/ScoringEngine.cpp \
           $(SRCDIR)/REPLShell.cpp \
           $(SRCDIR)/main.cpp

OBJECTS  = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
TARGET   = genome_firewall

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	-mkdir $(OBJDIR) 2>NUL || mkdir -p $(OBJDIR)

clean:
	-del /Q $(OBJDIR)\*.o 2>NUL || rm -f $(OBJDIR)/*.o
	-del /Q $(TARGET).exe $(TARGET) 2>NUL || rm -f $(TARGET).exe $(TARGET)

test: $(TARGET)
	@echo [*] Running Verification Test on CARD Top Markers and Sample Genome...
	./$(TARGET) --input data/sample_real.fasta --db data/markers_card_top.csv

.PHONY: all clean test
