#pragma once

#include <GL/glew.h>

class Structure {
public:
    Structure();
    ~Structure();

    void draw() const;

private:
    GLuint vao, vbo, ebo;
    int    index_count;
};
