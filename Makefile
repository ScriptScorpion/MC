CC := g++
CCF := -std=c++17 -Wall -Wextra -s -O1 # C++17 is recommended so make_unique will work faster(create pointer immediately(C++14 can move pointer to the variable or can assign pointer to the variable))
SOURCE := $(wildcard *.cpp)
OUTPUT := mclang

compile:
	$(CC) $(CCF) $(SOURCE) -o $(OUTPUT)
ifeq ($(OS), Windows_NT)
	@echo "Right now Windows not fully supported, recommend running Linux if you want to execute the code"
else
	@echo -n "Do you want to use executable from any place? [Y/n]: "
	@read INPUT; \
	if [ $$INPUT = "Y" ]; then \
	    sudo mv mclang /usr/local/bin/; \
	fi
endif
