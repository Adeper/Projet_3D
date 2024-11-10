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

#include "../include/shader.hpp"

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open()){
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}else{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	
	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);
	
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

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




