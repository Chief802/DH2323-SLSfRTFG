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
    // Core structural properties
    float baseRadius;
    float radiusDecay;
    float defaultStep;
    float defaultAngleDeg;
    
    // Generic Growth Modifiers (Replaces abop_lr, abop_vr)
    float elongationRatio; 
    float fatteningRatio;  

    // Universal Angular Configurations
    float divergenceAngle1; 
    float divergenceAngle2; 
    float branchAngle1;     
    float branchAngle2;     
    float pitchAngle;       
    float rollAngle;        

    // Universal Dimensions & Scales
    float internodeLen1;
    float internodeLen2;
    float leafSize;
    float flowerSize;
    float budSize;
    float fruitSize;

    // Stochastic Probabilities (0.0 to 1.0)
    float probPrimary;
    float probSecondary;
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
    PlantParams  params,
    const char*  customText
);

} // extern "C"