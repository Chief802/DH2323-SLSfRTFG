using System.Collections.Generic;
using UnityEngine;

public partial class PlantRenderer : MonoBehaviour
{
    /// <summary>Emits a circle of vertices to represent a slice of a branch.</summary>
    void EmitRing(Vector3 center, float radius, Vector3 frameN, Vector3 frameB, float vCoord, int R)
    {
        for (int s = 0; s < R; s++)
        {
            float theta = s / (float)R * Mathf.PI * 2f;
            Vector3 outDir = Mathf.Cos(theta) * frameN + Mathf.Sin(theta) * frameB;
            _verts.Add(center + outDir * radius);
            _norms.Add(outDir);
            _uvs.Add(new Vector2((float)s / R, vCoord));
        }
    }

    void EmitTeardrop(Vector3 pos, Vector3 fwd, Vector3 right, Vector3 n, float size)
    {
        float hw = size * 0.35f;
        Vector3 p0 = pos, p1 = pos + right * hw + fwd * size * 0.40f, p2 = pos + fwd * size, p3 = pos - right * hw + fwd * size * 0.40f;
        EmitDoubleSidedQuad(p0, p1, p2, p3, n);
    }

    void EmitOval(Vector3 pos, Vector3 fwd, Vector3 right, Vector3 n, float size, int lodDetail)
    {
        int segs = Mathf.Max(4, 12 / lodDetail);
        Vector3 center = pos + fwd * size * 0.5f;

        int cf = _verts.Count; _verts.Add(center); _norms.Add(n); _uvs.Add(new Vector2(0.5f, 0.5f));
        int cb = _verts.Count; _verts.Add(center); _norms.Add(-n); _uvs.Add(new Vector2(0.5f, 0.5f));
        int rimBase = _verts.Count;

        for (int s = 0; s <= segs; s++)
        {
            float a = s / (float)segs * Mathf.PI * 2f;
            float c = Mathf.Cos(a), si = Mathf.Sin(a);
            Vector3 rim = pos + fwd * (size * 0.5f + size * 0.5f * si) + right * (size * 0.38f * c);
            _verts.Add(rim); _norms.Add(n); _uvs.Add(new Vector2(0.5f + 0.5f * c, 0.5f + 0.5f * si));
            _verts.Add(rim); _norms.Add(-n); _uvs.Add(new Vector2(0.5f + 0.5f * c, 0.5f + 0.5f * si));
        }

        for (int s = 0; s < segs; s++)
        {
            int f0 = rimBase + s * 2, f1 = rimBase + (s + 1) * 2;
            _tris.Add(cf); _tris.Add(f0); _tris.Add(f1);
            _tris.Add(cb); _tris.Add(f1 + 1); _tris.Add(f0 + 1);
        }
    }

    void EmitCompound(Vector3 pos, Vector3 fwd, Vector3 right, Vector3 n, float size)
    {
        float leafLen = size * 0.32f;
        for (int i = 0; i < 3; i++)
        {
            float t = (i + 1f) / 4f;
            Vector3 pivot = pos + fwd * size * t;
            float ls = leafLen * (1f - t * 0.3f);
            EmitTeardrop(pivot, (fwd + right * 0.65f).normalized, right, n, ls);
            EmitTeardrop(pivot, (fwd - right * 0.65f).normalized, -right, -n, ls);
        }
        EmitTeardrop(pos + fwd * size * 0.85f, fwd, right, n, size * 0.28f);
    }

    void EmitNeedle(Vector3 pos, Vector3 fwd, Vector3 right, Vector3 n, float size)
    {
        float hw = size * 0.055f;
        EmitDoubleSidedQuad(pos, pos + right * hw, pos + fwd * size, pos - right * hw, n);
    }

    void EmitLobedRosette(Vector3 pos, Vector3 fwd, Vector3 right, Vector3 n, float size)
    {
        float len = size * 1.4f;
        int lobes = 4;
        for (int i = 0; i < lobes; i++)
        {
            float t = (i + 1f) / (lobes + 1f);
            Vector3 p = pos + fwd * (len * t);
            float lw = size * (0.35f * Mathf.Sin(t * Mathf.PI));
            EmitTeardrop(p, (fwd + right * 0.8f).normalized, right, n, lw);
            EmitTeardrop(p, (fwd - right * 0.8f).normalized, -right, -n, lw);
        }
        EmitTeardrop(pos + fwd * len * 0.9f, fwd, right, n, size * 0.3f);
    }

