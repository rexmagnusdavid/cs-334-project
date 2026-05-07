#include "Terrain.h"

#include <GL/glew.h>
#include <vector>

Terrain::Terrain(int grid_size, float spacing) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (int z = 0; z <= grid_size; ++z) {
        for (int x = 0; x <= grid_size; ++x) {
            vertices.push_back(x * spacing);
            vertices.push_back(z * spacing);
        }
    }

    for (int z = 0; z < grid_size; ++z) {
        for (int x = 0; x < grid_size; ++x) {
            unsigned int top_left  = z * (grid_size + 1) + x;
            unsigned int top_right = top_left + 1;
            unsigned int bot_left  = top_left + (grid_size + 1);
            unsigned int bot_right = bot_left + 1;

            indices.push_back(top_left);
            indices.push_back(bot_left);
            indices.push_back(top_right);
            indices.push_back(top_right);
            indices.push_back(bot_left);
            indices.push_back(bot_right);
        }
    }

    index_count = static_cast<int>(indices.size());

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Terrain::~Terrain() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}

void Terrain::draw() const {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}
