CC=g++
PYTHON=venv/bin/python3

# Dynamically fetch Python include, library paths, and shared library
PYTHON_INCLUDE=$(shell $(PYTHON) -c "from sysconfig import get_paths; print(get_paths()['include'])")
PYTHON_LIBDIR=$(shell $(PYTHON) -c "from sysconfig import get_config_var; print(get_config_var('LIBDIR'))")
PYTHON_SHARED_LIB=$(shell $(PYTHON) -c "from sysconfig import get_config_var; print(get_config_var('LDLIBRARY').lstrip('lib').rstrip('.so'))")
NUMPY_INCLUDE=$(shell $(PYTHON) -c "import numpy; print(numpy.get_include())")

CFLAGS=-std=c++20 -I$(PYTHON_INCLUDE) -I$(NUMPY_INCLUDE)
LDFLAGS=-L$(PYTHON_LIBDIR) -l$(PYTHON_SHARED_LIB) -lpython3.13

TARGET=main
SRC=$(TARGET).cpp
TESTDATA=testdata.txt
OUTPUT=out.txt

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

run: $(TARGET) $(TESTDATA)
	./$(TARGET) $(TESTDATA) $(OUTPUT)

$(TESTDATA): generate_data.py
	$(PYTHON) generate_data.py > $(TESTDATA)

clean:
	rm -f $(TARGET) $(TESTDATA) $(OUTPUT)

