#pragma once

class Terrain {
public:
    Terrain(int grid_size, float spacing);
    ~Terrain();

    Terrain(const Terrain&) = delete;
    Terrain& operator=(const Terrain&) = delete;

    void draw() const;

private:
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;
    int index_count;
};
