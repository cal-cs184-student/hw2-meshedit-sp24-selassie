#include "student_code.h"
#include "CGL/vector3D.h"
#include "halfEdgeMesh.h"
#include "mutablePriorityQueue.h"
#include <vector>

using namespace std;

namespace CGL
{

  /**
   * Evaluates one step of the de Casteljau's algorithm using the given points and
   * the scalar parameter t (class member).
   *
   * @param points A vector of points in 2D
   * @return A vector containing intermediate points or the final interpolated vector
   */
  std::vector<Vector2D> BezierCurve::evaluateStep(std::vector<Vector2D> const &points)
  { 
    // TODO Part 1.
    std::vector<Vector2D> new_control_pts;
    for (int i = 0; i < points.size() - 1; i++) {
      Vector2D intermediate_pt = lerp2D(points[i], points[i + 1], t);
      new_control_pts.push_back(intermediate_pt);
    }
    return new_control_pts;
  }

  Vector2D lerp2D(Vector2D p0, Vector2D p1, float t)
  {
    return (1 - t) * p0 + t * p1;
  }

  /**
   * Evaluates one step of the de Casteljau's algorithm using the given points and
   * the scalar parameter t (function parameter).
   *
   * @param points    A vector of points in 3D
   * @param t         Scalar interpolation parameter
   * @return A vector containing intermediate points or the final interpolated vector
   */
  std::vector<Vector3D> BezierPatch::evaluateStep(std::vector<Vector3D> const &points, double t) const
  {
    // TODO Part 2.
    std::vector<Vector3D> new_control_pts;
    for (int i = 0; i < points.size() - 1; i++) {
      Vector3D intermediate_pt = lerp3D(points[i], points[i + 1], t);
      new_control_pts.push_back(intermediate_pt);
    }
    return new_control_pts;
  }

  Vector3D lerp3D(Vector3D p0, Vector3D p1, double t)
  {
    return (1 - t) * p0 + t * p1;
  }

  /**
   * Fully evaluates de Casteljau's algorithm for a vector of points at scalar parameter t
   *
   * @param points    A vector of points in 3D
   * @param t         Scalar interpolation parameter
   * @return Final interpolated vector
   */
  Vector3D BezierPatch::evaluate1D(std::vector<Vector3D> const &points, double t) const
  {
    // TODO Part 2.
    vector<Vector3D> current_controls = points;
    while (current_controls.size() > 1) {
      current_controls = evaluateStep(current_controls, t);
    }
    return current_controls[0];
  }

  /**
   * Evaluates the Bezier patch at parameter (u, v)
   *
   * @param u         Scalar interpolation parameter
   * @param v         Scalar interpolation parameter (along the other axis)
   * @return Final interpolated vector
   */
  Vector3D BezierPatch::evaluate(double u, double v) const 
  {  
    // TODO Part 2.
    vector<Vector3D> row_points;
    for (int r = 0; r < controlPoints.size(); r++) {
      row_points.push_back(evaluate1D(controlPoints[r], u));
    }

    return evaluate1D(row_points, v);
  }

  Vector3D Vertex::normal( void ) const
  {
    // TODO Part 3.
    // Returns an approximate unit normal at this vertex, computed by
    // taking the area-weighted average of the normals of neighboring
    // triangles, then normalizing.
    Vector3D normal_sum = Vector3D();
    HalfedgeCIter current_edge = this->halfedge();
    do {
      normal_sum += weighted_normal(*current_edge->face());
      current_edge = current_edge->twin()->next();
    } while (current_edge != this->halfedge());
    return normal_sum.unit();
  }

  Vector3D weighted_normal(Face f)
  {
    Vector3D N( 0., 0., 0. );

    HalfedgeCIter h = f.halfedge();
    do
    {
      Vector3D pi = h->vertex()->position;
      Vector3D pj = h->next()->vertex()->position;

      N += cross( pi, pj );

      h = h->next();
    }
    while( h != f.halfedge() );

    return N;
  }

