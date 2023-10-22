#pragma once
#include <tga/tga.hpp>
#include <tga/tga_math.hpp>



struct CamData{
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
};

struct CamMetaData{
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 lookDirection;
    alignas(16) glm::vec3 fovNearFar;

};

class CameraController
{
    public:
    CameraController(tga::Interface& _tgai, tga::Window _window, 
        float _fov,float _aspectRatio, float _nearPlane, float _farPlane,
        glm::vec3 _position, glm::vec3 _front, glm::vec3 _up);
    void update(float deltaTime);

    tga::StagingBuffer& Data();
    tga::StagingBuffer& MetaData();
    glm::vec3& Position();

    float speed = 4.;
    float speedBoost = 8;
    float turnSpeed = 75;

    private:

    void processInput(float dt);
    void updateData();

    tga::Interface& tgai;
    tga::Window window;
    tga::StagingBuffer camStaging, camMetaStaging;
    CamData* camData;
    CamMetaData* camMetaData;
    float fov, aspectRatio, nearPlane, farPlane;
    glm::vec3 position, front, up, right, lookDir;
    float pitch = 0 , yaw = 0;
    float lastMouseX = 0, lastMouseY = 0;
    float mouseSensitivity = 1;
    
};