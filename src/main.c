#include <stdio.h>
#include <stdint.h>
#include <vips/vips.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "megagraph.h"

#include "file.h"
#include "math/glob.h"
#include "camera.h"

const int WIDTH = 1024*2;
const int HEIGHT = 768*2;

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#define MAX_LINE_LEN (8192*2)

GLFWwindow    *g_win = 0;

struct tcam *g_camera = 0;
float cam_angle = 0.f;
float cam_dist = 50.f;

GLuint g_vertex_buf;
GLuint g_vao;

extern GLuint g_program; /* shaders.c */
extern GLuint g_uniform_mv; /* shaders.c */
extern GLuint g_uniform_p; /* shaders.c */
extern GLuint g_uniform_tex0; /* shaders.c */

int g_image_height = 64;
int g_image_width = 64;
int g_texture_width = 4096;
int g_texture_height = 4096;
GLint g_texture_format = GL_COMPRESSED_RGB;

GLuint *g_textures;
int g_num_textures;

static inline int num_images_per_texture() {
    return (g_texture_width / g_image_width) * (g_texture_height / g_image_height);
}

int g_num_objects = 0;

static int frame();
static int load(const char *filename);
int compile_shaders(); /* shaders.c */
static void on_glfw_error(int error, const char *description);
static void on_glfw_key(GLFWwindow *win, int key, int scancode, int action, int mods);

