source_files := main.c cpu_core.c daa_table.c bus.c interrupt.c timers.c

run_program:
	mkdir -p build
	gcc $(source_files) -o build/main
	build/main test_roms/04-op.gb

clean:
	rm -rf build
