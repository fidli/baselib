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
    diff.pos = A.pos - B.pos;
    v2 lowerLeftCorner = diff.pos - diff.size/2.0f;
    v2 upperRightCorner = lowerLeftCorner + diff.size;
    // is 0 inside?
    return lowerLeftCorner.x < 0 && upperRightCorner.x > 0 && lowerLeftCorner.y < 0 && upperRightCorner.y > 0;
}

v2 collidePop(CollisionRect A, CollisionRect B, v2 direction)
{
    CollisionRect diff = {};
    diff.size = A.size + B.size;
    diff.pos = B.pos;
    v2 lowerLeftCorner = diff.pos - diff.size/2.0f;
    v2 upperRightCorner = lowerLeftCorner + diff.size;
    v2 result = V2(clamp(direction.x + A.pos.x, lowerLeftCorner.x, upperRightCorner.x), clamp(direction.y + A.pos.y, lowerLeftCorner.y, upperRightCorner.y)) - A.pos;
    return result;
}

v2 collideReflect(CollisionRect A, CollisionRect B, v2 direction)
{
    CollisionRect diff = {};
    diff.size = A.size + B.size;
    diff.pos = B.pos;
    v2 lowerLeftCorner = diff.pos - diff.size/2.0f;
    v2 upperRightCorner = lowerLeftCorner + diff.size;
    v2 contact = V2(clamp(-direction.x + A.pos.x, lowerLeftCorner.x, upperRightCorner.x), clamp(-direction.y + A.pos.y, lowerLeftCorner.y, upperRightCorner.y));
    if(contact.x == lowerLeftCorner.x || contact.x == upperRightCorner.x){
        direction.x *= -1.0f;
    }
    else if(contact.y == lowerLeftCorner.y || contact.y == upperRightCorner.y){
        direction.y *= -1.0f;
    }
    return direction;
}