  EdgeIter HalfedgeMesh::flipEdge( EdgeIter e0 )
  {
    // TODO Part 4.
    // This method should flip the given edge and return an iterator to the flipped edge.

    // boundary check
    if (e0->isBoundary()) {
      return EdgeIter();
    }

    // left triangle sides
    HalfedgeIter bc = e0->halfedge();
    HalfedgeIter ca = bc->next();
    HalfedgeIter ab = ca->next();
    // right triangle sides
    HalfedgeIter cb = bc->twin();
    HalfedgeIter bd = cb->next();
    HalfedgeIter dc = bd->next();

    // set neighbors
    e0->halfedge()->setNeighbors(dc, e0->halfedge()->twin(), ab->vertex(), e0, e0->halfedge()->face());
    e0->halfedge()->twin()->setNeighbors(ab, e0->halfedge(), dc->vertex(), e0, e0->halfedge()->twin()->face());
    ab->setNeighbors(bd, ab->twin(), ab->vertex(), ab->edge(), e0->halfedge()->twin()->face());
    bd->setNeighbors(e0->halfedge()->twin(), bd->twin(), bd->vertex(), bd->edge(), ab->face());
    dc->setNeighbors(ca, dc->twin(), dc->vertex(), dc->edge(), e0->halfedge()->face());
    ca->setNeighbors(e0->halfedge(), ca->twin(), ca->vertex(), ca->edge(), dc->face());
    
    // update vertices
    // vertex a
    e0->halfedge()->vertex()->halfedge() = e0->halfedge();
    // vertex b
    bd->vertex()->halfedge() = bd;
    // vertex c
    ca->vertex()->halfedge() = ca;
    // vertex d
    e0->halfedge()->twin()->vertex()->halfedge() = e0->halfedge()->twin();

    // update faces + return edge
    e0->halfedge()->face()->halfedge() = e0->halfedge();
    e0->halfedge()->twin()->face()->halfedge() = e0->halfedge()->twin();
    return e0;
  }

  VertexIter HalfedgeMesh::splitEdge( EdgeIter e0 )
  {
    // TODO Part 5.
    // This method should split the given edge and return an iterator to the newly inserted vertex.
    // The halfedge of this vertex should point along the edge that was split, rather than the new edges.
    
    // boundary check
    if (e0->isBoundary()) {
      return VertexIter();
    }

    // create vertex and set position to midpoint
    VertexIter new_vertex = newVertex();
    Vector3D midpoint = (e0->halfedge()->vertex()->position + e0->halfedge()->twin()->vertex()->position) / 2;
    new_vertex->position = midpoint;
    new_vertex->isNew = true;

    // define cross edges
    HalfedgeIter top_edge = e0->halfedge();
    HalfedgeIter top_twin = top_edge->twin();
    EdgeIter right = newEdge();
    HalfedgeIter right_edge = newHalfedge();
    HalfedgeIter right_twin = newHalfedge();
    EdgeIter left = newEdge();
    HalfedgeIter left_edge = newHalfedge();
    HalfedgeIter left_twin = newHalfedge();
    EdgeIter bottom = newEdge();
    HalfedgeIter bottom_edge = newHalfedge();
    HalfedgeIter bottom_twin = newHalfedge();
    // create new faces
    FaceIter bottom_right = newFace();
    FaceIter bottom_left = newFace();
    // initialize some variables for outer edges
    HalfedgeIter ca = top_edge->next();
    HalfedgeIter ab = ca->next();
    HalfedgeIter bd = top_twin->next();
    HalfedgeIter dc = bd->next();

    // set top edge neighbors
    top_edge->setNeighbors(top_edge->next(), top_twin, 
    new_vertex, e0, top_edge->face());
    top_twin->setNeighbors(right_edge, top_edge, 
    top_edge->next()->vertex(), e0, top_twin->face());
    // set right perpendicular edge neighbors
    right_edge->setNeighbors(dc, right_twin, 
    new_vertex, right, top_twin->face());
    right_twin->setNeighbors(bottom_twin, right_edge, 
    dc->vertex(), right, bottom_right);
    // set left perpendicular edge neighbors
    left_edge->setNeighbors(top_edge, left_twin, 
    top_edge->next()->next()->vertex(), left, top_edge->face());
    left_twin->setNeighbors(top_edge->next()->next(), left_edge, 
    new_vertex, left, bottom_left);
    // set bottom edge neighbors
    bottom_edge->setNeighbors(left_twin, bottom_twin, 
    bd->vertex(), bottom, bottom_left);
    bottom_twin->setNeighbors(bd, bottom_edge, 
    new_vertex, bottom, bottom_right);

    // update existing outer edges
    ca->setNeighbors(left_edge, ca->twin(), ca->vertex(), 
    ca->edge(), top_edge->face());
    ab->setNeighbors(bottom_edge, ab->twin(), ab->vertex(), 
    ab->edge(), bottom_left);
    bd->setNeighbors(right_twin, bd->twin(), bd->vertex(), 
    bd->edge(), bottom_right);
    dc->setNeighbors(top_twin, dc->twin(), dc->vertex(), 
    dc->edge(), top_twin->face());

    // update vertices' halfedge pointer
    new_vertex->halfedge() = top_edge;
    left_edge->vertex()->halfedge() = left_edge;
    bottom_edge->vertex()->halfedge() = bottom_edge;
    right_twin->vertex()->halfedge() = right_twin;

    // set face values
    top_edge->face()->halfedge() = top_edge;
    top_twin->face()->halfedge() = top_twin;
    bottom_right->halfedge() = bottom_twin;
    bottom_left->halfedge() = bottom_edge;

    // update input edge + set new edges
    e0->halfedge() = top_edge;
    e0->isNew = false;
    e0->isSplit = true;
    right->halfedge() = right_edge;
    right->isNew = true;
    right->isSplit = true;
    left->halfedge() = left_edge;
    left->isNew = true;
    left->isSplit = true;
    bottom->halfedge() = bottom_edge;
    bottom->isNew = false;
    bottom->isSplit = true;
    return new_vertex;
  }



