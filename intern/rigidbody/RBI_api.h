/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2013 Blender Foundation,
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): Joshua Leung, Sergej Reich
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file RBI_api.h
 *  \ingroup RigidBody
 *  \brief Rigid Body API for interfacing with external Physics Engines
 */

#ifndef __RB_API_H__
#define __RB_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/* API Notes:
 * Currently, this API is optimised for Bullet RigidBodies, and doesn't
 * take into account other Physics Engines. Some tweaking may be necessary
 * to allow other systems to be used, in particular there may be references
 * to datatypes that aren't used here...
 *
 * -- Joshua Leung (22 June 2010)
 */

/* ********************************** */
/* Partial Type Defines - Aliases for the type of data we store */

// ----------

typedef enum rbObjectType {
	RB_OBJECT_RIGIDBODY,
	RB_OBJECT_GHOST,
} rbObjectType;

/* Dynamics World */
typedef struct rbDynamicsWorld rbDynamicsWorld;

/* Rigid Body */
typedef struct rbRigidBody rbRigidBody;

/* Ghost Collision Object */
typedef struct rbGhostObject rbGhostObject;

/* Collision Shape */
typedef struct rbCollisionShape rbCollisionShape;

/* Mesh Data (for Collision Shapes of Meshes) */
typedef struct rbMeshData rbMeshData;

/* Constraint */
typedef struct rbConstraint rbConstraint;

/* Manifold Contact Point */
typedef struct rbManifoldPoint rbManifoldPoint;

/* ********************************** */
/* Dynamics World Methods */

/* Setup ---------------------------- */

/* Create a new dynamics world instance */
// TODO: add args to set the type of constraint solvers, etc.
rbDynamicsWorld *RB_dworld_new(const float gravity[3]);

/* Delete the given dynamics world, and free any extra data it may require */
void RB_dworld_delete(rbDynamicsWorld *world);

/* Settings ------------------------- */

/* Gravity */
void RB_dworld_get_gravity(rbDynamicsWorld *world, float g_out[3]);
void RB_dworld_set_gravity(rbDynamicsWorld *world, const float g_in[3]);

/* Constraint Solver */
void RB_dworld_set_solver_iterations(rbDynamicsWorld *world, int num_solver_iterations);
/* Split Impulse */
void RB_dworld_set_split_impulse(rbDynamicsWorld *world, int split_impulse);

/* Simulation ----------------------- */

/* Step the simulation by the desired amount (in seconds) with extra controls on substep sizes and maximum substeps */
void RB_dworld_step_simulation(rbDynamicsWorld *world, float timeStep, int maxSubSteps, float timeSubStep);
/* Perform collision detection step only */
void RB_dworld_test_collision(rbDynamicsWorld *world);

/* Export -------------------------- */

/* Exports the dynamics world to physics simulator's serialisation format */
void RB_dworld_export(rbDynamicsWorld *world, const char *filename);

/* ********************************** */
/* Manifold Point Methods */

void RB_manifold_point_local_A(const rbManifoldPoint *pt, float vec[3]);
void RB_manifold_point_local_B(const rbManifoldPoint *pt, float vec[3]);
void RB_manifold_point_world_A(const rbManifoldPoint *pt, float vec[3]);
void RB_manifold_point_world_B(const rbManifoldPoint *pt, float vec[3]);
void RB_manifold_point_normal_world_B(const rbManifoldPoint *pt, float vec[3]);
float RB_manifold_point_distance(const rbManifoldPoint *pt);
float RB_manifold_point_combined_friction(const rbManifoldPoint *pt);
float RB_manifold_point_combined_rolling_friction(const rbManifoldPoint *pt);
float RB_manifold_point_combined_restitution(const rbManifoldPoint *pt);
int RB_manifold_point_part_id0(const rbManifoldPoint *pt);
int RB_manifold_point_index0(const rbManifoldPoint *pt);
int RB_manifold_point_part_id1(const rbManifoldPoint *pt);
int RB_manifold_point_index1(const rbManifoldPoint *pt);
void *RB_manifold_point_get_user_persistent_data(const rbManifoldPoint *pt);
void RB_manifold_point_set_user_persistent_data(const rbManifoldPoint *pt, void *data);
float RB_manifold_point_lifetime(const rbManifoldPoint *pt);

/* ********************************** */
/* Rigid Body Methods */

/* Setup ---------------------------- */

/* Add RigidBody to dynamics world */
void RB_dworld_add_body(rbDynamicsWorld *world, rbRigidBody *body, int col_groups);

