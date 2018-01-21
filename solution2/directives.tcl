############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2017 Xilinx, Inc. All Rights Reserved.
############################################################
set_directive_dependence -variable R_in -type inter -dependent false "fermat_test/fermat_test_outer_loop"
set_directive_dependence -variable M_in -type inter -dependent false "fermat_test/fermat_test_outer_loop"
set_directive_dependence -variable Mi_in -type inter -dependent false "fermat_test/fermat_test_outer_loop"
set_directive_dependence -variable is_prime -type inter -dependent false "fermat_test/fermat_test_outer_loop"
set_directive_unroll "fermat_test/fermat_test_outer_loop"
set_directive_pipeline "one_test/one_test_label1"
