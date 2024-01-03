/*****************************************************************************
*  Copyright © 2024 - 2024 dzming.                                           *
*                                                                            *
*  @file     image_renderer_server.h                                         *
*  @brief    gRPC impl: image renderer                                       *
*  @author   dzming                                                          *
*  @email    dzm_work@163.com                                                *
*                                                                            *
*----------------------------------------------------------------------------*
*  Remark  :  proto file: resources/protos/image_renderer.proto              *
*****************************************************************************/

#ifndef _IMAGE_RENDERER_SERVER_H_
#define _IMAGE_RENDERER_SERVER_H_

#include <string>
#include "protos/image_renderer/image_renderer.grpc.pb.h"
#include "protos/image_renderer/image_renderer.pb.h"
#include "grpc/servers/grpc_server.h"
#include "grpc/servers/grpc_server_builder.h"

class ImageRendererServer: public imageRenderer::Communicate::Service {
public:
    ImageRendererServer();
    virtual ~ImageRendererServer();

    virtual grpc::Status getImageByImageId(grpc::ServerContext*, const imageRenderer::GetImageByImageIdRequest*, imageRenderer::GetImageByImageIdResponse*) override;
private:
};

#endif /* _IMAGE_RENDERER_SERVER_H_ */
