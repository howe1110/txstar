src_files = $(wildcard *.cpp)
src_file_cc = $(wildcard ./proto/*.cc)
objects = $(src_files:%.cpp=%.o)
objects_cc = $(src_file_cc:%.cc=%.o)

target = myp2p

INCLUDEPATH = -I../../3rd/spdlog/include
LIBPATH = -L../../3rd/spdlog/build
CFALG = -g -c

$(target): $(objects) $(objects_cc)
	g++ $(objects) $(objects_cc) $(LIBPATH) -lpthread -lspdlog -lprotobuf -o $@
$(objects): %.o: %.cpp
	g++ -MMD  $(INCLUDEPATH) $(CFALG) $< -o $@
$(objects_cc): %.o: %.cc
	g++ -MMD  $(INCLUDEPATH) $(CFALG) $< -o $@

.PHONY: clean
clean:
	rm -f $(objects) $(target)

-include *.d
