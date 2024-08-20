#include "collisions.h"

bool CubePoint(glm::vec3 bbox_min, glm::vec3 bbox_max){
    if(0 >= bbox_min.x && 0 <= bbox_max.x){
        if(0 >= bbox_min.y && 0 <= bbox_max.y){
            if(0 >= bbox_min.z && 0 <= bbox_max.z){
                printf("Colidiu -> asteoide.min x: %f y: %f z: %f\n",bbox_min.x, bbox_min.y, bbox_min.z);
                printf("Colidiu -> asteoide.max x: %f y: %f z: %f\n",bbox_max.x, bbox_max.y, bbox_max.z);
                return true;
            }
        }
    }
    return false;
}