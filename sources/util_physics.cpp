#ifndef UTIL_PHYSICS
#define UTIL_PHYSICS
#include "common.h"

#include "util_math.cpp"
#include "util_sort.cpp"

//NOTE(AK): Alert, this util file expects +x to right, +y to upper, +z forward through the screen

struct Box{
    v3 lowerCorner;
    v3 upperCorner;
};

struct Box_64{
    v3_64 lowerCorner;
    v3_64 upperCorner;
};


struct Sphere_64{
    v3_64 origin;
    float64 radius;
};

struct Plane_64{
    v3_64 point;
    v3_64 norm;
};

struct Circle3D_64{
    v3_64 origin;
    float64 radius;
    Plane_64 plane;
};

struct Segment3D_64{
    v3_64 A;
    v3_64 B;
};

#define Segment1D_64 v2_64

bool isInBox(const Box * check, const v3 * point){
    return(point->x >= check->lowerCorner.x && point->y >= check->lowerCorner.y && point->z <= check->lowerCorner.z
           && point->x <= check->upperCorner.x && point->y <= check->upperCorner.y && point->z >= check->upperCorner.z);
}

bool isInBox64(const Box_64 * check, const v3_64 * point){
    return(point->x >= check->lowerCorner.x && point->y >= check->lowerCorner.y && point->z <= check->lowerCorner.z
           && point->x <= check->upperCorner.x && point->y <= check->upperCorner.y && point->z >= check->upperCorner.z);
}

struct PointOnSegment1D_64{
    const float64 * point;
    const Segment1D_64 * segment;
};

bool intersect1DSegments64(const Segment1D_64 * A, const Segment1D_64 * B, Segment1D_64 * result){
    
    PointOnSegment1D_64 points[4];
    points[0].point = &A->a;
    points[0].segment = A;
    
    points[1].point = &A->b;
    points[1].segment = A;
    
    points[2].point = &B->a;
    points[2].segment = B;
    
    points[3].point = &B->b;
    points[3].segment = B;
    
    //insert sort, is not stable, but we do not care
    insertSort((byte*)points, sizeof(PointOnSegment1D_64), ARRAYSIZE(points), [](void * a, void *b) -> int8 {
               return *(((PointOnSegment1D_64 *) a)->point) > *(((PointOnSegment1D_64 *) b)->point) ? 1 : -1;
               });
    
    if(points[0].segment == points[1].segment){
        if(*points[1].point == *points[2].point){
            result->b = result->a = *points[2].point;
            return true;
        }
        return false;
    }
    result->a = *points[1].point;
    result->b = *points[2].point;
    return true;
}



bool intersectBoxes64(const Box_64 * A, const Box_64 * B, Box_64 * result){
    Segment1D_64 Ax = {A->lowerCorner.x, A->upperCorner.x};
    Segment1D_64 Bx = {B->lowerCorner.x, B->upperCorner.x};
    Segment1D_64 Rx;
    
    if(intersect1DSegments64(&Ax, &Bx, &Rx)){
        Segment1D_64 Ay = {A->lowerCorner.y, A->upperCorner.y};
        Segment1D_64 By = {B->lowerCorner.y, B->upperCorner.y};
        Segment1D_64 Ry;
        if(intersect1DSegments64(&Ay, &By, &Ry)){
            Segment1D_64 Az = {A->lowerCorner.z, A->upperCorner.z};
            Segment1D_64 Bz = {B->lowerCorner.z, B->upperCorner.z};
            Segment1D_64 Rz;
            if(intersect1DSegments64(&Az, &Bz, &Rz)){
                result->lowerCorner = {Rx.a, Ry.a, Rz.a};
                result->upperCorner = {Rx.b, Ry.b, Rz.b};
                return true;
            }
        }
    }
    
    return false;
}



bool intersectSpheresAABB64(const Sphere_64 * A, const Sphere_64 * B, Box_64 * result){
    
    float64 distance = length64(A->origin - B->origin);
    //the spheres are too far
    if(aseqr64(B->radius + A->radius, distance)){
        return false;
    }
    //one sphere is inside the other
    if(aseql64(A->radius, distance + B->radius) || aseql64(B->radius, distance + A->radius)){
        const Sphere_64 * source;
        if(A->radius > B->radius){
            source = B;
        }else{
            source = A;
        }
        result->upperCorner = source->origin + V3_64(source->radius, source->radius, source->radius);
        result->lowerCorner = source->origin - V3_64(source->radius, source->radius, source->radius);
        return true;
    }
    
    //else, there must be an intersection
    v3_64 AtoB = B->origin - A->origin;
    v3_64 AtoBNormalised = normalize64(AtoB);
    
    v3_64 A1 = A->origin + AtoBNormalised * A->radius;
    v3_64 B1 = B->origin + AtoBNormalised * (-1) * B->radius;
    
    float64 halfIntersectionWidth = length64(A1-B1) / 2.0f;
    
    v3_64 origin = B1 + AtoBNormalised * halfIntersectionWidth;
    float64 upperAndBackwards = sqrt64(powd(A->radius) - powd(A->radius - halfIntersectionWidth));
    
    float64 sideways = halfIntersectionWidth;
    
    result->upperCorner = origin + V3_64(sideways, upperAndBackwards, upperAndBackwards);
    result->lowerCorner = origin - V3_64(sideways, upperAndBackwards, upperAndBackwards);
    
    return true;
}

