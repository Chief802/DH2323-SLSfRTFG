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

    // p1: Vegetative apex growth
    sys.AddRule(ProductionRule{
        'a', 1.0f,
        [](const std::vector<float> &p) { return !p.empty() && p[0] > 0; },
        [params](const std::vector<float> &p) {
            return Sentence{
                Symbol('['), Symbol('&', {params.branchAngle1}), Symbol('L'), Symbol(']'),
                Symbol('/', {params.divergenceAngle1}),
                Symbol('I', {params.internodeLen1}),
                Symbol('a', {p[0] - 1.0f})};
        }});

    // p2: Transition from vegetative to floral
    sys.AddRule(ProductionRule{
        'a', 1.0f,
        [](const std::vector<float> &p) { return p.empty() || p[0] <= 0; },
        [params](const std::vector<float> &) {
            return Sentence{
                Symbol('['), Symbol('&', {params.branchAngle1}), Symbol('L'), Symbol(']'),
                Symbol('/', {params.divergenceAngle1}),
                Symbol('I', {params.internodeLen1}),
                Symbol('A')};
        }});

    // p3: Floral apex producing raceme inflorescence
    sys.AddRule(ProductionRule{
        'A', 1.0f, nullptr,
        [params](const std::vector<float> &) {
            return Sentence{
                Symbol('['),
                Symbol('&', {params.branchAngle2}),
                Symbol('u', {4.0f}),
                Symbol('F', {params.defaultStep}), Symbol('F', {params.defaultStep}),
                Symbol('I', {params.internodeLen1}), Symbol('I', {params.internodeLen2}),
                Symbol('X', {5.0f}),
                Symbol(']'),
                Symbol('/', {params.divergenceAngle1}),
                Symbol('I', {params.internodeLen2}),
                Symbol('A')};
        }});

    // p4 & p5: Stem internode elongation
    sys.AddRule(ProductionRule{
        'I', 1.0f,
        [](const std::vector<float> &p) { return !p.empty() && p[0] > 0; },
        [params](const std::vector<float> &p) {
            return Sentence{Symbol('F', {params.elongationRatio}), Symbol('I', {p[0] - 1.0f})};
        }});
    sys.AddRule(ProductionRule{
        'I', 1.0f,
        [](const std::vector<float> &p) { return p.empty() || p[0] <= 0; },
        [params](const std::vector<float> &) {
            return Sentence{Symbol('F', {params.elongationRatio})};
        }});

    // p6 & p7: Pedicel drooping/pitching downward
    sys.AddRule(ProductionRule{
        'u', 1.0f,
        [](const std::vector<float> &p) { return !p.empty() && p[0] > 0; },
        [params](const std::vector<float> &p) {
            return Sentence{Symbol('&', {params.pitchAngle}), Symbol('u', {p[0] - 1.0f})};
        }});
    sys.AddRule(ProductionRule{
        'u', 1.0f,
        [](const std::vector<float> &p) { return p.empty() || p[0] <= 0; },
        [params](const std::vector<float> &) {
            return Sentence{Symbol('&', {params.pitchAngle})};
        }});

    // Morphological rules
    sys.AddRule('L', [params](const std::vector<float> &) { return Sentence{Symbol('~', {params.leafSize})}; });
    sys.AddRule('K', [params](const std::vector<float> &) { return Sentence{Symbol('@', {params.flowerSize}), Symbol('/', {90.0f})}; });
    sys.AddRule('b', [params](const std::vector<float> &p) { return Sentence{Symbol('b', {p.empty() ? params.budSize : p[0]})}; });
    sys.AddRule('o', [params](const std::vector<float> &p) { return Sentence{Symbol('o', {p.empty() ? (params.budSize * 1.2f) : p[0]})}; });

    // p10 & p11: Maturation
    sys.AddRule(ProductionRule{ 'X', 1.0f, [](const std::vector<float> &p) { return !p.empty() && p[0] > 0; }, [](const std::vector<float> &p) { return Sentence{Symbol('X', {p[0] - 1.0f})}; }});
    sys.AddRule(ProductionRule{ 'X', 1.0f, [](const std::vector<float> &p) { return p.empty() || p[0] <= 0; }, [](const std::vector<float> &) { return Sentence{Symbol('X', {0.0f})}; }});

    Sentence axiom{ Symbol('!', {params.baseRadius}), Symbol('I', {9.0f}), Symbol('a', {13.0f}) };
    Sentence result = sys.Generate(axiom, iters);
    return InterpretFull(result, params.defaultStep, params.defaultAngleDeg, out, maxNodes, params.baseRadius, params.radiusDecay);
}

