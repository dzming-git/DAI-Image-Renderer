/*****************************************************************************
*  Copyright © 2023 - 2023 dzming.                                           *
*                                                                            *
*  @file     image_harmony_client.h                                          *
*  @brief                                                                    *
*  @author   dzming                                                          *
*  @email    dzm_work@163.com                                                *
*                                                                            *
*----------------------------------------------------------------------------*
*  Remark  :                                                                 *
*****************************************************************************/

#ifndef _IMAGE_HARMONY_CLIENT_H_
#define _IMAGE_HARMONY_CLIENT_H_

#include <string>
#include "grpc++/grpc++.h"
#include "protos/image_harmony/image_harmony.grpc.pb.h"
#include "protos/image_harmony/image_harmony.pb.h"

class ImageHarmonyClient {
public:
    ImageHarmonyClient();
    ~ImageHarmonyClient();

    bool setAddress(std::string ip, std::string port);

    bool setLoaderArgsHash(int64_t loaderArgsHash);
private:
    imageHarmony::Communicate::Stub* stub;
    int64_t connectId;
};



#endif /* _IMAGE_HARMONY_CLIENT_H_ */
