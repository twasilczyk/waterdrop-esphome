/**
 * Common CXXFLAGS for all C++ files in the project.
 *
 * This is a workaround for the fact that ESPHome doesn't allow setting CXXFLAGS per module / file
 * and enabling these warnings would trip too manu ESPHome source files.
 */

#ifdef _CXXFLAGS_INC
#error "cxxflags.inc should be inclided exactly once, as the first include in a .cpp file"
#endif
#define _CXXFLAGS_INC

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wpedantic"
#pragma GCC diagnostic error "-Whardened"
#pragma GCC diagnostic ignored "-Wc++11-compat"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#pragma GCC diagnostic error "-Wshadow"
#pragma GCC diagnostic error "-Wdouble-promotion"
#pragma GCC diagnostic error "-Wformat=2"
#pragma GCC diagnostic error "-Wnull-dereference"
#pragma GCC diagnostic error "-Woverloaded-virtual"
#pragma GCC diagnostic error "-Wduplicated-cond"
#pragma GCC diagnostic error "-Wduplicated-branches"
#pragma GCC diagnostic error "-Wlogical-op"
#pragma GCC diagnostic error "-Wcast-qual"
#pragma GCC diagnostic error "-Wmissing-declarations"
#pragma GCC diagnostic error "-Wcast-align"

/* TODO: consider enabling these if we can avoid tripping ESPHome includes:
 * -Wundef
 * -Wsign-conversion
 * -Wuseless-cast
 * -Wold-style-cast
 */