# qsys scripting (.tcl) file for SOC
package require -exact qsys 16.0

create_system {SOC}

set_project_property DEVICE_FAMILY {Cyclone V}
set_project_property DEVICE {5CGXFC7C7F23C8}
set_project_property HIDE_FROM_IP_CATALOG {false}

# Instances and instance parameters
# (disabled instances are intentionally culled)
add_instance CLK clock_source 18.0
set_instance_parameter_value CLK {clockFrequency} {50000000.0}
set_instance_parameter_value CLK {clockFrequencyKnown} {1}
set_instance_parameter_value CLK {resetSynchronousEdges} {NONE}

add_instance FIFO altera_avalon_fifo 18.0
set_instance_parameter_value FIFO {avalonMMAvalonMMDataWidth} {32}
set_instance_parameter_value FIFO {avalonMMAvalonSTDataWidth} {32}
set_instance_parameter_value FIFO {bitsPerSymbol} {16}
set_instance_parameter_value FIFO {channelWidth} {8}
set_instance_parameter_value FIFO {errorWidth} {8}
set_instance_parameter_value FIFO {fifoDepth} {1024}
set_instance_parameter_value FIFO {fifoInputInterfaceOptions} {AVALONMM_WRITE}
set_instance_parameter_value FIFO {fifoOutputInterfaceOptions} {AVALONMM_READ}
set_instance_parameter_value FIFO {showHiddenFeatures} {0}
set_instance_parameter_value FIFO {singleClockMode} {1}
set_instance_parameter_value FIFO {singleResetMode} {0}
set_instance_parameter_value FIFO {symbolsPerBeat} {2}
set_instance_parameter_value FIFO {useBackpressure} {1}
set_instance_parameter_value FIFO {useIRQ} {1}
set_instance_parameter_value FIFO {usePacket} {1}
set_instance_parameter_value FIFO {useReadControl} {0}
set_instance_parameter_value FIFO {useRegister} {0}
set_instance_parameter_value FIFO {useWriteControl} {1}

