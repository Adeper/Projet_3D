#ifndef CURVE_HPP
#define CURVE_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <PlaneLOD.hpp>
#include <Noise.hpp>
#include <string>
#include <vector>
#include <limits>

// Include GLM
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// structure node pour A*
struct Node {
    glm::vec3 position;
    float gScore;
    float hScore;
    float fScore;
    Node* parent;

    static float stepSize;

    Node(const glm::vec3& pos)
        : position(pos), gScore(std::numeric_limits<float>::infinity()),
          hScore(std::numeric_limits<float>::infinity()),
          fScore(std::numeric_limits<float>::infinity()),
          parent(nullptr) {}

    bool operator<(const Node& other) const {
        return fScore > other.fScore;
    }

    static void setStepSize(float newStepSize){
        stepSize = newStepSize;
    }

    static float getStepSize(){
        return stepSize;
    }
};

class Curve {
public:
    enum CurveType {
        CATMULL_ROM,
        ASTAR
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
    glm::vec3 startPoint;                  // Point de debut de la courbe
    glm::vec3 endPoint;                    // Point de fin de la courbe
    int iterationGradiant;                 // Nb d'iteration pour le calcule de la courbe
    int nbControlPoints;                   // nombre de points de controle
    PlaneLOD* terrain;                     // Terrain pour les données de hauteur et autres attributs
    Noise* noise;                          // Bruit pour les textures
    GLuint VAO, VBO;                       // Buffers OpenGL
    GLuint shaderProgram;                  // Programme shader
    CurveType curveType;                   // Type de courbe sélectionné
    glm::vec3 color;                       // Couleur de la courbe
    float curveWidth;                      // Largeur de la courbe
    bool showControlPoints;
    GLuint sphereVAO;                      // VAO pour la sphère
    GLuint sphereVBO;                      // VBO pour la sphère
    std::vector<glm::vec3> sphereVertices; // Contient les sommets de la sphère
    std::vector<unsigned short> sphereFaces;
    bool sphereLoaded;                     // Booléen pour vérifier si la sphère est chargée
    bool useTexture;                       // Utilisation d'une texture
    bool showRoad;
    GLuint textureID;                      // Texture de la courbe
    float heightOffset;                    // Décalage pour éviter l'interpénétration
    float heightWeight;
    

    void initControlPoints();

    void updateStartEndPoints();
    void updateControlPoints();
    void adjustControlPoints();
    void loadSphere(const std::string& filePath);
    void drawControlPoints();
    void drawRoad();

    // catmullRom fonctions
    void computeCatmullRomCurve();
    glm::vec3 catmullRom(float t, const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) const;

    // A* fonctions
    float heuristic(const glm::vec3& current, const glm::vec3& goal);
    void computeAStar();
    std::vector<glm::vec3> getNeighbors(const glm::vec3& position);
    std::vector<glm::vec3> reconstructPath(Node* goalNode);

    // Helper pour appliquer la hauteur depuis le terrain
    void applyHeightToCurve();
    float getHeightForDrawAtCoord(const float x, const float z);

    // Helper pour apliquer le bruit à la courbe
    void applyNoiseToCurve();


    void reloadShaders();
    void loadTexture(const std::string& path);

    // pour imgui
    float ImVec2DistanceSqr(const ImVec2& a, const ImVec2& b);
    void Draw2DSliderWithMultiplePoints(const char* label, ImVec2& point1, ImVec2& point2, const ImVec2& size, const ImVec2& min, const ImVec2& max);
};

#endif
