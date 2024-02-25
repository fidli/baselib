#pragma once
#include "common.h"

#include "util_math.cpp"

struct CollisionRectAxisAligned{
    v2 size;
    v2 pos;
};

bool collide(CollisionRectAxisAligned A, CollisionRectAxisAligned B)
{
    // minkowski difference
    CollisionRectAxisAligned diff = {};
    diff.size = A.size + B.size;
    diff.pos = B.pos - A.pos;
    v2 lowerLeftCorner = diff.pos - diff.size/2.0f;
    v2 upperRightCorner = lowerLeftCorner + diff.size;
    // is 0 inside?
    return lowerLeftCorner.x < 0 && upperRightCorner.x > 0 && lowerLeftCorner.y < 0 && upperRightCorner.y > 0;
}

v2 collidePop(CollisionRectAxisAligned A, CollisionRectAxisAligned B, v2 direction)
{
    ASSERT(collide(A, B));
    ASSERT(length(direction) > 0.00005f);
    CollisionRectAxisAligned diff = {};
    diff.size = A.size + B.size;
    diff.pos = B.pos - A.pos;
    // epsilons to pop just above the border
    v2 halo = V2(0.001f, 0.001f);
    v2 lowerLeftCorner = diff.pos - diff.size/2.0f - halo;
    v2 upperRightCorner = lowerLeftCorner + diff.size + halo;
    ASSERT(lowerLeftCorner.x < 0 && upperRightCorner.x > 0 && lowerLeftCorner.y < 0 && upperRightCorner.y > 0);
    f32 scale = 0;
    // TODO corner hit?
    if (direction.x < 0){
        scale = lowerLeftCorner.x/direction.x;
    }else if (direction.x > 0){
        scale = upperRightCorner.x/direction.x;
    }
    if (direction.y < 0){
        f32 scale2 = lowerLeftCorner.y/direction.y;
        if (scale2 < scale || scale == 0.0f){
            scale = scale2;
        }
    }else if (direction.y > 0){
        f32 scale2 = upperRightCorner.y/direction.y;
        if (scale2 < scale || scale == 0.0f){
            scale = scale2;
        }
    }
    ASSERT(scale > 0);
    A.pos += (direction*scale);
    ASSERT(!collide(A, B));
    return direction * scale;
}

v2 collideSlide(CollisionRectAxisAligned A, CollisionRectAxisAligned B, v2 direction)
{
    ASSERT(length(direction) > 0.005f);
    CollisionRectAxisAligned diff = {};
    diff.size = A.size + B.size;
    diff.pos = B.pos - A.pos;
    v2 lowerLeftCorner = diff.pos - diff.size/2.0f;
    v2 upperRightCorner = lowerLeftCorner + diff.size;
    ASSERT(ABS(lowerLeftCorner.x) < 0.005f|| ABS(upperRightCorner.x) < 0.005f || ABS(lowerLeftCorner.y) < 0.005f || ABS(upperRightCorner.y) < 0.005f);

    // TODO corner hit dont slide?
    if(ABS(lowerLeftCorner.x) < 0.005f || ABS(upperRightCorner.x) < 0.005f){
        return dot(direction, V2(0.0f, 1.0f)) * V2(0.0f, 1.0f);
    }
    if(ABS(lowerLeftCorner.y) < 0.005f || ABS(upperRightCorner.y) < 0.005f){
        return dot(direction, V2(1.0f, 0.0f)) * V2(1.0f, 0.0f);
    }
    return direction;
}

v2 collideReflect(CollisionRectAxisAligned A, CollisionRectAxisAligned B, v2 direction)
{
    // TODO corner hit
    ASSERT(length(direction) > 0.005f);
    CollisionRectAxisAligned diff = {};
    diff.size = A.size + B.size;
    diff.pos = B.pos - A.pos;
    v2 lowerLeftCorner = diff.pos - diff.size/2.0f;
    v2 upperRightCorner = lowerLeftCorner + diff.size;
    ASSERT(ABS(lowerLeftCorner.x) < 0.005f|| ABS(upperRightCorner.x) < 0.005f || ABS(lowerLeftCorner.y) < 0.005f || ABS(upperRightCorner.y) < 0.005f);
    if(ABS(lowerLeftCorner.x) < 0.005f || ABS(upperRightCorner.x) < 0.005f){
        direction.x *= -1.0f;
    }
    if(ABS(lowerLeftCorner.y) < 0.005f || ABS(upperRightCorner.y) < 0.005f){
        direction.y *= -1.0f;
    }
    return direction;
}
