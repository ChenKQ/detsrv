/* GStreamer
 * Copyright (C) 2008 Wim Taymans <wim.taymans at gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <gst/gst.h>

#include <gst/rtsp-server/rtsp-server.h>

typedef struct
{
  gboolean white;
  GstClockTime timestamp;
} MyContext;

/* called when we need to give data to appsrc */
static void
need_data (GstElement * appsrc, guint unused, MyContext * ctx)
{
  g_print("one frame...\n");
  GstBuffer *buffer;
  guint size;
  GstFlowReturn ret;

  size = 385 * 288 * 2;

  buffer = gst_buffer_new_allocate (NULL, size, NULL);

  /* this makes the image black/white */
  gst_buffer_memset (buffer, 0, ctx->white ? 0xff : 0x0, size);

  ctx->white = !ctx->white;

  /* increment the timestamp every 1/2 second */
  GST_BUFFER_PTS (buffer) = ctx->timestamp;
  GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND, 2);
  ctx->timestamp += GST_BUFFER_DURATION (buffer);

  g_signal_emit_by_name (appsrc, "push-buffer", buffer, &ret);
  gst_buffer_unref (buffer);
}

/* called when a new media pipeline is constructed. We can query the
 * pipeline and configure our appsrc */
static void
media_configure (GstRTSPMediaFactory * factory, GstRTSPMedia * media,
    gpointer user_data)
{
  GstElement *element, *appsrc;
  MyContext *ctx;

  /* get the element used for providing the streams of the media */
  element = gst_rtsp_media_get_element (media);

  /* get our appsrc, we named it 'mysrc' with the name property */
  appsrc = gst_bin_get_by_name_recurse_up (GST_BIN (element), "mysrc");

  /* this instructs appsrc that we will be dealing with timed buffer */
  gst_util_set_object_arg (G_OBJECT (appsrc), "format", "time");
  /* configure the caps of the video */
  g_object_set (G_OBJECT (appsrc), "caps",
      gst_caps_new_simple ("video/x-raw",
          "format", G_TYPE_STRING, "RGB16",
          "width", G_TYPE_INT, 384,
          "height", G_TYPE_INT, 288,
          "framerate", GST_TYPE_FRACTION, 30, 1, NULL), NULL);

  ctx = g_new0 (MyContext, 1);
  ctx->white = FALSE;
  ctx->timestamp = 0;
  /* make sure ther datais freed when the media is gone */
  g_object_set_data_full (G_OBJECT (media), "my-extra-data", ctx,
      (GDestroyNotify) g_free);

  /* install the callback that will be called when a buffer is needed */
  g_signal_connect (appsrc, "need-data", (GCallback) need_data, ctx);
  gst_object_unref (appsrc);
  gst_object_unref (element);
  g_print("end configure...\n");
}

int
main (int argc, char *argv[])
{
  GMainLoop *loop;
  GstRTSPServer *server;
  GstRTSPMountPoints *mounts;
  GstRTSPMediaFactory *factory;

  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);

  /* create a server instance */
  server = gst_rtsp_server_new ();

  /* get the mount points for this server, every server has a default object
   * that be used to map uri mount points to media factories */
  mounts = gst_rtsp_server_get_mount_points (server);

  /* make a media factory for a test stream. The default media factory can use
   * gst-launch syntax to create pipelines.
   * any launch line works as long as it contains elements named pay%d. Each
   * element with pay%d names will be a stream */
  factory = gst_rtsp_media_factory_new ();
  gst_rtsp_media_factory_set_launch (factory,
      "( appsrc name=mysrc ! videoconvert ! omxh264enc ! rtph264pay name=pay0 pt=96 )");

  /* notify when our media is ready, This is called whenever someone asks for
   * the media and a new pipeline with our appsrc is created */
  g_signal_connect (factory, "media-configure", (GCallback) media_configure,
      NULL);

  /* attach the test factory to the /test url */
  gst_rtsp_mount_points_add_factory (mounts, "/test", factory);

  /* don't need the ref to the mounts anymore */
  g_object_unref (mounts);

  /* attach the server to the default maincontext */
  gst_rtsp_server_attach (server, NULL);

  /* start serving */
  g_print ("stream ready at rtsp://127.0.0.1:8554/test\n");
  g_main_loop_run (loop);

  return 0;
}


