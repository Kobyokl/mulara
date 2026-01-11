source_files := main.c gbstructs.h gbstructs.c cpu_core.h cpu_core.c
run_program:
	mkdir -p build
	gcc $(source_files) -o build/main
	build/main test_roms/tetris.gb
clean:
	rm -r build

#testing working on different device