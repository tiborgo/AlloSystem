#include "allocore/system/al_Config.h"

// Note: this is a renamed version of the library header serial/serial.h
#include "allocore/io/al_Serial.hpp"

//using namespace al;

#include "serial/serial.cc"

#ifdef AL_WINDOWS
	#include "serial/win.cc"
	#include "serial/list_ports_win.cc"
#elif defined AL_OSX
	#include "serial/unix.cc"
	#include "serial/list_ports_osx.cc"
#else
	#include "serial/unix.cc"
	#include "serial/list_ports_linux.cc"
#endif

