source_files := main.c cpu_core.c daa_table.c bus.c interrupt.c timers.c ppu.c


tt: # tt = test timings
	mkdir -p build
	gcc $(source_files) -o build/main
	build/main roms/blargg/instr_timing.gb
	build/main roms/blargg/interrupt_time.gb


ti: #ti = test instructions
	mkdir -p build
	gcc $(source_files) -o build/main
	build/main roms/blargg/1.gb
	build/main roms/blargg/2.gb
	build/main roms/blargg/3.gb
	build/main roms/blargg/4.gb
	build/main roms/blargg/5.gb
	build/main roms/blargg/6.gb
	build/main roms/blargg/7.gb
	build/main roms/blargg/8.gb
	build/main roms/blargg/9.gb
	build/main roms/blargg/10.gb
	build/main roms/blargg/11.gb


clean:
	rm -rf build
	