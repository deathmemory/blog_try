#pragma once

#define HSMAIN_PROGRAM_NAMEA				"hearthstone.exe"
#define HSMAIN_PROGRAM_NAMEW				L"hearthstone.exe"

#ifdef _UNICODE
#	define HSMAIN_PROGRAM_NAME				HSMAIN_PROGRAM_NAMEW
#else
#	define HSMAIN_PROGRAM_NAME				HSMAIN_PROGRAM_NAMEA
#endif
