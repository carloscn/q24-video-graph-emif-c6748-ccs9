################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
src/%.obj: ../src/%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C6000 Compiler'
	"C:/opt/ti/ccs9/ccs/tools/compiler/ti-cgt-c6000_8.3.2/bin/cl6x" -mv6740 --include_path="D:/workspace/CCS_V8/Q24_VIDEO_SAMPLE_C6748_CCSV8" --include_path="../include" --include_path="D:/workspace/CCS_V7/include" --include_path="C:/opt/ti/ccs9/ccs/tools/compiler/ti-cgt-c6000_8.3.2/include" --define=c6748 -g --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

src/%.obj: ../src/%.asm $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C6000 Compiler'
	"C:/opt/ti/ccs9/ccs/tools/compiler/ti-cgt-c6000_8.3.2/bin/cl6x" -mv6740 --include_path="D:/workspace/CCS_V8/Q24_VIDEO_SAMPLE_C6748_CCSV8" --include_path="../include" --include_path="D:/workspace/CCS_V7/include" --include_path="C:/opt/ti/ccs9/ccs/tools/compiler/ti-cgt-c6000_8.3.2/include" --define=c6748 -g --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