/* Remove RigidBody from dynamics world */
void RB_dworld_remove_body(rbDynamicsWorld *world, rbRigidBody *body);

/* Collision detection */

typedef void (*rbContactCallback)(void *userdata, rbManifoldPoint *cp,
                                  const void *collob0, rbObjectType type0, int part0, int index0,
                                  const void *collob1, rbObjectType type1, int part1, int index1);

void RB_dworld_convex_sweep_closest_body(
        rbDynamicsWorld *world, rbRigidBody *object,
        const float loc_start[3], const float loc_end[3],
        float v_location[3],  float v_hitpoint[3],  float v_normal[3], int *r_hit);

void RB_dworld_contact_test_body(rbDynamicsWorld *world, rbRigidBody *object, rbContactCallback cb, void *userdata, int col_groups);
void RB_dworld_contact_test_ghost(rbDynamicsWorld *world, rbGhostObject *object, rbContactCallback cb, void *userdata, int col_groups);

/* ............ */

/* Create new RigidBody instance */
rbRigidBody *RB_body_new(rbCollisionShape *shape, const float loc[3], const float rot[4]);

/* Delete the given RigidBody instance */
void RB_body_delete(rbRigidBody *body);

/* Settings ------------------------- */

/* 'Type' */
void RB_body_set_type(rbRigidBody *body, int type, float mass);

/* ............ */

/* Collision Shape */
void RB_body_set_collision_shape(rbRigidBody *body, rbCollisionShape *shape);

/* ............ */

/* Mass */
float RB_body_get_mass(rbRigidBody *body);
void RB_body_set_mass(rbRigidBody *body, float value);

/* Friction */
float RB_body_get_friction(rbRigidBody *body);
void RB_body_set_friction(rbRigidBody *body, float value);

/* Restitution */
float RB_body_get_restitution(rbRigidBody *body);
void RB_body_set_restitution(rbRigidBody *body, float value);

/* Damping */
float RB_body_get_linear_damping(rbRigidBody *body);
void RB_body_set_linear_damping(rbRigidBody *body, float value);

float RB_body_get_angular_damping(rbRigidBody *body);
void RB_body_set_angular_damping(rbRigidBody *body, float value);

void RB_body_set_damping(rbRigidBody *object, float linear, float angular);

/* Sleeping Thresholds */
float RB_body_get_linear_sleep_thresh(rbRigidBody *body);
void RB_body_set_linear_sleep_thresh(rbRigidBody *body, float value);

float RB_body_get_angular_sleep_thresh(rbRigidBody *body);
void RB_body_set_angular_sleep_thresh(rbRigidBody *body, float value);

void RB_body_set_sleep_thresh(rbRigidBody *body, float linear, float angular);

/* Linear Velocity */
void RB_body_get_linear_velocity(rbRigidBody *body, float v_out[3]);
void RB_body_set_linear_velocity(rbRigidBody *body, const float v_in[3]);

/* Angular Velocity */
void RB_body_get_angular_velocity(rbRigidBody *body, float v_out[3]);
void RB_body_set_angular_velocity(rbRigidBody *body, const float v_in[3]);

/* Linear/Angular Factor, used to lock translation/roation axes */
void RB_body_set_linear_factor(rbRigidBody *object, float x, float y, float z);
void RB_body_set_angular_factor(rbRigidBody *object, float x, float y, float z);

/* Kinematic State */
void RB_body_set_kinematic_state(rbRigidBody *body, bool kinematic);

/* RigidBody Interface - Rigid Body Activation States */
int RB_body_get_activation_state(rbRigidBody *body);
void RB_body_set_activation_state(rbRigidBody *body, int use_deactivation);
void RB_body_activate(rbRigidBody *body);
void RB_body_deactivate(rbRigidBody *body);

/* Simulation ----------------------- */

/* Get current transform matrix of RigidBody to use in Blender (OpenGL format) */
void RB_body_get_transform_matrix(rbRigidBody *body, float m_out[4][4]);

/* Set RigidBody's location and rotation */
void RB_body_set_loc_rot(rbRigidBody *body, const float loc[3], const float rot[4]);
/* Set RigidBody's local scaling */
void RB_body_set_scale(rbRigidBody *body, const float scale[3]);

/* ............ */

/* Get RigidBody's position as vector */
void RB_body_get_position(rbRigidBody *body, float v_out[3]);
/* Get RigidBody's orientation as quaternion */
void RB_body_get_orientation(rbRigidBody *body, float v_out[4]);

