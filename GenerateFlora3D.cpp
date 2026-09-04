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

    // p3: Floral apex producing raceme inflorescence with bud->flower->fruit sequence X(5)
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

    // New explicit bud terminal rules
    sys.AddRule('b', [](const std::vector<float> &p)
                { return Sentence{Symbol('b', {p.empty() ? 0.08f : p[0]})}; });
    sys.AddRule('o', [](const std::vector<float> &p)
                { return Sentence{Symbol('o', {p.empty() ? 0.10f : p[0]})}; });

    // p10 & p11: Heart-shaped silique and floral organ maturation X(t)
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
            return Sentence{Symbol('X', {0.0f})};
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

    // 1. Vegetative Apex (a)
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

    sys.AddRule(ProductionRule{
        'a', 0.30f,
        [](const std::vector<float> &p)
        { return !p.empty() && p[0] > 0; },
        [](const std::vector<float> &p)
        {
            float t = p[0];
            return Sentence{
                Symbol('['), Symbol('&', {60.0f}), Symbol('L'), Symbol(']'),
                Symbol('/', {145.0f}),
                Symbol('I', {8.0f}),
                Symbol('a', {t - 1.0f})};
        }});

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

    // 2. Floral Apex (A) - Raceme Inflorescence with maturation X(t)
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
                Symbol(']'),
                Symbol('/', {137.5f}),
                Symbol('I', {8.0f}),
                Symbol('A')};
        }});

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
                Symbol(']'),
                Symbol('/', {120.0f}),
                Symbol('I', {6.0f}),
                Symbol('A')};
        }});

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
                Symbol(']'),
                Symbol('/', {150.0f}),
                Symbol('I', {10.0f}),
                Symbol('A')};
        }});

    // 3. Stem Internode Elongation (I)
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

    // 4. Terminals
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

    sys.AddRule('b', [](const std::vector<float> &p)
                { return Sentence{Symbol('b', {p.empty() ? 0.08f : p[0]})}; });
    sys.AddRule('o', [](const std::vector<float> &p)
                { return Sentence{Symbol('o', {p.empty() ? 0.10f : p[0]})}; });

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
        { return Sentence{Symbol('X', {0.0f})}; }});

    Sentence axiom{
        Symbol('!', {params.baseRadius}),
        Symbol('I', {9.0f}),
        Symbol('a', {13.0f})};

    Sentence result = sys.Generate(axiom, iters);

    std::cout << "[CapsellaBursaPastoris/Stochastic Fixed] iter=" << iters
              << " nodes=" << result.size()
              << " seed=" << seed << "\n";

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
//  Mycelis muralis - Model III (Context-Sensitive / Signal Flow)
// =============================================================
static int BuildMycelisMuralis(int iters, PlantNode *out, int maxNodes, unsigned int seed, const PlantParams &params)
{
    Sentence cur = { Symbol('I', {20.0f}), Symbol('F'), Symbol('A', {0.0f}) };

    // Scanner for Left Context
    auto getLeftContext = [](const Sentence& s, int idx) -> char {
        int skip = 0;
        for (int i = idx - 1; i >= 0; --i) {
            char c = s[i].letter;
            if (c == ']') skip++;
            else if (c == '[') {
                if (skip > 0) skip--;
            }
            else if (skip == 0) {
                // W is strictly required here to prevent T from skipping the 1-step delay
                if (c == 'M' || c == 'S' || c == 'T' || c == 'V' || c == 'W') return c;
            }
        }
        return '\0';
    };

    // Scanner for Right Context
    auto getRightContext = [](const Sentence& s, int idx) -> char {
        int skip = 0;
        for (int i = idx + 1; i < (int)s.size(); ++i) {
            char c = s[i].letter;
            if (c == '[') skip++;
            else if (c == ']') {
                if (skip > 0) skip--;
                else return '\0'; 
            }
            else if (skip == 0) {
                // W is strictly required here as well
                if (c == 'M' || c == 'S' || c == 'T' || c == 'V' || c == 'W') return c;
            }
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
                if (lc == 'S' || lc == 'V') { // p1, p2
                    next.push_back(Symbol('T'));
                    next.push_back(Symbol('V'));
                    next.push_back(Symbol('K'));
                } else if (t > 0) { // p3
                    next.push_back(Symbol('A', {t - 1.0f}));
                } else { // p4
                    next.push_back(Symbol('M'));
                    // Missing foliage rule natively injected to support the Unity renderer
                    next.push_back(Symbol('['));
                    next.push_back(Symbol('~')); 
                    next.push_back(Symbol(']'));
                    
                    next.push_back(Symbol('['));
                    next.push_back(Symbol('+', {30.0f}));
                    next.push_back(Symbol('G'));
                    next.push_back(Symbol(']'));
                    next.push_back(Symbol('F')); 
                    next.push_back(Symbol('/', {180.0f}));
                    next.push_back(Symbol('A', {2.0f}));
                }
            } else if (sym.letter == 'M') {
                if (lc == 'S' || lc == 'V') next.push_back(Symbol('S')); // p5, p8
                else next.push_back(sym);
            } else if (sym.letter == 'S') {
                if (rc == 'T') next.push_back(Symbol('T')); // p6
                else next.push_back(sym);
            } else if (sym.letter == 'T') {
                if (rc == 'V') next.push_back(Symbol('W')); // p9
                else next.push_back(sym);
            } else if (sym.letter == 'G') {
                if (lc == 'T') { // p7
                    next.push_back(Symbol('F'));
                    next.push_back(Symbol('A', {2.0f}));
                } else {
                    next.push_back(sym);
                }
            } else if (sym.letter == 'W') { // p10
                next.push_back(Symbol('V'));
            } else if (sym.letter == 'I') {
                float t = sym.param(0, 0.0f);
                if (t > 0) next.push_back(Symbol('I', {t - 1.0f})); // p11
                else next.push_back(Symbol('S')); // p12
            } else {
                next.push_back(sym); 
            }
        }
        cur = std::move(next);
    }

    std::cout << "[MycelisMuralis] iter=" << iters << " nodes=" << cur.size() << "\n";
    return InterpretFull(cur, params.defaultStep, params.defaultAngleDeg, out, maxNodes, params.baseRadius, params.radiusDecay);
}

