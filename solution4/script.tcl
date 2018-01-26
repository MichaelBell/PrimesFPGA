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
open_solution "solution4"
set_part {xcvu9p-flgb2104-2-i}
create_clock -period 8 -name default
source "./primetest/solution4/directives.tcl"
csim_design
csynth_design
cosim_design
export_design -format ip_catalog
