############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2017 Xilinx, Inc. All Rights Reserved.
############################################################
set_directive_pipeline "one_test/one_test_label1"
set_directive_pipeline "one_test/fermat_test_label0"
set_directive_pipeline "one_test/fermat_test_label1"
set_directive_pipeline "one_test/mul1_label"
set_directive_pipeline "one_test/sum1_label"
set_directive_pipeline "one_test/sub1_label"
set_directive_pipeline "one_test/shift1_label"
set_directive_pipeline "one_test/sub2_label"
set_directive_pipeline "fermat_test/fermat_test_label4"
set_directive_loop_flatten -off "one_test/one_test_label6"
set_directive_loop_flatten -off "one_test/one_test_label5"
set_directive_dependence -variable T -type inter -dependent false "one_test/fermat_test_label0"
set_directive_dependence -variable P -type inter -dependent false "one_test/fermat_test_label1"
set_directive_dependence -variable T -type inter -dependent false "one_test/mul1_label"
set_directive_array_partition -type cyclic -factor 2 -dim 1 "fermat_test/fermat_test_outer_loop" R0
set_directive_array_partition -type cyclic -factor 2 -dim 1 "fermat_test/fermat_test_outer_loop" R1
set_directive_array_partition -type cyclic -factor 2 -dim 1 "fermat_test/fermat_test_outer_loop" R2
set_directive_array_partition -type cyclic -factor 2 -dim 1 "fermat_test/fermat_test_outer_loop" R3
set_directive_array_partition -type cyclic -factor 2 -dim 1 "fermat_test/fermat_test_outer_loop" R4
set_directive_array_partition -type cyclic -factor 2 -dim 1 "fermat_test/fermat_test_outer_loop" R5
set_directive_array_partition -type cyclic -factor 2 -dim 1 "fermat_test/fermat_test_outer_loop" R6
set_directive_array_partition -type cyclic -factor 2 -dim 1 "fermat_test/fermat_test_outer_loop" R7
set_directive_array_partition -type cyclic -factor 2 -dim 1 "fermat_test/fermat_test_outer_loop" R8
set_directive_array_partition -type cyclic -factor 2 -dim 1 "fermat_test/fermat_test_outer_loop" R9
set_directive_array_partition -type cyclic -factor 2 -dim 1 "fermat_test/fermat_test_outer_loop" R10
set_directive_array_partition -type cyclic -factor 2 -dim 1 "fermat_test/fermat_test_outer_loop" R11
set_directive_array_partition -type cyclic -factor 2 -dim 1 "fermat_test/fermat_test_outer_loop" R12
set_directive_array_partition -type cyclic -factor 2 -dim 1 "fermat_test/fermat_test_outer_loop" R13
set_directive_array_partition -type cyclic -factor 2 -dim 1 "fermat_test/fermat_test_outer_loop" R14
set_directive_array_partition -type cyclic -factor 2 -dim 1 "fermat_test/fermat_test_outer_loop" R15
set_directive_pipeline "fermat_test/copy_label0"
set_directive_pipeline "fermat_test/copy_label1"
set_directive_pipeline "fermat_test/copy_label2"
set_directive_pipeline "fermat_test/copy_label3"
set_directive_pipeline "fermat_test/copy_label4"
set_directive_pipeline "fermat_test/copy_label5"
set_directive_pipeline "fermat_test/copy_label6"
set_directive_pipeline "fermat_test/copy_label7"
set_directive_pipeline "fermat_test/copy_label8"
set_directive_pipeline "fermat_test/copy_label9"
set_directive_pipeline "fermat_test/copy_label10"
set_directive_pipeline "fermat_test/copy_label11"
set_directive_pipeline "fermat_test/copy_label12"
set_directive_pipeline "fermat_test/copy_label13"
set_directive_pipeline "fermat_test/copy_label14"
set_directive_pipeline "fermat_test/copy_label15"
set_directive_dependence -variable R -type inter -dependent false "one_test/sub1_label"
set_directive_dependence -variable R -type inter -dependent false "one_test/shift1_label"
set_directive_dependence -variable R -type inter -dependent false "one_test/sub2_label"
set_directive_pipeline "one_test/one_test_label4"
set_directive_dependence -variable P -type inter -dependent false "one_test/one_test_label4"
set_directive_pipeline -II 2 "one_test/check1_label"
set_directive_pipeline "one_test/check2_label"
