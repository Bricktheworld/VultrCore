#include <core/components/camera_component.h>
#include <core/components/transform_component.h>
#include <glm/glm.hpp>
#include <hashtable.h>

namespace Vultr
{
    struct InternalRenderContext
    {
        InternalRenderContext();
        ~InternalRenderContext();
        // Vec2 dimensions;
        // TransformComponent camera_transform;
        // CameraComponent camera_component;
        // Mat4 view_matrix;
        // Mat4 projection_matrix;
    };
} // namespace Vultr
