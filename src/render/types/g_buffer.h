// #pragma once
// #include "frame_buffer.h"

// namespace Vultr
// {
// class GBuffer : public FrameBuffer
// {
//   public:
//     GBuffer();
//     ~GBuffer();

//     void Init(int width, int height);

//     void Resize(int width, int height);

//     void BindPositionTexture(unsigned int slot);

//     void BindNormalTexture(unsigned int slot);

//     void BindColorSpecularTexture(unsigned int slot);

//     void BindLightSpecularTexture(unsigned int slot);

//     // Set the draw buffers to be the postion, color, and normals
//     void SetWritePNC();

//     // Set the draw buffers to be the light
//     void SetWriteLight();

//     void SetWriteNone();

//   private:
//     unsigned int position = 0;
//     unsigned int normal = 0;
//     unsigned int color_specular = 0;
//     unsigned int light = 0;
//     unsigned int rbo_depth = 0;

//     unsigned int *pnc_attachments = nullptr;
//     unsigned int *light_attachments = nullptr;

//     int width = 1920;
//     int height = 1080;
// };
// } // namespace Vultr
