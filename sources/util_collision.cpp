#pragma once
#include "common.h"

#include "util_math.cpp"

struct CollisionRect{
    v2 size;
    v2 pos;
};

bool collide(CollisionRect A, CollisionRect B)
{
    // minkowski difference
    CollisionRect diff = {};
    diff.size = A.size + B.size;
    diff.pos = B.pos - A.pos;
    v2 lowerLeftCorner = diff.pos - diff.size/2.0f;
    v2 upperRightCorner = lowerLeftCorner + diff.size;
    // is 0 inside?
    return lowerLeftCorner.x < 0 && upperRightCorner.x > 0 && lowerLeftCorner.y < 0 && upperRightCorner.y > 0;
}

v2 collidePop(CollisionRect A, CollisionRect B, v2 direction)
{
    ASSERT(collide(A, B));
    ASSERT(length(direction) > 0.005f);
    CollisionRect diff = {};
    diff.size = A.size + B.size;
    diff.pos = B.pos - A.pos;
    v2 lowerLeftCorner = diff.pos - diff.size/2.0f;
    v2 upperRightCorner = lowerLeftCorner + diff.size;
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
    return direction * scale;
}

v2 collideSlide(CollisionRect A, CollisionRect B, v2 direction)
{
    ASSERT(length(direction) > 0.005f);
    CollisionRect diff = {};
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

v2 collideReflect(CollisionRect A, CollisionRect B, v2 direction)
{
    // TODO corner hit
    ASSERT(length(direction) > 0.005f);
    CollisionRect diff = {};
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
