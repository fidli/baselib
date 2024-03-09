#pragma once
#include "common.h"

#include "util_math.cpp"

struct CollisionCircle{
    v2 pos;
    f32 radius;
};

struct CollisionRectAxisAligned{
    v2 size;
    v2 pos;
};

struct CollisionRoundedRectAxisAligned{
    v2 size;
    v2 pos;
    f32 radius;
};


static v2 halo = {0.0001f, 0.0001f};

static CollisionRectAxisAligned minkowskiDiff(CollisionRectAxisAligned A, CollisionRectAxisAligned B)
{
    CollisionRectAxisAligned diff = {};
    diff.size = A.size + B.size;
    diff.pos = B.pos - A.pos;
    return diff;
}

static CollisionCircle minkowskiDiff(CollisionCircle A, CollisionCircle B)
{
    CollisionCircle diff = {};
    diff.radius = A.radius + B.radius;
    diff.pos = B.pos - A.pos;
    return diff;
}

static CollisionRoundedRectAxisAligned minkowskiDiff(CollisionCircle A, CollisionRectAxisAligned B)
{
    CollisionRoundedRectAxisAligned diff = {};
    diff.size = B.size;
    diff.radius = A.radius;
    diff.pos = B.pos - A.pos;
    return diff;
}

bool is0Inside(CollisionRectAxisAligned A)
{
    // is 0 inside?
    v2 lowerLeftCorner = A.pos - A.size/2.0f;
    v2 upperRightCorner = lowerLeftCorner + A.size;
    return lowerLeftCorner.x < 0 && upperRightCorner.x > 0 && lowerLeftCorner.y < 0 && upperRightCorner.y > 0;
}

bool is0Inside(CollisionCircle A)
{
    return length(-A.pos) < A.radius;
}

bool collide(CollisionRectAxisAligned A, CollisionRectAxisAligned B)
{
    // minkowski difference
    CollisionRectAxisAligned diff = minkowskiDiff(A, B);
    return is0Inside(diff);
}

bool collide(CollisionCircle A, CollisionCircle B)
{
    CollisionCircle diff = minkowskiDiff(A, B);
    return is0Inside(diff);
}

bool collide(CollisionCircle A, CollisionRectAxisAligned B)
{
    CollisionRoundedRectAxisAligned diff = minkowskiDiff(A, B);

    CollisionRectAxisAligned boxA;
    boxA.pos = diff.pos;
    boxA.size = diff.size + V2(A.radius, 0);

    CollisionRectAxisAligned boxB;
    boxB.pos = diff.pos;
    boxB.size = diff.size + V2(0, A.radius);

    CollisionCircle tr, br, bl, tl;
    tr.radius = br.radius = bl.radius = tl.radius = A.radius;
    tr.pos = -(diff.pos + B.size/2.0f);
    br.pos = -(diff.pos + V2(B.size.x, -B.size.y)/2.0f);
    bl.pos = -(diff.pos + V2(-B.size.x, -B.size.y)/2.0f);
    tl.pos = -(diff.pos + V2(-B.size.x, B.size.y)/2.0f);

    return is0Inside(boxA) || is0Inside(boxB) || is0Inside(tr) || is0Inside(br) || is0Inside(bl) || is0Inside(tl);

}

static v2 collidePop(CollisionCircle A, v2 direction)
{
    f32 b = 2*(A.pos.x*direction.x + A.pos.y*direction.y);
    f32 a = (direction.x*direction.x + direction.y*direction.y);
    f32 c = A.pos.x*A.pos.x + A.pos.y+A.pos.y - A.radius*A.radius;
    f32 D = b*b -4*a*c;
    ASSERT(D >= 0);
    f32 scale1 = (-b - sqrt(D)) / 2*a;
    f32 scale2 = (-b + sqrt(D)) / 2*a;
    ASSERT(scale1 > 0 || scale2 > 0);
    if (scale1 > scale2)
    {
        return scale1 * direction;
    }
    return scale2 * direction;
    
}

