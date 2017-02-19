include ../../Makefile.inc

CFLAGS:=-DHLS_COMPILE -I../../utils $(CFLAGS)
SLDFLAGS:=-DHLS_COMPILE -L. -lSdsCraftUtils -L./libhf_minizip -lminizip -laes -lz

SDSFLAGS=-sds-pf zed -dmclkid 1 -sds-hw top Accel.cpp -clkid 1 -hls-tcl sds.tcl -sds-end
CXX=sds++ $(SDSFLAGS)

# HDR are pure headers
HDR=
# OBJ must include a .cpp and .h with same name
UTILS=Common.o Timer.o DataIO.o ParamIO.o ZipIO.o
LIBUTILS=libSdsCraftUtils.a
OBJ=Accel.o AccelSchedule.o AccelTest.o AccelPrint.o Dense.o InputConv.o
EXE=accel_test_bnn.exe

all: $(EXE)

# Rule for object files, each object must have a header
$(OBJ): %.o: ../%.cpp ../%.h
	$(CXX) -c $< -o $@ $(CFLAGS)

$(UTILS): %.o: ../../utils/%.cpp ../../utils/%.h
	$(CXX) -c $< -o $@ $(CFLAGS)

%.o: ../%.cpp
	$(CXX) -c $< -o $@ $(CFLAGS)

# Rule for utils library built by SDSoc
$(LIBUTILS): $(UTILS)
	$(AR) $@ $^

# Rule for executables
$(EXE): %.exe: %.o $(OBJ) $(LIBUTILS)
	$(CXX) $^ -o $@ $(CFLAGS) $(SLDFLAGS)

.PHONY: clean
clean:
	rm -f *.o *.exe *.bit
	rm -rf _sds sd_card