    void EmitLyrateLeaf(Vector3 pos, Vector3 fwd, Vector3 right, Vector3 n, float size)
    {
        float len = size * 1.3f;
        for (int i = 1; i <= 2; i++)
        {
            float t = i * 0.25f;
            Vector3 p = pos + fwd * (len * t);
            float lw = size * 0.25f * t;
            EmitDoubleSidedQuad(p, p + right * lw + fwd * 0.1f, p + fwd * 0.3f, p - right * lw + fwd * 0.1f, n);
        }
        Vector3 baseTip = pos + fwd * (len * 0.6f);
        Vector3 apex = pos + fwd * len;
        float termWidth = size * 0.5f;
        EmitDoubleSidedQuad(baseTip, baseTip + right * termWidth, apex, baseTip - right * termWidth, n);
    }

    void EmitClosedBud(Vector3 pos, Vector3 fwd, Vector3 right, Vector3 n, float size)
    {
        int sepals = 4;
        float length = size * 1.2f;
        float radius = size * 0.45f;
        Vector3 apex = pos + fwd * length;
        Vector3 up2 = Vector3.Cross(fwd, right).normalized;

        int bApex = _verts.Count;
        _verts.Add(apex); _norms.Add(fwd); _uvs.Add(new Vector2(0.5f, 1.0f));

        int bBase = _verts.Count;
        _verts.Add(pos); _norms.Add(-fwd); _uvs.Add(new Vector2(0.5f, 0.0f));

        int ringStart = _verts.Count;
        for (int i = 0; i < sepals; i++)
        {
            float a = i / (float)sepals * Mathf.PI * 2f;
            Vector3 dir = (Mathf.Cos(a) * right + Mathf.Sin(a) * up2).normalized;
            Vector3 ringPos = pos + fwd * (length * 0.45f) + dir * radius;
            Vector3 norm = (dir + fwd * 0.3f).normalized;

            _verts.Add(ringPos);
            _norms.Add(norm);
            _uvs.Add(new Vector2((float)i / sepals, 0.5f));
        }

        for (int i = 0; i < sepals; i++)
        {
            int nextI = (i + 1) % sepals;
            int r0 = ringStart + i;
            int r1 = ringStart + nextI;

            _tris.Add(bBase); _tris.Add(r0); _tris.Add(r1);
            _tris.Add(bBase); _tris.Add(r1); _tris.Add(r0);

            _tris.Add(r0); _tris.Add(bApex); _tris.Add(r1);
            _tris.Add(r0); _tris.Add(r1); _tris.Add(bApex);
        }
    }

    void EmitOpeningBud(Vector3 pos, Vector3 fwd, Vector3 right, Vector3 n, float size)
    {
        int sepals = 4;
        float length = size * 1.1f;
        float radius = size * 0.55f;
        Vector3 up2 = Vector3.Cross(fwd, right).normalized;

        for (int i = 0; i < sepals; i++)
        {
            float a = i / (float)sepals * Mathf.PI * 2f;
            Vector3 dir = (Mathf.Cos(a) * right + Mathf.Sin(a) * up2).normalized;
            Vector3 lat = Vector3.Cross(dir, fwd).normalized;

            Vector3 sepalBase = pos;
            Vector3 sepalTip = pos + fwd * length + dir * (radius * 0.8f);
            Vector3 norm = (dir + fwd * 0.2f).normalized;

            int b = _verts.Count;
            _verts.Add(sepalBase - lat * (radius * 0.3f));
            _verts.Add(sepalBase + lat * (radius * 0.3f));
            _verts.Add(sepalTip + lat * (radius * 0.2f));
            _verts.Add(sepalTip - lat * (radius * 0.2f));

            for (int k = 0; k < 4; k++) _norms.Add(norm);
            _uvs.Add(new Vector2(0f, 0f)); _uvs.Add(new Vector2(1f, 0f));
            _uvs.Add(new Vector2(1f, 1f)); _uvs.Add(new Vector2(0f, 1f));

            _tris.Add(b); _tris.Add(b + 1); _tris.Add(b + 2);
            _tris.Add(b); _tris.Add(b + 2); _tris.Add(b + 3);
            _tris.Add(b); _tris.Add(b + 2); _tris.Add(b + 1);
            _tris.Add(b); _tris.Add(b + 3); _tris.Add(b + 2);
        }

        Vector3 petalCenter = pos + fwd * (length * 0.6f);
        float petalSize = size * 0.45f;
        for (int p = 0; p < 4; p++)
        {
            float pa = (p * Mathf.PI * 0.5f) + (Mathf.PI * 0.25f);
            Vector3 pd = (Mathf.Cos(pa) * right + Mathf.Sin(pa) * up2).normalized;
            Vector3 pTip = petalCenter + fwd * (petalSize * 0.8f) + pd * (petalSize * 0.4f);
            EmitTeardrop(petalCenter, (pTip - petalCenter).normalized, Vector3.Cross((pTip - petalCenter).normalized, fwd), fwd, petalSize);
        }
    }
    
