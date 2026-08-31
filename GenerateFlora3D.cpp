// =============================================================
//  GenerateFlora3D.cpp  –  C API implementation (Stable Topology)
// =============================================================
#define PLANTSIM_EXPORTS
#include "GenerateFlora3D.h"
#include <iostream>

// =============================================================
// Capsella bursa-pastoris Implementation
// =============================================================
static int BuildCapsellaBursaPastoris(int iters, PlantNode *out, int maxNodes, unsigned int seed, const PlantParams &params)
{
    LSystem sys(seed);

    // p1: Vegetative apex growth producing basal rosette leaves
    sys.AddRule(ProductionRule{
        'a', 1.0f,
        [](const std::vector<float> &p)
        { return !p.empty() && p[0] > 0; },
        [](const std::vector<float> &p)
        {
            float t = p[0];
            return Sentence{
                Symbol('['), Symbol('&', {70.0f}), Symbol('L'), Symbol(']'),
                Symbol('/', {137.5f}),
                Symbol('I', {10.0f}),
                Symbol('a', {t - 1.0f})};
        }});

    // p2: Transition from vegetative apex to floral apex
    sys.AddRule(ProductionRule{
        'a', 1.0f,
        [](const std::vector<float> &p)
        { return p.empty() || p[0] <= 0; },
        [](const std::vector<float> &)
        {
            return Sentence{
                Symbol('['), Symbol('&', {70.0f}), Symbol('L'), Symbol(']'),
                Symbol('/', {137.5f}),
                Symbol('I', {10.0f}),
                Symbol('A')};
        }});

    // p3: Floral apex producing raceme inflorescence with flowers and heart pods
    sys.AddRule(ProductionRule{
        'A', 1.0f, nullptr,
        [](const std::vector<float> &)
        {
            return Sentence{
                Symbol('['),
                Symbol('&', {18.0f}),
                Symbol('u', {4.0f}),
                Symbol('F', {0.1f}), Symbol('F', {0.1f}),
                Symbol('I', {10.0f}), Symbol('I', {5.0f}),
                Symbol('X', {5.0f}),
                Symbol('K'), Symbol('K'), Symbol('K'), Symbol('K'),
                Symbol(']'),
                Symbol('/', {137.5f}),
                Symbol('I', {8.0f}),
                Symbol('A')};
        }});

    // p4 & p5: Stem internode elongation
    sys.AddRule(ProductionRule{
        'I', 1.0f,
        [](const std::vector<float> &p)
        { return !p.empty() && p[0] > 0; },
        [](const std::vector<float> &p)
        {
            float t = p[0];
            return Sentence{Symbol('F', {0.15f}), Symbol('I', {t - 1.0f})};
        }});
    sys.AddRule(ProductionRule{
        'I', 1.0f,
        [](const std::vector<float> &p)
        { return p.empty() || p[0] <= 0; },
        [](const std::vector<float> &)
        {
            return Sentence{Symbol('F', {0.15f})};
        }});

    // p6 & p7: Pedicel drooping/pitching downward over time
    sys.AddRule(ProductionRule{
        'u', 1.0f,
        [](const std::vector<float> &p)
        { return !p.empty() && p[0] > 0; },
        [](const std::vector<float> &p)
        {
            float t = p[0];
            return Sentence{Symbol('&', {9.0f}), Symbol('u', {t - 1.0f})};
        }});
    sys.AddRule(ProductionRule{
        'u', 1.0f,
        [](const std::vector<float> &p)
        { return p.empty() || p[0] <= 0; },
        [](const std::vector<float> &)
        {
            return Sentence{Symbol('&', {9.0f})};
        }});

    // p8: Leaf rule -> Emits leaf primitive '~'
    sys.AddRule('L', [](const std::vector<float> &)
                { return Sentence{Symbol('~', {0.5f})}; });

    // p9: Petal rule -> 4-petal cross layout emitted as '@' rotated by 90°
    sys.AddRule('K', [](const std::vector<float> &)
                { return Sentence{Symbol('@', {0.12f}), Symbol('/', {90.0f})}; });

    // p10 & p11: Heart-shaped silique maturation -> Emits fruit pod primitive 'x'
    sys.AddRule(ProductionRule{
        'X', 1.0f,
        [](const std::vector<float> &p)
        { return !p.empty() && p[0] > 0; },
        [](const std::vector<float> &p)
        {
            float t = p[0];
            return Sentence{Symbol('X', {t - 1.0f})};
        }});
    sys.AddRule(ProductionRule{
        'X', 1.0f,
        [](const std::vector<float> &p)
        { return p.empty() || p[0] <= 0; },
        [](const std::vector<float> &)
        {
            return Sentence{Symbol('x', {0.22f})};
        }});

    // Axiom: Initial internode I(9) + Vegetative growing tip a(13)
    Sentence axiom{
        Symbol('!', {0.15f}),
        Symbol('I', {9.0f}),
        Symbol('a', {13.0f})};

    Sentence result = sys.Generate(axiom, iters);

    std::cout << "[CapsellaBursaPastoris] iter=" << iters << " nodes=" << result.size() << "\n";
    return InterpretFull(
        result,
        params.defaultStep,
        params.defaultAngleDeg,
        out,
        maxNodes,
        params.baseRadius,
        params.radiusDecay);
}

