#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/PIC32AK_SPI_CAN_BOOTLOADER.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/PIC32AK_SPI_CAN_BOOTLOADER.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../src/config/default/peripheral/clk/plib_clk.c ../src/config/default/peripheral/gpio/plib_gpio.c ../src/config/default/peripheral/intc/plib_intc.c ../src/config/default/peripheral/nvm/plib_nvm.c ../src/config/default/peripheral/spi/spi_host/plib_spi1_host.c ../src/config/default/stdio/xc32_monitor.c ../src/config/default/initialization.c ../src/config/default/interrupts.c ../src/config/default/traps.c ../src/driver/driver_CANbus.c ../src/driver/driver_mcp2515.c ../src/driver/flash_eeprom.c ../src/middleware/boot_handler.c ../src/middleware/device_startup.c ../src/software/state_engine.c ../src/main.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/60165520/plib_clk.o ${OBJECTDIR}/_ext/1865254177/plib_gpio.o ${OBJECTDIR}/_ext/1865312166/plib_intc.o ${OBJECTDIR}/_ext/60176403/plib_nvm.o ${OBJECTDIR}/_ext/1868329136/plib_spi1_host.o ${OBJECTDIR}/_ext/163028504/xc32_monitor.o ${OBJECTDIR}/_ext/1171490990/initialization.o ${OBJECTDIR}/_ext/1171490990/interrupts.o ${OBJECTDIR}/_ext/1171490990/traps.o ${OBJECTDIR}/_ext/362756510/driver_CANbus.o ${OBJECTDIR}/_ext/362756510/driver_mcp2515.o ${OBJECTDIR}/_ext/362756510/flash_eeprom.o ${OBJECTDIR}/_ext/1375476428/boot_handler.o ${OBJECTDIR}/_ext/1375476428/device_startup.o ${OBJECTDIR}/_ext/1200941409/state_engine.o ${OBJECTDIR}/_ext/1360937237/main.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/60165520/plib_clk.o.d ${OBJECTDIR}/_ext/1865254177/plib_gpio.o.d ${OBJECTDIR}/_ext/1865312166/plib_intc.o.d ${OBJECTDIR}/_ext/60176403/plib_nvm.o.d ${OBJECTDIR}/_ext/1868329136/plib_spi1_host.o.d ${OBJECTDIR}/_ext/163028504/xc32_monitor.o.d ${OBJECTDIR}/_ext/1171490990/initialization.o.d ${OBJECTDIR}/_ext/1171490990/interrupts.o.d ${OBJECTDIR}/_ext/1171490990/traps.o.d ${OBJECTDIR}/_ext/362756510/driver_CANbus.o.d ${OBJECTDIR}/_ext/362756510/driver_mcp2515.o.d ${OBJECTDIR}/_ext/362756510/flash_eeprom.o.d ${OBJECTDIR}/_ext/1375476428/boot_handler.o.d ${OBJECTDIR}/_ext/1375476428/device_startup.o.d ${OBJECTDIR}/_ext/1200941409/state_engine.o.d ${OBJECTDIR}/_ext/1360937237/main.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/60165520/plib_clk.o ${OBJECTDIR}/_ext/1865254177/plib_gpio.o ${OBJECTDIR}/_ext/1865312166/plib_intc.o ${OBJECTDIR}/_ext/60176403/plib_nvm.o ${OBJECTDIR}/_ext/1868329136/plib_spi1_host.o ${OBJECTDIR}/_ext/163028504/xc32_monitor.o ${OBJECTDIR}/_ext/1171490990/initialization.o ${OBJECTDIR}/_ext/1171490990/interrupts.o ${OBJECTDIR}/_ext/1171490990/traps.o ${OBJECTDIR}/_ext/362756510/driver_CANbus.o ${OBJECTDIR}/_ext/362756510/driver_mcp2515.o ${OBJECTDIR}/_ext/362756510/flash_eeprom.o ${OBJECTDIR}/_ext/1375476428/boot_handler.o ${OBJECTDIR}/_ext/1375476428/device_startup.o ${OBJECTDIR}/_ext/1200941409/state_engine.o ${OBJECTDIR}/_ext/1360937237/main.o

