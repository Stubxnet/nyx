#include "raylib.h"

static BoundingBox CameraToPlayerBB(const Vector3 camPos){
    float eyeY = camPos.y;
    float bodyBottomY = eyeY - EYES_Y;
    float bodyTopY    = bodyBottomY + BODY_HEIGHT;
    Vector3 min = { camPos.x - HALF_WIDTH, bodyBottomY, camPos.z - HALF_WIDTH };
    Vector3 max = { camPos.x + HALF_WIDTH, bodyTopY,  camPos.z + HALF_WIDTH  };
    BoundingBox bb = { min, max };
    return bb;
}
