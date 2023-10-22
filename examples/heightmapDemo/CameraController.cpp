#include "CameraController.hpp"



    CameraController::CameraController(tga::Interface& _tgai, tga::Window _window, 
        float _fov,float _aspectRatio, float _nearPlane, float _farPlane,
        glm::vec3 _position, glm::vec3 _front, glm::vec3 _up):
            tgai(_tgai),window(_window),
            camStaging(tgai.createStagingBuffer({sizeof(*camData)})),
            camMetaStaging(tgai.createStagingBuffer({sizeof(*camMetaData)})),
            camData(static_cast<CamData*>(tgai.getMapping(camStaging))),
            camMetaData(static_cast<CamMetaData*>(tgai.getMapping(camMetaStaging))),
            fov(_fov),aspectRatio(_aspectRatio),
            nearPlane(_nearPlane),farPlane(_farPlane),
            position(_position),front(_front),up(_up),right(glm::cross(up,front))
        {
            updateData();
        }

    void CameraController::update(float deltaTime)
    {
        processInput(deltaTime);
        updateData();

    }

    tga::StagingBuffer& CameraController::Data() 
    {
        return camStaging;
    }
    tga::StagingBuffer& CameraController::MetaData() 
    {
        return camMetaStaging;
    }
    glm::vec3& CameraController::Position()
    {
        return position;
    }


    void CameraController::processInput(float dt)
    {
        float moveSpeed = speed;

        if(tgai.keyDown(window,tga::Key::R))
            moveSpeed *= speedBoost;


        if(tgai.keyDown(window,tga::Key::Left))
            yaw += dt*turnSpeed;
        if(tgai.keyDown(window,tga::Key::Right))
            yaw -= dt*turnSpeed;
        if(tgai.keyDown(window,tga::Key::Up))
            pitch += dt*turnSpeed;
        if(tgai.keyDown(window,tga::Key::Down))
            pitch -= dt*turnSpeed;

        pitch = std::clamp(pitch,-89.f,89.f);

        auto rot = glm::mat3_cast(glm::quat(glm::vec3(-glm::radians(pitch),glm::radians(yaw),0.f)));
        lookDir = rot*front;
        auto r = rot*glm::cross(up,front);



        if(tgai.keyDown(window,tga::Key::W))
            position += lookDir*dt*moveSpeed;
        if(tgai.keyDown(window,tga::Key::S))
            position -= lookDir*dt*moveSpeed;

        if(tgai.keyDown(window,tga::Key::A))
            position += r*dt*moveSpeed;
        if(tgai.keyDown(window,tga::Key::D))
            position -= r*dt*moveSpeed;
        
        if(tgai.keyDown(window,tga::Key::Space))
            position += up*dt*moveSpeed;
        if(tgai.keyDown(window,tga::Key::Shift_Left))
            position -= up*dt*moveSpeed;
    }
    void CameraController::updateData()
    {
        /*
        camData.projection = glm::perspective(glm::radians(fov),aspectRatio,nearPlane,farPlane);
        camData.projection[1][1] *= -1;
        */
        camData->projection = glm::perspective_vk(glm::radians(fov),aspectRatio,nearPlane,farPlane);
        camData->view = glm::lookAt(position,position+lookDir,up);

        camMetaData->position = position;
        camMetaData->lookDirection = lookDir;
        camMetaData->fovNearFar = glm::vec3(fov,nearPlane,farPlane);
    }
    