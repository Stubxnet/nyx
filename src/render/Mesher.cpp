#pragma once
#include <memory>
#include <utility>
#include <vector>
#include "raylib.h"
#include "../lib/Chunk.hpp"
#include "../lib/World.hpp"
#include "../textures/atlas.cpp"

struct MeshSettings {
    int CHUNK_SIZE = CHUNK_SIZE;
    int ATLAS_COLS = 2;
    int ATLAS_ROWS = 2;
    int TILE = 32;
};

static Texture2D gAtlasTex = {0};
static MeshSettings gSettings;

static const Vector3 cubeVerts[8] = {
    {0,0,0},{1,0,0},{1,1,0},{0,1,0},
    {0,0,1},{1,0,1},{1,1,1},{0,1,1}
};
static const int faceIdx[6][4] = {
    {0,3,2,1}, {5,6,7,4}, {4,7,3,0}, {1,2,6,5}, {3,7,6,2}, {4,0,1,5}
};
static const int faceOffs[6][3] = {
    {0,0,-1}, {0,0,1}, {-1,0,0}, {1,0,0}, {0,1,0}, {0,-1,0}
};
static const Vector3 faceNormalFloats[6] = {
    {0,0,-1},{0,0,1},{-1,0,0},{1,0,0},{0,1,0},{0,-1,0}
};

void SetAtlasTexture(const Texture2D &tex){
    gAtlasTex = tex;
}

void SetAtlasParams(int tile, int cols, int rows){
    gSettings.TILE = tile;
    gSettings.ATLAS_COLS = cols;
    gSettings.ATLAS_ROWS = rows;
}

static inline void TileUV(int tileX, int tileY, float uLocal, float vLocal, float &uOut, float &vOut){
    float invW = 1.0f / (float)gSettings.ATLAS_COLS;
    float invH = 1.0f / (float)gSettings.ATLAS_ROWS;
    uOut = tileX * invW + uLocal * invW;
    vOut = tileY * invH + vLocal * invH;
}

static bool insideLocal(int x,int y,int z){
    return x>=0 && x<CHUNK_SIZE && y>=0 && y<CHUNK_SIZE && z>=0 && z<CHUNK_SIZE;
}