// =============================================================
// Stochastic Capsella bursa-pastoris Implementation
// =============================================================
static int BuildStochasticCapsellaBursaPastoris(int iters, PlantNode *out, int maxNodes, unsigned int seed, const PlantParams &params)
{
    LSystem sys(seed);

    // 1. Vegetative Apex (a)
    sys.AddRule(ProductionRule{
        'a', params.probPrimary,
        [](const std::vector<float> &p) { return !p.empty() && p[0] > 0; },
        [params](const std::vector<float> &p) {
            return Sentence{
                Symbol('['), Symbol('&', {params.branchAngle1}), Symbol('L'), Symbol(']'),
                Symbol('/', {params.divergenceAngle1}),
                Symbol('I', {params.internodeLen1}),
                Symbol('a', {p[0] - 1.0f})};
        }});

    sys.AddRule(ProductionRule{
        'a', params.probSecondary,
        [](const std::vector<float> &p) { return !p.empty() && p[0] > 0; },
        [params](const std::vector<float> &p) {
            return Sentence{
                Symbol('['), Symbol('&', {params.branchAngle2}), Symbol('L'), Symbol(']'),
                Symbol('/', {params.divergenceAngle2}),
                Symbol('I', {params.internodeLen2}),
                Symbol('a', {p[0] - 1.0f})};
        }});

    sys.AddRule(ProductionRule{
        'a', 1.0f,
        [](const std::vector<float> &p) { return p.empty() || p[0] <= 0; },
        [params](const std::vector<float> &) {
            return Sentence{
                Symbol('['), Symbol('&', {params.branchAngle1}), Symbol('L'), Symbol(']'),
                Symbol('/', {params.divergenceAngle1}),
                Symbol('I', {params.internodeLen1}),
                Symbol('A')};
        }});

    // 2. Floral Apex (A)
    sys.AddRule(ProductionRule{
        'A', params.probPrimary, nullptr,
        [params](const std::vector<float> &) {
            return Sentence{
                Symbol('['), Symbol('&', {params.branchAngle2}), Symbol('u', {4.0f}),
                Symbol('F', {params.defaultStep}), Symbol('F', {params.defaultStep}),
                Symbol('I', {params.internodeLen1}), Symbol('I', {params.internodeLen2}),
                Symbol('X', {5.0f}),
                Symbol(']'),
                Symbol('/', {params.divergenceAngle1}),
                Symbol('I', {params.internodeLen2}), Symbol('A')};
        }});

    sys.AddRule(ProductionRule{
        'A', params.probSecondary, nullptr,
        [params](const std::vector<float> &) {
            return Sentence{
                Symbol('['), Symbol('&', {params.branchAngle1 * 0.8f}), Symbol('u', {3.0f}),
                Symbol('F', {params.defaultStep}), Symbol('F', {params.defaultStep * 0.5f}),
                Symbol('I', {params.internodeLen2}), Symbol('I', {params.internodeLen2 * 0.5f}),
                Symbol('X', {4.0f}),
                Symbol(']'),
                Symbol('/', {params.divergenceAngle2}),
                Symbol('I', {params.internodeLen1 * 0.8f}), Symbol('A')};
        }});

    // 3. Stem Internode Elongation (I)
    sys.AddRule(ProductionRule{ 'I', params.probPrimary, [](const std::vector<float> &p) { return !p.empty() && p[0] > 0; }, [params](const std::vector<float> &p) { return Sentence{Symbol('F', {params.elongationRatio}), Symbol('I', {p[0] - 1.0f})}; }});
    sys.AddRule(ProductionRule{ 'I', params.probSecondary, [](const std::vector<float> &p) { return !p.empty() && p[0] > 0; }, [params](const std::vector<float> &p) { return Sentence{Symbol('F', {params.elongationRatio * 0.8f}), Symbol('I', {p[0] - 1.0f})}; }});
    sys.AddRule(ProductionRule{ 'I', 1.0f, [](const std::vector<float> &p) { return p.empty() || p[0] <= 0; }, [params](const std::vector<float> &) { return Sentence{Symbol('F', {params.elongationRatio})}; }});

    // Terminals
    sys.AddRule(ProductionRule{ 'u', 1.0f, [](const std::vector<float> &p) { return !p.empty() && p[0] > 0; }, [params](const std::vector<float> &p) { return Sentence{Symbol('&', {params.pitchAngle}), Symbol('u', {p[0] - 1.0f})}; }});
    sys.AddRule(ProductionRule{ 'u', 1.0f, [](const std::vector<float> &p) { return p.empty() || p[0] <= 0; }, [params](const std::vector<float> &) { return Sentence{Symbol('&', {params.pitchAngle})}; }});
    sys.AddRule('L', [params](const std::vector<float> &) { return Sentence{Symbol('~', {params.leafSize})}; });
    sys.AddRule('K', [params](const std::vector<float> &) { return Sentence{Symbol('@', {params.flowerSize}), Symbol('/', {90.0f})}; });
    sys.AddRule('b', [params](const std::vector<float> &p) { return Sentence{Symbol('b', {p.empty() ? params.budSize : p[0]})}; });
    sys.AddRule('o', [params](const std::vector<float> &p) { return Sentence{Symbol('o', {p.empty() ? (params.budSize * 1.2f) : p[0]})}; });
    sys.AddRule(ProductionRule{ 'X', 1.0f, [](const std::vector<float> &p) { return !p.empty() && p[0] > 0; }, [](const std::vector<float> &p) { return Sentence{Symbol('X', {p[0] - 1.0f})}; }});
    sys.AddRule(ProductionRule{ 'X', 1.0f, [](const std::vector<float> &p) { return p.empty() || p[0] <= 0; }, [](const std::vector<float> &) { return Sentence{Symbol('X', {0.0f})}; }});

    Sentence axiom{ Symbol('!', {params.baseRadius}), Symbol('I', {9.0f}), Symbol('a', {13.0f}) };
    Sentence result = sys.Generate(axiom, iters);
    return InterpretFull(result, params.defaultStep, params.defaultAngleDeg, out, maxNodes, params.baseRadius, params.radiusDecay);
}

