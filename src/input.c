
#include "megagraph.h" 

int key_down[1024] = {0};

void on_glfw_key(GLFWwindow *win, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        key_down[key] = 1;

        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(win, 1);
                break;
        }
    } else if (action == GLFW_RELEASE) {
        key_down[key] = 0;
    }
}

void input_tick(double dt) {
    if (key_down[GLFW_KEY_A]) {
        mg.cam_angle += 1.0 * dt;
    }
    if (key_down[GLFW_KEY_D]) {
        mg.cam_angle -= 1.0 * dt;
    }
    if (key_down[GLFW_KEY_S]) {
        mg.cam_dist -= 30.0 * dt;
    }
    if (key_down[GLFW_KEY_W]) {
        mg.cam_dist += 30.0 * dt;
    }
}