add_instance NIOS altera_nios2_gen2 18.0
set_instance_parameter_value NIOS {bht_ramBlockType} {Automatic}
set_instance_parameter_value NIOS {breakOffset} {32}
set_instance_parameter_value NIOS {breakSlave} {None}
set_instance_parameter_value NIOS {cdx_enabled} {0}
set_instance_parameter_value NIOS {cpuArchRev} {1}
set_instance_parameter_value NIOS {cpuID} {0}
set_instance_parameter_value NIOS {cpuReset} {0}
set_instance_parameter_value NIOS {data_master_high_performance_paddr_base} {0}
set_instance_parameter_value NIOS {data_master_high_performance_paddr_size} {0.0}
set_instance_parameter_value NIOS {data_master_paddr_base} {0}
set_instance_parameter_value NIOS {data_master_paddr_size} {0.0}
set_instance_parameter_value NIOS {dcache_bursts} {false}
set_instance_parameter_value NIOS {dcache_numTCDM} {0}
set_instance_parameter_value NIOS {dcache_ramBlockType} {Automatic}
set_instance_parameter_value NIOS {dcache_size} {2048}
set_instance_parameter_value NIOS {dcache_tagramBlockType} {Automatic}
set_instance_parameter_value NIOS {dcache_victim_buf_impl} {ram}
set_instance_parameter_value NIOS {debug_OCIOnchipTrace} {_128}
set_instance_parameter_value NIOS {debug_assignJtagInstanceID} {0}
set_instance_parameter_value NIOS {debug_datatrigger} {0}
set_instance_parameter_value NIOS {debug_debugReqSignals} {0}
set_instance_parameter_value NIOS {debug_enabled} {1}
set_instance_parameter_value NIOS {debug_hwbreakpoint} {0}
set_instance_parameter_value NIOS {debug_jtagInstanceID} {0}
set_instance_parameter_value NIOS {debug_traceStorage} {onchip_trace}
set_instance_parameter_value NIOS {debug_traceType} {none}
set_instance_parameter_value NIOS {debug_triggerArming} {1}
set_instance_parameter_value NIOS {dividerType} {no_div}
set_instance_parameter_value NIOS {exceptionOffset} {32}
set_instance_parameter_value NIOS {exceptionSlave} {RAM.s1}
set_instance_parameter_value NIOS {fa_cache_line} {2}
set_instance_parameter_value NIOS {fa_cache_linesize} {0}
set_instance_parameter_value NIOS {flash_instruction_master_paddr_base} {0}
set_instance_parameter_value NIOS {flash_instruction_master_paddr_size} {0.0}
set_instance_parameter_value NIOS {icache_burstType} {None}
set_instance_parameter_value NIOS {icache_numTCIM} {0}
set_instance_parameter_value NIOS {icache_ramBlockType} {Automatic}
set_instance_parameter_value NIOS {icache_size} {4096}
set_instance_parameter_value NIOS {icache_tagramBlockType} {Automatic}
set_instance_parameter_value NIOS {impl} {Fast}
set_instance_parameter_value NIOS {instruction_master_high_performance_paddr_base} {0}
set_instance_parameter_value NIOS {instruction_master_high_performance_paddr_size} {0.0}
set_instance_parameter_value NIOS {instruction_master_paddr_base} {0}
set_instance_parameter_value NIOS {instruction_master_paddr_size} {0.0}
set_instance_parameter_value NIOS {io_regionbase} {0}
set_instance_parameter_value NIOS {io_regionsize} {0}
set_instance_parameter_value NIOS {master_addr_map} {0}
set_instance_parameter_value NIOS {mmu_TLBMissExcOffset} {0}
set_instance_parameter_value NIOS {mmu_TLBMissExcSlave} {None}
set_instance_parameter_value NIOS {mmu_autoAssignTlbPtrSz} {1}
set_instance_parameter_value NIOS {mmu_enabled} {0}
set_instance_parameter_value NIOS {mmu_processIDNumBits} {8}
set_instance_parameter_value NIOS {mmu_ramBlockType} {Automatic}
set_instance_parameter_value NIOS {mmu_tlbNumWays} {16}
set_instance_parameter_value NIOS {mmu_tlbPtrSz} {7}
set_instance_parameter_value NIOS {mmu_udtlbNumEntries} {6}
set_instance_parameter_value NIOS {mmu_uitlbNumEntries} {4}
set_instance_parameter_value NIOS {mpu_enabled} {0}
set_instance_parameter_value NIOS {mpu_minDataRegionSize} {12}
set_instance_parameter_value NIOS {mpu_minInstRegionSize} {12}
set_instance_parameter_value NIOS {mpu_numOfDataRegion} {8}
set_instance_parameter_value NIOS {mpu_numOfInstRegion} {8}
set_instance_parameter_value NIOS {mpu_useLimit} {0}
set_instance_parameter_value NIOS {mpx_enabled} {0}
set_instance_parameter_value NIOS {mul_32_impl} {2}
set_instance_parameter_value NIOS {mul_64_impl} {0}
set_instance_parameter_value NIOS {mul_shift_choice} {0}
set_instance_parameter_value NIOS {ocimem_ramBlockType} {Automatic}
set_instance_parameter_value NIOS {ocimem_ramInit} {0}
set_instance_parameter_value NIOS {regfile_ramBlockType} {Automatic}
set_instance_parameter_value NIOS {register_file_por} {0}
set_instance_parameter_value NIOS {resetOffset} {0}
set_instance_parameter_value NIOS {resetSlave} {RAM.s1}
set_instance_parameter_value NIOS {resetrequest_enabled} {1}
set_instance_parameter_value NIOS {setting_HBreakTest} {0}
set_instance_parameter_value NIOS {setting_HDLSimCachesCleared} {1}
set_instance_parameter_value NIOS {setting_activateMonitors} {1}
set_instance_parameter_value NIOS {setting_activateTestEndChecker} {0}
set_instance_parameter_value NIOS {setting_activateTrace} {0}
set_instance_parameter_value NIOS {setting_allow_break_inst} {0}
set_instance_parameter_value NIOS {setting_alwaysEncrypt} {1}
set_instance_parameter_value NIOS {setting_asic_add_scan_mode_input} {0}
set_instance_parameter_value NIOS {setting_asic_enabled} {0}
set_instance_parameter_value NIOS {setting_asic_synopsys_translate_on_off} {0}
set_instance_parameter_value NIOS {setting_asic_third_party_synthesis} {0}
set_instance_parameter_value NIOS {setting_avalonDebugPortPresent} {0}
set_instance_parameter_value NIOS {setting_bhtPtrSz} {8}
set_instance_parameter_value NIOS {setting_bigEndian} {0}
set_instance_parameter_value NIOS {setting_branchpredictiontype} {Dynamic}
set_instance_parameter_value NIOS {setting_breakslaveoveride} {0}
set_instance_parameter_value NIOS {setting_clearXBitsLDNonBypass} {1}
set_instance_parameter_value NIOS {setting_dc_ecc_present} {1}
set_instance_parameter_value NIOS {setting_disable_tmr_inj} {0}
set_instance_parameter_value NIOS {setting_disableocitrace} {0}
set_instance_parameter_value NIOS {setting_dtcm_ecc_present} {1}
set_instance_parameter_value NIOS {setting_ecc_present} {0}
set_instance_parameter_value NIOS {setting_ecc_sim_test_ports} {0}
set_instance_parameter_value NIOS {setting_exportHostDebugPort} {0}
set_instance_parameter_value NIOS {setting_exportPCB} {0}
set_instance_parameter_value NIOS {setting_export_large_RAMs} {0}
set_instance_parameter_value NIOS {setting_exportdebuginfo} {0}
set_instance_parameter_value NIOS {setting_exportvectors} {0}
set_instance_parameter_value NIOS {setting_fast_register_read} {0}
set_instance_parameter_value NIOS {setting_ic_ecc_present} {1}
set_instance_parameter_value NIOS {setting_interruptControllerType} {Internal}
set_instance_parameter_value NIOS {setting_itcm_ecc_present} {1}
set_instance_parameter_value NIOS {setting_mmu_ecc_present} {1}
set_instance_parameter_value NIOS {setting_oci_export_jtag_signals} {0}
set_instance_parameter_value NIOS {setting_oci_version} {1}
set_instance_parameter_value NIOS {setting_preciseIllegalMemAccessException} {0}
set_instance_parameter_value NIOS {setting_removeRAMinit} {0}
set_instance_parameter_value NIOS {setting_rf_ecc_present} {1}
set_instance_parameter_value NIOS {setting_shadowRegisterSets} {0}
set_instance_parameter_value NIOS {setting_showInternalSettings} {0}
set_instance_parameter_value NIOS {setting_showUnpublishedSettings} {0}
set_instance_parameter_value NIOS {setting_support31bitdcachebypass} {1}
set_instance_parameter_value NIOS {setting_tmr_output_disable} {0}
set_instance_parameter_value NIOS {setting_usedesignware} {0}
set_instance_parameter_value NIOS {shift_rot_impl} {1}
set_instance_parameter_value NIOS {tightly_coupled_data_master_0_paddr_base} {0}
set_instance_parameter_value NIOS {tightly_coupled_data_master_0_paddr_size} {0.0}
set_instance_parameter_value NIOS {tightly_coupled_data_master_1_paddr_base} {0}
set_instance_parameter_value NIOS {tightly_coupled_data_master_1_paddr_size} {0.0}
set_instance_parameter_value NIOS {tightly_coupled_data_master_2_paddr_base} {0}
set_instance_parameter_value NIOS {tightly_coupled_data_master_2_paddr_size} {0.0}
set_instance_parameter_value NIOS {tightly_coupled_data_master_3_paddr_base} {0}
set_instance_parameter_value NIOS {tightly_coupled_data_master_3_paddr_size} {0.0}
set_instance_parameter_value NIOS {tightly_coupled_instruction_master_0_paddr_base} {0}
set_instance_parameter_value NIOS {tightly_coupled_instruction_master_0_paddr_size} {0.0}
set_instance_parameter_value NIOS {tightly_coupled_instruction_master_1_paddr_base} {0}
set_instance_parameter_value NIOS {tightly_coupled_instruction_master_1_paddr_size} {0.0}
set_instance_parameter_value NIOS {tightly_coupled_instruction_master_2_paddr_base} {0}
set_instance_parameter_value NIOS {tightly_coupled_instruction_master_2_paddr_size} {0.0}
set_instance_parameter_value NIOS {tightly_coupled_instruction_master_3_paddr_base} {0}
set_instance_parameter_value NIOS {tightly_coupled_instruction_master_3_paddr_size} {0.0}
set_instance_parameter_value NIOS {tmr_enabled} {0}
set_instance_parameter_value NIOS {tracefilename} {}
set_instance_parameter_value NIOS {userDefinedSettings} {}

