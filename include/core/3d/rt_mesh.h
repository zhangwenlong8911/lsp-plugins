/*
 * rt_mesh.h
 *
 *  Created on: 26 февр. 2019 г.
 *      Author: sadko
 */

#ifndef INCLUDE_CORE_3D_RT_MESH_H_
#define INCLUDE_CORE_3D_RT_MESH_H_

#include <core/3d/common.h>
#include <core/3d/Allocator3D.h>
#include <core/3d/Object3D.h>
#include <core/3d/Scene3D.h>

namespace lsp
{
    typedef struct rt_mesh_t
    {
        public:
            Allocator3D<rtm_vertex_t>   vertex;     // Collection of vertexes
            Allocator3D<rtm_edge_t>     edge;       // Collection of edges
            Allocator3D<rtm_triangle_t> triangle;   // Collection of triangles
            cstorage<rt_material_t>     material;   // Additional materials

        public:
            explicit rt_mesh_t();
            ~rt_mesh_t();

        protected:
            bool            validate_list(rtm_edge_t *e);
            static ssize_t  linked_count(rtm_triangle_t *t, rtm_edge_t *e);

            status_t        split_edge(rtm_edge_t* e, rtm_vertex_t* sp);
            status_t        split_triangle(rtm_triangle_t* t, rtm_vertex_t* sp);
            static bool     unlink_triangle(rtm_triangle_t *t, rtm_edge_t *e);
            static status_t arrange_triangle(rtm_triangle_t *ct, rtm_edge_t *e);

            rtm_vertex_t   *add_unique_vertex(const point3d_t *p);
            rtm_edge_t     *add_unique_edge(rtm_vertex_t *v1, rtm_vertex_t *v2);

            status_t        copy_object_data(Object3D *obj, ssize_t oid, const matrix3d_t *transform, rt_material_t *material);
            status_t        solve_conflicts_internal();
            status_t        split_edge_internal(rtm_edge_t* e, rtm_vertex_t* sp);
            status_t        split_triangle_internal(rtm_triangle_t* t, rtm_vertex_t* sp);
            status_t        paint_triangles_internal();

        public:
            /**
             * Clear mesh: clear underlying structures
             */
            inline void     clear()
            {
                vertex.clear();
                edge.clear();
                triangle.clear();
            }

            /**
             * Flush mesh: clear underlying structures and release memory
             */
            inline void     flush()
            {
                vertex.flush();
                edge.flush();
                triangle.flush();
            }

            /**
             * Swap internal mesh contents with another context
             * @param dst destination context to perform swap
             */
            inline void     swap(rt_mesh_t *dst)
            {
                vertex.swap(&dst->vertex);
                edge.swap(&dst->edge);
                triangle.swap(&dst->triangle);
            }

            /**
             * Add object to context
             * @param obj object to add
             * @param oid unique id to identify the object
             * @param material material that describes behaviour of reflected rays
             * @return status of operation
             */
            inline status_t add_object(Object3D *obj, ssize_t oid, rt_material_t *material)
            {
                return add_object(obj, oid, obj->matrix(), material);
            }

            /**
             * Add object to context
             * @param obj object to add
             * @param oid unique id to identify the object
             * @param transform transformation matrix to apply to object
             * @param material material that describes behaviour of reflected rays
             * @return status of operation
             */
            status_t        add_object(Object3D *obj, ssize_t oid, const matrix3d_t *transform, rt_material_t *material);

            /**
             * Add object exclusively to context
             * @param obj object to add
             * @param oid unique id to identify the object
             * @param material material that describes behaviour of reflected rays
             * @return status of operation
             */
            inline status_t add_object_exclusive(Object3D *obj, ssize_t oid, rt_material_t *material)
            {
                return add_object_exclusive(obj, oid, obj->matrix(), material);
            }

            /**
             * Add object exclusively to context
             * @param obj object to add
             * @param oid unique id to identify the object
             * @param transform transformation matrix to apply to object
             * @param material material that describes behaviour of reflected rays
             * @return status of operation
             */
            status_t        add_object_exclusive(Object3D *obj, ssize_t oid, const matrix3d_t *transform, rt_material_t *material);

            /**
             * Remove conflicts between triangles, does not modify the 'itag' field of
             * triangle, so it can be used to identify objects of the scene
             *
             * @return status of operation
             */
            status_t        solve_conflicts();

            /**
             * Check consistency of the context: that all stored pointers are valid
             * @return true if context is in valid state
             */
            bool            validate();

            /**
             * Copy all data from the source mesh
             * @param src source mesh to copy
             * @return status of operation
             */
            status_t        copy(rt_mesh_t *src);
    } rt_mesh_t;
}

#endif /* INCLUDE_CORE_3D_RT_MESH_H_ */
