PB_SRC_DIR := ./src/pb
PB_DST_DIR := ./src/pb
LINUX_DIR := ./assets/linux
ASSETS_DIR := ./assets
DST_DIR := ./build
HOME_DIR := $(HOME)

.PHONY: build
build:
	protoc -I=$(PB_SRC_DIR) --cpp_out=$(PB_DST_DIR) $(PB_SRC_DIR)/chunk.proto
	protoc -I=$(PB_SRC_DIR) --cpp_out=$(PB_DST_DIR) $(PB_SRC_DIR)/world.proto
	cd build && cmake ..
	make
	cd ..

.PHONY: install
install: 
	@sudo -v || { echo "Sudo permissions required."; exit 1; }
	sudo cp $(LINUX_DIR)/nyx.desktop /usr/local/share/applications
	sudo cp $(ASSETS_DIR)/icon.png /usr/local/share/icons
	mkdir -p $(HOME_DIR)/.nyx
	cp -r $(ASSETS_DIR) $(HOME_DIR)/.nyx/
	cp $(ASSETS_DIR)/config/nyx.ini $(HOME_DIR)/.nyx/
	cp $(DST_DIR)/nyx $(HOME_DIR)/.nyx/

.PHONY: uninstall
uninstall:
	@sudo -v || { echo "Sudo permissions required."; exit 1; }
	sudo rm -rf /usr/local/share/applications/nyx.desktop
	sudo rm -f /usr/local/share/icons/icon.png
	rm -rf $(HOME_DIR)/.nyx

.PHONY: run
run: clear
	cp $(ASSETS_DIR)/config/nyx.ini $(DST_DIR)
	$(DST_DIR)/nyx

.PHONY: clear
clear:
	rm -f $(DST_DIR)/nyx.ini
