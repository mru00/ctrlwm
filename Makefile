VERSION=0.6.0.5

build:
	$(MAKE) -C src

install: build
	sudo install src/ctrlwm /usr/bin

dist:
	rm -rf ./dist
	mkdir -p dist/ctrlwm-$(VERSION)/src
	cp Makefile README.asciidoc dist/ctrlwm-$(VERSION)
	cp src/Makefile src/ctrlwm.c dist/ctrlwm-$(VERSION)/src
	cd dist && tar cvzf ../ctrlwm-$(VERSION).tar.gz ctrlwm-$(VERSION)
	rm -rf dist

.PHONY: build install dist

