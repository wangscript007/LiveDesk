#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "OnDemandRTSPServer.h"
#include "rtspserver.h"

static void* rtsp_server_thread_proc(void *argv)
{
    RTSPSERVER *server = (RTSPSERVER*)argv;
    rtsp_servermain(server, &server->bexit);
    return NULL;
}

void* rtspserver_init(void *adev, PFN_IOCTL aioctl, void *vdev, PFN_IOCTL vioctl, int aenc_type, int venc_type, uint8_t *aac_config, int frate)
{
    RTSPSERVER *server = (RTSPSERVER*)calloc(1, sizeof(RTSPSERVER));
    server->audio_enctype = aenc_type;
    server->video_enctype = venc_type;
    server->adev  = adev;
    server->aioctl= aioctl;
    server->vdev  = vdev;
    server->vioctl= vioctl;
    server->frate = frate;

    if (aac_config) {
        server->aac_config[0] = aac_config[0];
        server->aac_config[1] = aac_config[1];
    }

    // create server thread
    pthread_create(&server->pthread, NULL, rtsp_server_thread_proc, server);
    return server;
}

void rtspserver_exit(void *ctx)
{
    RTSPSERVER *server = (RTSPSERVER*)ctx;
    if (!ctx) return;

    server->aioctl(server->adev, AENC_CMD_STOP, NULL, 0, NULL);
    server->vioctl(server->vdev, VENC_CMD_STOP, NULL, 0, NULL);
    server->bexit = 1;
    if (server->pthread) pthread_join(server->pthread, NULL);
    free(ctx);
}

int rtspserver_running_streams(void *ctx)
{
    RTSPSERVER *server = (RTSPSERVER*)ctx;
    return server ? server->running_streams : 0;
}