// =============================================================
// Stochastic Capsella bursa-pastoris Implementation
// =============================================================
static int BuildStochasticCapsellaBursaPastoris(int iters, PlantNode *out, int maxNodes, unsigned int seed, const PlantParams &params)
{
    LSystem sys(seed);

    // ---------------------------------------------------------
    // 1. Vegetative Apex (a)
    // ---------------------------------------------------------
    // 70% chance: Standard basal rosette leaf generation
    sys.AddRule(ProductionRule{
        'a', 0.70f,
        [](const std::vector<float> &p)
        { return !p.empty() && p[0] > 0; },
        [](const std::vector<float> &p)
        {
            float t = p[0];
            return Sentence{
                Symbol('['), Symbol('&', {70.0f}), Symbol('L'), Symbol(']'),
                Symbol('/', {137.5f}),
                Symbol('I', {10.0f}),
                Symbol('a', {t - 1.0f})};
        }});

    // 30% chance: Irregular spacing and divergence angle
    sys.AddRule(ProductionRule{
        'a', 0.30f,
        [](const std::vector<float> &p)
        { return !p.empty() && p[0] > 0; },
        [](const std::vector<float> &p)
        {
            float t = p[0];
            return Sentence{
                Symbol('['), Symbol('&', {60.0f}), Symbol('L'), Symbol(']'),
                Symbol('/', {145.0f}), // Altered divergence
                Symbol('I', {8.0f}),   // Shorter main branch internode
                Symbol('a', {t - 1.0f})};
        }});

    // Transition to floral apex (Deterministic)
    sys.AddRule(ProductionRule{
        'a', 1.0f,
        [](const std::vector<float> &p)
        { return p.empty() || p[0] <= 0; },
        [](const std::vector<float> &)
        {
            return Sentence{
                Symbol('['), Symbol('&', {70.0f}), Symbol('L'), Symbol(']'),
                Symbol('/', {137.5f}),
                Symbol('I', {10.0f}),
                Symbol('A')};
        }});

    // ---------------------------------------------------------
    // 2. Floral Apex (A) - Raceme Inflorescence
    // ---------------------------------------------------------
    // 60% chance: Standard 4-petal flower with normal branch spacing
    sys.AddRule(ProductionRule{
        'A', 0.60f, nullptr,
        [](const std::vector<float> &)
        {
            return Sentence{
                Symbol('['),
                Symbol('&', {18.0f}),
                Symbol('u', {4.0f}),
                Symbol('F', {0.1f}), Symbol('F', {0.1f}),
                Symbol('I', {10.0f}), Symbol('I', {5.0f}),
                Symbol('X', {5.0f}),
                Symbol('K'), Symbol('K'), Symbol('K'), Symbol('K'),
                Symbol(']'),
                Symbol('/', {137.5f}),
                Symbol('I', {8.0f}),
                Symbol('A')};
        }});

    // 20% chance: 3-petal mutation, stunted pedicel, tight spacing
    sys.AddRule(ProductionRule{
        'A', 0.20f, nullptr,
        [](const std::vector<float> &)
        {
            return Sentence{
                Symbol('['),
                Symbol('&', {22.0f}),
                Symbol('u', {3.0f}),
                Symbol('F', {0.1f}), Symbol('F', {0.05f}),
                Symbol('I', {8.0f}), Symbol('I', {4.0f}),
                Symbol('X', {4.0f}),
                Symbol('K'), Symbol('K'), Symbol('K'),
                Symbol(']'),
                Symbol('/', {120.0f}),
                Symbol('I', {6.0f}), // Tighter offshoot generation
                Symbol('A')};
        }});

    // 20% chance: 5-petal mutation, heavy fruit pod, wide spacing
    sys.AddRule(ProductionRule{
        'A', 0.20f, nullptr,
        [](const std::vector<float> &)
        {
            return Sentence{
                Symbol('['),
                Symbol('&', {15.0f}),
                Symbol('u', {5.0f}),
                Symbol('F', {0.12f}), Symbol('F', {0.12f}),
                Symbol('I', {12.0f}), Symbol('I', {6.0f}),
                Symbol('X', {6.0f}),
                Symbol('K'), Symbol('K'), Symbol('K'), Symbol('K'), Symbol('K'),
                Symbol(']'),
                Symbol('/', {150.0f}),
                Symbol('I', {10.0f}), // Wider offshoot generation
                Symbol('A')};
        }});

    // ---------------------------------------------------------
    // 3. Stem Internode Elongation (I)
    // ---------------------------------------------------------
    // 80% chance for standard elongation, 20% chance for stunted growth
    sys.AddRule(ProductionRule{
        'I', 0.80f,
        [](const std::vector<float> &p)
        { return !p.empty() && p[0] > 0; },
        [](const std::vector<float> &p)
        { return Sentence{Symbol('F', {0.15f}), Symbol('I', {p[0] - 1.0f})}; }});
    sys.AddRule(ProductionRule{
        'I', 0.20f,
        [](const std::vector<float> &p)
        { return !p.empty() && p[0] > 0; },
        [](const std::vector<float> &p)
        { return Sentence{Symbol('F', {0.10f}), Symbol('I', {p[0] - 1.0f})}; }});

    sys.AddRule(ProductionRule{
        'I', 0.80f,
        [](const std::vector<float> &p)
        { return p.empty() || p[0] <= 0; },
        [](const std::vector<float> &)
        { return Sentence{Symbol('F', {0.15f})}; }});
    sys.AddRule(ProductionRule{
        'I', 0.20f,
        [](const std::vector<float> &p)
        { return p.empty() || p[0] <= 0; },
        [](const std::vector<float> &)
        { return Sentence{Symbol('F', {0.10f})}; }});

    // ---------------------------------------------------------
    // 4. Deterministic Terminals (Pedicels, Leaves, Petals, Fruit)
    // ---------------------------------------------------------
    sys.AddRule(ProductionRule{
        'u', 1.0f,
        [](const std::vector<float> &p)
        { return !p.empty() && p[0] > 0; },
        [](const std::vector<float> &p)
        { return Sentence{Symbol('&', {9.0f}), Symbol('u', {p[0] - 1.0f})}; }});
    sys.AddRule(ProductionRule{
        'u', 1.0f,
        [](const std::vector<float> &p)
        { return p.empty() || p[0] <= 0; },
        [](const std::vector<float> &)
        { return Sentence{Symbol('&', {9.0f})}; }});

    sys.AddRule('L', [](const std::vector<float> &)
                { return Sentence{Symbol('~', {0.5f})}; });

    sys.AddRule('K', [](const std::vector<float> &)
                { return Sentence{Symbol('@', {0.12f}), Symbol('/', {90.0f})}; });

    sys.AddRule(ProductionRule{
        'X', 1.0f,
        [](const std::vector<float> &p)
        { return !p.empty() && p[0] > 0; },
        [](const std::vector<float> &p)
        { return Sentence{Symbol('X', {p[0] - 1.0f})}; }});
    sys.AddRule(ProductionRule{
        'X', 1.0f,
        [](const std::vector<float> &p)
        { return p.empty() || p[0] <= 0; },
        [](const std::vector<float> &)
        { return Sentence{Symbol('x', {0.22f})}; }});

    // ---------------------------------------------------------
    // Axiom & Generation
    // ---------------------------------------------------------
    Sentence axiom{
        Symbol('!', {params.baseRadius}),
        Symbol('I', {9.0f}),
        Symbol('a', {13.0f})};

    Sentence result = sys.Generate(axiom, iters);

    std::cout << "[CapsellaBursaPastoris/Stochastic Fixed] iter=" << iters
              << " nodes=" << result.size()
              << " seed=" << seed << "\n";

    // Rely on base default parameters, falling back to unity struct if absent
    return InterpretFull(result, params.defaultStep, params.defaultAngleDeg, out, maxNodes, params.baseRadius, params.radiusDecay);
}

