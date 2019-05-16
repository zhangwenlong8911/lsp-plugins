/*
 * CtlSource3D.cpp
 *
 *  Created on: 14 мая 2019 г.
 *      Author: sadko
 */

#include <ui/ctl/ctl.h>
#include <core/3d/raytrace.h>
#include <plugins/room_builder.h>

namespace lsp
{
    namespace ctl
    {
        
        CtlSource3D::CtlSource3D(CtlRegistry *src, LSPMesh3D *widget): CtlWidget(src, widget)
        {
            bRebuildMesh= true;
            fPosX       = 0.0f;
            fPosY       = 0.0f;
            fPosZ       = 0.0f;
            fYaw        = 0.0f;
            fPitch      = 0.0f;
            fRoll       = 0.0f;

            pMode       = NULL;
            pPosX       = NULL;
            pPosY       = NULL;
            pPosZ       = NULL;
            pYaw        = NULL;
            pPitch      = NULL;
            pRoll       = NULL;
            pCurvature  = NULL;
            pHeight     = NULL;
            pAngle      = NULL;

            dsp::init_matrix3d_identity(&sSource.pos);
            sSource.type        = RT_AS_ICO;
            sSource.size        = 1.0f;
            sSource.height      = 1.0f;
            sSource.angle       = 90.0f;
            sSource.curvature   = 1.0f;
        }
        
        CtlSource3D::~CtlSource3D()
        {
        }
    
        void CtlSource3D::init()
        {
            CtlWidget::init();

            LSPMesh3D *mesh = widget_cast<LSPMesh3D>(pWidget);
            if (mesh != NULL)
                sColor.init_hsl2(pRegistry, pWidget, mesh->color(), A_COLOR, A_HUE_ID, A_SAT_ID, A_LIGHT_ID);
        }

        void CtlSource3D::set(widget_attribute_t att, const char *value)
        {
            switch (att)
            {
                case A_XPOS_ID:
                    BIND_PORT(pRegistry, pPosX, value);
                    break;
                case A_YPOS_ID:
                    BIND_PORT(pRegistry, pPosY, value);
                    break;
                case A_ZPOS_ID:
                    BIND_PORT(pRegistry, pPosZ, value);
                    break;
                case A_YAW_ID:
                    BIND_PORT(pRegistry, pYaw, value);
                    break;
                case A_PITCH_ID:
                    BIND_PORT(pRegistry, pPitch, value);
                    break;
                case A_ROLL_ID:
                    BIND_PORT(pRegistry, pRoll, value);
                    break;
                case A_CURVATURE_ID:
                    BIND_PORT(pRegistry, pCurvature, value);
                    break;
                case A_MODE_ID:
                    BIND_PORT(pRegistry, pMode, value);
                    break;
                case A_ANGLE_ID:
                    BIND_PORT(pRegistry, pAngle, value);
                    break;
                case A_HEIGHT_ID:
                    BIND_PORT(pRegistry, pHeight, value);
                    break;

                default:
                    bool set = sColor.set(att, value);
                    if (!set)
                        CtlWidget::set(att, value);
                    break;
            }
        }

        void CtlSource3D::update_source_location()
        {
            LSPMesh3D *mesh = widget_cast<LSPMesh3D>(pWidget);
            if (mesh == NULL)
                return;

            matrix3d_t delta, m;

            // Compute rotation matrix
            dsp::init_matrix3d_translate(&delta, fPosX, fPosY, fPosZ);

            dsp::init_matrix3d_rotate_z(&m, fYaw * M_PI / 180.0f);
            dsp::apply_matrix3d_mm1(&delta, &m);

            dsp::init_matrix3d_rotate_y(&m, fPitch * M_PI / 180.0f);
            dsp::apply_matrix3d_mm1(&delta, &m);

            dsp::init_matrix3d_rotate_x(&m, fRoll * M_PI / 180.0f);
            dsp::apply_matrix3d_mm1(&delta, &m);

            // Commit matrix to mesh
            mesh->set_transform(&delta);
            mesh->query_draw();
        }

