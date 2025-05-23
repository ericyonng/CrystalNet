#[[
    This package version check file is based on the one that ships with CMake,
    but supports version ranges that span major versions, while also doing
    SameMajorVersion matching when a non-range is specified.
]]
set(PACKAGE_VERSION "2.0.0")
set(__package_version_major "2")

# Announce that we are incompatible unless some condition below is satisfied
set(PACKAGE_VERSION_COMPATIBLE FALSE)

if(PACKAGE_VERSION VERSION_LESS PACKAGE_FIND_VERSION)
    # The find_package() requested a version that is at least newer than our own version
    # Not compatible
elseif(NOT DEFINED PACKAGE_FIND_VERSION_RANGE)
    # A simple version check (no range)
    if(PACKAGE_FIND_VERSION_MAJOR EQUAL __package_version_major)
        # We are the same major version. Okay!
        set(PACKAGE_VERSION_COMPATIBLE TRUE)
    endif()
    if(PACKAGE_FIND_VERSION STREQUAL PACKAGE_VERSION)
        # We are an exact match to the requested version
        set(PACKAGE_VERSION_EXACT TRUE)
    endif()
elseif(PACKAGE_VERSION VERSION_GREATER PACKAGE_FIND_VERSION_MAX)
    # Package is newer than the range max (we already checked the range min)
    # Not compatible
else()
    # We are doing a version range check and our version is within the
    # requested inclusive range. Set compatible, but we may be incompatible if we are
    # an exclusive range:
    set(PACKAGE_VERSION_COMPATIBLE TRUE)
    # Check whether the range endpoints are compatible:
    if(PACKAGE_VERSION VERSION_EQUAL PACKAGE_FIND_VERSION_MAX
            AND PACKAGE_FIND_VERSION_RANGE_MAX STREQUAL "EXCLUDE")
        # The upper version is excluded from compatibility
        set(PACKAGE_VERSION_COMPATIBLE FALSE)
    elseif(PACKAGE_VERSION VERSION_EQUAL PACKAGE_FIND_VERSION_MIN
            AND PACKAGE_FIND_VERSION_RANGE_MIN STREQUAL "EXCLUDE")
        # The lower version is excluded from the range
        set(PACKAGE_VERSION_COMPATIBLE FALSE)
    endif()
endif()

# Check if pointer sizes match (this is relevant e.g. if we are a 32-bit build and
# CMake is building for 64-bit)
if(CMAKE_SIZEOF_VOID_P STREQUAL "" OR "8" STREQUAL "")
    # Unknown pointer size. Don't check
    return()
endif()
# Check:
if(NOT CMAKE_SIZEOF_VOID_P STREQUAL "8")
    math(EXPR bits "8 * 8")
    # Announce the bitness of this package for diagnostics:
    set(PACKAGE_VERSION "${PACKAGE_VERSION} (${bits}bit)")
    # This package is unsuitable, regardless of version requested in the importer
    set(PACKAGE_VERSION_UNSUITABLE TRUE)
endif()
