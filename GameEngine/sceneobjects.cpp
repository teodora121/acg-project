#include "SceneObjects.h"
#include "ObjLoader.h"

void SceneObjects::load() {
    building1 = ObjLoader::LoadObj("assets/models/building1/building1.obj");
    building2 = ObjLoader::LoadObj("assets/models/building2/building2.obj");
    windmill = ObjLoader::LoadObj("assets/models/windmill/windmill.obj");
}

void SceneObjects::draw(Shader& shader) {
    building1.draw(shader);
    building2.draw(shader);
    windmill.draw(shader);
}
