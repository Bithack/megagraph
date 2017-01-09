/** 
 * MegaGraph
 * Copyright (c) 2017 Teorem AB
 **/

#include <stdio.h>

#include "glad/glad.h"

GLuint g_program;
GLuint g_uniform_mv;
GLuint g_uniform_p;
GLuint g_uniform_tex0;

static const char* src_fs[] = {
    "#version 330 core                      \n",
    "uniform sampler2D tex0;                \n",
    "in vec2 uv;                            \n",
    "out vec3 color;                        \n",
    "void main() {                          \n",
    "   color = texture(tex0, uv).rgb;       \n",
    "}                                      \n",
    ""
};

static const char* src_vs[] = {
    "#version 330 core                      \n",
    "layout(location = 0) in vec4 position; \n",
    "uniform mat4 MV;                       \n",
    "out vec2 uvbase;                       \n"
    "void main() {                          \n",
    "   vec2 u = vec2(mod(position.w, 64.0), (floor(position.w / 64.0)));                          \n",
    "   uvbase = u * vec2(1.0/64.0);            \n",
    "   gl_Position = MV*vec4(position.xyz, 1.0);          \n",
    "}\n",
};

static const char* src_gs[] = {
    "#version 330 core                      \n",
    "layout(points) in;                     \n",
    "layout(triangle_strip) out;            \n",
    "layout(max_vertices = 4) out;          \n",
    "\n",
    "const float size = 2.0;                \n"
    "\n",
    "layout(max_vertices = 4) out;          \n",
    "\n",
    "in vec2 uvbase[];                           \n",
    "out vec2 uv;                           \n",
    "uniform mat4 P;                        \n",
    "const vec2 uvscale = vec2(1.0/64.0, 1.0/64.0);                  \n",
    "\n",
    "void main() {                                  \n",
    "   vec4 pos = gl_in[0].gl_Position;                    \n",
    "   vec2 vo = pos.xy + vec2(-0.5, -0.5) * size;   \n",
    "   gl_Position = P*vec4(vo, pos.zw);             \n",
    "   uv = uvbase[0]+vec2(0.0, 0.0)*uvscale;                         \n",
    //"   uv = vec2(0.0, 0.0);                         \n",
    "   EmitVertex();                               \n",
    "\n",
    "   vo = pos.xy + vec2(-0.5, 0.5) * size;   \n",
    "   gl_Position = P*vec4(vo, pos.zw);             \n",
    "   uv = uvbase[0]+vec2(0.0, 1.0)*uvscale;                         \n",
    //"   uv = vec2(0.0, 1.0);                         \n",
    "   EmitVertex();                               \n",
    "\n",
    "   vo = pos.xy + vec2(0.5, -0.5) * size;   \n",
    "   gl_Position = P*vec4(vo, pos.zw);             \n",
    "   uv = uvbase[0]+vec2(1.0, 0.0)*uvscale;                         \n",
    //"   uv = vec2(1.0, 0.0);                         \n",
    "   EmitVertex();                               \n",
    "\n",
    "   vo = pos.xy + vec2(0.5, 0.5) * size;   \n",
    "   gl_Position = P*vec4(vo, pos.zw);             \n",
    "   uv = uvbase[0]+vec2(1.0, 1.0)*uvscale;                         \n",
    //"   uv = vec2(1.0, 1.0);                         \n",
    "   EmitVertex();                               \n",
    "   EndPrimitive();                               \n",
    "}\n",
};

int compile_shaders() {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint gs = glCreateShader(GL_GEOMETRY_SHADER);
    GLint status;
    int log_length = 0;

    glShaderSource(vs, sizeof(src_vs)/sizeof(char*), src_vs, 0);
    glCompileShader(vs);

    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &log_length);

    if (status != GL_TRUE && log_length > 0) {
        char log[log_length+1];
        glGetShaderInfoLog(vs, log_length, 0, log);
        log[log_length] = '\0';
        fprintf(stderr, "!!! Error compiling vertex shader:\n%s", log);
        return 1;
    }
    
    glShaderSource(fs, sizeof(src_fs)/sizeof(char*), src_fs, 0);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &log_length);

    if (status != GL_TRUE && log_length > 0) {
        char log[log_length+1];
        glGetShaderInfoLog(fs, log_length, 0, log);
        log[log_length] = '\0';
        fprintf(stderr, "!!! Error compiling fragment shader:\n%s", log);
        return 1;
    }

    glShaderSource(gs, sizeof(src_gs)/sizeof(char*), src_gs, 0);
    glCompileShader(gs);

    glGetShaderiv(gs, GL_COMPILE_STATUS, &status);
    glGetShaderiv(gs, GL_INFO_LOG_LENGTH, &log_length);

    if (status != GL_TRUE && log_length > 0) {
        char log[log_length+1];
        glGetShaderInfoLog(gs, log_length, 0, log);
        log[log_length] = '\0';
        fprintf(stderr, "!!! Error compiling geometry shader:\n%s", log);
        return 1;
    }

    g_program = glCreateProgram();
    glAttachShader(g_program, vs);
    glAttachShader(g_program, fs);
    glAttachShader(g_program, gs);
    glLinkProgram(g_program);

    glGetProgramiv(g_program, GL_LINK_STATUS, &status);
    glGetProgramiv(g_program, GL_INFO_LOG_LENGTH, &log_length);

    if (status != GL_TRUE && log_length > 0) {
        char log[log_length+1];
        glGetProgramInfoLog(g_program, log_length, 0, log);
        log[log_length] = '\0';
        fprintf(stderr, "!!! Error linking shader program:\n%s", log);
        return 1;
    }

    g_uniform_mv = glGetUniformLocation(g_program, "MV");
    g_uniform_p = glGetUniformLocation(g_program, "P");
    g_uniform_tex0 = glGetUniformLocation(g_program, "tex0");

    return 0;
}