# Source Files
SOURCEFILES=../src/config/default/peripheral/clk/plib_clk.c ../src/config/default/peripheral/gpio/plib_gpio.c ../src/config/default/peripheral/intc/plib_intc.c ../src/config/default/peripheral/nvm/plib_nvm.c ../src/config/default/peripheral/spi/spi_host/plib_spi1_host.c ../src/config/default/stdio/xc32_monitor.c ../src/config/default/initialization.c ../src/config/default/interrupts.c ../src/config/default/traps.c ../src/driver/driver_CANbus.c ../src/driver/driver_mcp2515.c ../src/driver/flash_eeprom.c ../src/middleware/boot_handler.c ../src/middleware/device_startup.c ../src/software/state_engine.c ../src/main.c



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/PIC32AK_SPI_CAN_BOOTLOADER.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=32AK1216GC41064
MP_LINKER_FILE_OPTION=,--script="..\src\config\default\BOOT_p32AK1216GC41064.gld"
# ------------------------------------------------------------------------------------
# Rules for buildStep: pic32a_compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/60165520/plib_clk.o: ../src/config/default/peripheral/clk/plib_clk.c  .generated_files/flags/default/f299ef814a2fea9b57c9ac36daaf6627e7804375 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/60165520" 
	@${RM} ${OBJECTDIR}/_ext/60165520/plib_clk.o.d 
	@${RM} ${OBJECTDIR}/_ext/60165520/plib_clk.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/config/default/peripheral/clk/plib_clk.c  -o ${OBJECTDIR}/_ext/60165520/plib_clk.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/60165520/plib_clk.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PICKITBASIC=1    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/1865254177/plib_gpio.o: ../src/config/default/peripheral/gpio/plib_gpio.c  .generated_files/flags/default/62e3a402942df8bb1b379bc602ed8ce56e8b7a18 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1865254177" 
	@${RM} ${OBJECTDIR}/_ext/1865254177/plib_gpio.o.d 
	@${RM} ${OBJECTDIR}/_ext/1865254177/plib_gpio.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/config/default/peripheral/gpio/plib_gpio.c  -o ${OBJECTDIR}/_ext/1865254177/plib_gpio.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1865254177/plib_gpio.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PICKITBASIC=1    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/1865312166/plib_intc.o: ../src/config/default/peripheral/intc/plib_intc.c  .generated_files/flags/default/b9da4b04a14323e80aa5e9ee9d2f4d59f70f9ada .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1865312166" 
	@${RM} ${OBJECTDIR}/_ext/1865312166/plib_intc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1865312166/plib_intc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/config/default/peripheral/intc/plib_intc.c  -o ${OBJECTDIR}/_ext/1865312166/plib_intc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1865312166/plib_intc.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PICKITBASIC=1    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/60176403/plib_nvm.o: ../src/config/default/peripheral/nvm/plib_nvm.c  .generated_files/flags/default/b0fee49fcf949f5e81ac5a175f743bbe7d718ec0 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/60176403" 
	@${RM} ${OBJECTDIR}/_ext/60176403/plib_nvm.o.d 
	@${RM} ${OBJECTDIR}/_ext/60176403/plib_nvm.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/config/default/peripheral/nvm/plib_nvm.c  -o ${OBJECTDIR}/_ext/60176403/plib_nvm.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/60176403/plib_nvm.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PICKITBASIC=1    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/1868329136/plib_spi1_host.o: ../src/config/default/peripheral/spi/spi_host/plib_spi1_host.c  .generated_files/flags/default/d7ee3b4a48bed7eb454d4133ef53eb206fcc9854 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1868329136" 
	@${RM} ${OBJECTDIR}/_ext/1868329136/plib_spi1_host.o.d 
	@${RM} ${OBJECTDIR}/_ext/1868329136/plib_spi1_host.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/config/default/peripheral/spi/spi_host/plib_spi1_host.c  -o ${OBJECTDIR}/_ext/1868329136/plib_spi1_host.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1868329136/plib_spi1_host.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PICKITBASIC=1    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/163028504/xc32_monitor.o: ../src/config/default/stdio/xc32_monitor.c  .generated_files/flags/default/7645282362db350c5d888391a0705c96e92dbabe .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/163028504" 
	@${RM} ${OBJECTDIR}/_ext/163028504/xc32_monitor.o.d 
	@${RM} ${OBJECTDIR}/_ext/163028504/xc32_monitor.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/config/default/stdio/xc32_monitor.c  -o ${OBJECTDIR}/_ext/163028504/xc32_monitor.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/163028504/xc32_monitor.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PICKITBASIC=1    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/1171490990/initialization.o: ../src/config/default/initialization.c  .generated_files/flags/default/9843be99748836ff7a346a84b2ef46b2252ba47e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1171490990" 
	@${RM} ${OBJECTDIR}/_ext/1171490990/initialization.o.d 
	@${RM} ${OBJECTDIR}/_ext/1171490990/initialization.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/config/default/initialization.c  -o ${OBJECTDIR}/_ext/1171490990/initialization.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1171490990/initialization.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PICKITBASIC=1    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/1171490990/interrupts.o: ../src/config/default/interrupts.c  .generated_files/flags/default/4591b87e290353b467caa6511317edc855190ba3 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1171490990" 
	@${RM} ${OBJECTDIR}/_ext/1171490990/interrupts.o.d 
	@${RM} ${OBJECTDIR}/_ext/1171490990/interrupts.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/config/default/interrupts.c  -o ${OBJECTDIR}/_ext/1171490990/interrupts.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1171490990/interrupts.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PICKITBASIC=1    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/1171490990/traps.o: ../src/config/default/traps.c  .generated_files/flags/default/6ce06ea784c687634aa8c43dc71fc469a1939907 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1171490990" 
	@${RM} ${OBJECTDIR}/_ext/1171490990/traps.o.d 
	@${RM} ${OBJECTDIR}/_ext/1171490990/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/config/default/traps.c  -o ${OBJECTDIR}/_ext/1171490990/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1171490990/traps.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PICKITBASIC=1    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/362756510/driver_CANbus.o: ../src/driver/driver_CANbus.c  .generated_files/flags/default/997f0aa54d1a20d26245f1203ca584ae9efc6e5b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/362756510" 
	@${RM} ${OBJECTDIR}/_ext/362756510/driver_CANbus.o.d 
	@${RM} ${OBJECTDIR}/_ext/362756510/driver_CANbus.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/driver/driver_CANbus.c  -o ${OBJECTDIR}/_ext/362756510/driver_CANbus.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/362756510/driver_CANbus.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PICKITBASIC=1    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/362756510/driver_mcp2515.o: ../src/driver/driver_mcp2515.c  .generated_files/flags/default/f2a1795473d17d91e64ce1bfba732e8ab4fc00e5 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/362756510" 
	@${RM} ${OBJECTDIR}/_ext/362756510/driver_mcp2515.o.d 
	@${RM} ${OBJECTDIR}/_ext/362756510/driver_mcp2515.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/driver/driver_mcp2515.c  -o ${OBJECTDIR}/_ext/362756510/driver_mcp2515.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/362756510/driver_mcp2515.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PICKITBASIC=1    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/362756510/flash_eeprom.o: ../src/driver/flash_eeprom.c  .generated_files/flags/default/922440cb01c2b0736ad6cc34331cf56343e1e027 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/362756510" 
	@${RM} ${OBJECTDIR}/_ext/362756510/flash_eeprom.o.d 
	@${RM} ${OBJECTDIR}/_ext/362756510/flash_eeprom.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/driver/flash_eeprom.c  -o ${OBJECTDIR}/_ext/362756510/flash_eeprom.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/362756510/flash_eeprom.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PICKITBASIC=1    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/1375476428/boot_handler.o: ../src/middleware/boot_handler.c  .generated_files/flags/default/69c67347803c0ebdbf3e06fabd98df05c4656444 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1375476428" 
	@${RM} ${OBJECTDIR}/_ext/1375476428/boot_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1375476428/boot_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/middleware/boot_handler.c  -o ${OBJECTDIR}/_ext/1375476428/boot_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1375476428/boot_handler.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PICKITBASIC=1    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/1375476428/device_startup.o: ../src/middleware/device_startup.c  .generated_files/flags/default/6447fcdeee63425c13d1616b6b05a0234eb13122 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1375476428" 
	@${RM} ${OBJECTDIR}/_ext/1375476428/device_startup.o.d 
	@${RM} ${OBJECTDIR}/_ext/1375476428/device_startup.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/middleware/device_startup.c  -o ${OBJECTDIR}/_ext/1375476428/device_startup.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1375476428/device_startup.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PICKITBASIC=1    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/1200941409/state_engine.o: ../src/software/state_engine.c  .generated_files/flags/default/97240560e6fd33c406231500e0506decfec745c7 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1200941409" 
	@${RM} ${OBJECTDIR}/_ext/1200941409/state_engine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1200941409/state_engine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/software/state_engine.c  -o ${OBJECTDIR}/_ext/1200941409/state_engine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1200941409/state_engine.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PICKITBASIC=1    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/1360937237/main.o: ../src/main.c  .generated_files/flags/default/efd2f4b9d3277a9d66afb3ac72da714ce0758fd8 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1360937237" 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/main.c  -o ${OBJECTDIR}/_ext/1360937237/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1360937237/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PICKITBASIC=1    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
