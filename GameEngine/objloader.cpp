#include "ObjLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Mesh ObjLoader::LoadObj(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Failed to open OBJ: " << path << std::endl;
        return Mesh();
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<Vertex> vertices;
    std::vector<int> indices;

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "v") {
            glm::vec3 v;
            ss >> v.x >> v.y >> v.z;
            positions.push_back(v);
        }
        else if (type == "vn") {
            glm::vec3 n;
            ss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (type == "f") {
            int v, n;
            char slash;

            for (int i = 0; i < 3; i++) {
                ss >> v >> slash >> slash >> n;
                Vertex vert;
                vert.pos = positions[v - 1];
                vert.normals = normals[n - 1];
                vertices.push_back(vert);
                indices.push_back((int)indices.size());
            }
        }
    }

    return Mesh(vertices, indices);
}
