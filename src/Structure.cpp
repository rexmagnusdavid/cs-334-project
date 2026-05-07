#include "Structure.h"

#include <GL/glew.h>
#include <vector>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

static void push_box(std::vector<Vertex>& verts,
                     std::vector<unsigned int>& idx,
                     float x0, float y0, float z0,
                     float x1, float y1, float z1) {
    struct Face { glm::vec3 n; glm::vec3 c[4]; };
    Face faces[6] = {
        { {  0,  0,  1 }, { {x0,y0,z1},{x1,y0,z1},{x1,y1,z1},{x0,y1,z1} } },
        { {  0,  0, -1 }, { {x1,y0,z0},{x0,y0,z0},{x0,y1,z0},{x1,y1,z0} } },
        { {  1,  0,  0 }, { {x1,y0,z1},{x1,y0,z0},{x1,y1,z0},{x1,y1,z1} } },
        { { -1,  0,  0 }, { {x0,y0,z0},{x0,y0,z1},{x0,y1,z1},{x0,y1,z0} } },
        { {  0,  1,  0 }, { {x0,y1,z1},{x1,y1,z1},{x1,y1,z0},{x0,y1,z0} } },
        { {  0, -1,  0 }, { {x0,y0,z0},{x1,y0,z0},{x1,y0,z1},{x0,y0,z1} } },
    };

    for (auto& f : faces) {
        unsigned int fi = static_cast<unsigned int>(verts.size());
        for (auto& c : f.c)
            verts.push_back({ c, f.n });
        idx.push_back(fi + 0); idx.push_back(fi + 1); idx.push_back(fi + 2);
        idx.push_back(fi + 0); idx.push_back(fi + 2); idx.push_back(fi + 3);
    }
}

Structure::Structure() {
    std::vector<Vertex>       verts;
    std::vector<unsigned int> indices;

    push_box(verts, indices, -4.0f, 0.0f, -4.0f,  4.0f,  6.0f,  4.0f);
    push_box(verts, indices, -2.0f, 6.0f, -2.0f,  2.0f, 36.0f,  2.0f);
    push_box(verts, indices, -3.0f, 36.0f, -3.0f, 3.0f, 40.0f,  3.0f);

    index_count = static_cast<int>(indices.size());

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Structure::~Structure() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}

void Structure::draw() const {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}
