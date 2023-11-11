@echo off
del *.c  *.cpp *.obj *.exe *.log > NUL

SETLOCAL EnableDelayedExpansion

:: Loop through all .c files starting with t_
FOR %%G IN (..\test\t_*.c) DO (
    SET "base=%%~nG"
    COPY %%G . > NUL
    CL /I..\src /O2 /nologo !base!.c
    .\!base! --color-off 2>> test.log    
    COPY !base!.c "!base!++.cpp" > NUL
    CL /I..\src /O2 /nologo "!base!++.cpp"
    .\!base!++ --color-off 2>> test.log    
)

ENDLOCAL
