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
            throw std::invalid_argument("'image' argument must be an BGR image (3-dim array)");
        }
        if (info.shape[0] != m_height || info.shape[1] != m_width)
        {
            throw std::invalid_argument("unexpected image size");
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
            py::arg("float") = 60.0f
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
    ;
}