else
${OBJECTDIR}/_ext/60165520/plib_clk.o: ../src/config/default/peripheral/clk/plib_clk.c  .generated_files/flags/default/89d20884bcbb0bdcd5e8d06d78b9f2488a8da6af .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/60165520" 
	@${RM} ${OBJECTDIR}/_ext/60165520/plib_clk.o.d 
	@${RM} ${OBJECTDIR}/_ext/60165520/plib_clk.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/config/default/peripheral/clk/plib_clk.c  -o ${OBJECTDIR}/_ext/60165520/plib_clk.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/60165520/plib_clk.o.d"        -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/1865254177/plib_gpio.o: ../src/config/default/peripheral/gpio/plib_gpio.c  .generated_files/flags/default/4f5772035862ebbacf29c1fe600f5ea318fb8535 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1865254177" 
	@${RM} ${OBJECTDIR}/_ext/1865254177/plib_gpio.o.d 
	@${RM} ${OBJECTDIR}/_ext/1865254177/plib_gpio.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/config/default/peripheral/gpio/plib_gpio.c  -o ${OBJECTDIR}/_ext/1865254177/plib_gpio.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1865254177/plib_gpio.o.d"        -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/1865312166/plib_intc.o: ../src/config/default/peripheral/intc/plib_intc.c  .generated_files/flags/default/64883568a6a95b6da943fa4d40b7a41743b0c48 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1865312166" 
	@${RM} ${OBJECTDIR}/_ext/1865312166/plib_intc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1865312166/plib_intc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/config/default/peripheral/intc/plib_intc.c  -o ${OBJECTDIR}/_ext/1865312166/plib_intc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1865312166/plib_intc.o.d"        -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/60176403/plib_nvm.o: ../src/config/default/peripheral/nvm/plib_nvm.c  .generated_files/flags/default/9bb0a831791b92d9dd0ecbe95a885e8619b6764e .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/60176403" 
	@${RM} ${OBJECTDIR}/_ext/60176403/plib_nvm.o.d 
	@${RM} ${OBJECTDIR}/_ext/60176403/plib_nvm.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/config/default/peripheral/nvm/plib_nvm.c  -o ${OBJECTDIR}/_ext/60176403/plib_nvm.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/60176403/plib_nvm.o.d"        -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/1868329136/plib_spi1_host.o: ../src/config/default/peripheral/spi/spi_host/plib_spi1_host.c  .generated_files/flags/default/a891903de623d16250a942238704f8e020a9b774 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1868329136" 
	@${RM} ${OBJECTDIR}/_ext/1868329136/plib_spi1_host.o.d 
	@${RM} ${OBJECTDIR}/_ext/1868329136/plib_spi1_host.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/config/default/peripheral/spi/spi_host/plib_spi1_host.c  -o ${OBJECTDIR}/_ext/1868329136/plib_spi1_host.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1868329136/plib_spi1_host.o.d"        -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/163028504/xc32_monitor.o: ../src/config/default/stdio/xc32_monitor.c  .generated_files/flags/default/e028656efdf9a9c48b56062ae6261dad52be1a7b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/163028504" 
	@${RM} ${OBJECTDIR}/_ext/163028504/xc32_monitor.o.d 
	@${RM} ${OBJECTDIR}/_ext/163028504/xc32_monitor.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/config/default/stdio/xc32_monitor.c  -o ${OBJECTDIR}/_ext/163028504/xc32_monitor.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/163028504/xc32_monitor.o.d"        -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/1171490990/initialization.o: ../src/config/default/initialization.c  .generated_files/flags/default/5723d878dfec8ff9a568623f6fe1cba2c91946d6 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1171490990" 
	@${RM} ${OBJECTDIR}/_ext/1171490990/initialization.o.d 
	@${RM} ${OBJECTDIR}/_ext/1171490990/initialization.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/config/default/initialization.c  -o ${OBJECTDIR}/_ext/1171490990/initialization.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1171490990/initialization.o.d"        -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/1171490990/interrupts.o: ../src/config/default/interrupts.c  .generated_files/flags/default/462cfd61141061eb0040f9488e45015c17191466 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1171490990" 
	@${RM} ${OBJECTDIR}/_ext/1171490990/interrupts.o.d 
	@${RM} ${OBJECTDIR}/_ext/1171490990/interrupts.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/config/default/interrupts.c  -o ${OBJECTDIR}/_ext/1171490990/interrupts.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1171490990/interrupts.o.d"        -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/1171490990/traps.o: ../src/config/default/traps.c  .generated_files/flags/default/63a59edb022a83cb29878496be60504b3fd61f3c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1171490990" 
	@${RM} ${OBJECTDIR}/_ext/1171490990/traps.o.d 
	@${RM} ${OBJECTDIR}/_ext/1171490990/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/config/default/traps.c  -o ${OBJECTDIR}/_ext/1171490990/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1171490990/traps.o.d"        -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/362756510/driver_CANbus.o: ../src/driver/driver_CANbus.c  .generated_files/flags/default/bc9cba16377a7ada2ef1931edb37844b05d54236 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/362756510" 
	@${RM} ${OBJECTDIR}/_ext/362756510/driver_CANbus.o.d 
	@${RM} ${OBJECTDIR}/_ext/362756510/driver_CANbus.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/driver/driver_CANbus.c  -o ${OBJECTDIR}/_ext/362756510/driver_CANbus.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/362756510/driver_CANbus.o.d"        -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/362756510/driver_mcp2515.o: ../src/driver/driver_mcp2515.c  .generated_files/flags/default/94ff267247bc7e9f887d66cbf00d2c8dcb6e4238 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/362756510" 
	@${RM} ${OBJECTDIR}/_ext/362756510/driver_mcp2515.o.d 
	@${RM} ${OBJECTDIR}/_ext/362756510/driver_mcp2515.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/driver/driver_mcp2515.c  -o ${OBJECTDIR}/_ext/362756510/driver_mcp2515.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/362756510/driver_mcp2515.o.d"        -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/362756510/flash_eeprom.o: ../src/driver/flash_eeprom.c  .generated_files/flags/default/5a121c35b99b8f51c7500887ca9fbd1e43a6e47c .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/362756510" 
	@${RM} ${OBJECTDIR}/_ext/362756510/flash_eeprom.o.d 
	@${RM} ${OBJECTDIR}/_ext/362756510/flash_eeprom.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/driver/flash_eeprom.c  -o ${OBJECTDIR}/_ext/362756510/flash_eeprom.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/362756510/flash_eeprom.o.d"        -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/1375476428/boot_handler.o: ../src/middleware/boot_handler.c  .generated_files/flags/default/8baadd70bec5e32d553df852b847e2f170457d2b .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1375476428" 
	@${RM} ${OBJECTDIR}/_ext/1375476428/boot_handler.o.d 
	@${RM} ${OBJECTDIR}/_ext/1375476428/boot_handler.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/middleware/boot_handler.c  -o ${OBJECTDIR}/_ext/1375476428/boot_handler.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1375476428/boot_handler.o.d"        -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/1375476428/device_startup.o: ../src/middleware/device_startup.c  .generated_files/flags/default/5a753bd036cc207d0b372a74ba0802e3738855ba .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1375476428" 
	@${RM} ${OBJECTDIR}/_ext/1375476428/device_startup.o.d 
	@${RM} ${OBJECTDIR}/_ext/1375476428/device_startup.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/middleware/device_startup.c  -o ${OBJECTDIR}/_ext/1375476428/device_startup.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1375476428/device_startup.o.d"        -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/1200941409/state_engine.o: ../src/software/state_engine.c  .generated_files/flags/default/2ff75680b78289f1021e3693c8cb712102b492fb .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1200941409" 
	@${RM} ${OBJECTDIR}/_ext/1200941409/state_engine.o.d 
	@${RM} ${OBJECTDIR}/_ext/1200941409/state_engine.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/software/state_engine.c  -o ${OBJECTDIR}/_ext/1200941409/state_engine.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1200941409/state_engine.o.d"        -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
