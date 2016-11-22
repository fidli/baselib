#ifndef UTIL_PHYSICS
#define UTIL_PHYSICS
#include "util_math.cpp"

bool isInBox(const box * check, const v3 * point){
    return(point->x >= check->lowerCorner.x && point->y >= check->lowerCorner.y && point->z <= check->lowerCorner.z
        && point->x <= check->upperCorner.x && point->y <= check->upperCorner.y && point->z >= check->upperCorner.z);
}

v3 getForwardDirection(v2 * facing){
    return quaternionToMatrix(normalize(Quat(V3(1,0,0), facing->yaw) * Quat(V3(0,1,0), facing->pitch))) *V3(0,0,-1);
}

v3 getRightArmDirection(v2 * facing){
    return quaternionToMatrix(normalize(Quat(V3(1,0,0), facing->yaw) * Quat(V3(0,1,0), facing->pitch))) *V3(1,0,0);
}

v3 getHeadUpDirection(v2 * facing){
    return quaternionToMatrix(normalize(Quat(V3(1,0,0), facing->yaw) * Quat(V3(0,1,0), facing->pitch))) *V3(0,1,0);
}

v3 getUpwardDirection(v2 * facing){
    return V3(0,1,0);
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