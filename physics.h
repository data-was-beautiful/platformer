#ifndef PHYSICS_H
#define PHYSICS_H

#include <stdbool.h>

/* Axis-Aligned Bounding Box */
typedef struct {
    float x, y;
    float w, h;
} AABB;

/* Returns true if two AABBs overlap */
bool aabb_overlap(AABB a, AABB b);

#endif