// =============================================================
// ABOP Sympodial Tree
// =============================================================
static int BuildABOPTree(int iters, PlantNode *out, int maxNodes, unsigned int seed, const PlantParams &params)
{
    LSystem sys(seed);

    // p1 – Apex expansion with leaves and a single apical flower
    sys.AddRule('A', [params](const std::vector<float> &)
                { return Sentence{
                      Symbol('!', {params.fatteningRatio}),
                      Symbol('F', {params.internodeLen1}),

                      Symbol('['),
                      Symbol('&', {params.branchAngle1}),
                      Symbol('F', {params.internodeLen1}),
                      Symbol('A'),
                      Symbol('~', {params.leafSize}), 
                      Symbol(']'),

                      Symbol('/', {params.divergenceAngle1}),

                      Symbol('['),
                      Symbol('&', {params.branchAngle1}),
                      Symbol('F', {params.internodeLen1}),
                      Symbol('A'),
                      Symbol('~', {params.leafSize}),
                      Symbol(']'),

                      Symbol('/', {params.divergenceAngle2}),

                      Symbol('['),
                      Symbol('&', {params.branchAngle1}),
                      Symbol('F', {params.internodeLen1}),
                      Symbol('A'),
                      Symbol('~', {params.leafSize}),
                      Symbol(']'),

                      Symbol('@', {params.flowerSize}), 
                  }; });

    // p2 – Segment elongation
    sys.AddRule(ProductionRule{
        'F', 1.0f, nullptr,
        [params](const std::vector<float> &p) { 
            float l = p.empty() ? 1.f : p[0];
            return Sentence{Symbol('F', {l * params.elongationRatio})};
        }});

    // p3 – Radius fattening
    sys.AddRule(ProductionRule{
        '!', 1.0f, nullptr,
        [params](const std::vector<float> &p) { 
            float w = p.empty() ? 1.f : p[0];
            return Sentence{Symbol('!', {w * params.fatteningRatio})};
        }});

    Sentence axiom{ Symbol('!', {1.f}), Symbol('F', {params.internodeLen1 * 4.0f}), Symbol('/', {45.f}), Symbol('A') };
    Sentence result = sys.Generate(axiom, iters);
    return InterpretFull(result, params.defaultStep, params.defaultAngleDeg, out, maxNodes, params.baseRadius, params.radiusDecay);
}

