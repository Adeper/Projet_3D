#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
using namespace std;

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include <shader.hpp>
GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path, const char* geometry_file_path) {
    // Créez les shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint GeometryShaderID = 0;

    // Fonction pour lire et compiler un shader
    auto compileShader = [](const char* filePath, GLuint shaderID) {
        std::string ShaderCode;
        std::ifstream ShaderStream(filePath, std::ios::in);

        if (ShaderStream.is_open()) {
            std::stringstream sstr;
            sstr << ShaderStream.rdbuf();
            ShaderCode = sstr.str();
            ShaderStream.close();
        } else {
            std::cerr << "Impossible d'ouvrir le fichier " << filePath << std::endl;
            return false;
        }

        const char* ShaderCodePtr = ShaderCode.c_str();
        glShaderSource(shaderID, 1, &ShaderCodePtr, nullptr);
        glCompileShader(shaderID);

        // Vérification des erreurs de compilation
        GLint Result = GL_FALSE;
        int InfoLogLength;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if (InfoLogLength > 0) {
            std::vector<char> ShaderErrorMessage(InfoLogLength + 1);
            glGetShaderInfoLog(shaderID, InfoLogLength, nullptr, &ShaderErrorMessage[0]);
            std::cerr << &ShaderErrorMessage[0] << std::endl;
        }
        return Result == GL_TRUE;
    };

    // Compilez les shaders
    if (!compileShader(vertex_file_path, VertexShaderID)) return 0;
    if (!compileShader(fragment_file_path, FragmentShaderID)) return 0;

    // Si un fichier de shader géométrique est fourni, le compiler
    if (geometry_file_path) {
        GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
        if (!compileShader(geometry_file_path, GeometryShaderID)) return 0;
    }

    // Liez le programme
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    if (geometry_file_path) {
        glAttachShader(ProgramID, GeometryShaderID);
    }
    glLinkProgram(ProgramID);

    // Vérifiez les erreurs de linkage
    GLint Result = GL_FALSE;
    int InfoLogLength;
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);
        std::cerr << &ProgramErrorMessage[0] << std::endl;
    }

    // Nettoyez les shaders
    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
    if (geometry_file_path) {
        glDetachShader(ProgramID, GeometryShaderID);
        glDeleteShader(GeometryShaderID);
    }

    return ProgramID;
}

GLuint loadVertexShader(const char* shaderPath) {
    // Création du shader vertex
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    
    // Lecture du code du shader depuis le fichier
    std::string shaderCode;
    std::ifstream shaderStream(shaderPath, std::ios::in);
    if (!shaderStream.is_open()) {
        printf("Impossible d'ouvrir %s.\n", shaderPath);
        return 0;
    }

    std::stringstream sstr;
    sstr << shaderStream.rdbuf();
    shaderCode = sstr.str();
    shaderStream.close();

    // Compilation du shader vertex
    printf("Compilation du shader : %s\n", shaderPath);
    const char* sourcePointer = shaderCode.c_str();
    glShaderSource(vertexShaderID, 1, &sourcePointer, NULL);
    glCompileShader(vertexShaderID);

    // Vérification des erreurs de compilation
    GLint result = GL_FALSE;
    int infoLogLength;
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<char> shaderErrorMessage(infoLogLength + 1);
        glGetShaderInfoLog(vertexShaderID, infoLogLength, NULL, &shaderErrorMessage[0]);
        printf("%s\n", &shaderErrorMessage[0]);
    }

    if (result == GL_FALSE) {
        printf("Erreur de compilation du vertex shader.\n");
        glDeleteShader(vertexShaderID);
        return 0;
    }

    return vertexShaderID;
}


