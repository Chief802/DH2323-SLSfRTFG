using UnityEngine;
using UnityEditor;

[CustomEditor(typeof(PlantRenderer))]
public class PlantRendererEditor : Editor
{
    SerializedProperty treeType;
    SerializedProperty plantSettings;

    void OnEnable()
    {
        treeType = serializedObject.FindProperty("treeType");
        plantSettings = serializedObject.FindProperty("plantSettings");
    }

    // Helper method to keep the code clean and preserve tooltips
    private void DrawParam(string relativeName, string label = null)
    {
        SerializedProperty prop = plantSettings.FindPropertyRelative(relativeName);
        if (prop != null)
        {
            if (string.IsNullOrEmpty(label))
            {
                // Uses the default variable name and the tooltip defined in the struct
                EditorGUILayout.PropertyField(prop);
            }
            else
            {
                // Overrides the display name but preserves the original tooltip from the struct
                EditorGUILayout.PropertyField(prop, new GUIContent(label, prop.tooltip));
            }
        }
    }

    public override void OnInspectorGUI()
    {
        serializedObject.Update();

        // 1. Draw top-level settings
        EditorGUILayout.LabelField("Core Generation", EditorStyles.boldLabel);
        EditorGUILayout.PropertyField(treeType);
        EditorGUILayout.PropertyField(serializedObject.FindProperty("iterations"));
        EditorGUILayout.PropertyField(serializedObject.FindProperty("seed"));

        EditorGUILayout.Space();
        EditorGUILayout.LabelField("Universal Engine Parameters", EditorStyles.boldLabel);

        // 2. Draw parameters used by the Turtle Interpreter for all plants
        DrawParam("baseRadius");
        DrawParam("radiusDecay");
        DrawParam("defaultStep");
        DrawParam("defaultAngleDeg");

        EditorGUILayout.Space();
        EditorGUILayout.LabelField("Plant-Specific Geometry", EditorStyles.boldLabel);

        // 3. Switch based on selected TreeType to show only relevant fields
        PlantRenderer.TreeType currentType = (PlantRenderer.TreeType)treeType.enumValueIndex;

        switch (currentType)
        {
            case PlantRenderer.TreeType.CapsellaBursaPastoris:
                DrawParam("branchAngle1", "Vegetative Branch Angle");
                DrawParam("branchAngle2", "Floral Branch Angle");
                DrawParam("divergenceAngle1", "Golden Angle");
                DrawParam("internodeLen1", "Vegetative Internode");
                DrawParam("internodeLen2", "Floral Internode");
                DrawParam("elongationRatio");
                DrawParam("pitchAngle", "Pedicel Droop Angle");
                DrawParam("leafSize");
                DrawParam("flowerSize");
                DrawParam("budSize");
                break;

            case PlantRenderer.TreeType.StochasticCapsellaBursaPastoris:
                DrawParam("probPrimary", "Primary Branching Prob");
                DrawParam("probSecondary", "Secondary Branching Prob");
                DrawParam("branchAngle1", "Primary Branch Angle");
                DrawParam("branchAngle2", "Secondary Branch Angle");
                DrawParam("divergenceAngle1", "Primary Divergence");
                DrawParam("divergenceAngle2", "Secondary Divergence");
                DrawParam("internodeLen1", "Primary Internode");
                DrawParam("internodeLen2", "Secondary Internode");
                DrawParam("elongationRatio");
                DrawParam("pitchAngle", "Pedicel Droop Angle");
                DrawParam("leafSize");
                DrawParam("flowerSize");
                DrawParam("budSize");
                break;

            case PlantRenderer.TreeType.ABOPTree:
                DrawParam("fatteningRatio", "Radius Fattening Ratio");
                DrawParam("elongationRatio", "Segment Elongation Ratio");
                DrawParam("internodeLen1", "Base Segment Length");
                DrawParam("branchAngle1", "Branching Angle");
                DrawParam("divergenceAngle1", "Divergence Angle 1");
                DrawParam("divergenceAngle2", "Divergence Angle 2");
                DrawParam("leafSize");
                DrawParam("flowerSize");
                break;

            case PlantRenderer.TreeType.MycelisMuralis:
                DrawParam("internodeLen1", "Axiom Length");
                DrawParam("branchAngle1", "Lateral Pitch Angle");
                DrawParam("divergenceAngle1", "Lateral Roll Angle");
                DrawParam("leafSize");
                break;

            case PlantRenderer.TreeType.StochasticMycelisMuralis3D:
                DrawParam("probPrimary", "Lateral Spawn Probability");
                DrawParam("probSecondary", "Growth Delay Probability");
                DrawParam("internodeLen1", "Axiom Length");
                DrawParam("branchAngle1", "Min Pitch Angle");
                DrawParam("branchAngle2", "Max Pitch Angle");
                DrawParam("divergenceAngle1", "Min Roll Angle");
                DrawParam("divergenceAngle2", "Max Roll Angle");
                DrawParam("leafSize");
                break;

            case PlantRenderer.TreeType.DHTwentyTreeTwentyTree:
                EditorGUILayout.PropertyField(serializedObject.FindProperty("vineText"), new GUIContent("Vine Text Input"));
                DrawParam("branchAngle1", "Flower Branch Pitch Angle");
                DrawParam("leafSize");
                DrawParam("flowerSize");
                break;
        }

        EditorGUILayout.Space();

        // 4. Draw the rest of the component (Materials, LODs, Animation, etc.)
        DrawPropertiesExcluding(serializedObject, "m_Script", "treeType", "iterations", "seed", "plantSettings");

        serializedObject.ApplyModifiedProperties();
    }
}