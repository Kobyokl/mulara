run:
	mkdir -p build
	gcc main.c -o build/main
	build/main

clean:
	rm -r build/main
