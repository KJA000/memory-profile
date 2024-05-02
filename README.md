#bc 생성
clang -c -I /home/jiae/CMSIS-NN/Include/ -I /home/jiae/CMSIS-NN/Tests/UnitTest/Unity/src/ -emit-llvm arm_convolve_s8.c -o test.bc

opt -load ./MemoryProfiler.so -mempf arm_convolve_s8.bc -o s8.opt.bc

#실행파일 생성
clang -o test.exe main.c s8.opt.bc arm_nn_mat_mult_kernel_s8_s16.c arm_nn_mat_mult_kernel_row_offset_s8_s16.c arm_convolve_get_buffer_sizes_s8.c ../NNSupportFunctions/arm_q7_to_q15_with_offset.c runtime.cpp -lstdc++ -I ../../Include
