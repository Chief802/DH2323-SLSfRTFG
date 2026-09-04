#pragma once

#include <cmath>
#include <functional>
#include <random>
#include <stack>
#include <string>
#include <vector>

// == Geometry primitives =====================================================
struct Vec3
{
    float x, y, z;
};
struct Segment
{
    Vec3 start, end;
    float radius;
}; // kept for backward compat

// == Plant-node type tag =====================================================
enum class NodeType : int
{
    Branch = 0,
    Leaf = 1,
    Flower = 2,
    Fruit = 3,
    Bud = 4,
    BudOpening = 5
};

/**
 * One output element produced by InterpretFull().
 *
 * Memory layout (56 bytes, no internal padding, matches C# StructLayout.Sequential):
 *   origin   Vec3   12 B   branch base, or position of leaf / flower
 *   end      Vec3   12 B   branch tip  (equals origin for leaf / flower)
 *   heading  Vec3   12 B   turtle H at emission time  (branch forward / leaf face normal)
 *   left     Vec3   12 B   turtle L at emission time  (leaf lateral axis)
 *   radius   float   4 B   branch cross-section radius, or leaf / flower billboard size
 *   type     int     4 B   NodeType cast to int
 **/
struct PlantNode
{
    Vec3 origin;
    Vec3 end;
    Vec3 heading; // turtle H
    Vec3 left;    // turtle L
    float radius;
    NodeType type;
};

// =============================================================
//  Symbol
// =============================================================
struct Symbol
{
    char letter;
    std::vector<float> params;

    Symbol(char c) : letter(c) {}
    Symbol(char c, std::initializer_list<float> init) : letter(c), params(init) {}
    Symbol(char c, std::vector<float> p) : letter(c), params(std::move(p)) {}

    float param(std::size_t i, float defaultVal = 1.0f) const
    {
        return i < params.size() ? params[i] : defaultVal;
    }
};

using Sentence = std::vector<Symbol>;

inline Sentence MakeSentence(const std::string &s)
{
    Sentence out;
    out.reserve(s.size());
    for (char c : s)
        out.emplace_back(c);
    return out;
}

// =============================================================
//  ProductionRule
// =============================================================
struct ProductionRule
{
    char predecessor;
    float probability;

    std::function<bool(const std::vector<float> &)> condition;
    std::function<Sentence(const std::vector<float> &)> successor;
};

// =============================================================
//  LSystem
// =============================================================
class LSystem
{
public:
    explicit LSystem(unsigned int seed = 42u) : rng_(seed) {}

    void SetSeed(unsigned int seed) { rng_.seed(seed); }

    void AddRule(ProductionRule r) { rules_.push_back(std::move(r)); }

    void AddRule(char c, std::function<Sentence(const std::vector<float> &)> succ)
    {
        rules_.push_back({c, 1.0f, nullptr, std::move(succ)});
    }

    void AddRule(char c, float prob,
                 std::function<Sentence(const std::vector<float> &)> succ)
    {
        rules_.push_back({c, prob, nullptr, std::move(succ)});
    }

    Sentence Step(const Sentence &in)
    {
        Sentence out;
        out.reserve(in.size() * 4);

        for (const Symbol &sym : in)
        {
            struct Candidate
            {
                float prob;
                const ProductionRule *rule;
            };
            std::vector<Candidate> cands;
            float total = 0.f;

            for (const auto &r : rules_)
            {
                if (r.predecessor != sym.letter)
                    continue;
                if (r.condition && !r.condition(sym.params))
                    continue;
                cands.push_back({r.probability, &r});
                total += r.probability;
            }

            if (cands.empty())
            {
                out.push_back(sym);
                continue;
            }

            const ProductionRule *chosen = cands.back().rule;
            if (cands.size() == 1u)
            {
                chosen = cands[0].rule;
            }
            else
            {
                float roll = std::uniform_real_distribution<float>(0.f, total)(rng_);
                float cum = 0.f;
                for (auto &c : cands)
                {
                    cum += c.prob;
                    if (roll <= cum)
                    {
                        chosen = c.rule;
                        break;
                    }
                }
            }

            Sentence sub = chosen->successor(sym.params);
            out.insert(out.end(), sub.begin(), sub.end());
        }
        return out;
    }

    Sentence Generate(const Sentence &axiom, int iterations)
    {
        Sentence cur = axiom;
        for (int i = 0; i < iterations; ++i)
            cur = Step(cur);
        return cur;
    }

private:
    std::vector<ProductionRule> rules_;
    std::mt19937 rng_;
};

// =============================================================
//  Turtle interpreter — shared state
// =============================================================
namespace detail
{

    struct TurtleState
    {
        Vec3 pos;
        Vec3 U; // Up
        Vec3 L; // Left
        Vec3 H; // Heading (forward)
        float radius;
    };

