// =============================================================
//  GenerateFlora3D.h  –  Public C API
//
//  Single exported entry point: GeneratePlant().
//  Returns a flat array of PlantNode values that the caller
//  (e.g. the Unity bridge) can partition by NodeType and build
//  separate meshes for branches, leaves, and flowers.
// =============================================================
#pragma once

#include "LSystem.h"   // Vec3, Segment, PlantNode, NodeType, and the C++ engine

#if defined(_WIN32) || defined(_WIN64)
#  ifdef PLANTSIM_EXPORTS
#    define PLANTSIM_API __declspec(dllexport)
#  else
#    define PLANTSIM_API __declspec(dllimport)
#  endif
#else
#  define PLANTSIM_API __attribute__((visibility("default")))
#endif

extern "C" {

/**
 *
 * @param exampleId   0–3 (see above)
 * @param iterations  Number of L-System derivation steps
 * @param outNodes    Caller-allocated output buffer
 * @param maxNodes    Capacity of outNodes
 * @param seed        RNG seed (affects stochastic examples; ignored for example 0)
 * @return            Number of PlantNode values written, or 0 on error.
 */
PLANTSIM_API int GeneratePlant(
    int          exampleId,
    int          iterations,
    PlantNode*   outNodes,
    int          maxNodes,
    unsigned int seed
);

} // extern "C"