static v2 collidePop(CollisionRectAxisAligned A, v2 direction)
{
    v2 lowerLeftCorner = A.pos - A.size/2.0f;
    v2 upperRightCorner = lowerLeftCorner + A.size;
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

v2 collidePop(CollisionCircle A, CollisionCircle B, v2 direction)
{
    ASSERT(collide(A, B));
    ASSERT(!isTiny(direction));
    CollisionCircle diff = minkowskiDiff(A, B);
    // epsilons to pop just above the border
    diff.radius += halo.x;
    v2 res = collidePop(diff, direction);
#ifndef RELEASE
    A.pos += res;
    ASSERT(!collide(A, B));
#endif
    return res;

}

v2 collidePop(CollisionRectAxisAligned A, CollisionRectAxisAligned B, v2 direction)
{
    ASSERT(collide(A, B));
    ASSERT(!isTiny(direction));
    CollisionRectAxisAligned diff = minkowskiDiff(A, B);
    // epsilons to pop just above the border
    diff.size += halo;
    v2 res = collidePop(diff, direction);
#ifndef RELEASE
    A.pos += res;
    ASSERT(!collide(A, B));
#endif
    return res;

}

v2 collidePop(CollisionCircle A, CollisionRectAxisAligned B, v2 direction)
{
    ASSERT(collide(A, B));
    ASSERT(!isTiny(direction));
    v2 totalShift = V2(0, 0);

    CollisionRoundedRectAxisAligned diff = minkowskiDiff(A, B);
    diff.size += halo;

    CollisionRectAxisAligned boxA;
    boxA.pos = diff.pos;
    boxA.size = diff.size + V2(A.radius, 0);
    if (is0Inside(boxA))
    {
        v2 shift = collidePop(boxA, direction);
        totalShift += shift;
        diff.pos -= shift;
    }

    CollisionRectAxisAligned boxB;
    boxB.pos = diff.pos;
    boxB.size = diff.size + V2(0, A.radius);
    if (is0Inside(boxB))
    {
        v2 shift = collidePop(boxB, direction);
        totalShift += shift;
        diff.pos -= shift;
    }

    CollisionCircle circ;
    circ.radius = diff.radius + halo.x;
    circ.pos = -(diff.pos + B.size/2.0f);
    if (is0Inside(circ))
    {
        v2 shift = collidePop(circ, direction);
        totalShift += shift;
        diff.pos -= shift;
    }

    circ.pos = -(diff.pos + V2(B.size.x, -B.size.y)/2.0f);
    if (is0Inside(circ))
    {
        v2 shift = collidePop(circ, direction);
        totalShift += shift;
        diff.pos -= shift;
    }

    circ.pos = -(diff.pos + V2(-B.size.x, -B.size.y)/2.0f);
    if (is0Inside(circ))
    {
        v2 shift = collidePop(circ, direction);
        totalShift += shift;
        diff.pos -= shift;
    }

    circ.pos = -(diff.pos + V2(-B.size.x, B.size.y)/2.0f);
    if (is0Inside(circ))
    {
        v2 shift = collidePop(circ, direction);
        totalShift += shift;
    }

#ifndef RELEASE
    A.pos += totalShift;
    ASSERT(!collide(A, B));
#endif
    return totalShift;
}

v2 collideSlide(CollisionRectAxisAligned A, CollisionRectAxisAligned B, v2 direction)
{
    ASSERT(!isTiny(direction));
    CollisionRectAxisAligned diff = minkowskiDiff(A, B);
    diff.size += halo;
    v2 lowerLeftCorner = diff.pos - diff.size/2.0f;
    v2 upperRightCorner = lowerLeftCorner + diff.size;
    ASSERT(isTiny(lowerLeftCorner.x) || isTiny(upperRightCorner.x) || isTiny(lowerLeftCorner.y) || isTiny(upperRightCorner.y));

    // TODO corner hit dont slide?
    if(isTiny(lowerLeftCorner.x) || isTiny(upperRightCorner.x)){
        return dot(direction, V2(0.0f, 1.0f)) * V2(0.0f, 1.0f);
    }
    if(isTiny(lowerLeftCorner.y) || isTiny(upperRightCorner.y)){
        return dot(direction, V2(1.0f, 0.0f)) * V2(1.0f, 0.0f);
    }
    return direction;
}

v2 collideReflect(CollisionRectAxisAligned A, CollisionRectAxisAligned B, v2 direction)
{
    // TODO corner hit
    ASSERT(!isTiny(direction));
    CollisionRectAxisAligned diff = minkowskiDiff(A, B);
    diff.size += halo;
    v2 lowerLeftCorner = diff.pos - diff.size/2.0f;
    v2 upperRightCorner = lowerLeftCorner + diff.size;
    ASSERT(isTiny(lowerLeftCorner.x) || isTiny(upperRightCorner.x) || isTiny(lowerLeftCorner.y) || isTiny(upperRightCorner.y));
    if(isTiny(lowerLeftCorner.x) || isTiny(upperRightCorner.x)){
        direction.x *= -1.0f;
    }
    if(isTiny(lowerLeftCorner.y) || isTiny(upperRightCorner.y)){
        direction.y *= -1.0f;
    }
    return direction;
}

v2 collideReflect(CollisionCircle A, CollisionRectAxisAligned B, v2 direction)
{
    ASSERT(!isTiny(direction));
    CollisionRoundedRectAxisAligned diff = minkowskiDiff(A, B);
    diff.size += halo;

    v2 lowerLeftCorner = diff.pos - diff.size/2.0f;
    v2 upperRightCorner = lowerLeftCorner + diff.size;
    v2 lowerRightCorner = V2(upperRightCorner.x, lowerLeftCorner.y);
    v2 upperLeftCorner = V2(lowerLeftCorner.x, upperRightCorner.y);

    v2 norm = {};
    if (0 < upperLeftCorner.x)
    {
        if (0 < upperLeftCorner.y){
           norm = normalize(-upperLeftCorner); 
        }
        else if(0 < lowerLeftCorner.y)
        {
            norm = V2(-1, 0);
        }
        else
        {
            ASSERT(0 >= lowerLeftCorner.y);
            norm = normalize(-lowerLeftCorner);
        }
    }
    else if( 0 < upperRightCorner.x)
    {
        if (0 < upperRightCorner.y)
        {
            norm = V2(0, 1);
        }
        else
        {
            ASSERT(0 >= lowerRightCorner.y);
            norm = V2(0, -1);
        }
    }
    else
    {
        ASSERT(0 >= upperRightCorner.x);
        if (0 < upperRightCorner.y){
           norm = normalize(-upperRightCorner); 
        }
        else if(0 < lowerRightCorner.y)
        {
            norm = V2(1, 0);
        }
        else
        {
            ASSERT(0 >= lowerRightCorner.y);
            norm = normalize(-lowerRightCorner);
        }
    }

    ASSERT(norm.x != 0 || norm.y != 0);

    return direction - 2*dot(direction,norm)*norm;
}

v2 collideSlide(CollisionCircle A, CollisionRectAxisAligned B, v2 direction)
{
    ASSERT(!isTiny(direction));
    CollisionRoundedRectAxisAligned diff = minkowskiDiff(A, B);
    diff.size += halo;

    v2 lowerLeftCorner = diff.pos - diff.size/2.0f;
    v2 upperRightCorner = lowerLeftCorner + diff.size;
    v2 lowerRightCorner = V2(upperRightCorner.x, lowerLeftCorner.y);
    v2 upperLeftCorner = V2(lowerLeftCorner.x, upperRightCorner.y);

    v2 norm = {};
    if (0 < upperLeftCorner.x)
    {
        if (0 < upperLeftCorner.y){
           norm = normalize(-upperLeftCorner); 
        }
        else if(0 < lowerLeftCorner.y)
        {
            norm = V2(-1, 0);
        }
        else
        {
            ASSERT(0 >= lowerLeftCorner.y);
            norm = normalize(-lowerLeftCorner);
        }
    }
    else if( 0 < upperRightCorner.x)
    {
        if (0 < upperRightCorner.y)
        {
            norm = V2(0, 1);
        }
        else
        {
            ASSERT(0 >= lowerRightCorner.y);
            norm = V2(0, -1);
        }
    }
    else
    {
        ASSERT(0 >= upperRightCorner.x);
        if (0 < upperRightCorner.y){
           norm = normalize(-upperRightCorner); 
        }
        else if(0 < lowerRightCorner.y)
        {
            norm = V2(1, 0);
        }
        else
        {
            ASSERT(0 >= lowerRightCorner.y);
            norm = normalize(-lowerRightCorner);
        }
    }

    ASSERT(norm.x != 0 || norm.y != 0);
    v2 normRotated = V2(-norm.y, norm.x);
    
    v2 res = dot(normRotated, direction)*normRotated;
    if (dot(res, direction) < 0)
    {
        return -res;
    }
    return res;

}

v2 collideReflect(CollisionCircle A, CollisionCircle B, v2 direction)
{
    ASSERT(!isTiny(direction));
    v2 norm = normalize(A.pos - B.pos);
    return direction - 2*dot(direction,norm)*norm;
}
