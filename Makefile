.PHONY:
install: build/clashctl
	cp -r clashctl ~
	cp build/clashctl ~/clashctl

build/clashctl:
	mkdir build
	cd build && cmake .. && cmake --build .

.PHONY:
clean:
	rm -rf build/*