############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2016 Xilinx, Inc. All Rights Reserved.
############################################################
open_project -reset multi-eps
set_top amoebasat
add_files amoebasat_multi_eps_hls.cpp
add_files amoebasat.h
add_files rules_update.cpp
open_solution "solution1"
#set_part {xc7z035fbv676-3} -tool vivado
set_part {xcu280-fsvh2892-2L-e} -tool vivado
create_clock -period 10 -name default
#csim_design -compiler gcc
csynth_design
#cosim_design
export_design -flow impl -rtl verilog -format ip_catalog