    void EmitCrossFourPetal(Vector3 center, Vector3 axis, Vector3 right, Vector3 up2, float size)
    {
        float discR = size * 0.15f;
        float petalLen = size * 0.85f;
        float hw = size * 0.32f;

        for (int p = 0; p < 4; p++)
        {
            float a = p * Mathf.PI * 0.5f;
            Vector3 pd = (Mathf.Cos(a) * right + Mathf.Sin(a) * up2).normalized;
            Vector3 lat = Vector3.Cross(pd, axis).normalized;
            Vector3 rC = center + pd * discR;
            Vector3 tC = center + pd * petalLen + axis * (petalLen * petalCurvature);
            Vector3 pn = (axis + pd * petalCurvature).normalized;

            int pb = _verts.Count;
            _verts.Add(rC - lat * (hw * 0.3f));
            _verts.Add(rC + lat * (hw * 0.3f));
            _verts.Add(tC + lat * hw);
            _verts.Add(tC - lat * hw);

            for (int k = 0; k < 4; k++) _norms.Add(pn);
            _uvs.Add(Vector2.zero); _uvs.Add(new Vector2(1, 0)); _uvs.Add(Vector2.one); _uvs.Add(new Vector2(0, 1));

            _tris.Add(pb); _tris.Add(pb + 1); _tris.Add(pb + 2); _tris.Add(pb); _tris.Add(pb + 2); _tris.Add(pb + 3);
            _tris.Add(pb); _tris.Add(pb + 2); _tris.Add(pb + 1); _tris.Add(pb); _tris.Add(pb + 3); _tris.Add(pb + 2);
        }
    }

    void EmitHeartPod(Vector3 pos, Vector3 fwd, Vector3 right, Vector3 n, float size)
    {
        float h = size * 1.2f;
        float w = size * 0.95f;

        Vector3 pBase = pos;
        Vector3 pL = pos + fwd * (h * 0.65f) - right * (w * 0.5f);
        Vector3 pR = pos + fwd * (h * 0.65f) + right * (w * 0.5f);
        Vector3 pTipL = pos + fwd * h - right * (w * 0.35f);
        Vector3 pTipR = pos + fwd * h + right * (w * 0.35f);
        Vector3 pNotch = pos + fwd * (h * 0.82f);

        int b = _verts.Count;
        _verts.Add(pBase); _verts.Add(pL); _verts.Add(pTipL);
        _verts.Add(pNotch); _verts.Add(pTipR); _verts.Add(pR);
        for (int k = 0; k < 6; k++) _norms.Add(n);
        _uvs.Add(new Vector2(0.5f, 0f)); _uvs.Add(new Vector2(0f, 0.6f)); _uvs.Add(new Vector2(0.2f, 1f));
        _uvs.Add(new Vector2(0.5f, 0.8f)); _uvs.Add(new Vector2(0.8f, 1f)); _uvs.Add(new Vector2(1f, 0.6f));

        _tris.Add(b); _tris.Add(b + 1); _tris.Add(b + 2);
        _tris.Add(b); _tris.Add(b + 2); _tris.Add(b + 3);
        _tris.Add(b); _tris.Add(b + 3); _tris.Add(b + 4);
        _tris.Add(b); _tris.Add(b + 4); _tris.Add(b + 5);

        b = _verts.Count;
        _verts.Add(pBase); _verts.Add(pL); _verts.Add(pTipL);
        _verts.Add(pNotch); _verts.Add(pTipR); _verts.Add(pR);
        for (int k = 0; k < 6; k++) _norms.Add(-n);
        _uvs.Add(new Vector2(0.5f, 0f)); _uvs.Add(new Vector2(0f, 0.6f)); _uvs.Add(new Vector2(0.2f, 1f));
        _uvs.Add(new Vector2(0.5f, 0.8f)); _uvs.Add(new Vector2(0.8f, 1f)); _uvs.Add(new Vector2(1f, 0.6f));

        _tris.Add(b); _tris.Add(b + 2); _tris.Add(b + 1);
        _tris.Add(b); _tris.Add(b + 3); _tris.Add(b + 2);
        _tris.Add(b); _tris.Add(b + 4); _tris.Add(b + 3);
        _tris.Add(b); _tris.Add(b + 5); _tris.Add(b + 4);
    }

