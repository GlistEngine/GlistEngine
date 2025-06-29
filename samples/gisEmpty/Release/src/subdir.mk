################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/GameCanvas.cpp \
../src/gApp.cpp \
../src/main.cpp 

CPP_DEPS += \
./src/GameCanvas.d \
./src/gApp.d \
./src/main.d 

OBJS += \
./src/GameCanvas.o \
./src/gApp.o \
./src/main.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I"/home/sadettin/dev/glist/GlistEngine/samples/gisEmpty\..\..\..\zbin\win64\mingw64\include" -O2 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/GameCanvas.d ./src/GameCanvas.o ./src/gApp.d ./src/gApp.o ./src/main.d ./src/main.o

.PHONY: clean-src