// =============================================================
// Crocus (Crocus sativus) Implementation
// =============================================================
static int BuildCrocus(int iters, PlantNode *out, int maxNodes, unsigned int seed, const PlantParams &params)
{
    LSystem sys(seed);

    const float Ta = 7.0f; // Developmental switch time
    const float TL = 9.0f; // Leaf growth limit
    const float TK = 5.0f; // Flower growth limit

    // p1: Vegetative growth generating leaves spiraled by the golden angle
    // a(t) : t < Ta -> F(1) [ &(30) L(0) ] / (137.5) a(t+1)
    sys.AddRule(ProductionRule{
        'a', 1.0f,
        [Ta](const std::vector<float> &p)
        { return !p.empty() && p[0] < Ta; },
        [](const std::vector<float> &p)
        {
            float t = p[0];
            return Sentence{
                Symbol('F', {1.0f}),
                Symbol('['), Symbol('&', {30.0f}), Symbol('L', {0.0f}), Symbol(']'),
                Symbol('/', {137.5f}),
                Symbol('a', {t + 1.0f})};
        }});

    // p2: Transition to flowering apex
    // a(t) : t >= Ta -> F(20) A
    sys.AddRule(ProductionRule{
        'a', 1.0f,
        [Ta](const std::vector<float> &p)
        { return !p.empty() && p[0] >= Ta; },
        [](const std::vector<float> &)
        {
            return Sentence{
                Symbol('F', {20.0f}),
                Symbol('A')};
        }});

    // p3: Flower apex blooming
    // A : * -> K(0)
    sys.AddRule(ProductionRule{
        'A', 1.0f, nullptr,
        [](const std::vector<float> &)
        { return Sentence{Symbol('K', {0.0f})}; }});

    // p4: Leaf aging
    // L(t) : t < TL -> L(t+1)
    sys.AddRule(ProductionRule{
        'L', 1.0f,
        [TL](const std::vector<float> &p)
        { return !p.empty() && p[0] < TL; },
        [](const std::vector<float> &p)
        { return Sentence{Symbol('L', {p[0] + 1.0f})}; }});

    // p5: Flower aging
    // K(t) : t < TK -> K(t+1)
    sys.AddRule(ProductionRule{
        'K', 1.0f,
        [TK](const std::vector<float> &p)
        { return !p.empty() && p[0] < TK; },
        [](const std::vector<float> &p)
        { return Sentence{Symbol('K', {p[0] + 1.0f})}; }});

    // p6: Stem elongation
    // F(l) : l < 2 -> F(l+0.2)
    sys.AddRule(ProductionRule{
        'F', 1.0f,
        [](const std::vector<float> &p)
        { return !p.empty() && p[0] < 2.0f; },
        [](const std::vector<float> &p)
        { return Sentence{Symbol('F', {p[0] + 0.2f})}; }});

    // Axiom: w : a(1)
    Sentence axiom{Symbol('a', {1.0f})};

    Sentence result = sys.Generate(axiom, iters);
    std::cout << "[Crocus] iter=" << iters << " nodes=" << result.size() << "\n";

    return InterpretFull(
        result,
        params.defaultStep,
        params.defaultAngleDeg,
        out,
        maxNodes,
        params.baseRadius,
        params.radiusDecay);
}

