.PHONY:
install: build/clashctl
	cp -r clashctl ~
	cp build/clashctl ~/clashctl

.PHONY:
reinstall:
	make uninstall
	make clean
	make install

.PHONY:
uninstall:
	rm -rf ~/clashctl

build/clashctl:
	mkdir build
	cd build && cmake .. && cmake --build .

.PHONY:
clean:
	rm -rf build