// #include "../config.h"
// #include "../plugincore.h"
// #include "../utils.h"
// #include "detsvr/IDetect.h"
// #include "detsvr/detsvr.h"
// #include "io.h"
// #include <opencv2/opencv.hpp>
// #include <string>
// #include <memory>


// int main(int argc, char* argv[])
// {
//     detsvr::Logger& logger = detsvr::Logger::CreateInstance();
//     logger.initialize("VideoAnalysis", "videoanalysis");

//     detsvr::Config::load("./config.txt");
//     detsvr::Config& cfg = detsvr::Config::GetInstance();

//     // std::shared_ptr<detsvr::IDetect> pDetector = 
//     //         detsvr::PluginCore::CreateDetector(cfg.pluginCfg.filename.c_str());

//     // cv::namedWindow("CSI Camera", cv::WINDOW_AUTOSIZE);
//     // std::cout << "Hit ESC to exit" << "\n" ;
    
//     std::shared_ptr<detsvr::IInput> pReader = 
//         detsvr::Factory<detsvr::IInput>::CreateInstance("rtsp");
//     std::string srcUri = "rtsp://172.20.10.9:8554/live/test1";
//     // std::string srcUri = "rtmp://172.20.10.9:1935/live/test1";
//     if(!pReader->open(&srcUri))
//     {
//         return -1;
//     }    

//     std::shared_ptr<detsvr::IOutput> pWriter = 
//         detsvr::Factory<detsvr::IOutput>::CreateInstance("rtspserver");
//     detsvr::RtspServer::Params params
//     {
//         outIndex: "/index/output",
//         outPort: "10554",
//         outWidth : 1280,
//         outHeight: 720,
//         // outWidth: 1920,
//         // outHeight: 1080,
//         outFPS : 30
//     };
//     // detsvr::RtspWriter::Params params 
//     // {
//     //     // uri: "rtmp://172.20.10.9:1935/live/test2",
//     //     uri: "rtsp://172.20.10.9:8554/live/test2",
//     //     fps: 30,
//     //     // displayWidth: 1920,
//     //     // displayHeight: 1080,
//     //     displayWidth: 1280,
//     //     displayHeight: 720,
//     //     isColor: true
//     // };
//     // detsvr::RtmpWriter::Params params 
//     // {
//     //     uri: "rtmp://172.20.10.9:1935/live/test2",
//     //     // uri: "rtsp://172.20.10.9:8554/live/test2",
//     //     fps: 30,
//     //     // displayWidth: 1920,
//     //     // displayHeight: 1080,
//     //     displayWidth: 1280,
//     //     displayHeight: 720,
//     //     isColor: true
//     // };
//     // detsvr::ScreenWriter::Params params {"Display"};
//     if(!pWriter->open((void*)&params))
//     {
//         return -1;
//     }

//     detsvr::PlayManager pm(pReader, 8);
//     if(!pm.start())
//     {
//         return -1;
//     }

//     detsvr::WriteManager wm(pWriter, 8);
//     if(!wm.start())
//     {
//         return -1;
//     }

//     cv::Mat img;
//     detsvr::DetectionResult result;

//     int count = 0;
//     while(true)
//     {
//         if(pm.getStatus()!=detsvr::PlayManager::RUN)
//         {
//             wm.stop();
//             std::cerr << "Error: the input is status is: " << 
//                 static_cast<detsvr::PlayManager::Status>(pm.getStatus());
//             return -1;
//         }

//         if(wm.getStatus()!=detsvr::WriteManager::RUN)
//         {
//             pm.stop();
//             std::cerr << "Error: the output status is: " << 
//                 static_cast<detsvr::WriteManager::Status>(wm.getStatus());
//             return -1;
//         }

//     	if (!pm.read(img)) 
//         {
//             // std::cout<<"Capture read error"<<std::endl;
//             std::this_thread::sleep_for(std::chrono::milliseconds(10));
//             continue;
// 	    }
//         ++count;
//         // if(count %1 == 0)
//         // {
//         //     result = pDetector->detect(img.rows, img.cols, img.type(), img.data, img.step);
//         //     std::cout   << "{img_width: " << result.img_width 
//         //         << ", img_height: " << result.img_height
//         //         << ", pre_time: " << result.pre_time
//         //         << ", inf_time: " << result.inf_time
//         //         << ", list: " << result.list.size() << "}\n";
//         // }   
        
//         wm.write(img, result);
//     }

//     pm.stop();
//     pReader->close();
//     return 0;
// }