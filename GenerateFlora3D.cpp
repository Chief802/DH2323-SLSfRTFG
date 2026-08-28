// =============================================================
//  GenerateFlora3D.cpp  –  C API implementation (Stable Topology)
// =============================================================
#define PLANTSIM_EXPORTS
#include "GenerateFlora3D.h"
#include <iostream>

// =============================================================
// Capsella bursa-pastoris Implementation
// =============================================================
static int BuildCapsellaBursaPastoris(int iters, PlantNode *out, int maxNodes, unsigned int seed)
{
    LSystem sys(seed);

    // p1: Vegetative apex growth producing basal rosette leaves
    sys.AddRule(ProductionRule{
        'a', 1.0f,
        [](const std::vector<float> &p) { return !p.empty() && p[0] > 0; },
        [](const std::vector<float> &p) {
            float t = p[0];
            return Sentence{
                Symbol('['), Symbol('&', {70.0f}), Symbol('L'), Symbol(']'),
                Symbol('/', {137.5f}),
                Symbol('I', {10.0f}),
                Symbol('a', {t - 1.0f})
            };
        }
    });

    // p2: Transition from vegetative apex to floral apex
    sys.AddRule(ProductionRule{
        'a', 1.0f,
        [](const std::vector<float> &p) { return p.empty() || p[0] <= 0; },
        [](const std::vector<float> &) {
            return Sentence{
                Symbol('['), Symbol('&', {70.0f}), Symbol('L'), Symbol(']'),
                Symbol('/', {137.5f}),
                Symbol('I', {10.0f}),
                Symbol('A')
            };
        }
    });

    // p3: Floral apex producing raceme inflorescence with flowers and heart pods
    sys.AddRule(ProductionRule{
        'A', 1.0f, nullptr,
        [](const std::vector<float> &) {
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
                Symbol('A')
            };
        }
    });

    // p4 & p5: Stem internode elongation
    sys.AddRule(ProductionRule{
        'I', 1.0f,
        [](const std::vector<float> &p) { return !p.empty() && p[0] > 0; },
        [](const std::vector<float> &p) {
            float t = p[0];
            return Sentence{ Symbol('F', {0.15f}), Symbol('I', {t - 1.0f}) };
        }
    });
    sys.AddRule(ProductionRule{
        'I', 1.0f,
        [](const std::vector<float> &p) { return p.empty() || p[0] <= 0; },
        [](const std::vector<float> &) {
            return Sentence{ Symbol('F', {0.15f}) };
        }
    });

    // p6 & p7: Pedicel drooping/pitching downward over time
    sys.AddRule(ProductionRule{
        'u', 1.0f,
        [](const std::vector<float> &p) { return !p.empty() && p[0] > 0; },
        [](const std::vector<float> &p) {
            float t = p[0];
            return Sentence{ Symbol('&', {9.0f}), Symbol('u', {t - 1.0f}) };
        }
    });
    sys.AddRule(ProductionRule{
        'u', 1.0f,
        [](const std::vector<float> &p) { return p.empty() || p[0] <= 0; },
        [](const std::vector<float> &) {
            return Sentence{ Symbol('&', {9.0f}) };
        }
    });

    // p8: Leaf rule -> Emits leaf primitive '~'
    sys.AddRule('L', [](const std::vector<float> &) {
        return Sentence{ Symbol('~', {0.5f}) };
    });

    // p9: Petal rule -> 4-petal cross layout emitted as '@' rotated by 90°
    sys.AddRule('K', [](const std::vector<float> &) {
        return Sentence{ Symbol('@', {0.12f}), Symbol('/', {90.0f}) };
    });

    // p10 & p11: Heart-shaped silique maturation -> Emits fruit pod primitive 'x'
    sys.AddRule(ProductionRule{
        'X', 1.0f,
        [](const std::vector<float> &p) { return !p.empty() && p[0] > 0; },
        [](const std::vector<float> &p) {
            float t = p[0];
            return Sentence{ Symbol('X', {t - 1.0f}) };
        }
    });
    sys.AddRule(ProductionRule{
        'X', 1.0f,
        [](const std::vector<float> &p) { return p.empty() || p[0] <= 0; },
        [](const std::vector<float> &) {
            return Sentence{ Symbol('x', {0.22f}) };
        }
    });

    // Axiom: Initial internode I(9) + Vegetative growing tip a(13)
    Sentence axiom{
        Symbol('!', {0.15f}),
        Symbol('I', {9.0f}),
        Symbol('a', {13.0f})
    };

    Sentence result = sys.Generate(axiom, iters);

    std::cout << "[CapsellaBursaPastoris] iter=" << iters << " nodes=" << result.size() << "\n";
    return InterpretFull(result, 0.15f, 25.0f, out, maxNodes);
}

