# Build the embedded AssetBundle from its binary source on every platform.
if(NOT EXISTS "${INPUT}")
    message(FATAL_ERROR "AssetBundle not found: ${INPUT}")
endif()
file(READ "${INPUT}" asset_hex HEX)
string(LENGTH "${asset_hex}" hex_length)
math(EXPR byte_length "${hex_length} / 2")
string(REGEX REPLACE "(..)" "0x\\1," asset_bytes "${asset_hex}")
file(WRITE "${OUTPUT}" "// Generated from assets/vs_assets. Do not edit.\n#include <cstddef>\n#include <cstdint>\nextern const size_t vs_assets_bytes_len = ${byte_length};\nextern const uint8_t vs_assets_bytes[] = {${asset_bytes}};\n")
