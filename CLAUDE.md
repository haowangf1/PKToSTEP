# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

PKToSTEP is a Parasolid PK_BODY to STEP file converter. It converts Parasolid geometric models to STEP format through an intermediate Xchg (AMCAXExchangeBase) format.

**Conversion Chain**:
```
PK_BODY → Xchg Format → STEP File
         (this project)  (future work)
```

**Current Implementation**: PK_BODY → Xchg conversion with topology comparison against reference STEP→Xchg→PK roundtrip.

## Build System

### Quick Build (Debug)

Use the `/build-debug` skill or run manually:

```bash
# Check dependencies and install if needed
if [ ! -f "build/Debug/conan_toolchain.cmake" ]; then
    conan install . -of=build/Debug -s build_type=Debug --build=missing
fi

# Configure and build
cmake -S . -B build/Debug -G "Visual Studio 17 2022" \
    -DCMAKE_TOOLCHAIN_FILE=build/Debug/conan_toolchain.cmake \
    -DCMAKE_POLICY_DEFAULT_CMP0091=NEW
cmake --build build/Debug --config Debug
```

**Output**: `build/Debug/Debug/PKToSTEP.exe`

### Dependencies (Conan)

- `amcax-exchange-base/0.0.1.121` - Xchg format library
- `pskernel/36.0` - Parasolid kernel
- `amcax-amxt_stp/0.0.1.39` - STEP reader
- `gtest/1.14.0` - Testing framework

**Important**: DLLs must be copied to `build/Debug/Debug/` after first build:
- `AMCAXExchangeBase.dll`
- `AMXT_STP.dll`
- `pskernel.dll`

The `/build-debug` skill handles this automatically.

## Architecture

### Core Converter: `PKToXchgConverter`

**Location**: `include/pk_to_xchg_converter.hpp`, `src/pk_to_xchg_converter.cpp`

Converts Parasolid topology and geometry to Xchg format through hierarchical traversal:

```
PK_BODY
  └─ PK_REGION (solid/void)
      └─ PK_SHELL
          └─ PK_FACE
              └─ PK_LOOP (outer/inner)
                  └─ PK_FIN (coedge)
                      └─ PK_EDGE
                          └─ PK_VERTEX
```

**Key Methods**:
- `Convert()` - Entry point, converts PK_BODY to Xchg_Body
- `ConvertRegions()` - Handles solid/void regions, determines shell types
- `ConvertShell()` - Converts shell with face orientation calculation
- `ConvertFace()` - Converts face, passes orientation to loops
- `ConvertLoop()` - Converts loop, sets orientation based on face
- `ConvertFin()` - Converts fin (coedge) to Xchg_Coedge
- `ConvertEdge()` - Converts edge with deduplication
- `ConvertVertex()` - Converts vertex with deduplication

### Critical Orientation Logic

**Face Orientation** (in `ConvertShell`):
```cpp
// BackFaceShell (solid region): extra reversal needed
xchg_orient = !(orients[i] ^ face_orient);

// FrontFaceShell (void region): original logic
xchg_orient = (orients[i] != face_orient);
```

**Loop Orientation** (in `ConvertLoop`):
```cpp
// Loop orientation matches face orientation in shell
xchg_loop->SetOrientation(face_orientation ? XCHG_TRUE : XCHG_FALSE);
```

**Coedge inLoop** (in `ConvertLoop`):
```cpp
// Simplified to XCHG_TRUE to ensure GetOrderedCoedges succeeds
// Complex XOR logic caused -40 errors in X→PK roundtrip
xchg_loop->AddCoedge(xchg_coedge, XCHG_TRUE);
```

### Main Program Flow

**Location**: `src/main.cpp`

1. Read STEP file → Xchg_Body (reference)
2. Convert Xchg_Body → PK_BODY
3. Convert PK_BODY → Xchg_Body (our implementation)
4. Compare topologies using `XchgTopoCompare::Compare()`
5. Roundtrip test: Xchg_Body → PK_BODY → verify

### Topology Comparison

**Location**: `include/xchg_topo_compare.hpp`

Compares two Xchg_Body instances and reports differences in:
- Face shellOrient (face direction in shell)
- Loop orientation
- Coedge inLoop flags
- Geometry parameters (surfaces, curves, vertices)

## Key Concepts

### Xchg Format Orientation Rules

**From X→PK conversion** (`exchange_base/src/parasolid/xchg_body_convertto_pk.cpp`):

```cpp
// GetOrderedCoedges uses this to determine edge direction:
finalOrientation = !(inLoop ^ coedgeOrientation);

// If finalOrientation is wrong, coedges cannot be ordered into
// a head-to-tail chain, causing error -40 (XCHG_ERROR_UNDEFINED)
```

**Critical**: The `inLoop` parameter in `AddCoedge()` must be set correctly to ensure `GetOrderedCoedges` can arrange coedges into a closed loop during X→PK conversion.

### Region Types and Shell Classification

- **SolidRegion**: Material region → BackFaceShell (faces point inward)
- **InfVoidRegion**: Infinite void → FrontFaceShell (faces point outward)
- **VoidRegion**: Cavity → requires special handling

### Deduplication

Edges and Vertices are deduplicated using `pk_edge_map_` and `pk_vertex_map_` to ensure shared topology entities are reused.

## Documentation

- `docs/step_to_x_to_pk_process.md` - Detailed analysis of STEP↔Xchg↔PK conversion rules
- `docs/implementation_issues.md` - Known issues and solutions in PK→Xchg conversion

## Testing

Current test: Roundtrip validation comparing STEP→Xchg→PK→Xchg against reference.

Test files in `resource/`:
- `cube214.step` - Simple cube (6 faces)
- `hollow_cube.step` - Cube with cavity (30 faces, inner loops)
- `cylinder214.step` - Cylindrical geometry

**Run test**:
```bash
cd build/Debug/Debug
./PKToSTEP.exe ../../../resource/cube214.step
```

## Common Issues

### Error -40: GetOrderedCoedges Failed

**Cause**: Incorrect `inLoop` parameter in `AddCoedge()` causes coedges to have wrong start/end vertex order.

**Solution**: Use `XCHG_TRUE` for `inLoop` parameter (simplified approach that works).

### Missing DLLs

**Symptom**: "cannot open shared object file" errors

**Solution**: Copy DLLs from Conan cache to `build/Debug/Debug/`:
```bash
cp ~/.conan2/p/amcax*/p/bin/Debug/*.dll build/Debug/Debug/
cp ~/.conan2/p/psker*/p/bin/*.dll build/Debug/Debug/
```

### Face/Loop Orientation Mismatches

**Cause**: Incorrect orientation calculation in `ConvertShell` or `ConvertLoop`.

**Check**: Compare test output for `shellOrient` and `orient` fields. Both should match between REF and OURS.

## Skills Available

- `/build-debug` - Configure and build Debug version with dependency management
- `/check-pk-api` - Query Parasolid API documentation from internal server
