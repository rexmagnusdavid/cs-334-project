#pragma once

#include <glm/glm.hpp>

enum class MoveDirection {
    Forward,
    Backward,
    Left,
    Right,
    Up,
    Down
};

class Camera {
public:
    Camera(glm::vec3 position, float yaw, float pitch);

    glm::mat4 get_view_matrix() const;
    glm::vec3 get_position() const;

    void process_keyboard(MoveDirection direction, float delta_time);
    void process_mouse(float x_offset, float y_offset);

private:
    void update_vectors();

    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;

    float yaw;
    float pitch;
};
