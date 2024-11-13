#include <Noise.hpp>

// Consctructeur

void Noise::init(){
    
    noiseType = 0; 
    scale = 1.0f;
    gain = 1.0f; 
    octaves = 4; 
    persistence = 2.0f; 
    power = 1.0f; 

    int major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    if (major > 4 || (major == 4 && minor >= 3)){
        useComputeShader = true;
    }
    else {
        useComputeShader = false;
    }

}

// 