        status_t CtlSource3D::slot_on_draw3d(LSPWidget *sender, void *ptr, void *data)
        {
            CtlSource3D *_this = static_cast<CtlSource3D *>(ptr);
            if (_this == NULL)
                return STATUS_BAD_STATE;

            _this->update_mesh_data();
            return STATUS_OK;
        }

        void CtlSource3D::update_mesh_data()
        {
            if (!bRebuildMesh)
                return;

            LSPMesh3D *mesh = widget_cast<LSPMesh3D>(pWidget);
            if (mesh == NULL)
                return;

            // Generate source mesh depending on current configuration
            cstorage<rt_group_t> groups;
            status_t res    = gen_source_mesh(groups, &sSource);
            if (res != STATUS_OK)
                return;

            // Now we need to process the mesh and submit it to the mesh
            cstorage<point3d_t> vp;
            cstorage<point3d_t> vn;

            // Allocate space for triangles
            size_t  nt      = groups.size();
            point3d_t *dp   = vp.append_n(nt * 3); // 1 triangle x 3 vertices
            if (dp == NULL)
                return;
            point3d_t *dl   = vn.append_n(nt * 6); // 3 lines x 2 vertices
            if (dl == NULL)
                return;

            const rt_group_t *grp   = groups.get_array();

            // Generate the final data
            vector3d_t dv[3];
            for (size_t i=0; i<nt; ++i, ++grp, dp += 3, dl += 6)
            {
                dp[0]   = grp->p[0];
                dp[1]   = grp->p[1];
                dp[2]   = grp->p[2];

                dl[0]   = grp->p[0];
                dl[2]   = grp->p[1];
                dl[4]   = grp->p[2];

                dsp::init_vector_p2(&dv[0], &grp->s, &grp->p[0]);
                dsp::init_vector_p2(&dv[1], &grp->s, &grp->p[1]);
                dsp::init_vector_p2(&dv[2], &grp->s, &grp->p[2]);

                dsp::scale_vector1(&dv[0], 0.3f);
                dsp::scale_vector1(&dv[1], 0.3f);
                dsp::scale_vector1(&dv[2], 0.3f);

                dsp::add_vector_pv2(&dl[1], &grp->p[0], &dv[0]);
                dsp::add_vector_pv2(&dl[3], &grp->p[1], &dv[1]);
                dsp::add_vector_pv2(&dl[5], &grp->p[2], &dv[2]);
            }

            // Apply this data as layers
            mesh->clear();
            res = mesh->add_triangles(vp.get_array(), NULL, vp.size());
            if (res != STATUS_OK)
                return;

            res = mesh->add_lines(vn.get_array(), vn.size());
            if (res != STATUS_OK)
                return;

            // Cleanup rebuild flag
            bRebuildMesh    = false;
        }

        void CtlSource3D::notify(CtlPort *port)
        {
            bool sync       = false;
            bool rebuild    = false;

            if (port == pPosX)
            {
                fPosX       = port->get_value();
                sync    = true;
            }
            if (port == pPosY)
            {
                fPosY       = port->get_value();
                sync    = true;
            }
            if (port == pPosZ)
            {
                fPosZ       = port->get_value();
                sync    = true;
            }
            if (port == pYaw)
            {
                fYaw        = port->get_value();
                sync    = true;
            }
            if (port == pPitch)
            {
                fPitch      = port->get_value();
                sync    = true;
            }
            if (port == pRoll)
            {
                fRoll       = port->get_value();
                sync    = true;
            }

            if (port == pMode)
            {
                sSource.type    = room_builder_base::decode_source_type(port->get_value());
                rebuild         = true;
            }
            if (port == pHeight)
            {
                sSource.height  = port->get_value();
                rebuild         = true;
            }
            if (port == pAngle)
            {
                sSource.angle   = port->get_value();
                rebuild         = true;
            }
            if (port == pCurvature)
            {
                sSource.curvature   = port->get_value();
                rebuild             = true;
            }

            if (sync)
                update_source_location();
            if ((rebuild) && (!bRebuildMesh))
            {
                bRebuildMesh    = true;
                pWidget->query_draw();
            }
        }

    } /* namespace ctl */
} /* namespace lsp */