utmconvert.so: utmconvert_wrap.o utmconvert.o
	g++ -O2 -shared -fPIC utmconvert_wrap.o utmconvert.o -o utmconvert.so -lGeographic

utmconvert.o: utmconvert.h utmconvert.cpp
	g++ -O2 -fPIC -c utmconvert.cpp

utmconvert_wrap.cxx: utmconvert.i utmconvert.h
	swig -c++ -ruby utmconvert.i

utmconvert_wrap.o: utmconvert_wrap.cxx
	g++ -O2 -fPIC -c utmconvert_wrap.cxx -I`ruby -r rbconfig.rb -e 'include Config; puts CONFIG["rubyhdrdir"]'` -I`ruby -r rbconfig.rb -e 'include Config; puts ("%s/%s" % [CONFIG["rubyhdrdir"], CONFIG["sitearch"]])'`

.PHONY: clean
clean:
	rm -f *.o *.so utmconvert_wrap.cxx

# Should use this instead of cp utmconvert.so ..
.PHONY: install
install:
	cp utmconvert.so `ruby -r rbconfig.rb -e 'include Config; puts CONFIG["sitearchdir"]'`