/* ............ */

void RB_body_apply_central_force(rbRigidBody *body, const float v_in[3]);

/* ********************************** */
/* Ghost Collision Object Methods */

/* Add ghost to dynamics world */
void RB_dworld_add_ghost(rbDynamicsWorld *world, rbGhostObject *ghost, int col_groups);
/* Remove ghost from dynamics world */
void RB_dworld_remove_ghost(rbDynamicsWorld *world, rbGhostObject *ghost);

/* Create new ghost instance */
rbGhostObject *RB_ghost_new(rbCollisionShape *shape, const float loc[3], const float rot[4]);
/* Delete the given ghost instance */
void RB_ghost_delete(rbGhostObject *ghost);

/* Collision Shape */
void RB_ghost_set_collision_shape(rbGhostObject *body, rbCollisionShape *shape);

/* Transform */
void RB_ghost_get_transform_matrix(rbGhostObject *object, float m_out[4][4]);
void RB_ghost_set_loc_rot(rbGhostObject *object, const float loc[3], const float rot[4]);
void RB_ghost_set_scale(rbGhostObject *object, const float scale[3]);

/* Collision detection */
void RB_dworld_convex_sweep_closest_ghost(
        rbDynamicsWorld *world, rbGhostObject *object,
        const float loc_start[3], const float loc_end[3],
        float v_location[3],  float v_hitpoint[3],  float v_normal[3], int *r_hit);

/* ********************************** */
/* Collision Shape Methods */

/* Setup (Standard Shapes) ----------- */

rbCollisionShape *RB_shape_new_box(float x, float y, float z);
rbCollisionShape *RB_shape_init_box(void *mem, float x, float y, float z);
extern const size_t RB_shape_size_box;

rbCollisionShape *RB_shape_new_sphere(float radius);
rbCollisionShape *RB_shape_init_sphere(void *mem, float radius);
extern const size_t RB_shape_size_sphere;

rbCollisionShape *RB_shape_new_capsule(float radius, float height);
rbCollisionShape *RB_shape_init_capsule(void *mem, float radius, float height);
extern const size_t RB_shape_size_capsule;

rbCollisionShape *RB_shape_new_cone(float radius, float height);
rbCollisionShape *RB_shape_init_cone(void *mem, float radius, float height);
extern const size_t RB_shape_size_cone;

rbCollisionShape *RB_shape_new_cylinder(float radius, float height);
rbCollisionShape *RB_shape_init_cylinder(void *mem, float radius, float height);
extern const size_t RB_shape_size_cylinder;

/* Setup (Convex Hull) ------------ */

rbCollisionShape *RB_shape_new_convex_hull(float *verts, int stride, int count, float margin, bool *can_embed);

/* Setup (Triangle Mesh) ---------- */

/* 1 */
rbMeshData *RB_trimesh_data_new(int num_tris, int num_verts);
void RB_trimesh_data_delete(rbMeshData *mesh);
void RB_trimesh_add_vertices(rbMeshData *mesh, float *vertices, int num_verts, int vert_stride);
void RB_trimesh_add_triangle_indices(rbMeshData *mesh, int num, int index0, int index1, int index2);
void RB_trimesh_finish(rbMeshData *mesh);
/* 2a - Triangle Meshes */
rbCollisionShape *RB_shape_new_trimesh(rbMeshData *mesh);
/* 2b - GImpact Meshes */
rbCollisionShape *RB_shape_new_gimpact_mesh(rbMeshData *mesh);

/* Setup (Compound) ---------- */

rbCollisionShape *RB_shape_new_compound(bool enable_dynamic_aabb_tree);
void RB_shape_compound_add_child_shape(rbCollisionShape *shape, const float loc[3], const float rot[4], rbCollisionShape *child);
int RB_shape_compound_get_num_child_shapes(rbCollisionShape *shape);
rbCollisionShape *RB_shape_compound_get_child_shape(rbCollisionShape *shape, int index);
void RB_shape_compound_get_child_transform(rbCollisionShape *shape, int index, float mat[4][4]);
/* Note: after updating transforms, RB_shape_compound_update_local_aabb should be called! */
void RB_shape_compound_set_child_transform(rbCollisionShape *shape, int index, const float loc[3], const float rot[4]);
void RB_shape_compound_update_local_aabb(rbCollisionShape *shape);


/* Cleanup --------------------------- */

