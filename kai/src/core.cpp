#include <kai/core.h>

void kai_get_version(kai_Version* ver) {
	ver->major = KAI_VERSION_MAJOR;
	ver->minor = KAI_VERSION_MINOR;
	ver->patch = KAI_VERSION_PATCH;
}

char const* kai_get_version_string() {
	return "Kai Compiler " KAI_VERSION_STR;
}

// String Functions

bool kai_string_equals( kai_str a, kai_str b )
{
	if( a.count != b.count ) return false;

	for( kai_int i = 0; i < a.count; ++i ) {
		if( a.data[i] != b.data[i] )
			return false;
	}

	return true;
}

#include <iostream>

void panic() {
	std::cout << "\nPanic triggered.\nNow exiting...\n";
	std::exit(1);
}