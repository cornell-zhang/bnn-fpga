include ../Makefile.inc

CFLAGS:=-I../utils $(CFLAGS)
LDFLAGS:=-L../utils -L../minizip -lCraftUtils $(LDFLAGS)

# HDR are pure headers
HDR=
# OBJ must include a .cpp and .h with same name
OBJ=Accel.o AccelSchedule.o AccelTest.o AccelPrint.o Dense.o InputConv.o
EXE=accel_test_bnn.exe accel_test_layer.exe accel_test_random.exe

all: $(EXE)

# Rule for object files, each object must have a header
$(OBJ): %.o: %.cpp %.h
	$(CXX) -c $< -o $@ $(CFLAGS)

%.o: %.cpp
	$(CXX) -c $< -o $@ $(CFLAGS)

# Rule for executables
$(EXE): %.exe: %.o $(OBJ)
	g++ $^ -o $@ $(CFLAGS) $(LDFLAGS)

.PHONY: hls clean hlsclean
hls:
	vivado_hls hls.tcl

hlsclean:
	rm -rf hls.prj vivado_hls.log

clean: hlsclean
	rm -f *.o *.exe
