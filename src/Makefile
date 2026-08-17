source_files := main.c gbstructs.h gbstructs.c instructionhandler.h instructionhandler.c daa_table.c
run_program:
	mkdir -p build
	gcc $(source_files) -o build/main
	build/main test_roms/tetris.gb
clean:
	rm -r build