add_instance RAM altera_avalon_onchip_memory2 18.0
set_instance_parameter_value RAM {allowInSystemMemoryContentEditor} {0}
set_instance_parameter_value RAM {blockType} {AUTO}
set_instance_parameter_value RAM {copyInitFile} {0}
set_instance_parameter_value RAM {dataWidth} {32}
set_instance_parameter_value RAM {dataWidth2} {32}
set_instance_parameter_value RAM {dualPort} {0}
set_instance_parameter_value RAM {ecc_enabled} {0}
set_instance_parameter_value RAM {enPRInitMode} {0}
set_instance_parameter_value RAM {enableDiffWidth} {0}
set_instance_parameter_value RAM {initMemContent} {1}
set_instance_parameter_value RAM {initializationFileName} {onchip_mem.hex}
set_instance_parameter_value RAM {instanceID} {NONE}
set_instance_parameter_value RAM {memorySize} {16384.0}
set_instance_parameter_value RAM {readDuringWriteMode} {DONT_CARE}
set_instance_parameter_value RAM {resetrequest_enabled} {1}
set_instance_parameter_value RAM {simAllowMRAMContentsFile} {0}
set_instance_parameter_value RAM {simMemInitOnlyFilename} {0}
set_instance_parameter_value RAM {singleClockOperation} {0}
set_instance_parameter_value RAM {slave1Latency} {1}
set_instance_parameter_value RAM {slave2Latency} {1}
set_instance_parameter_value RAM {useNonDefaultInitFile} {0}
set_instance_parameter_value RAM {useShallowMemBlocks} {0}
set_instance_parameter_value RAM {writable} {1}