    void EmitDoubleSidedQuad(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, Vector3 n)
    {
        int b = _verts.Count;
        _verts.Add(p0); _verts.Add(p1); _verts.Add(p2); _verts.Add(p3);
        for (int k = 0; k < 4; k++) _norms.Add(n);
        _uvs.Add(new Vector2(0.5f, 0f)); _uvs.Add(new Vector2(1f, 0.4f)); _uvs.Add(new Vector2(0.5f, 1f)); _uvs.Add(new Vector2(0f, 0.4f));
        _tris.Add(b); _tris.Add(b + 1); _tris.Add(b + 2); _tris.Add(b); _tris.Add(b + 2); _tris.Add(b + 3);

        b = _verts.Count;
        _verts.Add(p0); _verts.Add(p1); _verts.Add(p2); _verts.Add(p3);
        for (int k = 0; k < 4; k++) _norms.Add(-n);
        _uvs.Add(new Vector2(0.5f, 0f)); _uvs.Add(new Vector2(1f, 0.4f)); _uvs.Add(new Vector2(0.5f, 1f)); _uvs.Add(new Vector2(0f, 0.4f));
        _tris.Add(b); _tris.Add(b + 2); _tris.Add(b + 1); _tris.Add(b); _tris.Add(b + 3); _tris.Add(b + 2);
    }

    void EmitRadialFlower(Vector3 center, Vector3 axis, Vector3 right, Vector3 up2, float size, int petals, float discFrac, float hwFrac, float curvature)
    {
        float discR = size * discFrac, petalLen = size, hw = size * hwFrac;
        int db = _verts.Count;
        _verts.Add(center); _norms.Add(axis); _uvs.Add(new Vector2(0.5f, 0.5f));
        for (int p = 0; p < petals; p++)
        {
            float a = p / (float)petals * Mathf.PI * 2f;
            Vector3 d = Mathf.Cos(a) * right + Mathf.Sin(a) * up2;
            _verts.Add(center + d * discR); _norms.Add(axis); _uvs.Add(new Vector2(0.5f + 0.25f * Mathf.Cos(a), 0.5f + 0.25f * Mathf.Sin(a)));
        }
        for (int p = 0; p < petals; p++) { _tris.Add(db); _tris.Add(db + 1 + p); _tris.Add(db + 1 + (p + 1) % petals); }

        for (int p = 0; p < petals; p++)
        {
            float a = p / (float)petals * Mathf.PI * 2f;
            Vector3 pd = (Mathf.Cos(a) * right + Mathf.Sin(a) * up2).normalized;
            Vector3 lat = Vector3.Cross(pd, axis).normalized;
            Vector3 rC = center + pd * discR;
            Vector3 tC = center + pd * petalLen + axis * (petalLen * curvature);
            Vector3 pn = (axis + pd * curvature).normalized;

            int pb = _verts.Count;
            _verts.Add(rC - lat * hw); _verts.Add(rC + lat * hw); _verts.Add(tC + lat * hw * 0.35f); _verts.Add(tC - lat * hw * 0.35f);
            for (int k = 0; k < 4; k++) _norms.Add(pn);
            _uvs.Add(Vector2.zero); _uvs.Add(new Vector2(1, 0)); _uvs.Add(Vector2.one); _uvs.Add(new Vector2(0, 1));

            _tris.Add(pb); _tris.Add(pb + 1); _tris.Add(pb + 2); _tris.Add(pb); _tris.Add(pb + 2); _tris.Add(pb + 3);
            _tris.Add(pb); _tris.Add(pb + 2); _tris.Add(pb + 1); _tris.Add(pb); _tris.Add(pb + 3); _tris.Add(pb + 2);
        }
    }