bool intersectSpheres64(const Sphere_64 * A, const Sphere_64 * B, Circle3D_64 * result){
    float64 distance = length64(A->origin - B->origin);
    //the spheres are too far
    if(aseqr64(B->radius + A->radius, distance)){
        return false;
    }
    //one sphere is inside the other
    if(aseql64(A->radius, distance + B->radius) || aseql64(B->radius, distance + A->radius)){
        return false;
    }
    //else, there must be an intersection
    v3_64 AtoB = B->origin - A->origin;
    v3_64 AtoBNormalised = normalize64(AtoB);
    
    v3_64 A1 = A->origin + AtoBNormalised * A->radius;
    v3_64 B1 = B->origin + AtoBNormalised * (-1) * B->radius;
    
    float64 halfIntersectionWidth = length64(A1-B1) / 2.0f;
    
    result->origin = B1 + AtoBNormalised * halfIntersectionWidth;
    result->radius = sqrt64(powd(A->radius) - powd(A->radius - halfIntersectionWidth));
    
    result->plane.point = result->origin;
    result->plane.norm = A->origin - B->origin;
    
    return true;
}

bool intersectCircles3D64(const Circle3D_64 * A, const Circle3D_64 * B, Segment3D_64 * result){
    //NOTE(AK)::this is insane, consider solving the problem in a completely different way
    ASSERT(false);
    /* NOTE(AK):: if implemented, this is on good way i guess 
    v3_64 planeANorm = normalize64(A->plane.norm);
    v3_64 planeBNorm = normalize64(B->plane.norm);
    
    //parallel
    if(aseq64(planeANorm.x, planeBNorm.x) && aseq64(planeANorm.y, planeBNorm.y) && aseq64(planeANorm.z, planeBNorm.z)
    ||
    aseq64(-planeANorm.x, planeBNorm.x) && aseq64(-planeANorm.y, planeBNorm.y) && aseq64(-planeANorm.z, planeBNorm.z)){
    //are they the same, or parallel?
    if(aseq64(A->plane.point.x, B->plane.point.x) && aseq64(A->plane.point.y, B->plane.point.y) && aseq64(A->plane.point.z, B->plane.point.z)
    ||
    aseq64(dot64(A->plane.point - B->plane.point, planeBNorm), 0)){
    //they are the same, this is circle intersect in plane 
    float64 circlesDistance = length64(A->origin - B->origin);
    //the circles are too far
    if(aseqr64(B->radius + A->radius, circlesDistance)){
    return false;
    }
    //one circle is inside the other
    if(aseql64(A->radius, circlesDistance + B->radius) || aseql64(B->radius, circlesDistance + A->radius)){
    return false;
    }
    //else, there must be an intersection
    v3_64 AtoB = B->origin - A->origin;
    v3_64 AtoBNormalised = normalize64(AtoB);
    
    v3_64 A1 = A->origin + AtoBNormalised * A->radius;
    v3_64 B1 = B->origin + AtoBNormalised * (-1) * B->radius;
    
    float64 halfIntersectionWidth = length64(A1-B1) / 2.0f;
    
    v3_64 halfSegmentPoint = B1 + AtoBNormalised * halfIntersectionWidth;
    float64 halfSegmentLength= sqrt64(powd(A->radius) - powd(A->radius - halfIntersectionWidth));
    v3_64 AtoHalfSegmentPoint = halfSegmentPoint - A->origin;
    
    v4_64 eq1 = V4_64(planeANorm.x, planeANorm.y, planeANorm.z, A->origin.x*planeANorm.x + A->origin.y*planeANorm.y + A->origin.z*planeANorm.z);
    v4_64 eq2 = V4_64(planeANorm.x, planeANorm.y, planeANorm.z, halfSegmentPoint.x*planeANorm.x + halfSegmentPoint.y*planeANorm.y + halfSegmentPoint.z*planeANorm.z);
    v4_64 eq3 = V4_64(AtoHalfSegmentPoint.x, AtoHalfSegmentPoint.y, AtoHalfSegmentPoint.z, halfSegmentPoint.x*halfSegmentPoint.x + halfSegmentPoint.y*halfSegmentPoint.y + halfSegmentPoint.z*halfSegmentPoint.z);
    
    v3_64 halfSegmentToIntersectionPoint1;
    
    //ad hoc GEM
    if(eq1.x == 0){
    if(eq3.x == 0){
    //x is a degree of freedom
    if(eq1.y == 0){
    
    //this would mean that norm vector of a plane is (0, 0, 0), which is never the case
    ASSERT(eq1.z != 0);
    
    
    halfSegmentToIntersectionPoint1.z = eq2.w / eq2.z;
    if(aseq64(eq1.w, eq1.z * halfSegmentToIntersectionPoint1.z)){
    //y is degree of freedom, but the coef is 0
    halfSegmentToIntersectionPoint1.y = 0;
    if(aseq64((eq3.w - (eq3.y * halfSegmentToIntersectionPoint1.y) - (eq3.z * halfSegmentToIntersectionPoint1.z)), 0)){
    halfSegmentToIntersectionPoint1.x = 0;
    }else{
    //no solution
    return false;
    }
    
    }else{
    //no solution
    return false;
    }
    }else{
    if(aseq64(eq1.w, eq2.w)){
    if(eq1.w == 0){
    halfSegmentToIntersectionPoint1.z = 0;
    halfSegmentToIntersectionPoint1.y = 0;
    halfSegmentToIntersectionPoint1.x = (eq3.w - (eq3.y * halfSegmentToIntersectionPoint1.y) - (eq3.z * halfSegmentToIntersectionPoint1.z)) / eq3.x;
    }else{
    halfSegmentToIntersectionPoint1.z = 0;
    halfSegmentToIntersectionPoint1.y = eq2.w / eq2.y;
    if(aseq64(eq3.w - (eq3.y * halfSegmentToIntersectionPoint1.y) - (eq3.z * halfSegmentToIntersectionPoint1.z), 0)){
    halfSegmentToIntersectionPoint1.x = 0;
    }else{
    //no solution
    return false;
    }
    
    }
    }else{
    //no solution
    return false;
    }
    }
    }else{
    //eq3 is top Row now
    if(eq1.y == 0){
    
    //this would mean that norm vector of a plane is (0, 0, 0), which is never the case
    ASSERT(eq1.z != 0);
    
    
    halfSegmentToIntersectionPoint1.z = eq2.w / eq2.z;
    if(aseq64(eq1.w, eq1.z * halfSegmentToIntersectionPoint1.z)){
    //y is degree of freedom, but the coef is 0
    halfSegmentToIntersectionPoint1.y = 0;
    halfSegmentToIntersectionPoint1.x = (eq3.w - (eq3.y * halfSegmentToIntersectionPoint1.y) - (eq3.z * halfSegmentToIntersectionPoint1.z)) / eq3.x;
    }else{
    //no solution
    return false;
    }
    }else{
    if(aseq64(eq1.w, eq2.w)){
    if(eq1.w == 0){
    halfSegmentToIntersectionPoint1.z = 0;
    halfSegmentToIntersectionPoint1.y = 0;
    halfSegmentToIntersectionPoint1.x = (eq3.w - (eq3.y * halfSegmentToIntersectionPoint1.y) - (eq3.z * halfSegmentToIntersectionPoint1.z)) / eq3.x;
    }else{
    halfSegmentToIntersectionPoint1.z = 0;
    halfSegmentToIntersectionPoint1.y = eq2.w / eq2.y;
    halfSegmentToIntersectionPoint1.x = (eq3.w - (eq3.y * halfSegmentToIntersectionPoint1.y) - (eq3.z * halfSegmentToIntersectionPoint1.z)) / eq3.x;
    }
    }else{
    //no solution
    return false;
    }
    }
    }
    }else{
    //eq1 and eq2.x != 0 
    float64 attun = eq3.x / eq1.x;
    eq3 -= attun * eq1;
    if(aseq64(eq3.y, 0)){
    
    if(aseq64(eq3.z, 0)){
    if(aseq64(eq3.w, 0)){
    //z is degree of freedom
    halfSegmentToIntersectionPoint1.z = 0;
    if(aseq64(eq2.y, 0)){
    if(!aseq64(eq2.w, 0)){
    return false;
    }else{
    //y is degree of freedom
    halfSegmentToIntersectionPoint1.y = 0;
    halfSegmentToIntersectionPoint1.x = eq1.w / eq1.x;
    }
    }else{
    halfSegmentToIntersectionPoint1.y = (eq2.w) / eq2.y;
    halfSegmentToIntersectionPoint1.x = (eq1.w - eq1.y * halfSegmentToIntersectionPoint1.y))  / eq1.x;
    }
    }else{
    return false;
    }
    }else{
    halfSegmentToIntersectionPoint1.z = eq3.w / eq3.z;
    if(aseq64(eq2.y, 0)){
    if(aseq64(eq2.w, eq2.z * halfSegmentToIntersectionPoint1.z)){
    //y is degree of freedom, let it be 0
    halfSegmentToIntersectionPoint1.y = 0;
    halfSegmentToIntersectionPoint1.x = (eq1.w - eq1.z * halfSegmentToIntersectionPoint1.z) / eq1.x;
    }else{
    return false;
    }
    }else{
    halfSegmentToIntersectionPoint1.y = (eq2.w - eq2.z * halfSegmentToIntersectionPoint1.z) / eq2.y;
    halfSegmentToIntersectionPoint1.x = (eq1.w - eq1.z * halfSegmentToIntersectionPoint1.z - eq1.y * halfSegmentToIntersectionPoint1.y))  / eq1.x;
    }
    }
    }
    
    }
    
    }
    //parallel and not the same
    return false;
    }else{
    //intersection is a circle intersect sitting on a line (which is on the intersection of the planes)
    //circleA
    v4_64 eq1 = V4_64(planeANorm.x, planeANorm.y, planeANorm.z, A->origin.x*planeANorm.x + A->origin.y*planeANorm.y + A->origin.z*planeANorm.z);
    v4_64 eq2 = V4_64(planeBNorm.x, planeBNorm.y, planeBNorm.z, B->origin.x*planeBNorm.x + B->origin.y*planeBNorm.y + B->origin.z*planeBNorm.z);
    
    }
    return true;
    */
    return false;
    
}