add_instance REGLEDS altera_avalon_pio 18.0
set_instance_parameter_value REGLEDS {bitClearingEdgeCapReg} {0}
set_instance_parameter_value REGLEDS {bitModifyingOutReg} {0}
set_instance_parameter_value REGLEDS {captureEdge} {0}
set_instance_parameter_value REGLEDS {direction} {Output}
set_instance_parameter_value REGLEDS {edgeType} {RISING}
set_instance_parameter_value REGLEDS {generateIRQ} {0}
set_instance_parameter_value REGLEDS {irqType} {LEVEL}
set_instance_parameter_value REGLEDS {resetValue} {0.0}
set_instance_parameter_value REGLEDS {simDoTestBenchWiring} {0}
set_instance_parameter_value REGLEDS {simDrivenValue} {0.0}
set_instance_parameter_value REGLEDS {width} {32}

# exported interfaces
add_interface clk clock sink
set_interface_property clk EXPORT_OF CLK.clk_in
add_interface leds conduit end
set_interface_property leds EXPORT_OF REGLEDS.external_connection
add_interface reset reset sink
set_interface_property reset EXPORT_OF CLK.clk_in_reset

# connections and connection parameters
add_connection CLK.clk FIFO.clk_in

add_connection CLK.clk NIOS.clk

