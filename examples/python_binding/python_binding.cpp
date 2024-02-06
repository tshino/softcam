#include <stdexcept>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <softcam/softcam.h>


namespace py = pybind11;


class Camera
{
 public:
    Camera(int width, int height, float framerate = 60.0f)
    {
        m_camera = scCreateCamera(width, height, framerate);
        if (!m_camera)
        {
            throw std::runtime_error("creating a virtual camera instance failed");
        }
        m_width = width;
        m_height = height;
    }

    ~Camera()
    {
        Delete();
    }

    void Delete()
    {
        scDeleteCamera(m_camera);
        m_camera = nullptr;
    }

    void SendFrame(py::array_t<uint8_t, py::array::c_style | py::array::forcecast> image)
    {
        if (!m_camera)
        {
            throw std::runtime_error("the camera instance has been deleted");
        }
        py::buffer_info info = image.request();
        if (info.ndim != 3 || info.shape[2] != 3)
        {
            std::string actual_shape;
            for (int i = 0; i < info.ndim; i++) {
                actual_shape += std::to_string(info.shape[i]);
                if (info.ndim == 1 || i + 1 < info.ndim) {
                    actual_shape += ',';
                }
            }
            throw std::invalid_argument(
                "'image' argument must be an BGR image (3-dim array): "
                "expected shape=("
                    + std::to_string(m_height) + "," + std::to_string(m_width) + ",3), "
                "actual shape=(" + actual_shape + ")"
            );
        }
        if (info.shape[0] != m_height || info.shape[1] != m_width)
        {
            throw std::invalid_argument(
                "unexpected image size: "
                "expected=" + std::to_string(m_width) + "x" + std::to_string(m_height) + ", "
                "actual=" + std::to_string(info.shape[1]) + "x" + std::to_string(info.shape[0])
            );
        }

        py::gil_scoped_release release;
        scSendFrame(m_camera, image.data(0, 0));
    }

    bool WaitForConnection(float timeout = 0.0f)
    {
        if (!m_camera)
        {
            throw std::runtime_error("the camera instance has been deleted");
        }

        py::gil_scoped_release release;
        return scWaitForConnection(m_camera, timeout);
    }

    bool IsConnected()
    {
        if (!m_camera)
        {
            throw std::runtime_error("the camera instance has been deleted");
        }

        return scIsConnected(m_camera);
    }

 private:
    scCamera    m_camera{};
    int         m_width = 0;
    int         m_height = 0;
};


PYBIND11_MODULE(softcam, m) {
    m.doc() = "Softcam";

    py::class_<Camera>(m, "camera")
        .def(
            py::init<int, int, float>(),
            py::arg("width"),
            py::arg("height"),
            py::arg("framerate") = 60.0f
        )
        .def(
            "delete",
            &Camera::Delete
        )
        .def(
            "send_frame",
            &Camera::SendFrame,
            py::arg("image")
        )
        .def(
            "wait_for_connection",
            &Camera::WaitForConnection,
            py::arg("timeout") = 0.0f
        )
        .def(
            "is_connected",
            &Camera::IsConnected
        )
    ;
}
