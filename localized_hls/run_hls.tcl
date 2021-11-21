open_project test
set_top amoebasat
add_files ./amoebasat.cpp
add_files ./local_rules_update.cpp
#add_files -tb test3/amoebasat_test.cpp -cflags "-Wno-unknown-pragmas"
open_solution "solution1"
set_part {xc7z030-ffv676-3}
create_clock -period 5 -name default
#config_export -format ip_catalog -rtl verilog -vivado_optimization_level 2 -vivado_phys_opt place -vivado_report_level 0
set_clock_uncertainty 12.5%
#source "./test3/solution1/directives.tcl"
#csim_design -compiler gcc
csynth_design
#cosim_design -trace_level all
export_design -flow impl -rtl verilog -format ip_catalog