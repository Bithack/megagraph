#ifndef _PLOTTER__H_
#define _PLOTTER__H_

#include <stdio.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#define MG_NAME "MegaGraph"
#define MG_VERSION "0.9"

#define LOG_I(x, ...) fprintf(stdout, x "\n", ##__VA_ARGS__)
#define LOG_E(x, ...) fprintf(stderr, x "\n", ##__VA_ARGS__)

extern struct megagraph {
    struct tcam *cam;
    float        cam_angle;
    float        cam_dist;
    float        dt;
    float        last_time;
} mg;

void on_glfw_key(GLFWwindow *win, int key, int scancode, int action, int mods);
void input_tick(double dt);

#endif
