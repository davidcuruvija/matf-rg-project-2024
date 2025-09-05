//
// Created by matfrg on 7/14/25.
//

#include "MainController.hpp"
#include <GuiController.hpp>
#include <LampEvent.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <engine/util/Configuration.hpp>
#include <spdlog/spdlog.h>
#include <random>

namespace app {

class MainPlatformEventObserver : public engine::platform::PlatformEventObserver {
public:
    void on_mouse_move(engine::platform::MousePosition position) override;

    void on_key(engine::platform::Key key) override;
};

void MainPlatformEventObserver::on_key(engine::platform::Key key) {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    auto main_controller = engine::platform::PlatformController::get<MainController>();
    if (key.id() == engine::platform::KeyId::KEY_B) {
        main_controller->lamp_event_handler->start_automatic_cycle();
        main_controller->lamp_event_handler->cycle_active = true;
    }
    if (key.id() == engine::platform::KeyId::KEY_N && key.state() == engine::platform::Key::State::Pressed) { main_controller->lamp_event_handler->cycle_manual_color(); }
}

void MainPlatformEventObserver::on_mouse_move(engine::platform::MousePosition position) {
    auto gui_controller = engine::core::Controller::get<GUIController>();
    if (!gui_controller->is_enabled()) {
        auto camera = engine::core::Controller::get<engine::graphics::GraphicsController>()->camera();
        camera->rotate_camera(position.dx, position.dy);
    }
}

void MainController::initialize() {
    spdlog::info("MainController initialized");
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    platform->register_platform_event_observer(std::make_unique<MainPlatformEventObserver>());
    lamp_event_handler = new LampEvent();
    engine::graphics::OpenGL::enable_depth_testing();
}

bool MainController::loop() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    if (platform->key(engine::platform::KeyId::KEY_ESCAPE).is_down()) { return false; }
    return true;
}

void MainController::draw() {
    draw_floor();
    draw_graves();
    draw_lamp();
    draw_dog();
    draw_gulls();
}

void MainController::begin_draw() { engine::graphics::OpenGL::clear_buffers(); }

void MainController::draw_floor() {
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();

    auto floor = resources->model("cobble");
    auto shader = resources->shader("basic");

    shader->use();

    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-7.0f, -10.0f, -9.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(15.0f, 1.0f, 13.0f));

    shader->set_mat4("model", model);

    point_light.linear = 0.001f;
    point_light.quadratic = 0.00005f;

    shader->set_vec3("point_light.position", point_light.position);
    shader->set_vec3("point_light.ambient", point_light.ambient);
    shader->set_vec3("point_light.diffuse", point_light.diffuse);
    shader->set_vec3("point_light.specular", point_light.specular);
    shader->set_float("point_light.linear", point_light.linear);
    shader->set_float("point_light.quadratic", point_light.quadratic);

    shader->set_vec3("spot_light.position", graphics->camera()->Position);
    shader->set_vec3("spot_light.direction", graphics->camera()->Front);

    shader->set_vec3("spot_light.ambient", spot_light.ambient);
    shader->set_vec3("spot_light.diffuse", spot_light.diffuse);
    shader->set_vec3("spot_light.specular", spot_light.specular);

    shader->set_float("spot_light.constant", spot_light.constant);
    shader->set_float("spot_light.linear", spot_light.linear);
    shader->set_float("spot_light.quadratic", spot_light.quadratic);

    shader->set_float("spot_light.cutOff", spot_light.cutOff);
    shader->set_float("spot_light.outerCutOff", spot_light.outerCutOff);

    shader->set_vec3("cameraPos", graphics->camera()->Position);

    floor->draw(shader);
}

