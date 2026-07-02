#include "hal/Esp32WebServer.h"

namespace hal {

void Esp32WebServer::begin(uint16_t port) {
    _server.begin(port);
}

void Esp32WebServer::stop() {
    _server.stop();
}

void Esp32WebServer::handle() {
    _server.handleClient();
}

void Esp32WebServer::on(const char* path, std::function<void()> handler) {
    _server.on(path, handler);
}

void Esp32WebServer::send(uint16_t code, const char* contentType, const char* body) {
    _server.send(code, contentType, body);
}

String Esp32WebServer::arg(const char* name) {
    return _server.arg(name);
}

void Esp32WebServer::onUpload(const char* path,
                               std::function<void()> completionHandler,
                               std::function<void(UploadStatus, const uint8_t*, size_t)> uploadHandler) {
    _server.on(path, HTTP_POST,
        completionHandler,
        [this, uploadHandler]() {
            HTTPUpload& upload = _server.upload();
            switch (upload.status) {
                case UPLOAD_FILE_START:
                    uploadHandler(UploadStatus::Start, nullptr, 0);
                    break;
                case UPLOAD_FILE_WRITE:
                    uploadHandler(UploadStatus::Write, upload.buf, upload.currentSize);
                    break;
                case UPLOAD_FILE_END:
                    uploadHandler(UploadStatus::End, nullptr, upload.totalSize);
                    break;
                case UPLOAD_FILE_ABORTED:
                    uploadHandler(UploadStatus::Abort, nullptr, 0);
                    break;
            }
        }
    );
}

} // namespace hal
