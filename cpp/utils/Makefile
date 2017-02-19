include ../Makefile.inc

# HDR are pure headers
HDR=Debug.h BitVector.h QuantizeParams.h Layers.h Typedefs.h
# OBJ must include a .cpp and .h with same name
OBJ=DataIO.o ParamIO.o ZipIO.o Timer.o Common.o
EXE=open_zip.exe
ART=libCraftUtils.a

all: $(ART)

# Rule for object files, each object must have a header
$(OBJ): %.o: %.cpp %.h
	$(CXX) -c $< -o $@ $(CFLAGS)

%.o: %.cpp
	$(CXX) -c $< -o $@ $(CFLAGS)

# Rule for executables
$(EXE): %.exe: %.o $(OBJ)
	g++ $^ -o $@ $(CFLAGS) $(LDFLAGS)

# Rule for libs
$(ART): $(OBJ)
	$(AR) $@ $^

.PHONY: hlsclean
clean:
	rm -f *.o *.exe *.a
