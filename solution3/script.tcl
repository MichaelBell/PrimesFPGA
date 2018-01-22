############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2017 Xilinx, Inc. All Rights Reserved.
############################################################
open_project primetest
set_top fermat_test
add_files primetest/prime.cpp
add_files -tb primetest/test_prime.cpp
add_files -tb primetest/test_setup.cpp
open_solution "solution3"
set_part {xc7z010clg400-1}
create_clock -period 10 -name default
set_clock_uncertainty 15%
source "./primetest/solution3/directives.tcl"
csim_design
csynth_design
cosim_design
export_design -rtl verilog -format ip_catalog
