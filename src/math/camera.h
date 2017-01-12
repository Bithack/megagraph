#ifndef _TCAM__H_
#define _TCAM__H_

/** @relates tcam @{ */

#include <stdint.h>
#include "vector.h"

#define TCAM_VELOCITY    1 /**< Set to enable camera velocity. lol jk this isnt even implemented yet */
#define TCAM_PERSPECTIVE 2 /**< Enable perspective camera. If unset, the camera is orthographic. */
#define TCAM_LOOKAT      4 /**< If set, the camera's direction is calculated from a point given to tcam_lookat, otherwise the camera's direction is set using tcam_set_direction */

/** 
 * Perspective/ortho2d camera convenience stuff.
 *
 * tcam will generate projection and view matrices
 * for you. 
 **/
struct tcam {
    /* call tcam_calculate() before fetching these */
    float             view[16];
    float             projection[16];
    float             combined[16];

    tvec3              _velocity;
    tvec3              _position;
    tvec3              _direction;
    tvec3              _lookat;
    tvec3              up;
    uint32_t          _flags;
    float             fov;
    float             aspect;
    float             near;
    float             far;
    float             velocity_damping;

    float width;
    float height;

    float owidth;
    float oheight;
};

struct tcam* tcam_alloc(void);
void tcam_init(struct tcam *c);
void tcam_set_position(struct tcam *cam, float x, float y, float z);
void tcam_translate(struct tcam *c, float x, float y, float z);
void tcam_confine(struct tcam *c, float x, float y, float z, float factor_x, float factor_y, float factor_z);
void tcam_set_lookat(struct tcam *c, float x, float y, float z);
void tcam_enable(struct tcam *cam, uint32_t flag);
int tcam_enabled(struct tcam *cam, uint32_t flag);
void tcam_disable(struct tcam *cam, uint32_t flag);
void tcam_free(struct tcam *cam);
void tcam_set_direction(struct tcam *c, float x, float y, float z);
void tcam_calculate(struct tcam *c);

tvec3 tcam_unproject(struct tcam *c, float x, float y, float z);
tvec3 tcam_project(struct tcam *c, float x, float y, float z);

#endif