//redo these. when needed. so they are more general

/*
//easy AABB check for now
bool doCollide(const box * a, const entity * b){
//minkowski sum
v3 halfDim = lerp(&b->common.AABB.lowerCorner, &b->common.AABB.upperCorner, 0.5f);
v3 worldPoint = b->common.position + halfDim;

box worldBox = a->common.AABB;

worldBox.lowerCorner += a->common.position - halfDim;
worldBox.upperCorner += a->common.position + halfDim;

return isInBox(&worldBox, &worldPoint);
}*/

/*
static bool raycast(void * data, Uint16 byteSkip, Uint32 size, const v3 * rayBegin, const v3 * rayEnd, Uint32 * dataIndex){
//TODO: select the smaller one if multiple in ray, or if similar size, the closer to ray begin
bool found = false;
common_entity * candidate = 0;
common_entity * target = (common_entity * ) data;
for(int entityIndex = 0; entityIndex < size; entityIndex++, target = (common_entity *)(((Uint8 * ) target) + byteSkip)){
box check = target->AABB;
//to to worldSpace
check.lowerCorner += target->position;
check.upperCorner += target->position;
//stupid naive, bruteforce technique get rid of it as soon as possible
for(float32 c = 0; c < 1.0f; c += 0.0025f){
v3 point = lerp(rayBegin, rayEnd, c);
if(isInBox(&check, &point)){
if(found){
v3 previousLower = candidate->AABB.lowerCorner + candidate->position;
if(length(previousLower - *rayBegin) > length(check.lowerCorner - *rayBegin)){
candidate = target;
*dataIndex = entityIndex;
}
}else{
found = true;
*dataIndex = entityIndex;
candidate = target;
}
}
}
}
return found;
}

//note raycasting calls will change with upcoming entity system and storage change

//raycast over common entities
bool raycast(common_entity * data, Uint32 size, const v3 * rayBegin, const v3 * rayEnd, Uint32 * dataIndex){
return raycast((void * ) data, sizeof(common_entity), size, rayBegin, rayEnd, dataIndex);
}

//raycast over unions
bool raycast(entity * data, Uint32 size, const v3 * rayBegin, const v3 * rayEnd, Uint32 * dataIndex){
return raycast((void * ) data, sizeof(entity), size, rayBegin, rayEnd, dataIndex);
}


*/

/*
//calculate AABB according to model geometry
void calculateAABB(entity * target, modelDatabase * modelDb){
switch(target->type){
case entity_common:
case entity_ghost:
//todo: get rid of scaling once baked in parser
target->common.AABB.lowerCorner = modelDb->models[target->common.modelIndex].predefinedScale * modelDb->models[target->common.modelIndex].AABB.lowerCorner;
target->common.AABB.upperCorner = modelDb->models[target->common.modelIndex].predefinedScale * modelDb->models[target->common.modelIndex].AABB.upperCorner;
break;
case entity_point_light:
target->common.AABB.lowerCorner = V3(-0.5f, -0.5f, 0.5f);
target->common.AABB.upperCorner = V3(0.5f, 0.5f, -0.5f);
break;
case entity_player:
target->common.AABB.lowerCorner = V3(-0.5f, -0.5f, 0.5f);
target->common.AABB.upperCorner = V3(0.5f, 0.5f, -0.5f);
break;
default:
break;
}
}*/
#endif