    inline void RotateTurtle(TurtleState &t, char axis, float alpha)
    {
        const float ca = std::cos(alpha), sa = std::sin(alpha);
        const Vec3 oH = t.H, oU = t.U, oL = t.L;
        switch (axis)
        {
        case 'U':
            t.H = {oH.x * ca + oL.x * sa, oH.y * ca + oL.y * sa, oH.z * ca + oL.z * sa};
            t.L = {-oH.x * sa + oL.x * ca, -oH.y * sa + oL.y * ca, -oH.z * sa + oL.z * ca};
            break;
        case 'L':
            t.H = {oH.x * ca - oU.x * sa, oH.y * ca - oU.y * sa, oH.z * ca - oU.z * sa};
            t.U = {oH.x * sa + oU.x * ca, oH.y * sa + oU.y * ca, oH.z * sa + oU.z * ca};
            break;
        case 'H':
            t.L = {oL.x * ca - oU.x * sa, oL.y * ca - oU.y * sa, oL.z * ca - oU.z * sa};
            t.U = {oL.x * sa + oU.x * ca, oL.y * sa + oU.y * ca, oL.z * sa + oU.z * ca};
            break;
        default:
            break;
        }
    }

} 

// =============================================================
//  InterpretFull  –  primary interpreter, emits PlantNode
// =============================================================
inline int InterpretFull(
    const Sentence &commands,
    float defaultStep,
    float defaultAngleDeg,
    PlantNode *outNodes,
    int maxNodes,
    float baseRadius = 0.15f,
    float radiusDecay = 0.7071f)
{
    using namespace detail;
    constexpr float kPi = 3.14159265f;
    const float kDefaultAngleRad = defaultAngleDeg * kPi / 180.f;

    TurtleState turtle = {
        {0.f, 0.f, 0.f},  // pos
        {0.f, 0.f, -1.f}, // U
        {-1.f, 0.f, 0.f}, // L
        {0.f, 1.f, 0.f},  // H  (+Y = upward growth)
        baseRadius        // Initial branch width
    };

    std::stack<TurtleState> stk;
    int count = 0;

    auto angleOf = [&](const Symbol &s) -> float
    {
        return s.params.empty() ? kDefaultAngleRad : s.params[0] * kPi / 180.f;
    };

    auto emit = [&](NodeType t, float r, const Vec3 &end)
    {
        if (count < maxNodes)
            outNodes[count++] = {turtle.pos, end, turtle.H, turtle.L, r, t};
    };

    for (const Symbol &sym : commands)
    {
        switch (sym.letter)
        {
        case 'F':
        {
            float step = sym.param(0, defaultStep);
            Vec3 np = {
                turtle.pos.x + step * turtle.H.x,
                turtle.pos.y + step * turtle.H.y,
                turtle.pos.z + step * turtle.H.z};
            emit(NodeType::Branch, turtle.radius, np);
            turtle.pos = np;
            break;
        }

        case 'f':
        {
            float step = sym.param(0, defaultStep);
            turtle.pos.x += step * turtle.H.x;
            turtle.pos.y += step * turtle.H.y;
            turtle.pos.z += step * turtle.H.z;
            break;
        }

        case '~': emit(NodeType::Leaf, sym.param(0, 0.3f), turtle.pos); break;
        case '@': emit(NodeType::Flower, sym.param(0, 0.15f), turtle.pos); break;
        case 'x': emit(NodeType::Fruit, sym.param(0, 0.22f), turtle.pos); break;
        case 'b': emit(NodeType::Bud, sym.param(0, 0.08f), turtle.pos); break;
        case 'o': emit(NodeType::BudOpening, sym.param(0, 0.10f), turtle.pos); break;
        case 'L': emit(NodeType::Leaf, sym.param(0, 0.0f) * 0.05f + 0.1f, turtle.pos); break;
        case 'K': emit(NodeType::Flower, sym.param(0, 0.0f) * 0.15f + 0.2f, turtle.pos); break;
        case 'G': emit(NodeType::Bud, 0.08f, turtle.pos); break;
        case 'A': emit(NodeType::BudOpening, 0.10f, turtle.pos); break;
        case 'X':
        {
            float t = sym.param(0, 0.0f);
            if (t >= 4.0f)
                emit(NodeType::Bud, 0.08f, turtle.pos);
            else if (t >= 2.0f)
                emit(NodeType::BudOpening, 0.10f, turtle.pos);
            else if (t > 0.0f)
                emit(NodeType::Flower, 0.12f, turtle.pos);
            else
                emit(NodeType::Fruit, 0.22f, turtle.pos);
            break;
        }
        case '!': turtle.radius = sym.param(0, turtle.radius); break;
        case '+': RotateTurtle(turtle, 'U', angleOf(sym)); break;
        case '-': RotateTurtle(turtle, 'U', -angleOf(sym)); break;
        case '&': RotateTurtle(turtle, 'L', angleOf(sym)); break;
        case '^': RotateTurtle(turtle, 'L', -angleOf(sym)); break;
        case '\\': RotateTurtle(turtle, 'H', angleOf(sym)); break;
        case '/': RotateTurtle(turtle, 'H', -angleOf(sym)); break;
        case '|': RotateTurtle(turtle, 'U', kPi); break;

        case '[':
            stk.push(turtle);
            turtle.radius *= radiusDecay;
            break;
        case ']':
            if (!stk.empty())
            {
                turtle = stk.top();
                stk.pop();
            }
            break;

        default:
            break;
        }
    }
    return count;
}