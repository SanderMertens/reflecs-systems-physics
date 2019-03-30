#include <include/physics.h>
#include <string.h>
#include <math.h>

#include "systems.h"

void EcsSystemsPhysics(
    ecs_world_t *world,
    int flags,
    void *handles_out)
{
    EcsSystemsPhysicsHandles *handles = handles_out;
    bool do_2d = !flags || flags & ECS_2D;
    bool do_3d = !flags || flags & ECS_3D;

    memset(handles, 0, sizeof(EcsSystemsPhysicsHandles));
    
    ECS_IMPORT(world, EcsComponentsPhysics, flags);
    ECS_IMPORT(world, EcsComponentsGeometry, flags);
    ECS_IMPORT(world, EcsSystemsTransform, flags);

    if (do_2d) {
        ECS_SYSTEM(world, EcsMove2D_w_Rotation, EcsOnUpdate,
            EcsPosition2D, EcsRotation2D, EcsSpeed, !EcsVelocity2D, SYSTEM.EcsHidden);

        ECS_SYSTEM(world, EcsMove2D_w_Velocity, EcsOnUpdate,
            EcsPosition2D, ?EcsSpeed, EcsVelocity2D, SYSTEM.EcsHidden);

        ECS_SYSTEM(world, EcsRotate2D, EcsOnUpdate,
            EcsRotation2D, EcsAngularSpeed, SYSTEM.EcsHidden);

        ECS_SYSTEM(world, EcsAddRotate2D, EcsOnLoad, EcsAngularSpeed, !EcsRotation2D, SYSTEM.EcsHidden);

        ECS_TYPE(world, EcsMove2D, EcsMove2D_w_Rotation, EcsMove2D_w_Velocity, EcsRotate2D);

        /* Auto-add colliders to geometry entities that have EcsCollider */
        ECS_SYSTEM(world, EcsAddColliderForRectangle, EcsOnLoad, EcsRectangle, EcsCollider, !EcsPolygon8Collider, SYSTEM.EcsHidden);
        ECS_SYSTEM(world, EcsAddColliderForCircle,    EcsOnLoad, EcsCircle, EcsCollider, !EcsCircleCollider, SYSTEM.EcsHidden);

        /* Add world space colliders */
        ECS_SYSTEM(world, EcsAddPolygon8ColliderWorld,  EcsOnLoad, EcsPolygon8Collider, !EcsPolygon8ColliderWorld, SYSTEM.EcsHidden);
        ECS_SYSTEM(world, EcsAddCircleColliderWorld,    EcsOnLoad, EcsCircleCollider, !EcsCircleColliderWorld, SYSTEM.EcsHidden);

        /* Transform colliders to world space */
        ECS_SYSTEM(world, EcsTransformPolygon8Colliders,  EcsOnValidate, EcsMatTransform2D, EcsPolygon8Collider, EcsPolygon8ColliderWorld, SYSTEM.EcsHidden);
        ECS_SYSTEM(world, EcsTransformCircleColliders,    EcsOnValidate, EcsMatTransform2D, EcsCircleCollider, EcsCircleColliderWorld, SYSTEM.EcsHidden);

        /* Do collision testing */
        ECS_SYSTEM(world, EcsTestColliders, EcsManual,
            EcsPolygon8ColliderWorld | EcsCircleColliderWorld,
            ID.EcsPolygon8ColliderWorld,
            ID.EcsCircleColliderWorld,
            ID.EcsCollision2D, SYSTEM.EcsHidden);

        ECS_SYSTEM(world, EcsWalkColliders, EcsOnValidate, EcsPolygon8ColliderWorld | EcsCircleColliderWorld, ID.EcsTestColliders, SYSTEM.EcsHidden);
        
        /* Clean collisions for next frame */
        ECS_SYSTEM(world, EcsCleanCollisions, EcsPreUpdate, EcsCollision2D, SYSTEM.EcsHidden);

        /* Collide feature */
        ECS_TYPE(world, EcsCollide,
            EcsTransformPolygon8Colliders, EcsTransformCircleColliders, EcsTestColliders, EcsWalkColliders, EcsCleanCollisions);

        ECS_SET_ENTITY(handles, EcsMove2D_w_Rotation);
        ECS_SET_ENTITY(handles, EcsMove2D_w_Velocity);
        ECS_SET_ENTITY(handles, EcsMove2D);
        ECS_SET_ENTITY(handles, EcsRotate2D);
    }

    if (do_3d) {
        ECS_SYSTEM(world, EcsMove3D_w_Rotation, EcsOnUpdate,
            EcsPosition3D, ?EcsRotation3D, EcsSpeed, !EcsVelocity3D, SYSTEM.EcsHidden);

        ECS_SYSTEM(world, EcsMove3D_w_Velocity, EcsOnUpdate,
            EcsPosition3D, ?EcsSpeed, EcsVelocity3D, SYSTEM.EcsHidden);

        ECS_SYSTEM(world, EcsRotate3D, EcsOnUpdate,
            EcsRotation3D, ?EcsAngularSpeed, EcsAngularVelocity, SYSTEM.EcsHidden);

        ECS_TYPE(world, EcsMove3D, EcsMove3D_w_Rotation, EcsMove3D_w_Velocity, EcsRotate3D);

        ECS_SET_ENTITY(handles, EcsMove3D_w_Rotation);
        ECS_SET_ENTITY(handles, EcsMove3D_w_Velocity);
        ECS_SET_ENTITY(handles, EcsMove3D);
        ECS_SET_ENTITY(handles, EcsRotate3D);
    }

    if (do_2d && do_3d) {
        ECS_TYPE(world, EcsMove, EcsMove2D, EcsMove3D);
        ECS_SET_ENTITY(handles, EcsMove);
    } else if (!do_2d) {
        ECS_TYPE(world, EcsMove, EcsMove3D);
        ECS_SET_ENTITY(handles, EcsMove);
    } else if (!do_3d) {
        ECS_TYPE(world, EcsMove, EcsMove2D);
        ECS_SET_ENTITY(handles, EcsMove);
    }
}