    void EmitCup(Vector3 center, Vector3 axis, Vector3 right, Vector3 up2, float size)
    {
        float discR = size * 0.18f, pLen = size, pW = size * 0.32f;
        int db = _verts.Count;
        _verts.Add(center); _norms.Add(axis); _uvs.Add(new Vector2(0.5f, 0.5f));
        for (int p = 0; p < 6; p++)
        {
            float a = p / 6f * Mathf.PI * 2f;
            _verts.Add(center + (Mathf.Cos(a) * right + Mathf.Sin(a) * up2) * discR);
            _norms.Add(axis); _uvs.Add(new Vector2(0.5f, 0.5f));
        }
        for (int p = 0; p < 6; p++) { _tris.Add(db); _tris.Add(db + 1 + p); _tris.Add(db + 1 + (p + 1) % 6); }

        for (int p = 0; p < 6; p++)
        {
            float a = p / 6f * Mathf.PI * 2f;
            Vector3 pd = (Mathf.Cos(a) * right + Mathf.Sin(a) * up2).normalized;
            Vector3 lat = Vector3.Cross(pd, axis).normalized;
            int vBase = _verts.Count;

            for (int ri = 0; ri <= 3; ri++)
            {
                float t = ri / 3f;
                float curl = Mathf.Sin(t * Mathf.PI) * 0.35f;
                Vector3 rc = center + pd * (discR + pLen * t) + axis * (pLen * t * (0.55f - curl));
                float w = pW * (1f - t * 0.28f);
                Vector3 n2 = (axis + pd * (1f - t * 0.7f)).normalized;
                _verts.Add(rc - lat * w); _norms.Add(n2); _uvs.Add(new Vector2(0f, t));
                _verts.Add(rc + lat * w); _norms.Add(n2); _uvs.Add(new Vector2(1f, t));
            }

            for (int ri = 0; ri < 3; ri++)
            {
                int i0 = vBase + ri * 2;
                _tris.Add(i0); _tris.Add(i0 + 1); _tris.Add(i0 + 2); _tris.Add(i0 + 1); _tris.Add(i0 + 3); _tris.Add(i0 + 2);
                _tris.Add(i0); _tris.Add(i0 + 2); _tris.Add(i0 + 1); _tris.Add(i0 + 1); _tris.Add(i0 + 2); _tris.Add(i0 + 3);
            }
        }
    }

    void EmitStar(Vector3 center, Vector3 axis, Vector3 right, Vector3 up2, float size)
    {
        float discR = size * 0.10f, pLen = size * 1.25f, pW = size * 0.09f;
        int db = _verts.Count;
        _verts.Add(center); _norms.Add(axis); _uvs.Add(new Vector2(0.5f, 0.5f));
        for (int p = 0; p < 8; p++)
        {
            float a = p / 8f * Mathf.PI * 2f;
            _verts.Add(center + (Mathf.Cos(a) * right + Mathf.Sin(a) * up2) * discR);
            _norms.Add(axis); _uvs.Add(new Vector2(0.5f + 0.25f * Mathf.Cos(a), 0.5f + 0.25f * Mathf.Sin(a)));
        }
        for (int p = 0; p < 8; p++) { _tris.Add(db); _tris.Add(db + 1 + p); _tris.Add(db + 1 + (p + 1) % 8); }

        for (int p = 0; p < 8; p++)
        {
            float a = p / 8f * Mathf.PI * 2f;
            Vector3 pd = (Mathf.Cos(a) * right + Mathf.Sin(a) * up2).normalized;
            Vector3 lat = Vector3.Cross(pd, axis).normalized;
            int pb = _verts.Count;
            _verts.Add(center + pd * discR - lat * pW); _norms.Add(axis); _uvs.Add(new Vector2(0f, 0f));
            _verts.Add(center + pd * discR + lat * pW); _norms.Add(axis); _uvs.Add(new Vector2(1f, 0f));
            _verts.Add(center + pd * pLen); _norms.Add(axis); _uvs.Add(new Vector2(0.5f, 1f));
            _tris.Add(pb); _tris.Add(pb + 1); _tris.Add(pb + 2); _tris.Add(pb); _tris.Add(pb + 2); _tris.Add(pb + 1);
        }
    }

    void EmitCordate(Vector3 pos, Vector3 fwd, Vector3 right, Vector3 n, float size)
    {
        float h = size * 1.2f;
        float w = size * 0.75f;
        Vector3 pBase = pos;
        Vector3 pMidL = pos + fwd * (h * 0.35f) - right * (w * 0.5f);
        Vector3 pMidR = pos + fwd * (h * 0.35f) + right * (w * 0.5f);
        Vector3 pTip  = pos + fwd * h;

        EmitDoubleSidedQuad(pBase, pMidR, pTip, pMidL, n);
    }

