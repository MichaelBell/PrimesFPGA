############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2017 Xilinx, Inc. All Rights Reserved.
############################################################
set_directive_pipeline "one_test/one_test_label1"
set_directive_pipeline "one_test/fermat_test_label0"
set_directive_pipeline "one_test/fermat_test_label1"
set_directive_pipeline "one_test/mul1_label"
set_directive_dependence -variable T -type inter -dependent false "one_test/fermat_test_label0"
set_directive_dependence -variable P -type inter -dependent false "one_test/fermat_test_label1"
set_directive_pipeline "one_test/sum1_label"
set_directive_pipeline "one_test/sub1_label"
set_directive_pipeline "one_test/shift1_label"
set_directive_pipeline "one_test/sub2_label"