void MainController::draw_lamp() {
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto lamp = resources->model("streetlamp");
    auto shader = resources->shader("lampShader");

    lamp_event_handler->update_lamp(get<engine::platform::PlatformController>()->dt());

    shader->use();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-20.0f, -10.0f, 17.0f));
    model = glm::scale(model, glm::vec3(3.0f));

    shader->set_vec3("spot_light.position", graphics->camera()->Position);
    shader->set_vec3("spot_light.direction", graphics->camera()->Front);

    shader->set_vec3("spot_light.ambient", spot_light.ambient);
    shader->set_vec3("spot_light.diffuse", spot_light.diffuse);
    shader->set_vec3("spot_light.specular", spot_light.specular);

    shader->set_float("spot_light.constant", spot_light.constant);
    shader->set_float("spot_light.linear", spot_light.linear);
    shader->set_float("spot_light.quadratic", spot_light.quadratic);

    shader->set_float("spot_light.cutOff", spot_light.cutOff);
    shader->set_float("spot_light.outerCutOff", spot_light.outerCutOff);

    shader->set_mat4("model", model);
    shader->set_vec3("lightIntensity", point_light.diffuse);
    shader->set_float("lower", 15.0f);
    shader->set_float("upper", 20.8f);
    shader->set_float("margin", 0.4f);

    lamp->draw(shader);
}

void MainController::draw_dog() {
    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto dog = resources->model("dog");
    auto shader = resources->shader("basic");

    shader->use();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(40.0f, -10.0f, -20.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.3f));
    shader->set_mat4("model", model);

    shader->set_vec3("point_light.position", point_light.position);
    shader->set_vec3("point_light.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
    shader->set_vec3("point_light.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
    shader->set_vec3("point_light.specular", glm::vec3(0.5f, 0.5f, 0.5f));
    shader->set_float("point_light.linear", point_light.linear);
    shader->set_float("point_light.quadratic", point_light.quadratic);

    shader->set_vec3("spot_light.position", graphics->camera()->Position);
    shader->set_vec3("spot_light.direction", graphics->camera()->Front);

    shader->set_vec3("spot_light.ambient", spot_light.ambient);
    shader->set_vec3("spot_light.diffuse", spot_light.diffuse);
    shader->set_vec3("spot_light.specular", spot_light.specular);

    shader->set_float("spot_light.constant", spot_light.constant);
    shader->set_float("spot_light.linear", spot_light.linear);
    shader->set_float("spot_light.quadratic", spot_light.quadratic);

    shader->set_float("spot_light.cutOff", spot_light.cutOff);
    shader->set_float("spot_light.outerCutOff", spot_light.outerCutOff);

    shader->set_vec3("cameraPos", graphics->camera()->Position);

    dog->draw(shader);
}

void MainController::init_gulls() {
    if (gullsInitialized) return;

    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    engine::resources::Model *gull = resources->model("gull");

    const int instanceCount = 250;
    gullMatrices.reserve(instanceCount);

    float centerX = -20.0f;
    float centerY = 26.0f;
    float centerZ = -26.0f;
    float radiusX = 50.0f;
    float radiusZ = 25.0f;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

    for (int i = 0; i < instanceCount; i++) {
        float angle = (float(i) / instanceCount) * 2.0f * glm::pi<float>();
        float x = centerX + radiusX * cos(angle) + (dist01(gen) - 0.5f) * 5.0f;
        float z = centerZ + radiusZ * sin(angle) + (dist01(gen) - 0.5f) * 5.0f;
        float y = centerY + dist01(gen) * 5.0f;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, y, z));
        float scale = 2.0f + dist01(gen) * 1.0f;
        model = glm::scale(model, glm::vec3(scale));

        gullMatrices.push_back(model);
    }

    gull->setup_instance_matrices(gullMatrices);
    gullsInitialized = true;
}

