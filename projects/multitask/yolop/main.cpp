#include "yolov5.hpp"
#include "utils.h"
// #include "zedcam.hpp"
#include <opencv2/opencv.hpp>
#include <csignal>

static volatile bool keep_running = true;


void keyboard_handler(int sig) {
    // handle keyboard interrupt
    if (sig == SIGINT)
        keep_running = false;
}

void saveresult(cv::Mat& cvt_img, cv::Mat& seg_res, cv::Mat& lane_res, std::vector<Yolo::Detection>& res)
{
    static const std::vector<cv::Vec3b> segColor{cv::Vec3b(0, 0, 0), cv::Vec3b(0, 255, 0), cv::Vec3b(255, 0, 0)};
    static const std::vector<cv::Vec3b> laneColor{cv::Vec3b(0, 0, 0), cv::Vec3b(0, 0, 255), cv::Vec3b(0, 0, 0)};
    cv::Mat cvt_img_cpu = cvt_img;
    // cvt_img.download(cvt_img_cpu);

    // handling seg and lane results
    for (int row = 0; row < cvt_img_cpu.rows; ++row) {
        uchar* pdata = cvt_img_cpu.data + row * cvt_img_cpu.step;
        for (int col = 0; col < cvt_img_cpu.cols; ++col) {
            int seg_idx = seg_res.at<int>(row, col);
            int lane_idx = lane_res.at<int>(row, col);
            //std::cout << "enter" << ix << std::endl;
            for (int i = 0; i < 3; ++i) {
                if (lane_idx) {
                    if (i != 2)
                        pdata[i] = pdata[i] / 2 + laneColor[lane_idx][i] / 2;
                }
                else if (seg_idx)
                    pdata[i] = pdata[i] / 2 + segColor[seg_idx][i] / 2;
            }
            pdata += 3;
        }
    }

    // handling det results
    std::cout << "number of detection result: " << res.size() << '\n';
    for (size_t j = 0; j < res.size(); ++j) {
        std::cout << "class: " << res[j].class_id 
                  << ", confidence: " << res[j].conf
                  << ", [centerx: " << res[j].bbox[0] 
                  << ", centery: " << res[j].bbox[1]
                  << ", width: " << res[j].bbox[2]
                  << ", height: " << res[j].bbox[3] << "]\n";
        // if(int(res[j].class_id) != 1)
        //     continue;
        cv::Rect r = get_rect(cvt_img_cpu, res[j].bbox);
        cv::rectangle(cvt_img_cpu, r, cv::Scalar(0x27, 0xC1, 0x36), 2);
        cv::putText(cvt_img_cpu, std::to_string((int)res[j].class_id), cv::Point(r.x, r.y - 1), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
    }
    
    cv::imwrite("./zed_result.jpg", cvt_img_cpu);

}

int main(int argc, char** argv) {
    signal(SIGINT, keyboard_handler);
    cudaSetDevice(DEVICE);

    std::string wts_name = "yolop.wts";
    std::string engine_name = "yolop.engine";

    // deserialize the .engine and run inference
    std::ifstream file(engine_name, std::ios::binary);
    if (!file.good()) {
        std::cerr << "read " << engine_name << " error!" << std::endl;
        return -1;
    }
    char *trtModelStream = nullptr;
    size_t size = 0;
    file.seekg(0, file.end);
    size = file.tellg();
    file.seekg(0, file.beg);
    trtModelStream = new char[size];
    assert(trtModelStream);
    file.read(trtModelStream, size);
    file.close();

    // prepare data ---------------------------
    static float data[BATCH_SIZE * 3 * INPUT_H * INPUT_W];
    static float det_out[BATCH_SIZE * OUTPUT_SIZE];
    static int seg_out[BATCH_SIZE * IMG_H * IMG_W];
    static int lane_out[BATCH_SIZE * IMG_H * IMG_W];
    IRuntime* runtime = createInferRuntime(gLogger);
    assert(runtime != nullptr);
    ICudaEngine* engine = runtime->deserializeCudaEngine(trtModelStream, size);
    assert(engine != nullptr);
    IExecutionContext* context = engine->createExecutionContext();
    assert(context != nullptr);
    delete[] trtModelStream;
    assert(engine->getNbBindings() == 4);
    void* buffers[4];
    // In order to bind the buffers, we need to know the names of the input and output tensors.
    // Note that indices are guaranteed to be less than IEngine::getNbBindings()
    const int inputIndex = engine->getBindingIndex(INPUT_BLOB_NAME);
    const int output_det_index = engine->getBindingIndex(OUTPUT_DET_NAME);
    const int output_seg_index = engine->getBindingIndex(OUTPUT_SEG_NAME);
    const int output_lane_index = engine->getBindingIndex(OUTPUT_LANE_NAME);
    assert(inputIndex == 0);
    assert(output_det_index == 1);
    assert(output_seg_index == 2);
    assert(output_lane_index == 3);
    // Create GPU buffers on device
    CUDA_CHECK(cudaMalloc(&buffers[inputIndex], BATCH_SIZE * 3 * INPUT_H * INPUT_W * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&buffers[output_det_index], BATCH_SIZE * OUTPUT_SIZE * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&buffers[output_seg_index], BATCH_SIZE * IMG_H * IMG_W * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&buffers[output_lane_index], BATCH_SIZE * IMG_H * IMG_W * sizeof(int)));
    // Create stream
    cudaStream_t stream;
    CUDA_CHECK(cudaStreamCreate(&stream));

    cv::Mat img = cv::imread("./01.jpg");
    int width = img.cols;
    int height = img.rows;

    // store seg results
    cv::Mat tmp_seg(IMG_H, IMG_W, CV_32S, seg_out);
    // sotore lane results
    cv::Mat tmp_lane(IMG_H, IMG_W, CV_32S, lane_out);
    cv::Mat seg_res(height, width, CV_32S);
    cv::Mat lane_res(height, width, CV_32S);

    // preprocess ~3ms
    cv::Mat preprocessedImage = preprocess_img(img, INPUT_W, INPUT_H);
    int i = 0;
    std::cout << "width: " << preprocessedImage.cols 
                << ", height: " <<  preprocessedImage.rows
                << ", step: " << preprocessedImage.step
                << "\n" ;
    for (int row = 0; row < INPUT_H; ++row) {
        uchar* uc_pixel = preprocessedImage.data + row * preprocessedImage.step;
        for (int col = 0; col < INPUT_W; ++col) {
            data[i + 0 * INPUT_H * INPUT_W] =  *(float*)(uc_pixel+0*sizeof(float));
            data[i + 1 * INPUT_H * INPUT_W] = *(float*)(uc_pixel+1*sizeof(float));
            data[i + 2 * INPUT_H * INPUT_W] = *(float*)(uc_pixel+2*sizeof(float));
            uc_pixel += 3*sizeof(float);
            ++i;
        }
    }

    // Run inference
    auto start = std::chrono::system_clock::now();
    // cuCtxPushCurrent(ctx);
    doInferenceCpu(*context, stream, buffers, data, det_out, seg_out, lane_out, BATCH_SIZE);
    // cuCtxPopCurrent(&ctx);
    auto end = std::chrono::system_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    // postprocess ~0ms
    std::vector<Yolo::Detection> batch_res;
    nms(batch_res, det_out, CONF_THRESH, NMS_THRESH);
    // nms(batch_res, det_out, 0.0, 0.0);
    cv::resize(tmp_seg, seg_res, seg_res.size(), 0, 0, cv::INTER_NEAREST);
    cv::resize(tmp_lane, lane_res, lane_res.size(), 0, 0, cv::INTER_NEAREST);

    // show results
    //std::cout << res.size() << std::endl;
    saveresult(img, seg_res, lane_res, batch_res);
    // destroy windows
#ifdef SHOW_IMG
    cv::destroyAllWindows();
#endif
    // Release stream and buffers
    cudaStreamDestroy(stream);
    CUDA_CHECK(cudaFree(buffers[inputIndex]));
    CUDA_CHECK(cudaFree(buffers[output_det_index]));
    CUDA_CHECK(cudaFree(buffers[output_seg_index]));
    CUDA_CHECK(cudaFree(buffers[output_lane_index]));
    // Destroy the engine
    context->destroy();
    engine->destroy();
    runtime->destroy();
    return 0;
}