  void MeshResampler::upsample( HalfedgeMesh& mesh )
  {
    // TODO Part 6.
    // This routine should increase the number of triangles in the mesh using Loop subdivision.
    // One possible solution is to break up the method as listed below.

    // 1. Compute new positions for all the vertices in the input mesh, using the Loop subdivision rule,
    // and store them in Vertex::newPosition. At this point, we also want to mark each vertex as being
    // a vertex of the original mesh.
    
    // 2. Compute the updated vertex positions associated with edges, and store it in Edge::newPosition.
    
    // 3. Split every edge in the mesh, in any order. For future reference, we're also going to store some
    // information about which subdivide edges come from splitting an edge in the original mesh, and which edges
    // are new, by setting the flat Edge::isNew. Note that in this loop, we only want to iterate over edges of
    // the original mesh---otherwise, we'll end up splitting edges that we just split (and the loop will never end!)
    
    // 4. Flip any new edge that connects an old and new vertex.

    // 5. Copy the new vertex positions into final Vertex::position.

    // reset isSplit to false for all edges
    for (EdgeIter e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++) {
      e->isSplit = false;
    }
    // mark as original + compute new positions for original vertices
    for (VertexIter v = mesh.verticesBegin(); v != mesh.verticesEnd(); v++) {
      v->isNew = false;
      Vector3D neighbor_position_sum(0, 0, 0);
      HalfedgeCIter h = v->halfedge(); 
      do {
          HalfedgeCIter h_twin = h->twin();
          VertexCIter neighbor = h_twin->vertex();
          neighbor_position_sum += neighbor->position;   
          h = h_twin->next();               
      } while(h != v->halfedge());
      float u;
      if (v->degree() == 3) {
        u = 3.0 / 16.0;
      } else {
        u = 3.0 / (8.0 * v->degree());
      }
      v->newPosition = (1 - v->degree() * u) * v->position + u * neighbor_position_sum;
    }

    // compute new split vertices
    for (EdgeIter e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++) {
      HalfedgeCIter h = e->halfedge();
      VertexCIter a = h->vertex();
      VertexCIter b = h->twin()->vertex();
      VertexCIter c = h->twin()->next()->next()->vertex();
      VertexCIter d = h->next()->next()->vertex();
      e->newPosition = (3.0/8.0) * (a->position + b->position) + (1.0/8.0) * (c->position + d->position);
    }

    // split edges
    for (EdgeIter e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++) {
      if (!e->isSplit) {
        VertexIter v = mesh.splitEdge(e);
        v->newPosition = e->newPosition;
      }
    }
    // flip edges
    for (EdgeIter e = mesh.edgesBegin(); e != mesh.edgesEnd(); e++) {
      if (e->isNew && (!e->halfedge()->vertex()->isNew || !e->halfedge()->twin()->vertex()->isNew)) {
        mesh.flipEdge(e);
      }
    }

    // update vertex positions
    for (VertexIter v = mesh.verticesBegin(); v != mesh.verticesEnd(); v++) {
      v->position = v->newPosition;
    }
  }
}