add_connection CLK.clk RAM.clk1

add_connection CLK.clk REGLEDS.clk

add_connection CLK.clk_reset FIFO.reset_in

add_connection CLK.clk_reset NIOS.reset

add_connection CLK.clk_reset RAM.reset1

add_connection CLK.clk_reset REGLEDS.reset

add_connection NIOS.data_master FIFO.in
set_connection_parameter_value NIOS.data_master/FIFO.in arbitrationPriority {1}
set_connection_parameter_value NIOS.data_master/FIFO.in baseAddress {0x3000}
set_connection_parameter_value NIOS.data_master/FIFO.in defaultConnection {0}

add_connection NIOS.data_master FIFO.in_csr
set_connection_parameter_value NIOS.data_master/FIFO.in_csr arbitrationPriority {1}
set_connection_parameter_value NIOS.data_master/FIFO.in_csr baseAddress {0x3020}
set_connection_parameter_value NIOS.data_master/FIFO.in_csr defaultConnection {0}

add_connection NIOS.data_master FIFO.out
set_connection_parameter_value NIOS.data_master/FIFO.out arbitrationPriority {1}
set_connection_parameter_value NIOS.data_master/FIFO.out baseAddress {0x3010}
set_connection_parameter_value NIOS.data_master/FIFO.out defaultConnection {0}

add_connection NIOS.data_master NIOS.debug_mem_slave
set_connection_parameter_value NIOS.data_master/NIOS.debug_mem_slave arbitrationPriority {1}
set_connection_parameter_value NIOS.data_master/NIOS.debug_mem_slave baseAddress {0x2000}
set_connection_parameter_value NIOS.data_master/NIOS.debug_mem_slave defaultConnection {0}

add_connection NIOS.data_master RAM.s1
set_connection_parameter_value NIOS.data_master/RAM.s1 arbitrationPriority {1}
set_connection_parameter_value NIOS.data_master/RAM.s1 baseAddress {0x0000}
set_connection_parameter_value NIOS.data_master/RAM.s1 defaultConnection {0}

add_connection NIOS.data_master REGLEDS.s1
set_connection_parameter_value NIOS.data_master/REGLEDS.s1 arbitrationPriority {1}
set_connection_parameter_value NIOS.data_master/REGLEDS.s1 baseAddress {0x2800}
set_connection_parameter_value NIOS.data_master/REGLEDS.s1 defaultConnection {0}

add_connection NIOS.instruction_master NIOS.debug_mem_slave
set_connection_parameter_value NIOS.instruction_master/NIOS.debug_mem_slave arbitrationPriority {1}
set_connection_parameter_value NIOS.instruction_master/NIOS.debug_mem_slave baseAddress {0x2000}
set_connection_parameter_value NIOS.instruction_master/NIOS.debug_mem_slave defaultConnection {0}

add_connection NIOS.instruction_master RAM.s1
set_connection_parameter_value NIOS.instruction_master/RAM.s1 arbitrationPriority {1}
set_connection_parameter_value NIOS.instruction_master/RAM.s1 baseAddress {0x0000}
set_connection_parameter_value NIOS.instruction_master/RAM.s1 defaultConnection {0}

add_connection NIOS.irq FIFO.in_irq
set_connection_parameter_value NIOS.irq/FIFO.in_irq irqNumber {0}

# interconnect requirements
set_interconnect_requirement {$system} {qsys_mm.clockCrossingAdapter} {HANDSHAKE}
set_interconnect_requirement {$system} {qsys_mm.enableEccProtection} {FALSE}
set_interconnect_requirement {$system} {qsys_mm.insertDefaultSlave} {FALSE}
set_interconnect_requirement {$system} {qsys_mm.maxAdditionalLatency} {1}

save_system {SOC.qsys}
