CC = g++

TARGET = app.cpp \
		.\lib\mathplot.cpp \
		.\InterfaceApp.cpp

OUT = app

INCLUDE = -I .\include \
		  -I .\app \
		  -I $(VCPKG_ROOT)\installed\x64-mingw-static\include \
		  -I C:\Wxwidget_13_2\bin\include \
		  -I C:\Wxwidget_13_2\bin\lib\Win64\mswu

LIBRARY = -L $(VCPKG_ROOT)\installed\x64-mingw-static\lib \
		  -L C:\Wxwidget_13_2\bin\lib\Win64	\
		  -L .\lib

LIBS = -lstdc++\
	   -lws2_32\
	   -lwxbase32u \
	   -lwxmsw32u_core \
	   -lSerial \
	   -lfmt

all:
	$(CC) -o $(OUT) $(TARGET) $(INCLUDE) $(LIBRARY) $(LIBS)