// =============================================================
// ABOP Sympodial Tree
// =============================================================
static int BuildABOPTree(int iters, PlantNode *out, int maxNodes, unsigned int seed, const PlantParams &params)
{
    // Pull the parameters from the struct instead of using constexpr
    float d1 = params.abop_d1; // Default was 94.74f
    float d2 = params.abop_d2; // Default was 132.63f
    float a = params.abop_a;   // Default was 18.95f
    float lr = params.abop_lr; // Default was 1.109f
    float vr = params.abop_vr; // Default was 1.732f

    LSystem sys(seed);

    // p1 – Apex expansion with leaves and a single apical flower
    // MUST capture a, d1, d2, and vr in the lambda capture block [...]
    sys.AddRule('A', [a, d1, d2, vr](const std::vector<float> &)
                { return Sentence{
                      Symbol('!', {vr}),
                      Symbol('F', {50.f}),

                      Symbol('['),
                      Symbol('&', {a}),
                      Symbol('F', {50.f}),
                      Symbol('A'),
                      Symbol('~', {2.0f}), // leaf near this arm's apex
                      Symbol(']'),

                      Symbol('/', {d1}),

                      Symbol('['),
                      Symbol('&', {a}),
                      Symbol('F', {50.f}),
                      Symbol('A'),
                      Symbol('~', {2.0f}),
                      Symbol(']'),

                      Symbol('/', {d2}),

                      Symbol('['),
                      Symbol('&', {a}),
                      Symbol('F', {50.f}),
                      Symbol('A'),
                      Symbol('~', {2.0f}),
                      Symbol(']'),

                      Symbol('@', {1.5f}), // flower at the meristem apex
                  }; });

    // p2 – Segment elongation
    sys.AddRule(ProductionRule{
        'F', 1.0f, nullptr,
        [lr](const std::vector<float> &p) { // Capture lr
            float l = p.empty() ? 1.f : p[0];
            return Sentence{Symbol('F', {l * lr})};
        }});

    // p3 – Radius fattening (pipe model)
    sys.AddRule(ProductionRule{
        '!', 1.0f, nullptr,
        [vr](const std::vector<float> &p) { // Capture vr
            float w = p.empty() ? 1.f : p[0];
            return Sentence{Symbol('!', {w * vr})};
        }});

    Sentence axiom{
        Symbol('!', {1.f}),
        Symbol('F', {200.f}),
        Symbol('/', {45.f}),
        Symbol('A'),
    };

    Sentence result = sys.Generate(axiom, iters);

    std::cout << "[ABOPTree]  iter=" << iters
              << "  seed=" << seed
              << "  nodes=" << result.size() << "\n";

    return InterpretFull(
        result,
        params.defaultStep,
        params.defaultAngleDeg,
        out,
        maxNodes,
        params.baseRadius,
        params.radiusDecay);
}

// =============================================================
//  Exported C API
// =============================================================
extern "C"
{

    PLANTSIM_API int GeneratePlant(
        int exampleId, int iterations,
        PlantNode *outNodes, int maxNodes, unsigned int seed,
        PlantParams params)
    {
        switch (exampleId)
        {
        case 0:
            return BuildCapsellaBursaPastoris(iterations, outNodes, maxNodes, seed, params);
        case 1:
            return BuildStochasticCapsellaBursaPastoris(iterations, outNodes, maxNodes, seed, params);
        case 2:
            return BuildCrocus(iterations, outNodes, maxNodes, seed, params);
        case 3:
            return BuildABOPTree(iterations, outNodes, maxNodes, seed, params);
        default:
            std::cerr << "[GeneratePlant] Unknown exampleId: " << exampleId << "\n";
            return 0;
        }
    }
}