int main(int argc, char* argv[]) {
    VIPS_INIT(argv[0]);

    glfwSetErrorCallback(on_glfw_error);

    if (!glfwInit()) {
        LOG_E("glfw init error");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    if (!(g_win = glfwCreateWindow(WIDTH, HEIGHT, "MegaGraph", NULL, NULL))) {
        LOG_E("Could not create window");
        return 1;
    }

    glfwMakeContextCurrent(g_win);

    LOG_I("GLFW did its job.");

    if (!gladLoadGL()) {
        LOG_E("GLAD failed");
        return 1;
    }

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    LOG_I("OpenGL %s, GLSL %s", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

    const char *filename = argc > 1 ? argv[1] : "test/data";

    if (compile_shaders() != 0) {
        LOG_E("failed compiling shaders");
        exit(1);
    }
    if (load(filename) != 0) {
        LOG_E("loading data failed");
        exit(1);
    }

    g_camera = tcam_alloc();

    g_camera->width = WIDTH;
    g_camera->height = HEIGHT;

    tcam_enable(g_camera, TCAM_PERSPECTIVE);
    tcam_set_position(g_camera, 0.0f, 0.0f, 0.0f);
    tcam_set_direction(g_camera, 0.0f, 0.0f, 1.0f);

    glfwSetKeyCallback(g_win, on_glfw_key);

    glfwSwapInterval(1);

    while (!glfwWindowShouldClose(g_win)) {
        glfwPollEvents();

        if (frame() != 0) {
            break;
        }
    }

    glfwDestroyWindow(g_win);
    glfwTerminate();

    vips_shutdown();
    return 0;
}

static int load(const char *filename) {
    LOG_I("Loading '%s'", filename);
    char line[MAX_LINE_LEN];
    int num_read = 0;

    FILE *fp = fopen(filename, "rb+");

    if (!fp) {
        LOG_E("could not open file");
        exit(1);
    }

    int num_lines = file_count_occurrences(fp, '\n');

    LOG_I("Object count:\t%d", num_lines);

    int vertex_buf_size = num_lines * sizeof(tvec4);
    int image_buf_size = num_lines * 16 * 16 * sizeof(uint32_t);

    g_num_textures = num_lines / num_images_per_texture() + 1;

    LOG_I("Vertex buffer:\t%d bytes", vertex_buf_size);
    LOG_I("Image buffer:\t%d bytes", image_buf_size);
    LOG_I("Num textures:\t%d", g_num_textures);

    /* create textures */
    g_textures = (GLuint*)malloc(g_num_textures * sizeof(GLuint));
    glGenTextures(g_num_textures, g_textures);
    for (int x=0; x<g_num_textures; x++) {
        glBindTexture(GL_TEXTURE_2D, g_textures[x]);
        glTexImage2D(GL_TEXTURE_2D, 0, g_texture_format, g_texture_width, g_texture_height,
                     0, GL_RGB, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);

    glGenBuffers(1, &g_vertex_buf);
    glBindBuffer(GL_ARRAY_BUFFER, g_vertex_buf);
    glBufferData(GL_ARRAY_BUFFER, vertex_buf_size, 0, GL_STATIC_DRAW);

    tvec4 *buf = (tvec4*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

    if (!buf) {
        LOG_E("Could not map buffer: %d", glGetError());
        exit(1);
    }

    rewind(fp);

    int current_texture = -1;
    int images_per_texture = num_images_per_texture();
    int images_per_line = (g_texture_width / g_image_width);

    size_t texture_size = 3*sizeof(unsigned char)*g_texture_width*g_texture_height;

    unsigned char *pbuf = (unsigned char*)malloc(texture_size);

    memset(pbuf, 0, texture_size);

    glActiveTexture(GL_TEXTURE0);

    int i = 0;
    while (fgets(line, MAX_LINE_LEN, fp) == line && num_read < num_lines) {

        fprintf(stdout, "\rLoading buffers (%d/%d) ...", num_read, num_lines);

        i = (num_read % images_per_texture);
        if (i == 0) {
            current_texture ++;

            if (current_texture > 0) {
                glTexImage2D(GL_TEXTURE_2D, 0, g_texture_format, g_texture_width, g_texture_height,
                            0, GL_RGB, GL_UNSIGNED_BYTE, pbuf);
                memset(pbuf, 0, texture_size);
            }

            glBindTexture(GL_TEXTURE_2D, g_textures[current_texture]);

            fprintf(stdout, "\n");
            LOG_I("Texture %d...", current_texture);
        }

        /* pack the index into the texture in the w component */
        buf->a = (float)i;
        char filename[1024];
        sscanf(line, "%f %f %f %1023s", &buf->r, &buf->g, &buf->b, filename);

        int sx = i % images_per_line;
        int sy = i / images_per_line;

        if (strlen(filename) <= 0) {
            strcpy(filename, "test.jpg");
        }

        VipsImage *img,
                  *img_cropped = 0,
                  *img_scaled = 0;

        img = vips_image_new_from_file(filename, NULL);

        if (!img) {
            LOG_E("could not load: %s", filename);
            for (int y=0; y<g_image_height; y++) {
                for (int x=0; x<g_image_width; x++) {
                    pbuf[((sx*g_image_width+x)*3) + ((sy*g_image_height+y)*g_texture_width*3) + 0] = 255;
                    pbuf[((sx*g_image_width+x)*3) + ((sy*g_image_height+y)*g_texture_width*3) + 1] = 255;
                    pbuf[((sx*g_image_width+x)*3) + ((sy*g_image_height+y)*g_texture_width*3) + 2] = 0;
                }
            }
        } else {
            int image_height = vips_image_get_height(img);
            int image_width = vips_image_get_width(img);
            /* opening jpeg file succeeded, scale it down to 64x64 */

            int size = image_height < image_width ? image_height : image_width;

            if (vips_crop(img, &img_cropped, 0, 0, size, size, NULL) != 0) {
                LOG_E("Cropping failed.");
                exit(1);
            }

            double scale = (double)g_image_width/(double)size;

            if (vips_similarity(img_cropped, &img_scaled, "scale", scale, "interpolate", vips_interpolate_bilinear_static(), NULL) != 0) {
                LOG_E("Resize failed!");
                exit(1);
            }

            g_object_unref(img);
            g_object_unref(img_cropped);

            /* make sure image is available in memory */
            vips_image_wio_input(img_scaled);

            /* we expect these to be 64, but just in case clamp them */
            int scaled_w = MIN(g_image_width, vips_image_get_width(img_scaled));
            int scaled_h = MIN(g_image_height, vips_image_get_height(img_scaled));

            for (int y=0; y<scaled_h; y++) {
                for (int x=0; x<scaled_w; x++) {
                    VipsPel *pixel = VIPS_IMAGE_ADDR(img_scaled, x, y);

                    pbuf[((sx*g_image_width+x)*3) + (((sy+1)*g_image_height-1-y)*g_texture_width*3) + 0] = *(pixel);
                    pbuf[((sx*g_image_width+x)*3) + (((sy+1)*g_image_height-1-y)*g_texture_width*3) + 1] = *(pixel+1);
                    pbuf[((sx*g_image_width+x)*3) + (((sy+1)*g_image_height-1-y)*g_texture_width*3) + 2] = *(pixel+2);
                }

            }
            g_object_unref(img_scaled);
        }

        buf ++;
        num_read ++;
    }
    fprintf(stdout, "\n");

    if (i > 0) {
        LOG_I("i: %d", i);
        glTexImage2D(GL_TEXTURE_2D, 0, g_texture_format, g_texture_width, g_texture_height,
                    0, GL_RGB, GL_UNSIGNED_BYTE, pbuf);
    }
    free(pbuf);

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    LOG_I("parsed %d lines", num_read);

    g_num_objects = num_read;

    return 0;
}

static int frame() {
    int w,h;

    glfwGetFramebufferSize(g_win, &w, &h);

    glViewport(0, 0, w, h);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glBindVertexArray(g_vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, g_vertex_buf);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glActiveTexture(GL_TEXTURE0);
    glUseProgram(g_program);

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

    g_camera->_position = (tvec3){cosf(cam_angle)*cam_dist, 0.f, sinf(cam_angle)*cam_dist};
    tcam_set_lookat(g_camera, 0.f, 0.f, 0.f);
    tcam_enable(g_camera, TCAM_LOOKAT);
    tcam_calculate(g_camera);
    glUniformMatrix4fv(g_uniform_p, 1, GL_FALSE, g_camera->projection);
    glUniformMatrix4fv(g_uniform_mv, 1, GL_FALSE, g_camera->view);
    glUniform1i(g_uniform_tex0, 0);

    int left = g_num_objects;
    int objects_per_texture = num_images_per_texture();

    for (int x=0; x<g_num_textures && left > 0; x++) {
        glBindTexture(GL_TEXTURE_2D, g_textures[x]);
        int count = MIN(left, objects_per_texture);
        glDrawArrays(GL_POINTS, x*objects_per_texture, count);
        left -= objects_per_texture;
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    glDisableVertexAttribArray(0);

    glfwSwapBuffers(g_win);

    return 0;
}

static void on_glfw_key(GLFWwindow *win, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(win, 1);
                break;
        }
    }
    switch (key) {
        case GLFW_KEY_A:
            cam_angle += 0.05f;
            break;

        case GLFW_KEY_D:
            cam_angle -= 0.05f;
            break;

        case GLFW_KEY_S:
            cam_dist += 1.0;
            break;

        case GLFW_KEY_W:
            cam_dist -= 1.0;
            break;
    }
}

static void on_glfw_error(int error, const char *description) {
    LOG_E("GLFW error: (%d): %s", error, description);
}