Model BuildModelForChunk(const std::shared_ptr<Chunk>& chunk, const World* world){
    std::vector<float> verts;
    std::vector<float> norms;
    std::vector<float> texcoords;
    std::vector<unsigned char> cols;
    std::vector<unsigned short> indices;

    verts.reserve(65536);
    norms.reserve(65536);
    texcoords.reserve(65536);
    cols.reserve(65536);
    indices.reserve(65536);

    if(!chunk) return Model{0};

    int baseWorldX = chunk->GetChunkX() * CHUNK_SIZE;
    int baseWorldY = chunk->GetChunkY() * CHUNK_SIZE;
    int baseWorldZ = chunk->GetChunkZ() * CHUNK_SIZE;

    for(int x=0;x<CHUNK_SIZE;++x){
      for(int y=0;y<CHUNK_SIZE;++y){
        for(int z=0;z<CHUNK_SIZE;++z){
          int bid = 0;
          auto bptr = chunk->GetBlock(x,y,z);
          if(bptr) bid = bptr->GetId();
          if(bid == 0) continue;

          auto tile = AtlasCoordsForBlock(bid);
          int tileX = tile.first;
          int tileY = tile.second;

          for(int f=0; f<6; ++f){
            int nx = x + faceOffs[f][0];
            int ny = y + faceOffs[f][1];
            int nz = z + faceOffs[f][2];
            bool visible = true;

            if(insideLocal(nx,ny,nz)){
                auto nb = chunk->GetBlock(nx,ny,nz);
                if(nb && nb->GetId() != 0) visible = false;
            } else {
                // neighbor is outside chunk: ask world if provided
                if(world){
                    int worldNx = baseWorldX + nx;
                    int worldNy = baseWorldY + ny;
                    int worldNz = baseWorldZ + nz;
                    if(!world->IsBlockTransparent(worldNx, worldNy, worldNz)) visible = false;
                } else {
                    // if no world provided, assume outside is air -> visible remains true
                }
            }

            if(!visible) continue;

            unsigned int base = (unsigned int)(verts.size() / 3);
            if(base + 4 > 0xFFFFu){
                fprintf(stderr, "Warning: Chunk exceeds 16-bit index limit\n");
                goto build_mesh;
            }

            auto pushVert = [&](int vi){
              Vector3 v = cubeVerts[vi];
              verts.push_back(v.x + (float)x);
              verts.push_back(v.y + (float)y);
              verts.push_back(v.z + (float)z);
              norms.push_back(faceNormalFloats[f].x);
              norms.push_back(faceNormalFloats[f].y);
              norms.push_back(faceNormalFloats[f].z);
              cols.push_back(255); cols.push_back(255); cols.push_back(255); cols.push_back(255);
              float uLocal=0.f, vLocal=0.f;
              if(f==0 || f==1){ uLocal = v.x; vLocal = v.y; }
              else if(f==2 || f==3){ uLocal = v.z; vLocal = v.y; }
              else { uLocal = v.x; vLocal = v.z; }
              float uA,vA;
              TileUV(tileX, tileY, uLocal, vLocal, uA, vA);
              texcoords.push_back(uA); texcoords.push_back(vA);
            };

            int a = faceIdx[f][0], b = faceIdx[f][1], c = faceIdx[f][2], d = faceIdx[f][3];
            pushVert(a); pushVert(b); pushVert(c); pushVert(d);

            indices.push_back((unsigned short)(base + 0));
            indices.push_back((unsigned short)(base + 1));
            indices.push_back((unsigned short)(base + 2));
            indices.push_back((unsigned short)(base + 0));
            indices.push_back((unsigned short)(base + 2));
            indices.push_back((unsigned short)(base + 3));
          }
        }
      }
    }

build_mesh:
    if(verts.empty()){
        return Model{0};
    }

    Mesh mesh = {0};
    mesh.vertexCount = (int)(verts.size()/3);
    mesh.triangleCount = (int)(indices.size()/3);

    float *vbuf = (float*)malloc(verts.size()*sizeof(float));
    float *nbuf = (float*)malloc(norms.size()*sizeof(float));
    float *tbuf = (float*)malloc(texcoords.size()*sizeof(float));
    unsigned char *cbuf = (unsigned char*)malloc(cols.size()*sizeof(unsigned char));
    unsigned short *ibuf = (unsigned short*)malloc(indices.size()*sizeof(unsigned short));
    
    if(!vbuf || !nbuf || !tbuf || !cbuf || !ibuf){
        free(vbuf); free(nbuf); free(tbuf); free(cbuf); free(ibuf);
        return Model{0};
    }

    memcpy(vbuf, verts.data(), verts.size()*sizeof(float));
    memcpy(nbuf, norms.data(), norms.size()*sizeof(float));
    memcpy(tbuf, texcoords.data(), texcoords.size()*sizeof(float));
    memcpy(cbuf, cols.data(), cols.size()*sizeof(unsigned char));
    memcpy(ibuf, indices.data(), indices.size()*sizeof(unsigned short));

    mesh.vertices = vbuf;
    mesh.normals = nbuf;
    mesh.texcoords = tbuf;
    mesh.colors = cbuf;
    mesh.indices = ibuf;
    mesh.vboId = 0;

    UploadMesh(&mesh, true);

    Model m = LoadModelFromMesh(mesh);

    if (gAtlasTex.id) {
        if (m.materialCount > 0) SetMaterialTexture(&m.materials[0], MATERIAL_MAP_DIFFUSE, gAtlasTex);
    }

    return m;
}

void UnloadModelIfLoaded(Model &m, bool &loaded){
    if(loaded){
        UnloadModel(m);
        m = Model{0};
        loaded = false;
    }
}
