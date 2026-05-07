#include "Camera.h"
#include "Terrain.h"
#include "Shader.h"
#include "Structure.h"
#include "noise.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <set>
#include <map>
#include <optional>
#include <vector>
#include <cmath>

#include <glm/gtc/constants.hpp>

static constexpr int   SCR_WIDTH        = 1280;
static constexpr int   SCR_HEIGHT       = 720;
static constexpr float FOV              = 60.0f;
static constexpr float NEAR_PLANE       = 0.5f;
static constexpr float FAR_PLANE        = 2000.0f;
static constexpr int   CHUNK_QUADS      = 64;
static constexpr float CHUNK_SPACING    = 2.0f;
static constexpr float CHUNK_SIZE       = CHUNK_QUADS * CHUNK_SPACING;
static constexpr int   RENDER_DISTANCE  = 8;
static constexpr float ZONE_SIZE        = 512.0f;
static constexpr float CLOUD_ZONE_SIZE  = 700.0f;
static constexpr float CLOUD_ALTITUDE   = 350.0f;
static constexpr float SUN_SIZE         = 130.0f;
static constexpr float CLOUD_SIZE       = CLOUD_ZONE_SIZE;
static constexpr int   SHADOW_RES       = 2048;
static constexpr float SHADOW_RANGE     = 1300.0f;

struct AppState {
    Camera* camera;
    int     width          = SCR_WIDTH;
    int     height         = SCR_HEIGHT;
    float   last_x         = SCR_WIDTH  * 0.5f;
    float   last_y         = SCR_HEIGHT * 0.5f;
    bool    first_mouse    = true;
    bool    mouse_dragging = false;
    bool    wireframe      = false;
};

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    auto* s = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    s->width  = width;
    s->height = height;
    glViewport(0, 0, width, height);
}

static void key_callback(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    auto* s = static_cast<AppState*>(glfwGetWindowUserPointer(window));

    if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, true);
    if (key == GLFW_KEY_TAB) {
        s->wireframe = !s->wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, s->wireframe ? GL_LINE : GL_FILL);
    }
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int) {
    auto* s = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    if (action == GLFW_PRESS) {
        s->mouse_dragging = true;
        s->first_mouse    = true;
    } else if (action == GLFW_RELEASE) {
        s->mouse_dragging = false;
    }
}

static void cursor_pos_callback(GLFWwindow* window, double x, double y) {
    auto* s = static_cast<AppState*>(glfwGetWindowUserPointer(window));
    if (!s->mouse_dragging) return;

    float fx = static_cast<float>(x);
    float fy = static_cast<float>(y);

    if (s->first_mouse) {
        s->last_x      = fx;
        s->last_y      = fy;
        s->first_mouse = false;
    }

    float x_offset =  (fx - s->last_x);
    float y_offset = -(fy - s->last_y);
    s->last_x = fx;
    s->last_y = fy;

    s->camera->process_mouse(x_offset, y_offset);
}

static void process_movement(GLFWwindow* window, Camera& camera, float dt) {
    if (glfwGetKey(window, GLFW_KEY_W)          == GLFW_PRESS) camera.process_keyboard(MoveDirection::Forward,  dt);
    if (glfwGetKey(window, GLFW_KEY_S)          == GLFW_PRESS) camera.process_keyboard(MoveDirection::Backward, dt);
    if (glfwGetKey(window, GLFW_KEY_A)          == GLFW_PRESS) camera.process_keyboard(MoveDirection::Left,     dt);
    if (glfwGetKey(window, GLFW_KEY_D)          == GLFW_PRESS) camera.process_keyboard(MoveDirection::Right,    dt);
    if (glfwGetKey(window, GLFW_KEY_SPACE)      == GLFW_PRESS) camera.process_keyboard(MoveDirection::Up,       dt);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camera.process_keyboard(MoveDirection::Down,     dt);
}

static float zone_hash(float zx, float zz) {
    return std::fmod(std::abs(std::sin(zx * 127.1f + zz * 311.7f) * 43758.5f), 1.0f);
}

