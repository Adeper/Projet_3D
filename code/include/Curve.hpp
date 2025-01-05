#ifndef CURVE_HPP
#define CURVE_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <PlaneLOD.hpp>
#include <Noise.hpp>
#include <string>
#include <vector>

// Include GLM
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class Curve {
public:
    enum CurveType {
        BEZIER,
        CATMULL_ROM,
        APPROXIMATION
    };

    Curve(PlaneLOD* terrain, Noise* noise);
    ~Curve();

    // void initControlPoints(const glm::vec3& startPoint, const glm::vec3& endPoint, int nbControlPoints);
    void setCurveType(CurveType type);
    void update();
    void draw();
    void showImGuiInterface();

private:
    std::vector<glm::vec3> controlPoints;  // Points de contrôle
    std::vector<glm::vec3> curvePoints;    // Points calculés sur la courbe
    PlaneLOD* terrain;                     // Terrain pour les données de hauteur et autres attributs
    Noise* noise;                          // Bruit pour les textures
    GLuint VAO, VBO;                       // Buffers OpenGL
    GLuint shaderProgram;                  // Programme shader
    CurveType curveType;                   // Type de courbe sélectionné
    glm::vec3 color;                       // Couleur de la courbe
    float curveWidth;                      // Largeur de la courbe
    bool useTexture;                       // Utilisation d'une texture
    GLuint textureID;                      // Texture de la courbe
    float heightOffset;                    // Décalage pour éviter l'interpénétration

    void initControlPointsFromTerrain();

    // Méthodes spécifiques aux types de courbes
    void computeBezierCurve();
    void computeCatmullRomCurve();
    void computeApproximationCurve(const glm::vec3& startPoint, const glm::vec3& endPoint);

    // Helper pour appliquer la hauteur depuis le terrain
    void applyHeightToCurve();

    // Helper pour apliquer le bruit à la courbe
    void applyNoiseToCurve();

    glm::vec3 deCasteljau(float t) const;  // Algorithme pour Bézier
    glm::vec3 catmullRom(float t, const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) const;

    void reloadShaders();
    void loadTexture(const std::string& path);

    void addPointForWidth();
};

#endif