// =============================================================
//  Stochastic 3D Mycelis muralis - Model III 
// =============================================================
static int BuildStochasticMycelisMuralis3D(int iters, PlantNode *out, int maxNodes, unsigned int seed, const PlantParams &params)
{
    // Initialize RNG based on the deterministic seed passed from Unity
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> probDist(0.0f, 1.0f);
    std::uniform_real_distribution<float> pitchDist(20.0f, 45.0f);
    std::uniform_real_distribution<float> rollDist(110.0f, 160.0f);

    Sentence cur = { Symbol('I', {20.0f}), Symbol('F'), Symbol('A', {0.0f}) };

    // Scanner for Left Context
    auto getLeftContext = [](const Sentence& s, int idx) -> char {
        int skip = 0;
        for (int i = idx - 1; i >= 0; --i) {
            char c = s[i].letter;
            if (c == ']') skip++;
            else if (c == '[') {
                if (skip > 0) skip--;
            }
            else if (skip == 0) {
                if (c == 'M' || c == 'S' || c == 'T' || c == 'V' || c == 'W') return c;
            }
        }
        return '\0';
    };

    // Scanner for Right Context
    auto getRightContext = [](const Sentence& s, int idx) -> char {
        int skip = 0;
        for (int i = idx + 1; i < (int)s.size(); ++i) {
            char c = s[i].letter;
            if (c == '[') skip++;
            else if (c == ']') {
                if (skip > 0) skip--;
                else return '\0'; 
            }
            else if (skip == 0) {
                if (c == 'M' || c == 'S' || c == 'T' || c == 'V' || c == 'W') return c;
            }
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
                if (lc == 'S' || lc == 'V') { // Apical flower transformation & Signal emission
                    next.push_back(Symbol('T'));
                    next.push_back(Symbol('V'));
                    next.push_back(Symbol('K'));
                } else if (t > 0) { // Apex aging
                    next.push_back(Symbol('A', {t - 1.0f}));
                } else { 
                    // p4: Lateral branch initiation (Now Stochastic & 3D)
                    next.push_back(Symbol('M')); // M MUST always spawn to pass signals
                    
                    // Foliage is always generated at the node
                    next.push_back(Symbol('['));
                    next.push_back(Symbol('~')); 
                    next.push_back(Symbol(']'));

                    // 75% chance to actually spawn a lateral branching apex
                    if (probDist(rng) < 0.75f) { 
                        next.push_back(Symbol('['));
                        next.push_back(Symbol('+', {pitchDist(rng)}));
                        next.push_back(Symbol('G'));
                        next.push_back(Symbol(']'));
                    }

                    // 3D Roll/Divergence and main stem extension
                    next.push_back(Symbol('F')); 
                    next.push_back(Symbol('/', {rollDist(rng)}));

                    // 20% chance to slightly delay the next apex phase for organic irregularity
                    float nextDelay = probDist(rng) < 0.20f ? 3.0f : 2.0f;
                    next.push_back(Symbol('A', {nextDelay}));
                }
            } else if (sym.letter == 'M') {
                if (lc == 'S' || lc == 'V') next.push_back(Symbol('S'));
                else next.push_back(sym);
            } else if (sym.letter == 'S') {
                if (rc == 'T') next.push_back(Symbol('T'));
                else next.push_back(sym);
            } else if (sym.letter == 'T') {
                if (rc == 'V') next.push_back(Symbol('W'));
                else next.push_back(sym);
            } else if (sym.letter == 'G') {
                if (lc == 'T') {
                    next.push_back(Symbol('F'));
                    next.push_back(Symbol('A', {2.0f}));
                } else {
                    next.push_back(sym);
                }
            } else if (sym.letter == 'W') { 
                next.push_back(Symbol('V'));
            } else if (sym.letter == 'I') {
                float t = sym.param(0, 0.0f);
                if (t > 0) next.push_back(Symbol('I', {t - 1.0f})); 
                else next.push_back(Symbol('S')); 
            } else {
                next.push_back(sym); 
            }
        }
        cur = std::move(next);
    }

    std::cout << "[StochasticMycelis3D] iter=" << iters << " nodes=" << cur.size() << "\n";
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
            return BuildCrocus(iterations, outNodes, maxNodes, seed, params);
        case 3:
            return BuildABOPTree(iterations, outNodes, maxNodes, seed, params);
        case 4:
            return BuildMycelisMuralis(iterations, outNodes, maxNodes, seed, params);
        case 5:
            return BuildStochasticMycelisMuralis3D(iterations, outNodes, maxNodes, seed, params);

        default:
            std::cerr << "[GeneratePlant] Unknown exampleId: " << exampleId << "\n";
            return 0;
        }
    }
}