#=========================================================================
# hls.tcl
#=========================================================================

set top "top"
set cflags "-DHLS_COMPILE -O3 -std=c++0x -I../utils"
set tbflags "-DHLS_COMPILE -O3 -std=c++0x -I../utils -lminizip -laes -lz"
set utils "../utils/Common.cpp ../utils/DataIO.cpp ../utils/ParamIO.cpp ../utils/ZipIO.cpp"

open_project hls.prj

set_top $top

add_files Accel.cpp -cflags $cflags
add_files -tb accel_test_random.cpp -cflags $tbflags
add_files -tb AccelSchedule.cpp -cflags $cflags
add_files -tb AccelTest.cpp -cflags $cflags
add_files -tb AccelPrint.cpp -cflags $cflags
add_files -tb InputConv.cpp -cflags $tbflags
add_files -tb Dense.cpp -cflags $tbflags
add_files -tb $utils -cflags $tbflags

open_solution "solution1" -reset

set_part {xc7z020clg484-1}
create_clock -period 5

config_rtl -reset state

# Apply optimizations
source opt.tcl

csim_design

csynth_design
cosim_design -rtl verilog -trace_level all

#export_design -evaluate verilog

exit
