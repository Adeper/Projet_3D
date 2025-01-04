#ifndef CURVE_HPP
#define CURVE_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <PlaneLOD.hpp>
#include <string>
#include <vector>

class Curve {
public:
    enum CurveType {
        BEZIER,
        CATMULL_ROM,
        APPROXIMATION
    };

    Curve(PlaneLOD* terrain);
    ~Curve();

    void initControlPoints(const glm::vec3& startPoint, const glm::vec3& endPoint, int nbControlPoints);
    void setCurveType(CurveType type);
    void update();
    void draw();

private:
    std::vector<glm::vec3> controlPoints;  // Points de contrôle
    std::vector<glm::vec3> curvePoints;    // Points calculés sur la courbe
    PlaneLOD* terrain;                     // Terrain pour les données de hauteur et autres attributs
    GLuint VAO, VBO;                       // Buffers OpenGL
    GLuint shaderProgram;                  // Programme shader
    CurveType curveType;                   // Type de courbe sélectionné

    // Méthodes spécifiques aux types de courbes
    void computeBezierCurve();
    void computeCatmullRomCurve();
    void computeApproximationCurve(const glm::vec3& startPoint, const glm::vec3& endPoint);

    // Helper pour appliquer la hauteur depuis le terrain
    void applyHeightToCurve();

    glm::vec3 deCasteljau(float t) const;  // Algorithme pour Bézier
    glm::vec3 catmullRom(float t, const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) const;
};

#endif
