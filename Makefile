CC=g++
CFLAGS=-std=c++20 -I/opt/anaconda3/bin/python -lpython \
       -I/home/petersonguo/.local/lib/python3.12/site-packages/numpy/core/include \
			 -I/Applications/Xcode.app/Contents/Developer/Library/Frameworks/Python3.framework/Versions/3.9/Resources/Python.app/Contents/MacOS/Python \
       -shared -framework Python -g

TARGET=main
SRC=$(TARGET).cpp
TESTDATA=testdata.txt
OUTPUT=out.txt

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET) $(TESTDATA)
	./$(TARGET) $(TESTDATA) $(OUTPUT)

$(TESTDATA): generate_data.py
	python3 generate_data.py > $(TESTDATA)

clean:
	rm -f $(TARGET) $(TESTDATA) $(OUTPUT)
