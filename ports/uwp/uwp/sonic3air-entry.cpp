#include "SDL.h"

extern "C" int sonic3air_main(int argc, char** argv);

extern "C" int SDL_main(int argc, char** argv)
{
	SDL_SetMainReady();

	if (argc <= 0 || nullptr == argv || nullptr == argv[0])
	{
		static char appName[] = "sonic3air-uwp";
		static char* fallbackArgv[] = { appName, nullptr };
		return sonic3air_main(1, fallbackArgv);
	}

	return sonic3air_main(argc, argv);
}
