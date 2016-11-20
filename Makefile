
TARGET := test
WEB_TARGET := $(TARGET).html

CPP_FLAGS := -std=c++11

native: $(TARGET)

$(TARGET): main.cpp
	g++ main.cpp $(CPP_FLAGS) -o $@

web: $(WEB_TARGET)

$(WEB_TARGET): main.cpp
	em++ main.cpp -o $@ --emrun $(CPP_FLAGS)

run: web
	emrun $(WEB_TARGET)

clean:
	rm -f $(TARGET) $(WEB_TARGET) *.o *.js
