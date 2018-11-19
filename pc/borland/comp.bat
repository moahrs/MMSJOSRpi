d:\moacir\borland\bcc55\bin\bcc32 -c -tW -I"d:\moacir\borland\bcc55\include" -IH:\trabalho -L"d:\moacir\borland\bcc55\lib" %1.cpp
d:\moacir\borland\bcc55\bin\ilink32 -ap -c -x -Gn -L"d:\moacir\borland\bcc55\lib" %1.obj c0x32.obj,%1.exe,,import32.lib cw32.lib