void MainController::init_graves() {
    if (gravesInitialized) return;

    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    engine::resources::Model *grave = resources->model("grave");

    float TRANSLATE_X_FACTOR = 9.0f;
    float TRANSLATE_Z_FACTOR = -7.0f;
    float GRAVE_COLS = 8, GRAVE_ROWS = 7;

    for (int i = 1; i < GRAVE_ROWS; i++) {
        for (int j = 1; j < GRAVE_COLS; j++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-5.0f + i * TRANSLATE_X_FACTOR, -9.0f, TRANSLATE_Z_FACTOR * (j + 4.0f)));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, -1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f));
            graveMatrices.push_back(model);
        }
    }

    grave->setup_instance_matrices(graveMatrices);
    gravesInitialized = true;
}

void MainController::draw_gulls() {
    init_gulls();

    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();

    engine::resources::Model *gull = resources->model("gull");
    engine::resources::Shader *shader = resources->shader("gullShader");

    shader->use();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());

    gull->draw_instanced(shader, gullMatrices.size());
}

void MainController::draw_graves() {
    init_graves();

    auto resources = engine::core::Controller::get<engine::resources::ResourcesController>();
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();

    engine::resources::Model *grave = resources->model("grave");
    engine::resources::Shader *shader = resources->shader("graveShader");

    shader->use();
    shader->set_mat4("projection", graphics->projection_matrix());
    shader->set_mat4("view", graphics->camera()->view_matrix());

    shader->set_vec3("point_light.position", point_light.position);
    shader->set_vec3("point_light.ambient", point_light.ambient);
    shader->set_vec3("point_light.diffuse", point_light.diffuse);
    shader->set_vec3("point_light.specular", point_light.specular);
    shader->set_float("point_light.linear", point_light.linear);
    shader->set_float("point_light.quadratic", point_light.quadratic);
    shader->set_float("point_light.shininess", point_light.shininess);

    shader->set_vec3("spot_light.position", graphics->camera()->Position);
    shader->set_vec3("spot_light.direction", graphics->camera()->Front);

    shader->set_vec3("spot_light.ambient", spot_light.ambient);
    shader->set_vec3("spot_light.diffuse", spot_light.diffuse);
    shader->set_vec3("spot_light.specular", spot_light.specular);

    shader->set_float("spot_light.constant", spot_light.constant);
    shader->set_float("spot_light.linear", spot_light.linear);
    shader->set_float("spot_light.quadratic", spot_light.quadratic);

    shader->set_float("spot_light.cutOff", spot_light.cutOff);
    shader->set_float("spot_light.outerCutOff", spot_light.outerCutOff);

    shader->set_vec3("cameraPos", graphics->camera()->Position);

    grave->draw_instanced(shader, graveMatrices.size());
}

void MainController::update() { update_camera(); }

void MainController::update_camera() {
    auto gui_controller = engine::core::Controller::get<GUIController>();
    if (gui_controller->is_enabled()) { return; }
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    auto graphics = engine::core::Controller::get<engine::graphics::GraphicsController>();
    auto camera = graphics->camera();
    camera->MovementSpeed = 10.0;
    float dt = platform->dt();
    if (platform->key(engine::platform::KeyId::KEY_W).is_down()) { camera->move_camera(engine::graphics::Camera::Movement::FORWARD, dt); }
    if (platform->key(engine::platform::KeyId::KEY_S).is_down()) { camera->move_camera(engine::graphics::Camera::Movement::BACKWARD, dt); }
    if (platform->key(engine::platform::KeyId::KEY_A).is_down()) { camera->move_camera(engine::graphics::Camera::Movement::LEFT, dt); }
    if (platform->key(engine::platform::KeyId::KEY_D).is_down()) { camera->move_camera(engine::graphics::Camera::Movement::RIGHT, dt); }
    if (platform->key(engine::platform::KeyId::KEY_UP).is_down()) { camera->move_camera(engine::graphics::Camera::Movement::UP, dt); }
    if (platform->key(engine::platform::KeyId::KEY_DOWN).is_down()) { camera->move_camera(engine::graphics::Camera::Movement::DOWN, dt); }
}

void MainController::end_draw() {
    auto platform = engine::core::Controller::get<engine::platform::PlatformController>();
    platform->swap_buffers();
}
}// app