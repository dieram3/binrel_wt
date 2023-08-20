# This module wraps doctestConfig.cmake so as to produce a proper
# success/failure message.

include(FindPackageHandleStandardArgs)

find_package(doctest QUIET CONFIG)
find_package_handle_standard_args(doctest CONFIG_MODE)
