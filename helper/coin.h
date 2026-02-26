#pragma once

#include "trianglemesh.h"
#include <glm/glm.hpp>
#include <vector>

class Coin : public TriangleMesh
{
private:
    void generateCoin(float radius, float thickness, int segments);

public:
    Coin(float radius = 0.5f, float thickness = 0.1f, int segments = 32);
};
