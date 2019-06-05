CXX=g++

CURDIR =$(shell pwd)
BINDIR = ./bin
TESTBINDIR = ./testbin
LIBDIR = ./lib
OBJDIR = ./obj
INCLUDEDIR = ./include
SRCDIR = ./src
LIBCSVDIR = libcsv-3.0.3
LIBCSV_NAME = libcsv.a

INCLUDE = -I $(INCLUDEDIR) -I ./googletest/googletest/include
CXXFLAGS = -std=c++14 $(INCLUDE)
TESTLDFLAGS = -L /opt/local/lib -L ./googletest/build/lib -lgtest_main -lgtest -lexpat
LDFLAGS = -lexpat -L /usr/local/lib -lcsv

CSV_READER_OBJ = $(OBJDIR)/CSVReader.o
MAP_ROUTER_OBJ = $(OBJDIR)/MapRouter.o
NODE_CLASS_OBJ = $(OBJDIR)/nodeClass.o
SPEED_TEST_OBJ = $(OBJDIR)/speedtest.o
TEST_ROUTER_OBJ = $(OBJDIR)/testrouter.o
XML_READER_OBJ = $(OBJDIR)/XMLReader.o
MAIN_OBJ = $(OBJDIR)/Main.o

SPEED_TEST_NAME = speedtest
TEST_ROUTER_NAME = testrouter
MAIN_NAME = Main


all: $(LIBDIR)/$(LIBCSV_NAME) directories runtest $(TESTBINDIR)/$(MAIN_NAME)

runtest: $(TESTBINDIR)/$(CSV_TEST_NAME) $(TESTBINDIR)/$(XML_TEST_NAME)
	$(TESTBINDIR)/$(SPEED_TEST_NAME)
	$(TESTBINDIR)/$(TEST_ROUTER_NAME)

directories:
	mkdir -p $(OBJDIR)
	mkdir -p $(TESTBINDIR)

$(CSV_READER_OBJ): $(SRCDIR)/CSVReader.cpp $(INCLUDEDIR)/CSVReader.h
	$(CXX) $(SRCDIR)/CSVReader.cpp -c $(CXXFLAGS) -o $(CSV_READER_OBJ)

$(XML_READER_OBJ): $(SRCDIR)/XMLReader.cpp $(INCLUDEDIR)/XMLReader.h
	$(CXX) $(SRCDIR)/XMLReader.cpp -c $(CXXFLAGS) -o $(XML_READER_OBJ)

	#nodeClass.cpp
$(NODE_CLASS_OBJ): $(SRCDIR)/nodeClass.cpp $(INCLUDEDIR)/nodeclass.h
	$(CXX) $(SRCDIR)/nodeClass.cpp -c $(CXXFLAGS) -o $(NODE_CLASS_OBJ)

	#mapRouter.cpp
$(MAP_ROUTER_OBJ): $(SRCDIR)/mapRouter.cpp $(INCLUDEDIR)/mapRouter.h
	$(CXX) $(SRCDIR)/mapRouter.cpp -c $(CXXFLAGS) -o $(MAP_ROUTER_OBJ)

#speedtest.cpp
$(SPEED_TEST_OBJ): $(SRCDIR)/speedtest.cpp $(INCLUDEDIR)/MapRouter.h
	$(CXX) $(SRCDIR)/speedtest.cpp -c $(CXXFLAGS) -o $(SPEED_TEST_OBJ)

$(TESTBINDIR)/$(SPEED_TEST_NAME): $(MAP_ROUTER_OBJ) $(SPEED_TEST_OBJ) $(LIBDIR)/$(LIBCSV_NAME)
	$(CXX) $(MAP_ROUTER_OBJ) $(SPEED_TEST_OBJ) /usr/local/lib/$(LIBCSV_NAME) $(CXXFLAGS) $(TESTLDFLAGS) -o $(TESTBINDIR)/$(SPEED_TEST_NAME)

#testrouter.cpp
$(TEST_ROUTER_OBJ): $(SRCDIR)/testrouter.cpp $(INCLUDEDIR)/MapRouter.h
	$(CXX) $(SRCDIR)/testrouter.cpp -c $(CXXFLAGS) -o $(TEST_ROUTER_OBJ)

$(TESTBINDIR)/$(TEST_ROUTER_NAME): $(MAP_ROUTER_OBJ) $(TEST_ROUTER_OBJ)
	$(CXX) $(MAP_ROUTER_OBJ) $(TEST_ROUTER_OBJ) $(CXXFLAGS) $(TESTLDFLAGS) -o $(TESTBINDIR)/$(TEST_ROUTER_NAME)

$(MAIN_OBJ): $(SRCDIR)/Main.cpp
	$(CXX) $(SRCDIR)/Main.cpp -c $(CXXFLAGS) -o $(MAIN_OBJ)

$(TESTBINDIR)/$(MAIN_NAME): $(MAIN_OBJ) $(CSV_READER_OBJ) $(XML_READER_OBJ) $(NODE_CLASS_OBJ) $(MAP_ROUTER_OBJ) $(LIBDIR)/$(LIBCSV_NAME)
	$(CXX) $(MAIN_OBJ) $(CSV_READER_OBJ) $(XML_READER_OBJ) $(NODE_CLASS_OBJ) $(MAP_ROUTER_OBJ) $(CXXFLAGS) $(LDFLAGS) -o $(TESTBINDIR)/$(MAIN_NAME)

$(LIBCSVDIR)/Makefile:
	cd $(LIBCSVDIR); ./configure --prefix=$(CURDIR); cd ..

$(LIBCSVDIR)/libcsv.la: $(LIBCSVDIR)/Makefile
	cd $(LIBCSVDIR); make; cd ..

$(LIBDIR)/$(LIBCSV_NAME): $(LIBCSVDIR)/libcsv.la
	cd $(LIBCSVDIR); make install; cd ..

clean:
	rm -f $(LIBDIR)/*
	rm -f include/csv.h
	cd $(LIBCSVDIR); make clean; cd ..
	rm -f $(LIBCSVIDR)/Makefile
	rm -r $(OBJDIR)
	rm -r $(TESTBINDIR)
