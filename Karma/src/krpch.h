#pragma once

// Utilities
#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <filesystem>

#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stdint.h>
#include <stdio.h>
#include <list>
#include <utility>
#include <set>
#include <cstdint>
#include <optional>
#include <fstream>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <float.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

// No clue about this ImGui include. May or maynot be excluded in future.
#if defined(_MSC_VER) && _MSC_VER <= 1500 // MSVC 2008 or earlier
#include <stddef.h>
#else
#include <stdint.h>
#endif

// Intel intrinsics?
//#include <immintrin.h>

// Karma's general includes
#include "Karma/Core.h"
#include "Karma/Log.h"