// =============================================================
//  Mycelis muralis - Model III (Context-Sensitive / Signal Flow)
// =============================================================
static int BuildMycelisMuralis(int iters, PlantNode *out, int maxNodes, unsigned int seed, const PlantParams &params)
{
    Sentence cur = { Symbol('I', {params.internodeLen1}), Symbol('F'), Symbol('A', {0.0f}) };

    auto getLeftContext = [](const Sentence& s, int idx) -> char {
        int skip = 0;
        for (int i = idx - 1; i >= 0; --i) {
            char c = s[i].letter;
            if (c == ']') skip++;
            else if (c == '[') { if (skip > 0) skip--; }
            else if (skip == 0 && (c == 'M' || c == 'S' || c == 'T' || c == 'V' || c == 'W')) return c;
        }
        return '\0';
    };

    auto getRightContext = [](const Sentence& s, int idx) -> char {
        int skip = 0;
        for (int i = idx + 1; i < (int)s.size(); ++i) {
            char c = s[i].letter;
            if (c == '[') skip++;
            else if (c == ']') {
                if (skip > 0) skip--;
                else return '\0'; 
            }
            else if (skip == 0 && (c == 'M' || c == 'S' || c == 'T' || c == 'V' || c == 'W')) return c;
        }
        return '\0';
    };

    for (int i = 0; i < iters; ++i) {
        Sentence next;
        next.reserve(cur.size() * 2);

        for (int j = 0; j < (int)cur.size(); ++j) {
            const Symbol& sym = cur[j];
            char lc = getLeftContext(cur, j);
            char rc = getRightContext(cur, j);

            if (sym.letter == 'A') {
                float t = sym.param(0, 0.0f);
                if (lc == 'S' || lc == 'V') { 
                    next.push_back(Symbol('T')); next.push_back(Symbol('V')); next.push_back(Symbol('K'));
                } else if (t > 0) { 
                    next.push_back(Symbol('A', {t - 1.0f}));
                } else { 
                    next.push_back(Symbol('M'));
                    next.push_back(Symbol('[')); next.push_back(Symbol('~', {params.leafSize})); next.push_back(Symbol(']'));
                    next.push_back(Symbol('[')); next.push_back(Symbol('+', {params.branchAngle1})); next.push_back(Symbol('G')); next.push_back(Symbol(']'));
                    next.push_back(Symbol('F')); 
                    next.push_back(Symbol('/', {params.divergenceAngle1}));
                    next.push_back(Symbol('A', {2.0f}));
                }
            } else if (sym.letter == 'M') {
                if (lc == 'S' || lc == 'V') next.push_back(Symbol('S')); else next.push_back(sym);
            } else if (sym.letter == 'S') {
                if (rc == 'T') next.push_back(Symbol('T')); else next.push_back(sym);
            } else if (sym.letter == 'T') {
                if (rc == 'V') next.push_back(Symbol('W')); else next.push_back(sym);
            } else if (sym.letter == 'G') {
                if (lc == 'T') { next.push_back(Symbol('F')); next.push_back(Symbol('A', {2.0f})); }
                else next.push_back(sym);
            } else if (sym.letter == 'W') { 
                next.push_back(Symbol('V'));
            } else if (sym.letter == 'I') {
                float t = sym.param(0, 0.0f);
                if (t > 0) next.push_back(Symbol('I', {t - 1.0f})); else next.push_back(Symbol('S')); 
            } else { next.push_back(sym); }
        }
        cur = std::move(next);
    }
    return InterpretFull(cur, params.defaultStep, params.defaultAngleDeg, out, maxNodes, params.baseRadius, params.radiusDecay);
}

