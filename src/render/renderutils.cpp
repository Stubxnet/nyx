#include "raylib.h"
#include "rlgl.h"

void DrawCubeTexture(Texture2D texture, Rectangle source, Vector3 position, float width, float height, float length, bool drawFront, bool drawBack, bool drawTop, bool drawBottom, bool drawRight, bool drawLeft, Color color)
{
    float x = position.x;
    float y = position.y;
    float z = position.z;
    float texWidth = (float)texture.width;
    float texHeight = (float)texture.height;

    rlSetTexture(texture.id);

    rlBegin(RL_QUADS);
        rlColor4ub(color.r, color.g, color.b, color.a);

        if (drawFront) {
            // Front face
            rlNormal3f(0.0f, 0.0f, 1.0f);
            rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x - width/2, y - height/2, z + length/2);
            rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x + width/2, y - height/2, z + length/2);
            rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
            rlVertex3f(x + width/2, y + height/2, z + length/2);
            rlTexCoord2f(source.x/texWidth, source.y/texHeight);
            rlVertex3f(x - width/2, y + height/2, z + length/2);
        }

        if (drawBack) {
            // Back face
            rlNormal3f(0.0f, 0.0f, -1.0f);
            rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x - width/2, y - height/2, z - length/2);
            rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
            rlVertex3f(x - width/2, y + height/2, z - length/2);
            rlTexCoord2f(source.x/texWidth, source.y/texHeight);
            rlVertex3f(x + width/2, y + height/2, z - length/2);
            rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x + width/2, y - height/2, z - length/2);
        }

        if (drawTop) {
            // Top face
            rlNormal3f(0.0f, 1.0f, 0.0f);
            rlTexCoord2f(source.x/texWidth, source.y/texHeight);
            rlVertex3f(x - width/2, y + height/2, z - length/2);
            rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x - width/2, y + height/2, z + length/2);
            rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x + width/2, y + height/2, z + length/2);
            rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
            rlVertex3f(x + width/2, y + height/2, z - length/2);
        }

        if (drawBottom) {
            // Bottom face
            rlNormal3f(0.0f, -1.0f, 0.0f);
            rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
            rlVertex3f(x - width/2, y - height/2, z - length/2);
            rlTexCoord2f(source.x/texWidth, source.y/texHeight);
            rlVertex3f(x + width/2, y - height/2, z - length/2);
            rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x + width/2, y - height/2, z + length/2);
            rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x - width/2, y - height/2, z + length/2);
        }

        if (drawRight) {
            // Right face
            rlNormal3f(1.0f, 0.0f, 0.0f);
            rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x + width/2, y - height/2, z - length/2);
            rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
            rlVertex3f(x + width/2, y + height/2, z - length/2);
            rlTexCoord2f(source.x/texWidth, source.y/texHeight);
            rlVertex3f(x + width/2, y + height/2, z + length/2);
            rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x + width/2, y - height/2, z + length/2);
        }

        if (drawLeft) {
            // Left face
            rlNormal3f(-1.0f, 0.0f, 0.0f);
            rlTexCoord2f(source.x/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x - width/2, y - height/2, z - length/2);
            rlTexCoord2f((source.x + source.width)/texWidth, (source.y + source.height)/texHeight);
            rlVertex3f(x - width/2, y - height/2, z + length/2);
            rlTexCoord2f((source.x + source.width)/texWidth, source.y/texHeight);
            rlVertex3f(x - width/2, y + height/2, z + length/2);
            rlTexCoord2f(source.x/texWidth, source.y/texHeight);
            rlVertex3f(x - width/2, y + height/2, z - length/2);
        }

    rlEnd();

    rlSetTexture(0);
}
