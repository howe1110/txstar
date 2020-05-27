src_files = $(wildcard *.cpp)
head_files = $(wildcard *.h)
objects = $(src_files:%.cpp=%.o)
target = testNode

INCLUDEPATH = -I../../../3rd/spdlog/include -I../../../3rd/googletest/googletest/include -I../../../3rd/googletest/googlemock/include -I../
LIBPATH = -L../../../3rd/spdlog/build -L../../../3rd/googletest/googletest/build/lib -L../../../3rd/googletest/googlemock/lib
CFALG = -g -c

$(target): $(objects)
	g++ $(objects) $(LIBPATH) -lgmock -lgtest -lgtest_main -lpthread -lprotobuf -g -o $@
$(objects): %.o: %.cpp
	g++ -MMD $(INCLUDEPATH) $(CFALG) $<

.PHONY: clean
clean:
	rm -f $(objects) $(target)

-include *.d
