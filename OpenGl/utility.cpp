#include "mainview.h"
#include <cstring>

std::vector<quint8> MainView::imageToBytes(QImage image) {
    // needed since (0,0) is bottom left in OpenGL
    image = image.mirrored();
    image = image.convertToFormat(QImage::Format_RGBA8888);
    std::vector<quint8> pixelData;
    pixelData.reserve(image.byteCount());
    std::memcpy(pixelData.data(), image.constBits(), image.byteCount());
    return pixelData;
}
