#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "AMCAXExchangeBase::AMCAXExchangeBase" for configuration "Release"
set_property(TARGET AMCAXExchangeBase::AMCAXExchangeBase APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(AMCAXExchangeBase::AMCAXExchangeBase PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/Release/AMCAXExchangeBase.lib"
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "AMCAXCommon;AMCAXPart"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/Release/AMCAXExchangeBase.dll"
  )

list(APPEND _cmake_import_check_targets AMCAXExchangeBase::AMCAXExchangeBase )
list(APPEND _cmake_import_check_files_for_AMCAXExchangeBase::AMCAXExchangeBase "${_IMPORT_PREFIX}/lib/Release/AMCAXExchangeBase.lib" "${_IMPORT_PREFIX}/bin/Release/AMCAXExchangeBase.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
