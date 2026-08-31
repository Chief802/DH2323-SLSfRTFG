// =============================================================
//  GenerateFlora3D.h  –  Public C API
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
 * Plant generation parameters passed directly from Unity.
 */
struct PlantParams {
    float baseRadius;       // Base radius/width of main trunk
    float radiusDecay;      // Scaling factor applied to radius at branches
    float defaultStep;      // Default length per segment
    float defaultAngleDeg;  // Default branching angle in degrees
    
    // --- ABOP Tree Parameters ---
    float abop_d1; // Divergence angle 1
    float abop_d2; // Divergence angle 2
    float abop_a;  // Branching angle
    float abop_lr; // Length elongation ratio
    float abop_vr; // Radius fattening ratio
};

/**
 * @param exampleId   0 = Capsella, 1 = Stochastic Capsella, 2 = Crocus
 * @param iterations  Number of L-System derivation steps
 * @param outNodes    Caller-allocated output buffer
 * @param maxNodes    Capacity of outNodes
 * @param seed        RNG seed
 * @param params      Configurable structural parameters
 * @return            Number of PlantNode values written, or 0 on error.
 */
PLANTSIM_API int GeneratePlant(
    int          exampleId,
    int          iterations,
    PlantNode*   outNodes,
    int          maxNodes,
    unsigned int seed,
    PlantParams  params
);

} // extern "C"