    void EmitBell(Vector3 center, Vector3 axis, Vector3 right, Vector3 up2, float size)
    {
        int lobes = 5;
        int rings = 4;
        float bellLength = size * 1.1f;
        float baseRadius = size * 0.12f;
        float rimRadius  = size * 0.55f;

        int vBase = _verts.Count;
        for (int r = 0; r <= rings; r++)
        {
            float t = (float)r / rings;
            float rad = Mathf.Lerp(baseRadius, rimRadius, Mathf.Pow(t, 1.8f));
            float dist = t * bellLength;

            for (int i = 0; i < lobes; i++)
            {
                float angle = i / (float)lobes * Mathf.PI * 2f;
                float lobeOffset = (r == rings) ? Mathf.Sin(angle * lobes) * 0.08f * size : 0f;
                Vector3 dir = (Mathf.Cos(angle) * right + Mathf.Sin(angle) * up2).normalized;
                Vector3 pos = center + axis * dist + dir * (rad + lobeOffset);
                Vector3 norm = (dir + axis * 0.3f).normalized;

                _verts.Add(pos);
                _norms.Add(norm);
                _uvs.Add(new Vector2((float)i / lobes, t));
            }
        }

        for (int r = 0; r < rings; r++)
        {
            for (int i = 0; i < lobes; i++)
            {
                int nextI = (i + 1) % lobes;
                int i0 = vBase + r * lobes + i;
                int i1 = vBase + r * lobes + nextI;
                int i2 = vBase + (r + 1) * lobes + i;
                int i3 = vBase + (r + 1) * lobes + nextI;

                _tris.Add(i0); _tris.Add(i2); _tris.Add(i1);
                _tris.Add(i1); _tris.Add(i2); _tris.Add(i3);
                _tris.Add(i0); _tris.Add(i1); _tris.Add(i2);
                _tris.Add(i1); _tris.Add(i3); _tris.Add(i2);
            }
        }
    }

    void EmitFiveRayFlowerHead(Vector3 center, Vector3 axis, Vector3 right, Vector3 up2, float size)
    {
        float discR = size * 0.12f;
        float rayLen = size * 0.9f;
        float rayWidth = size * 0.35f;

        int db = _verts.Count;
        _verts.Add(center); _norms.Add(axis); _uvs.Add(new Vector2(0.5f, 0.5f));
        for (int p = 0; p < 5; p++)
        {
            float a = p / 5f * Mathf.PI * 2f;
            Vector3 d = Mathf.Cos(a) * right + Mathf.Sin(a) * up2;
            _verts.Add(center + d * discR); _norms.Add(axis); _uvs.Add(new Vector2(0.5f + 0.25f * Mathf.Cos(a), 0.5f + 0.25f * Mathf.Sin(a)));
        }
        for (int p = 0; p < 5; p++) { _tris.Add(db); _tris.Add(db + 1 + p); _tris.Add(db + 1 + (p + 1) % 5); }

        for (int p = 0; p < 5; p++)
        {
            float a = p / 5f * Mathf.PI * 2f;
            Vector3 pd = (Mathf.Cos(a) * right + Mathf.Sin(a) * up2).normalized;
            Vector3 lat = Vector3.Cross(pd, axis).normalized;
            Vector3 rC = center + pd * discR;
            Vector3 tC = center + pd * rayLen + axis * (rayLen * 0.15f);
            Vector3 pn = (axis + pd * 0.2f).normalized;

            int pb = _verts.Count;
            _verts.Add(rC - lat * (rayWidth * 0.4f));
            _verts.Add(rC + lat * (rayWidth * 0.4f));
            _verts.Add(tC + lat * rayWidth);
            _verts.Add(tC - lat * rayWidth);

            for (int k = 0; k < 4; k++) _norms.Add(pn);
            _uvs.Add(Vector2.zero); _uvs.Add(new Vector2(1, 0)); _uvs.Add(Vector2.one); _uvs.Add(new Vector2(0, 1));

            _tris.Add(pb); _tris.Add(pb + 1); _tris.Add(pb + 2); _tris.Add(pb); _tris.Add(pb + 2); _tris.Add(pb + 3);
            _tris.Add(pb); _tris.Add(pb + 2); _tris.Add(pb + 1); _tris.Add(pb); _tris.Add(pb + 3); _tris.Add(pb + 2);
        }
    }
}