${OBJECTDIR}/_ext/1360937237/main.o: ../src/main.c  .generated_files/flags/default/b87f19b7fb452d2561959e13c702d5dfb87279 .generated_files/flags/default/da39a3ee5e6b4b0d3255bfef95601890afd80709
	@${MKDIR} "${OBJECTDIR}/_ext/1360937237" 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../src/main.c  -o ${OBJECTDIR}/_ext/1360937237/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MP -MMD -MF "${OBJECTDIR}/_ext/1360937237/main.o.d"        -g -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -ffunction-sections -fdata-sections -O1 -I"../src" -I"../src/config/default" -msmart-io=1 -msfr-warn=off   -mdfp="${DFP_DIR}/xc32"
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: pic32a_assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: pic32a_assemblePreproc
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: pic32a_link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${DISTDIR}/PIC32AK_SPI_CAN_BOOTLOADER.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    ../src/config/default/BOOT_p32AK1216GC41064.gld
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/PIC32AK_SPI_CAN_BOOTLOADER.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG=__DEBUG -D__MPLAB_DEBUGGER_PICKITBASIC=1  -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)      -Wl,,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D__DEBUG=__DEBUG,--defsym=__MPLAB_DEBUGGER_PICKITBASIC=1,$(MP_LINKER_FILE_OPTION),--heap=0,--data-init,--gc-sections,--isr,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST) -mdfp="${DFP_DIR}/xc32" 
	
else
${DISTDIR}/PIC32AK_SPI_CAN_BOOTLOADER.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   ../src/config/default/BOOT_p32AK1216GC41064.gld
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o ${DISTDIR}/PIC32AK_SPI_CAN_BOOTLOADER.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD)  -Wl,,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--heap=0,--data-init,--gc-sections,--isr,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--memorysummary,${DISTDIR}/memoryfile.xml$(MP_EXTRA_LD_POST) -mdfp="${DFP_DIR}/xc32" 
	${MP_CC_DIR}\\xc32-bin2hex ${DISTDIR}/PIC32AK_SPI_CAN_BOOTLOADER.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a   -mdfp="${DFP_DIR}/xc32" 
	
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${OBJECTDIR}
	${RM} -r ${DISTDIR}

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(wildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
