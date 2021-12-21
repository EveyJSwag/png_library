CPP_FILES := 	$(wildcard src/*cpp)
O_FILES := 		$(patsubst %.cpp,%.o,$(CPP_FILES))
COMPILE_FLAGS :=	-g -c -std=c++17 -fpic
LINKING_FLAGS :=	-shared
INCLUDE_PATHS :=	-I./include
LIBRARIES :=		-lpng -lstdc++
SHARED_OBJECT_NAME :=	libpngloader.dylib

lib/$(SHARED_OBJECT_NAME) : $(O_FILES)
	gcc $(LINKING_FLAGS) -o lib/$(SHARED_OBJECT_NAME) $(O_FILES) $(LIBRARIES)

%.o : %.cpp
	gcc $(INCLUDE_PATHS) $(COMPILE_FLAGS) $< -o $@

.PHONY: clean
clean:
	rm src/*\.o
	rm lib/*\.dylib