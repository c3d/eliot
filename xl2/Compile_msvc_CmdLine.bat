@rem Generate the C++ version of the XL compiler

@mkdir debug
cl /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "CONFIG_MSVC" /c /EHsc /GR /W3 /MLd main.cpp scanner.cpp tree.cpp context.cpp parser.cpp ctrans.cpp options.cpp errors.cpp
link /SUBSYSTEM:CONSOLE /OUT:debug\xl2.exe main.obj scanner.obj tree.obj context.obj parser.obj ctrans.obj options.obj errors.obj
@echo xl2.exe has beeen created in the "Debug" directory
@del /Q *.obj