// =============================================================
//  Stochastic 3D Mycelis muralis - Model III 
// =============================================================
static int BuildStochasticMycelisMuralis3D(int iters, PlantNode *out, int maxNodes, unsigned int seed, const PlantParams &params)
{
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> probDist(0.0f, 1.0f);
    // Bind uniform distributions to the new parameter bounds
    std::uniform_real_distribution<float> pitchDist(params.branchAngle1, params.branchAngle2);
    std::uniform_real_distribution<float> rollDist(params.divergenceAngle1, params.divergenceAngle2);

    Sentence cur = { Symbol('I', {params.internodeLen1}), Symbol('F'), Symbol('A', {0.0f}) };

    auto getLeftContext = [](const Sentence& s, int idx) -> char {
        int skip = 0;
        for (int i = idx - 1; i >= 0; --i) {
            char c = s[i].letter;
            if (c == ']') skip++;
            else if (c == '[') { if (skip > 0) skip--; }
            else if (skip == 0 && (c == 'M' || c == 'S' || c == 'T' || c == 'V' || c == 'W')) return c;
        }
        return '\0';
    };

    auto getRightContext = [](const Sentence& s, int idx) -> char {
        int skip = 0;
        for (int i = idx + 1; i < (int)s.size(); ++i) {
            char c = s[i].letter;
            if (c == '[') skip++;
            else if (c == ']') {
                if (skip > 0) skip--;
                else return '\0'; 
            }
            else if (skip == 0 && (c == 'M' || c == 'S' || c == 'T' || c == 'V' || c == 'W')) return c;
        }
        return '\0';
    };

    for (int i = 0; i < iters; ++i) {
        Sentence next;
        next.reserve(cur.size() * 2);

        for (int j = 0; j < (int)cur.size(); ++j) {
            const Symbol& sym = cur[j];
            char lc = getLeftContext(cur, j);
            char rc = getRightContext(cur, j);

            if (sym.letter == 'A') {
                float t = sym.param(0, 0.0f);
                if (lc == 'S' || lc == 'V') { 
                    next.push_back(Symbol('T')); next.push_back(Symbol('V')); next.push_back(Symbol('K'));
                } else if (t > 0) { 
                    next.push_back(Symbol('A', {t - 1.0f}));
                } else { 
                    next.push_back(Symbol('M'));
                    
                    next.push_back(Symbol('[')); next.push_back(Symbol('~', {params.leafSize})); next.push_back(Symbol(']'));

                    if (probDist(rng) < params.probPrimary) { 
                        next.push_back(Symbol('['));
                        next.push_back(Symbol('+', {pitchDist(rng)}));
                        next.push_back(Symbol('G'));
                        next.push_back(Symbol(']'));
                    }

                    next.push_back(Symbol('F')); 
                    next.push_back(Symbol('/', {rollDist(rng)}));

                    float nextDelay = probDist(rng) < params.probSecondary ? 3.0f : 2.0f;
                    next.push_back(Symbol('A', {nextDelay}));
                }
            } else if (sym.letter == 'M') {
                if (lc == 'S' || lc == 'V') next.push_back(Symbol('S')); else next.push_back(sym);
            } else if (sym.letter == 'S') {
                if (rc == 'T') next.push_back(Symbol('T')); else next.push_back(sym);
            } else if (sym.letter == 'T') {
                if (rc == 'V') next.push_back(Symbol('W')); else next.push_back(sym);
            } else if (sym.letter == 'G') {
                if (lc == 'T') { next.push_back(Symbol('F')); next.push_back(Symbol('A', {2.0f})); }
                else next.push_back(sym);
            } else if (sym.letter == 'W') { 
                next.push_back(Symbol('V'));
            } else if (sym.letter == 'I') {
                float t = sym.param(0, 0.0f);
                if (t > 0) next.push_back(Symbol('I', {t - 1.0f})); else next.push_back(Symbol('S')); 
            } else { next.push_back(sym); }
        }
        cur = std::move(next);
    }
    return InterpretFull(cur, params.defaultStep, params.defaultAngleDeg, out, maxNodes, params.baseRadius, params.radiusDecay);
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
            return BuildABOPTree(iterations, outNodes, maxNodes, seed, params);
        case 3:
            return BuildMycelisMuralis(iterations, outNodes, maxNodes, seed, params);
        case 4:
            return BuildStochasticMycelisMuralis3D(iterations, outNodes, maxNodes, seed, params);

        default:
            std::cerr << "[GeneratePlant] Unknown exampleId: " << exampleId << "\n";
            return 0;
        }
    }
}