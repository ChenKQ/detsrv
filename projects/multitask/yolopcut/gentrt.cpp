#include "yolov5.hpp"
#include <csignal>

int main(int argc, char** argv) {
    cudaSetDevice(DEVICE);
    // CUcontext ctx;
    // CUdevice device;
    // cuInit(0);
    // cuDeviceGet(&device, 0);
    // cuCtxCreate(&ctx, 0, device);

    std::string wts_name = "yolopcut.wts";
    std::string engine_name = "yolopcut.engine";

    // deserialize the .engine and run inference
    std::ifstream file(engine_name, std::ios::binary);
    if (!file.good()) {
        // std::cerr << "read " << engine_name << " error!" << std::endl;
        std::cout << "Building engine..." << std::endl;
        IHostMemory* modelStream{ nullptr };
        APIToModel(BATCH_SIZE, &modelStream, wts_name);
        assert(modelStream != nullptr);
        std::ofstream p(engine_name, std::ios::binary);
        if (!p) {
            std::cerr << "could not open plan output file" << std::endl;
            return -1;
        }
        p.write(reinterpret_cast<const char*>(modelStream->data()), modelStream->size());
        modelStream->destroy();
        std::cout << "Engine has been built and saved to file." << std::endl;
    }
    return 0;
}
