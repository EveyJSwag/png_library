CPP_FILES := 	$(wildcard src/*cpp)
O_FILES := 		$(patsubst %.cpp,%.o,$(CPP_FILES))
COMPILE_FLAGS :=	-g -c
INCLUDE_PATHS :=	-I./include
LIBRARIES :=		-lpng -lstdc++

exec : $(O_FILES)
	gcc -o exec $(O_FILES) $(LIBRARIES)

%.o : %.cpp
	gcc -std=c++17 $(INCLUDE_PATHS) $(COMPILE_FLAGS) $< -o $@

.PHONY: clean
clean:
	rm src/*.o
	rm exec