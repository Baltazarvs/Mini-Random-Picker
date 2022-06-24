# Mini-Random-Picker
Use this program to pick a random item from a list.

# Building
Use MinGW or MSVC...<br>
<kbd>
  windres resource.rc -o res.o <br>
  g++ -o mpicker.exe main.cpp res.o -std=c++11 -mwindows -lgdi32 -lcomctl32
</kbd><br>

  - -std=c++11 is optional, use 11 or 14 or newer if you want.
  - mpicker is name of executable. It must be mpicker if you want visual styles enabled.
  - "mpicker" can be renamed inside resource.rc's manifest line.
