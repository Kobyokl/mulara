source_files := main.c gbstructs.h cpu_core.h cpu_core.c daa_table.c bus.h bus.c interrupt.h interrupt.c timers.c timers.h 
run_program:
	mkdir -p build
	gcc $(source_files) -o build/main
	build/main test_roms/tetris.gb
clean:
	rm -r build
