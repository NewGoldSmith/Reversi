#pragma once
#include "pch.h"
#include < immintrin.h >
#include <cstdint>
#include <emmintrin.h>
#include <iostream>
#include <xmmintrin.h>
#include "../BitManip/bit_manip.h"
#include "../Debug_fnc/debug_fnc.h"
#include "../BitManip/bit_manip_gpl.h"
#include "../BitManip/bit_manip_okuhara.h"
#include "../BitManip/bit_manip_syuzuk81.h"
#include "../Engine/Engine8.h"

#pragma comment(lib,  "../BitManip/" _STRINGIZE($CONFIGURATION) "/BitManip-" _STRINGIZE($CONFIGURATION) ".lib")
#pragma comment(lib,  "../Debug_fnc/" _STRINGIZE($CONFIGURATION) "/Debug_fnc-" _STRINGIZE($CONFIGURATION) ".lib")
#pragma comment(lib,  "../Engine/" _STRINGIZE($CONFIGURATION) "/Engine-" _STRINGIZE($CONFIGURATION) ".lib")
