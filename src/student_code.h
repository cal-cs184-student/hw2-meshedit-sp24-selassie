#ifndef STUDENT_CODE_H
#define STUDENT_CODE_H

#include "halfEdgeMesh.h"
#include "bezierPatch.h"
#include "bezierCurve.h"

using namespace std;

namespace CGL {

  Vector2D lerp2D(Vector2D p0, Vector2D p1, float t);
  Vector3D lerp3D(Vector3D p0, Vector3D p1, double t);
  Vector3D weighted_normal(Face f);

  class MeshResampler{

  public:

    MeshResampler(){};
    ~MeshResampler(){}

    void upsample(HalfedgeMesh& mesh);
  };
}

#endif // STUDENT_CODE_H
