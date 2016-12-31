# - Try to find the VST library.
#
# The following are set after configuration is done:
#  VST_INCLUDE_DIR

include(FindPackageHandleStandardArgs)

message("Looking for Steinberg VST SDK in ${VST_ROOT_DIR}")

set(VST_INCLUDE_DIR "${VST_ROOT_DIR}/pluginterfaces/vst2.x")