GLuint loadFragmentShader(const char* shaderPath) {
    // Création du shader fragment
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    // Lecture du code du shader depuis le fichier
    std::string shaderCode;
    std::ifstream shaderStream(shaderPath, std::ios::in);
    if (!shaderStream.is_open()) {
        printf("Impossible d'ouvrir %s.\n", shaderPath);
        return 0;
    }

    std::stringstream sstr;
    sstr << shaderStream.rdbuf();
    shaderCode = sstr.str();
    shaderStream.close();

    // Compilation du shader fragment
    printf("Compilation du shader : %s\n", shaderPath);
    const char* sourcePointer = shaderCode.c_str();
    glShaderSource(fragmentShaderID, 1, &sourcePointer, NULL);
    glCompileShader(fragmentShaderID);

    // Vérification des erreurs de compilation
    GLint result = GL_FALSE;
    int infoLogLength;
    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<char> shaderErrorMessage(infoLogLength + 1);
        glGetShaderInfoLog(fragmentShaderID, infoLogLength, NULL, &shaderErrorMessage[0]);
        printf("%s\n", &shaderErrorMessage[0]);
    }

    if (result == GL_FALSE) {
        printf("Erreur de compilation du fragment shader.\n");
        glDeleteShader(fragmentShaderID);
        return 0;
    }

    // Création du programme et attachement du shader
    printf("Linkage du programme\n");
    GLuint programID = glCreateProgram();
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    // Vérification des erreurs de linkage
    glGetProgramiv(programID, GL_LINK_STATUS, &result);
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<char> programErrorMessage(infoLogLength + 1);
        glGetProgramInfoLog(programID, infoLogLength, NULL, &programErrorMessage[0]);
        printf("%s\n", &programErrorMessage[0]);
    }

    if (result == GL_FALSE) {
        printf("Erreur de linkage du programme.\n");
        glDeleteProgram(programID);
        glDeleteShader(fragmentShaderID);
        return 0;
    }

    // Libérer le shader (il est déjà attaché au programme)
    glDetachShader(programID, fragmentShaderID);
    glDeleteShader(fragmentShaderID);

    return programID;
}

GLuint loadComputeShader(const char* shaderPath) {
    // Création du shader compute
    GLuint computeShaderID = glCreateShader(GL_COMPUTE_SHADER);
    
    // Lecture du code du shader depuis le fichier
    std::string shaderCode;
    std::ifstream shaderStream(shaderPath, std::ios::in);
    if (!shaderStream.is_open()) {
        printf("Impossible d'ouvrir %s.\n", shaderPath);
        return 0;
    }

    std::stringstream sstr;
    sstr << shaderStream.rdbuf();
    shaderCode = sstr.str();
    shaderStream.close();

    // Compilation du shader compute
    printf("Compilation du shader : %s\n", shaderPath);
    const char* sourcePointer = shaderCode.c_str();
    glShaderSource(computeShaderID, 1, &sourcePointer, NULL);
    glCompileShader(computeShaderID);

    // Vérification des erreurs de compilation
    GLint result = GL_FALSE;
    int infoLogLength;
    glGetShaderiv(computeShaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(computeShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<char> shaderErrorMessage(infoLogLength + 1);
        glGetShaderInfoLog(computeShaderID, infoLogLength, NULL, &shaderErrorMessage[0]);
        printf("%s\n", &shaderErrorMessage[0]);
    }

    if (result == GL_FALSE) {
        printf("Erreur de compilation du compute shader.\n");
        glDeleteShader(computeShaderID);
        return 0;
    }

    // Création du programme et attachement du shader
    printf("Linkage du programme\n");
    GLuint programID = glCreateProgram();
    glAttachShader(programID, computeShaderID);
    glLinkProgram(programID);

    // Vérification des erreurs de linkage
    glGetProgramiv(programID, GL_LINK_STATUS, &result);
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<char> programErrorMessage(infoLogLength + 1);
        glGetProgramInfoLog(programID, infoLogLength, NULL, &programErrorMessage[0]);
        printf("%s\n", &programErrorMessage[0]);
    }

    if (result == GL_FALSE) {
        printf("Erreur de linkage du programme.\n");
        glDeleteProgram(programID);
        glDeleteShader(computeShaderID);
        return 0;
    }

    // Libérer le shader (il est déjà attaché au programme)
    glDetachShader(programID, computeShaderID);
    glDeleteShader(computeShaderID);

    return programID;
}




