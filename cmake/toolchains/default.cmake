if(NOT DEFINED ENV{VCPKG_ROOT})
  message(WARNING "The VCPKG_ROOT environment variable is not defined. "
                  "Falling back to using no package manager.")
  return()
endif()

set(VCPKG_INSTALL_OPTIONS "--no-print-usage")
include("$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
