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
    boxA.size = diff.size + V2(2*A.radius, 0);

    CollisionRectAxisAligned boxB;
    boxB.pos = diff.pos;
    boxB.size = diff.size + V2(0, 2*A.radius);

    CollisionCircle tr, br, bl, tl;
    tr.radius = br.radius = bl.radius = tl.radius = A.radius;
    tr.pos = diff.pos + diff.size/2.0f;
    br.pos = diff.pos + V2(diff.size.x, -diff.size.y)/2.0f;
    bl.pos = diff.pos - diff.size/2.0f;
    tl.pos = diff.pos + V2(-diff.size.x, diff.size.y)/2.0f;

    return is0Inside(boxA) || is0Inside(boxB) || is0Inside(tr) || is0Inside(br) || is0Inside(bl) || is0Inside(tl);

}

static v2 collidePop(CollisionCircle A, v2 direction)
{
    ASSERT(is0Inside(A));
    f32 a = ((direction.x*direction.x) + (direction.y*direction.y));
    f32 b = -2*((A.pos.x*direction.x) + (A.pos.y*direction.y));
    f32 c = (A.pos.x*A.pos.x) + (A.pos.y*A.pos.y) - (A.radius*A.radius);
    f32 D = (b*b) - (4*a*c);
    ASSERT(D >= 0);
    f32 scale1 = ((-b) - sqrt(D)) / (2.0f*a);
    f32 scale2 = ((-b) + sqrt(D)) / (2.0f*a);
    ASSERT(scale1 > 0 || scale2 > 0);
    ASSERT(scale1 * scale2 < 0);
    v2 res = {};
    if (scale1 > scale2)
    {
        res = scale1 * direction;
    }
    else
    {
        res =  scale2 * direction;
    }

    ASSERT(length(res-A.pos) > A.radius);
    // Sometimes numbers are small, 0.05 seems like ok cutoff, visually looks ok
    ASSERT(aseq(length(res-A.pos), A.radius, 0.05f));

    return res;
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
    diff.radius += 2*halo.x;
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
    diff.size += 2*halo;
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
    diff.size += 2*halo;

    CollisionRectAxisAligned box[2];
    box[0].size = diff.size + V2(2*A.radius, 0);
    box[0].pos = diff.pos;
    box[1].size = diff.size + V2(0, 2*A.radius);
    box[1].pos = diff.pos;

    CollisionCircle circ[4];
    circ[0].radius = diff.radius + halo.x;
    circ[0].pos = (diff.pos + (diff.size-halo)/2.0f);
    circ[1].radius = diff.radius + halo.x;
    circ[1].pos = (diff.pos + V2(-diff.size.x-halo.x, diff.size.y-halo.y)/2.0f);
    circ[2].radius = diff.radius + halo.x;
    circ[0].pos = (diff.pos + V2(diff.size.x-halo.x, -diff.size.y-halo.y)/2.0f);
    circ[3].radius = diff.radius + halo.x;
    circ[3].pos = (diff.pos + V2(-diff.size.x-halo.x, -diff.size.y-halo.y)/2.0f);

    u8 checks = 7;
    bool retest = true;
    while(checks > 0 && retest)
    {
        retest = false;
        for(i32 i = 0; i < ARRAYSIZE(box) && !retest; i++)
        {
            CollisionRectAxisAligned testbox = box[i];
            testbox.pos -= totalShift;
            if (is0Inside(testbox))
            {
                v2 shift = collidePop(testbox, direction);
                totalShift += shift;
                retest = true;
            }
        }
        for(i32 i = 0; i < ARRAYSIZE(circ) && !retest; i++)
        {
            CollisionCircle testcircle = circ[i];
            testcircle.pos -= totalShift;
            if (is0Inside(testcircle))
            {
                v2 shift = collidePop(testcircle, direction);
                totalShift += shift;
                retest = true;
            }
        }
    }
    // this should not happen, maximum is 6 checks not 7 for this shape
    ASSERT(checks != 0);

#ifndef RELEASE
    CollisionCircle C = A;
    C.pos += totalShift;
    ASSERT(!collide(C, B));
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

    B.size += halo/2.0f;
    v2 lowerLeftCorner = B.pos - B.size/2.0f;
    v2 upperRightCorner = lowerLeftCorner + B.size;
    v2 lowerRightCorner = V2(upperRightCorner.x, lowerLeftCorner.y);
    v2 upperLeftCorner = V2(lowerLeftCorner.x, upperRightCorner.y);

    v2 norm = {};
    if (A.pos.x < upperLeftCorner.x)
    {
        if (A.pos.y > upperLeftCorner.y){
           norm = normalize(A.pos-upperLeftCorner); 
        }
        else if(A.pos.y > lowerLeftCorner.y)
        {
            norm = V2(-1, 0);
        }
        else
        {
            ASSERT(A.pos.y <= lowerLeftCorner.y);
            norm = normalize(A.pos-lowerLeftCorner);
        }
    }
    else if( A.pos.x < upperRightCorner.x)
    {
        if (A.pos.y > upperRightCorner.y)
        {
            norm = V2(0, 1);
        }
        else
        {
            ASSERT(A.pos.y <= lowerRightCorner.y);
            norm = V2(0, -1);
        }
    }
    else
    {
        ASSERT(A.pos.x >= upperRightCorner.x);
        if (A.pos.y > upperRightCorner.y){
           norm = normalize(A.pos-upperRightCorner); 
        }
        else if(A.pos.y > lowerRightCorner.y)
        {
            norm = V2(1, 0);
        }
        else
        {
            ASSERT(A.pos.y <= lowerRightCorner.y);
            norm = normalize(A.pos-lowerRightCorner);
        }
    }

    ASSERT(norm.x != 0 || norm.y != 0);
    v2 normRotated = V2(-norm.y, norm.x);
    
    v2 res = dot(normRotated, direction)*normRotated;
    ASSERT(length(res) <= length(direction));
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
