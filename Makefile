.PHONY: all normal physics gamecore host ingame ingamemain install
.DEFAULT_GOAL:= default
default: all

# 默认构建使用 ingame
all: physics gamecore host ingame install

# normal 构建使用 ingamemain
normal: physics gamecore host ingamemain install

PHYSICS_BUILD_DIR := ./Physics/build
GAMECORE_BUILD_DIR := ./GameCore/build
HOST_BUILD_DIR := ./Host/build
INGAME_DIR := ./InGame


$(PHYSICS_BUILD_DIR):
	mkdir $(PHYSICS_BUILD_DIR)

$(GAMECORE_BUILD_DIR):
	mkdir $(GAMECORE_BUILD_DIR)

$(HOST_BUILD_DIR):
	mkdir $(HOST_BUILD_DIR)

physics: $(PHYSICS_BUILD_DIR)
	cd $(PHYSICS_BUILD_DIR)  && cmake ../ && make

gamecore: $(GAMECORE_BUILD_DIR)
	cd $(GAMECORE_BUILD_DIR)  && cmake ../ && make

host: $(HOST_BUILD_DIR)
	cd $(HOST_BUILD_DIR)  && cmake ../ && make

ingame:
	chmod -R +x $(INGAME_DIR)
	cd $(INGAME_DIR) && make

ingamemain:
	chmod -R +x $(INGAME_DIR)
	cd $(INGAME_DIR) && make main

install:
	cp $(PHYSICS_BUILD_DIR)/physics.so $(INGAME_DIR)/bin/game/cservice/Resource/libphysics.so
	cp $(GAMECORE_BUILD_DIR)/libgamecore.so $(INGAME_DIR)/bin/game/cservice/Resource/libgamecore.so
	cp $(HOST_BUILD_DIR)/libhost.so $(INGAME_DIR)/bin/game/cservice/Resource/libhost.so
	cp $(GAMECORE_BUILD_DIR)/../Res/*.bin $(INGAME_DIR)/bin/game/cservice/Resource

clean:
	rm -rf $(PHYSICS_BUILD_DIR)
	rm -rf $(GAMECORE_BUILD_DIR)
	rm -rf $(HOST_BUILD_DIR)
	rm -rf $(INGAME_DIR)/source/main/cmake-build
