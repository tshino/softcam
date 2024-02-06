import pytest
import numpy as np
from x64.Release import softcam


def test_camera_normalcase():
    assert softcam.camera is not None

    cam = softcam.camera(320, 240, 60)
    assert cam is not None
    cam = None

    cam = softcam.camera(1920, 1080, 30)
    assert cam is not None
    cam.delete()


def test_camera_multiple_instancing_fails():
    cam = softcam.camera(320, 240, 60)
    with pytest.raises(RuntimeError) as e:
        cam = softcam.camera(320, 240, 60)
    assert e.value.args == ('creating a virtual camera instance failed',)
    cam.delete()


def test_camera_framerate_is_optional():
    cam = softcam.camera(320, 240)
    cam.delete()


def test_camera_framerate_zero_is_allowed():
    cam = softcam.camera(320, 240, 0)
    cam.delete()


def test_camera_dimension_must_be_integer():
    with pytest.raises(TypeError):
        softcam.camera(320.0, 240)
    with pytest.raises(TypeError):
        softcam.camera(320, 240.0)


def test_camera_framerate_can_be_float():
    cam = softcam.camera(320, 240, 59.94)
    cam.delete()
    cam = softcam.camera(320, 240, 0.0)
    cam.delete()


def test_camera_invalid_args():
    with pytest.raises(RuntimeError):
        softcam.camera(0, 240, 60)
    with pytest.raises(RuntimeError):
        softcam.camera(320, 0, 60)
    with pytest.raises(RuntimeError):
        softcam.camera(0, 0, 60)
    with pytest.raises(RuntimeError):
        softcam.camera(-320, 240, 60)
    with pytest.raises(RuntimeError):
        softcam.camera(320, -240, 60)
    with pytest.raises(RuntimeError):
        softcam.camera(320, 240, -60)


def test_camera_too_big():
    with pytest.raises(RuntimeError):
        softcam.camera(32000, 240, 60)
    with pytest.raises(RuntimeError):
        softcam.camera(320, 24000, 60)


def test_camera_double_free():
    cam = softcam.camera(320, 240, 60)
    cam.delete()
    cam.delete() # no effect


def test_send_frame_normalcase():
    cam = softcam.camera(320, 240, 60)
    cam.send_frame(np.zeros((240, 320, 3), dtype=np.uint8))
    cam.delete()


def test_send_frame_invalid_dimension():
    cam = softcam.camera(320, 240, 60)
    with pytest.raises(ValueError):
        cam.send_frame(np.zeros((1, 240, 320, 3), dtype=np.uint8))
    with pytest.raises(ValueError):
        cam.send_frame(np.zeros((240, 320), dtype=np.uint8))
    with pytest.raises(ValueError):
        cam.send_frame(np.zeros((240, 320, 1), dtype=np.uint8))
    with pytest.raises(ValueError):
        cam.send_frame(np.zeros((320, 320, 3), dtype=np.uint8))
    with pytest.raises(ValueError):
        cam.send_frame(np.zeros((240, 240, 3), dtype=np.uint8))
    cam.delete()


def test_send_frame_use_after_free():
    cam = softcam.camera(320, 240, 60)
    cam.delete()
    with pytest.raises(RuntimeError) as e:
        cam.send_frame(np.zeros((240, 320, 3), dtype=np.uint8))
    assert e.value.args == ('the camera instance has been deleted',)


def test_wait_for_connection():
    cam = softcam.camera(320, 240, 60)
    assert cam.wait_for_connection(0.01) == False
    cam = None


def test_wait_for_connection_use_after_free():
    cam = softcam.camera(320, 240, 60)
    cam.delete()
    with pytest.raises(RuntimeError) as e:
        assert cam.wait_for_connection()
    assert e.value.args == ('the camera instance has been deleted',)


def test_is_connected():
    cam = softcam.camera(320, 240, 60)
    assert cam.is_connected() == False
    cam = None


def test_is_connected_use_after_free():
    cam = softcam.camera(320, 240, 60)
    cam.delete()
    with pytest.raises(RuntimeError) as e:
        assert cam.is_connected()
    assert e.value.args == ('the camera instance has been deleted',)