// =============================================================
// Crocus (Crocus sativus) Implementation
// =============================================================
static int BuildCrocus(int iters, PlantNode *out, int maxNodes, unsigned int seed)
{
    LSystem sys(seed);

    const float Ta = 7.0f; // Developmental switch time
    const float TL = 9.0f; // Leaf growth limit
    const float TK = 5.0f; // Flower growth limit

    // p1: Vegetative growth generating leaves spiraled by the golden angle
    // a(t) : t < Ta -> F(1) [ &(30) L(0) ] / (137.5) a(t+1)
    sys.AddRule(ProductionRule{
        'a', 1.0f,
        [Ta](const std::vector<float> &p) { return !p.empty() && p[0] < Ta; },
        [](const std::vector<float> &p) {
            float t = p[0];
            return Sentence{
                Symbol('F', {1.0f}),
                Symbol('['), Symbol('&', {30.0f}), Symbol('L', {0.0f}), Symbol(']'),
                Symbol('/', {137.5f}),
                Symbol('a', {t + 1.0f})
            };
        }
    });

    // p2: Transition to flowering apex
    // a(t) : t >= Ta -> F(20) A
    sys.AddRule(ProductionRule{
        'a', 1.0f,
        [Ta](const std::vector<float> &p) { return !p.empty() && p[0] >= Ta; },
        [](const std::vector<float> &) {
            return Sentence{
                Symbol('F', {20.0f}),
                Symbol('A')
            };
        }
    });

    // p3: Flower apex blooming
    // A : * -> K(0)
    sys.AddRule(ProductionRule{
        'A', 1.0f, nullptr,
        [](const std::vector<float> &) { return Sentence{ Symbol('K', {0.0f}) }; }
    });

    // p4: Leaf aging
    // L(t) : t < TL -> L(t+1)
    sys.AddRule(ProductionRule{
        'L', 1.0f,
        [TL](const std::vector<float> &p) { return !p.empty() && p[0] < TL; },
        [](const std::vector<float> &p) { return Sentence{ Symbol('L', {p[0] + 1.0f}) }; }
    });

    // p5: Flower aging
    // K(t) : t < TK -> K(t+1)
    sys.AddRule(ProductionRule{
        'K', 1.0f,
        [TK](const std::vector<float> &p) { return !p.empty() && p[0] < TK; },
        [](const std::vector<float> &p) { return Sentence{ Symbol('K', {p[0] + 1.0f}) }; }
    });

    // p6: Stem elongation
    // F(l) : l < 2 -> F(l+0.2)
    sys.AddRule(ProductionRule{
        'F', 1.0f,
        [](const std::vector<float> &p) { return !p.empty() && p[0] < 2.0f; },
        [](const std::vector<float> &p) { return Sentence{ Symbol('F', {p[0] + 0.2f}) }; }
    });

    // Axiom: w : a(1)
    Sentence axiom{ Symbol('a', {1.0f}) };

    Sentence result = sys.Generate(axiom, iters);
    std::cout << "[Crocus] iter=" << iters << " nodes=" << result.size() << "\n";
    return InterpretFull(result, 1.0f, 25.0f, out, maxNodes);
}

// =============================================================
//  Exported C API
// =============================================================
extern "C"
{

    PLANTSIM_API int GeneratePlant(
        int exampleId, int iterations,
        PlantNode *outNodes, int maxNodes, unsigned int seed)
    {
        switch (exampleId)
        {
        case 0: return BuildCapsellaBursaPastoris(iterations, outNodes, maxNodes, seed);
        case 1: return BuildCrocus(iterations, outNodes, maxNodes, seed);
        default:
            std::cerr << "[GeneratePlant] Unknown exampleId: " << exampleId << "\n";
            return 0;
        }
    }
}