/* only free internal data (no dealloc) */
void RB_shape_free(rbCollisionShape *shape);
/* free and dealloc */
void RB_shape_delete(rbCollisionShape *shape);

/* Settings --------------------------- */

/* Collision Margin */
float RB_shape_get_margin(rbCollisionShape *shape);
void RB_shape_set_margin(rbCollisionShape *shape, float value);

void RB_shape_trimesh_update(rbCollisionShape *shape, float *vertices, int num_verts, int vert_stride, float min[3], float max[3]);

/* ********************************** */
/* Constraints */

/* Setup ----------------------------- */

/* Add Rigid Body Constraint to simulation world */
void RB_dworld_add_constraint(rbDynamicsWorld *world, rbConstraint *con, int disable_collisions);

/* Remove Rigid Body Constraint from simulation world */
void RB_dworld_remove_constraint(rbDynamicsWorld *world, rbConstraint *con);

rbConstraint *RB_constraint_new_point(float pivot[3], rbRigidBody *rb1, rbRigidBody *rb2);
rbConstraint *RB_constraint_new_fixed(float pivot[3], float orn[4], rbRigidBody *rb1, rbRigidBody *rb2);
rbConstraint *RB_constraint_new_hinge(float pivot[3], float orn[4], rbRigidBody *rb1, rbRigidBody *rb2);
rbConstraint *RB_constraint_new_slider(float pivot[3], float orn[4], rbRigidBody *rb1, rbRigidBody *rb2);
rbConstraint *RB_constraint_new_piston(float pivot[3], float orn[4], rbRigidBody *rb1, rbRigidBody *rb2);
rbConstraint *RB_constraint_new_6dof(float pivot[3], float orn[4], rbRigidBody *rb1, rbRigidBody *rb2);
rbConstraint *RB_constraint_new_6dof_spring(float pivot[3], float orn[4], rbRigidBody *rb1, rbRigidBody *rb2);
rbConstraint *RB_constraint_new_motor(float pivot[3], float orn[4], rbRigidBody *rb1, rbRigidBody *rb2);

/* ............ */

/* Cleanup --------------------------- */

void RB_constraint_delete(rbConstraint *con);

/* Settings --------------------------- */

/* Enable or disable constraint */
void RB_constraint_set_enabled(rbConstraint *con, int enabled);

/* Limits */
#define RB_LIMIT_LIN_X 0
#define RB_LIMIT_LIN_Y 1
#define RB_LIMIT_LIN_Z 2
#define RB_LIMIT_ANG_X 3
#define RB_LIMIT_ANG_Y 4
#define RB_LIMIT_ANG_Z 5
/* Bullet uses the following convention:
 * - lower limit == upper limit -> axis is locked
 * - lower limit > upper limit -> axis is free
 * - lower limit < upper limit -> axis is limited in given range
 */
void RB_constraint_set_limits_hinge(rbConstraint *con, float lower, float upper);
void RB_constraint_set_limits_slider(rbConstraint *con, float lower, float upper);
void RB_constraint_set_limits_piston(rbConstraint *con, float lin_lower, float lin_upper, float ang_lower, float ang_upper);
void RB_constraint_set_limits_6dof(rbConstraint *con, int axis, float lower, float upper);

/* 6dof spring specific */
void RB_constraint_set_stiffness_6dof_spring(rbConstraint *con, int axis, float stiffness);
void RB_constraint_set_damping_6dof_spring(rbConstraint *con, int axis, float damping);
void RB_constraint_set_spring_6dof_spring(rbConstraint *con, int axis, int enable);
void RB_constraint_set_equilibrium_6dof_spring(rbConstraint *con);

/* motors */
void RB_constraint_set_enable_motor(rbConstraint *con, int enable_lin, int enable_ang);
void RB_constraint_set_max_impulse_motor(rbConstraint *con, float max_impulse_lin, float max_impulse_ang);
void RB_constraint_set_target_velocity_motor(rbConstraint *con, float velocity_lin, float velocity_ang);

/* Set number of constraint solver iterations made per step, this overrided world setting
 * To use default set it to -1 */
void RB_constraint_set_solver_iterations(rbConstraint *con, int num_solver_iterations);

/* Set breaking impulse threshold, if constraint shouldn't break it can be set to FLT_MAX */
void RB_constraint_set_breaking_threshold(rbConstraint *con, float threshold);

/* ********************************** */

#ifdef __cplusplus
}
#endif

#endif /* __RB_API_H__ */
 
