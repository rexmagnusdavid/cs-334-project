#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

static const glm::vec3 WORLD_UP        = glm::vec3(0.0f, 1.0f, 0.0f);
static constexpr float MOVE_SPEED      = 80.0f;
static constexpr float SENSITIVITY     = 0.1f;
static constexpr float PITCH_LIMIT     = 89.0f;

Camera::Camera(glm::vec3 position, float yaw, float pitch)
    : position(position), yaw(yaw), pitch(pitch) {
    update_vectors();
}

glm::mat4 Camera::get_view_matrix() const {
    return glm::lookAt(position, position + front, up);
}

glm::vec3 Camera::get_position() const {
    return position;
}

void Camera::process_keyboard(MoveDirection direction, float delta_time) {
    float velocity = MOVE_SPEED * delta_time;
    if (direction == MoveDirection::Forward)  position += front    * velocity;
    if (direction == MoveDirection::Backward) position -= front    * velocity;
    if (direction == MoveDirection::Left)     position -= right    * velocity;
    if (direction == MoveDirection::Right)    position += right    * velocity;
    if (direction == MoveDirection::Up)       position += WORLD_UP * velocity;
    if (direction == MoveDirection::Down)     position -= WORLD_UP * velocity;
}

void Camera::process_mouse(float x_offset, float y_offset) {
    yaw   += x_offset * SENSITIVITY;
    pitch += y_offset * SENSITIVITY;
    if (pitch >  PITCH_LIMIT) pitch =  PITCH_LIMIT;
    if (pitch < -PITCH_LIMIT) pitch = -PITCH_LIMIT;
    update_vectors();
}

void Camera::update_vectors() {
    glm::vec3 new_front;
    new_front.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    new_front.y = std::sin(glm::radians(pitch));
    new_front.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    front = glm::normalize(new_front);
    right = glm::normalize(glm::cross(front, WORLD_UP));
    up    = glm::normalize(glm::cross(right, front));
}