static glm::vec2 zone_candidate(int zx, int zz) {
    float ox = std::fmod(std::abs(std::sin(zx * 127.1f + zz * 311.7f) * 43758.5f), 1.0f);
    float oz = std::fmod(std::abs(std::sin(zx * 269.5f + zz * 183.3f) * 43758.5f), 1.0f);
    return glm::vec2((zx + ox) * ZONE_SIZE, (zz + oz) * ZONE_SIZE);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Minetale", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return -1;
    }

    Camera camera(glm::vec3(0.0f, 300.0f, 0.0f), -90.0f, -50.0f);

    AppState state;
    state.camera = &camera;

    glfwSetWindowUserPointer(window, &state);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    Terrain   terrain(CHUNK_QUADS, CHUNK_SPACING);
    Shader    shader("src/shaders/terrain.vert", "src/shaders/terrain.frag");
    Structure structure;
    Shader    structure_shader("src/shaders/structure.vert", "src/shaders/structure.frag");
    Shader    sun_shader("src/shaders/sun.vert", "src/shaders/sun.frag");
    Shader    cloud_shader("src/shaders/cloud.vert", "src/shaders/cloud.frag");
    Shader    shadow_terrain_shader("src/shaders/shadow_terrain.vert", "src/shaders/shadow.frag");
    Shader    shadow_structure_shader("src/shaders/shadow_structure.vert", "src/shaders/shadow.frag");
    Shader    occlusion_terrain_shader("src/shaders/shadow_terrain.vert", "src/shaders/occlusion.frag");
    Shader    occlusion_structure_shader("src/shaders/shadow_structure.vert", "src/shaders/occlusion.frag");
    Shader    godray_shader("src/shaders/godray.vert", "src/shaders/godray.frag");

    GLuint shadow_fbo, shadow_depth_tex;
    glGenFramebuffers(1, &shadow_fbo);
    glGenTextures(1, &shadow_depth_tex);
    glBindTexture(GL_TEXTURE_2D, shadow_depth_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_RES, SHADOW_RES,
                 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float border_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_depth_tex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    static constexpr int OCC_W = SCR_WIDTH  / 2;
    static constexpr int OCC_H = SCR_HEIGHT / 2;
    GLuint occ_fbo, occ_color_tex, occ_depth_rb;
    glGenFramebuffers(1, &occ_fbo);
    glGenTextures(1, &occ_color_tex);
    glBindTexture(GL_TEXTURE_2D, occ_color_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, OCC_W, OCC_H, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenRenderbuffers(1, &occ_depth_rb);
    glBindRenderbuffer(GL_RENDERBUFFER, occ_depth_rb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, OCC_W, OCC_H);
    glBindFramebuffer(GL_FRAMEBUFFER, occ_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, occ_color_tex, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, occ_depth_rb);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    float quad_verts[] = { -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  -0.5f,  0.5f };
    unsigned int quad_idx[] = { 0, 1, 2, 0, 2, 3 };

    GLuint quad_vao, quad_vbo, quad_ebo;
    glGenVertexArrays(1, &quad_vao);
    glGenBuffers(1, &quad_vbo);
    glGenBuffers(1, &quad_ebo);
    glBindVertexArray(quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_verts), quad_verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_idx), quad_idx, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    std::vector<glm::vec3>    sphere_verts;
    std::vector<unsigned int> sphere_idx;
    {
        const int stacks = 20, slices = 20;
        for (int i = 0; i <= stacks; ++i) {
            float phi = glm::pi<float>() * i / stacks;
            for (int j = 0; j <= slices; ++j) {
                float theta = 2.0f * glm::pi<float>() * j / slices;
                sphere_verts.push_back({
                    std::sin(phi) * std::cos(theta),
                    std::cos(phi),
                    std::sin(phi) * std::sin(theta)
                });
            }
        }
        for (int i = 0; i < stacks; ++i) {
            for (int j = 0; j < slices; ++j) {
                unsigned int a = i * (slices + 1) + j;
                unsigned int b = a + 1;
                unsigned int c = a + (slices + 1);
                unsigned int d = c + 1;
                sphere_idx.push_back(a); sphere_idx.push_back(c); sphere_idx.push_back(b);
                sphere_idx.push_back(b); sphere_idx.push_back(c); sphere_idx.push_back(d);
            }
        }
    }
    int    sphere_index_count;
    GLuint sphere_vao, sphere_vbo, sphere_ebo;
    sphere_index_count = static_cast<int>(sphere_idx.size());
    glGenVertexArrays(1, &sphere_vao);
    glGenBuffers(1, &sphere_vbo);
    glGenBuffers(1, &sphere_ebo);
    glBindVertexArray(sphere_vao);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo);
    glBufferData(GL_ARRAY_BUFFER, sphere_verts.size() * sizeof(glm::vec3), sphere_verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere_idx.size() * sizeof(unsigned int), sphere_idx.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float frequency = 0.02f;
    float amplitude = 60.0f;
    int   octaves   = 6;

    glm::vec3 light_dir = glm::normalize(glm::vec3(0.6f, -1.0f, 0.4f));

    std::set<std::pair<int,int>>                             loaded_chunks;
    std::map<std::pair<int,int>, std::optional<glm::vec3>>  tower_zones;
    std::map<std::pair<int,int>, bool>                       cloud_zones;

    float last_frame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float now        = static_cast<float>(glfwGetTime());
        float delta_time = now - last_frame;
        last_frame       = now;

        glfwPollEvents();
        process_movement(window, camera, delta_time);

        glm::vec3 cam_pos = camera.get_position();

        int cam_cx = static_cast<int>(std::floor(cam_pos.x / CHUNK_SIZE));
        int cam_cz = static_cast<int>(std::floor(cam_pos.z / CHUNK_SIZE));

        for (int dx = -RENDER_DISTANCE; dx <= RENDER_DISTANCE; ++dx)
            for (int dz = -RENDER_DISTANCE; dz <= RENDER_DISTANCE; ++dz)
                loaded_chunks.insert({cam_cx + dx, cam_cz + dz});

        int zone_radius = static_cast<int>(std::ceil(RENDER_DISTANCE * CHUNK_SIZE / ZONE_SIZE)) + 1;
        int cam_zx = static_cast<int>(std::floor(cam_pos.x / ZONE_SIZE));
        int cam_zz = static_cast<int>(std::floor(cam_pos.z / ZONE_SIZE));

        for (int dx = -zone_radius; dx <= zone_radius; ++dx) {
            for (int dz = -zone_radius; dz <= zone_radius; ++dz) {
                auto key = std::make_pair(cam_zx + dx, cam_zz + dz);
                if (tower_zones.count(key)) continue;
                glm::vec2 cand = zone_candidate(key.first, key.second);
                float h = zone_hash(static_cast<float>(key.first), static_cast<float>(key.second));
                if (h > 0.18f || continent_mask(cand) < 0.05f) {
                    tower_zones[key] = std::nullopt;
                } else {
                    float y = terrain_h(cand, octaves, frequency, amplitude);
                    tower_zones[key] = glm::vec3(cand.x, y, cand.y);
                }
            }
        }

        int cloud_cam_zx  = static_cast<int>(std::floor(cam_pos.x / CLOUD_ZONE_SIZE));
        int cloud_cam_zz  = static_cast<int>(std::floor(cam_pos.z / CLOUD_ZONE_SIZE));
        int cloud_radius  = static_cast<int>(std::ceil(RENDER_DISTANCE * CHUNK_SIZE / CLOUD_ZONE_SIZE)) + 2;

        for (int dx = -cloud_radius; dx <= cloud_radius; ++dx) {
            for (int dz = -cloud_radius; dz <= cloud_radius; ++dz) {
                auto key = std::make_pair(cloud_cam_zx + dx, cloud_cam_zz + dz);
                if (cloud_zones.count(key)) continue;
                cloud_zones[key] = true;
            }
        }

        glm::vec3 light_target  = glm::vec3(cam_pos.x, 0.0f, cam_pos.z);
        glm::vec3 light_pos     = light_target - light_dir * 1500.0f;
        glm::mat4 light_view    = glm::lookAt(light_pos, light_target, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 light_proj    = glm::ortho(-SHADOW_RANGE, SHADOW_RANGE,
                                             -SHADOW_RANGE, SHADOW_RANGE, 1.0f, 4000.0f);
        glm::mat4 light_space   = light_proj * light_view;

        glViewport(0, 0, SHADOW_RES, SHADOW_RES);
        glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(2.0f, 4.0f);

        shadow_terrain_shader.use();
        shadow_terrain_shader.set_mat4 ("light_space_matrix", light_space);
        shadow_terrain_shader.set_float("frequency",          frequency);
        shadow_terrain_shader.set_float("amplitude",          amplitude);
        shadow_terrain_shader.set_int  ("octaves",            octaves);
        for (auto& [cx, cz] : loaded_chunks) {
            shadow_terrain_shader.set_vec2("world_offset", glm::vec2(cx * CHUNK_SIZE, cz * CHUNK_SIZE));
            terrain.draw();
        }

        shadow_structure_shader.use();
        shadow_structure_shader.set_mat4("light_space_matrix", light_space);
        for (auto& [zone, opt_pos] : tower_zones) {
            if (!opt_pos) continue;
            int tcx = static_cast<int>(std::floor(opt_pos->x / CHUNK_SIZE));
            int tcz = static_cast<int>(std::floor(opt_pos->z / CHUNK_SIZE));
            if (!loaded_chunks.count({tcx, tcz})) continue;
            glm::mat4 model = glm::translate(glm::mat4(1.0f), *opt_pos);
            shadow_structure_shader.set_mat4("model", model);
            structure.draw();
        }

        glDisable(GL_POLYGON_OFFSET_FILL);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glm::mat4 projection = glm::perspective(
            glm::radians(FOV),
            static_cast<float>(state.width) / static_cast<float>(state.height),
            NEAR_PLANE, FAR_PLANE);
        glm::mat4 view       = camera.get_view_matrix();
        glm::mat4 pv         = projection * view;

        glm::vec3 sun_center = cam_pos + glm::normalize(-light_dir) * (FAR_PLANE * 0.9f);
        glm::vec4 sun_clip   = pv * glm::vec4(sun_center, 1.0f);
        glm::vec2 sun_screen(
            sun_clip.x / sun_clip.w * 0.5f + 0.5f,
            sun_clip.y / sun_clip.w * 0.5f + 0.5f);
        bool sun_in_front = sun_clip.w > 0.0f;

        glViewport(0, 0, OCC_W, OCC_H);
        glBindFramebuffer(GL_FRAMEBUFFER, occ_fbo);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        occlusion_terrain_shader.use();
        occlusion_terrain_shader.set_mat4 ("light_space_matrix", pv);
        occlusion_terrain_shader.set_float("frequency",          frequency);
        occlusion_terrain_shader.set_float("amplitude",          amplitude);
        occlusion_terrain_shader.set_int  ("octaves",            octaves);
        for (auto& [cx, cz] : loaded_chunks) {
            occlusion_terrain_shader.set_vec2("world_offset", glm::vec2(cx * CHUNK_SIZE, cz * CHUNK_SIZE));
            terrain.draw();
        }

        occlusion_structure_shader.use();
        occlusion_structure_shader.set_mat4("light_space_matrix", pv);
        for (auto& [zone, opt_pos] : tower_zones) {
            if (!opt_pos) continue;
            int tcx = static_cast<int>(std::floor(opt_pos->x / CHUNK_SIZE));
            int tcz = static_cast<int>(std::floor(opt_pos->z / CHUNK_SIZE));
            if (!loaded_chunks.count({tcx, tcz})) continue;
            glm::mat4 model = glm::translate(glm::mat4(1.0f), *opt_pos);
            occlusion_structure_shader.set_mat4("model", model);
            structure.draw();
        }

        sun_shader.use();
        sun_shader.set_mat4 ("projection", projection);
        sun_shader.set_mat4 ("view",       view);
        sun_shader.set_vec3 ("center",     sun_center);
        sun_shader.set_float("radius",     SUN_SIZE);
        glBindVertexArray(sphere_vao);
        glDrawElements(GL_TRIANGLES, sphere_index_count, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, state.width, state.height);

        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Minetale");
        ImGui::SliderFloat("Frequency", &frequency, 0.001f, 0.08f);
        ImGui::SliderFloat("Amplitude", &amplitude, 1.0f,   80.0f);
        ImGui::SliderInt  ("Octaves",   &octaves,   1,      8);
        ImGui::Separator();
        ImGui::Text("W/A/S/D          move");
        ImGui::Text("Space/Shift      up / down");
        ImGui::Text("LMB + drag       rotate camera");
        ImGui::Text("Tab              toggle wireframe");
        ImGui::Separator();
        ImGui::Text("Chunks loaded:   %d", (int)loaded_chunks.size());
        ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
        ImGui::End();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shadow_depth_tex);

        shader.use();
        shader.set_mat4 ("projection",         projection);
        shader.set_mat4 ("view",               view);
        shader.set_mat4 ("light_space_matrix", light_space);
        shader.set_float("frequency",          frequency);
        shader.set_float("amplitude",          amplitude);
        shader.set_int  ("octaves",            octaves);
        shader.set_vec3 ("light_dir",       light_dir);
        shader.set_float("cloud_altitude",  CLOUD_ALTITUDE);
        shader.set_float("time",            now);
        shader.set_vec3 ("cam_pos",         cam_pos);
        shader.set_int  ("shadow_map",      0);

        float far_sq = FAR_PLANE * FAR_PLANE;

        for (auto& [cx, cz] : loaded_chunks) {
            float wx = (cx + 0.5f) * CHUNK_SIZE;
            float wz = (cz + 0.5f) * CHUNK_SIZE;
            float dx = wx - cam_pos.x;
            float dz = wz - cam_pos.z;
            if (dx*dx + dz*dz > far_sq) continue;
            shader.set_vec2("world_offset", glm::vec2(cx * CHUNK_SIZE, cz * CHUNK_SIZE));
            terrain.draw();
        }

        structure_shader.use();
        structure_shader.set_mat4 ("projection",         projection);
        structure_shader.set_mat4 ("view",               view);
        structure_shader.set_mat4 ("light_space_matrix", light_space);
        structure_shader.set_vec3 ("light_dir",          light_dir);
        structure_shader.set_float("cloud_altitude",     CLOUD_ALTITUDE);
        structure_shader.set_int  ("shadow_map",         0);

        for (auto& [zone, opt_pos] : tower_zones) {
            if (!opt_pos) continue;
            float dx = opt_pos->x - cam_pos.x;
            float dz = opt_pos->z - cam_pos.z;
            if (dx*dx + dz*dz > far_sq) continue;
            int tcx = static_cast<int>(std::floor(opt_pos->x / CHUNK_SIZE));
            int tcz = static_cast<int>(std::floor(opt_pos->z / CHUNK_SIZE));
            if (!loaded_chunks.count({tcx, tcz})) continue;
            glm::mat4 model = glm::translate(glm::mat4(1.0f), *opt_pos);
            structure_shader.set_mat4("model", model);
            structure.draw();
        }

        sun_shader.use();
        sun_shader.set_mat4 ("projection", projection);
        sun_shader.set_mat4 ("view",       view);
        sun_shader.set_vec3 ("center",     sun_center);
        sun_shader.set_float("radius",     SUN_SIZE);
        glBindVertexArray(sphere_vao);
        glDrawElements(GL_TRIANGLES, sphere_index_count, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        cloud_shader.use();
        cloud_shader.set_mat4 ("projection", projection);
        cloud_shader.set_mat4 ("view",       view);
        cloud_shader.set_float("size",       CLOUD_SIZE);

        for (auto& [key, exists] : cloud_zones) {
            if (!exists) continue;
            float cx = (key.first  + 0.5f) * CLOUD_ZONE_SIZE;
            float cz = (key.second + 0.5f) * CLOUD_ZONE_SIZE;
            float dx = cx - cam_pos.x;
            float dz = cz - cam_pos.z;
            if (dx*dx + dz*dz > far_sq) continue;
            glm::vec3 center(cx, CLOUD_ALTITUDE, cz);
            cloud_shader.set_vec3("center", center);
            glBindVertexArray(quad_vao);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }

        float sun_weight = 0.0f;
        if (sun_in_front) {
            float edge   = glm::max(glm::abs(sun_screen.x - 0.5f),
                                    glm::abs(sun_screen.y - 0.5f));
            sun_weight   = 1.0f - glm::smoothstep(0.4f, 0.75f, edge);
        }

        if (sun_weight > 0.001f) {
            glBlendFunc(GL_ONE, GL_ONE);
            glDepthMask(GL_FALSE);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, occ_color_tex);
            godray_shader.use();
            godray_shader.set_int  ("occlusion_tex", 1);
            godray_shader.set_vec2 ("sun_pos",       sun_screen);
            godray_shader.set_float("exposure",      0.18f * sun_weight);
            godray_shader.set_float("decay",         0.97f);
            godray_shader.set_float("density",       0.96f);
            glBindVertexArray(quad_vao);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
            glActiveTexture(GL_TEXTURE0);
        }

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glPolygonMode(GL_FRONT_AND_BACK, state.wireframe ? GL_LINE : GL_FILL);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    glDeleteFramebuffers(1, &shadow_fbo);
    glDeleteTextures(1, &shadow_depth_tex);
    glDeleteFramebuffers(1, &occ_fbo);
    glDeleteTextures(1, &occ_color_tex);
    glDeleteRenderbuffers(1, &occ_depth_rb);
    glDeleteVertexArrays(1, &quad_vao);
    glDeleteBuffers(1, &quad_vbo);
    glDeleteBuffers(1, &quad_ebo);
    glDeleteVertexArrays(1, &sphere_vao);
    glDeleteBuffers(1, &sphere_vbo);
    glDeleteBuffers(1, &sphere_ebo);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();

    return 0;
}
