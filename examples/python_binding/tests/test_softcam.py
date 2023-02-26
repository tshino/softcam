import pytest
from x64.Release import softcam


def test_camera_normalcase():
    assert softcam.camera is not None

    cam = softcam.camera(320, 240, 60)
    assert cam is not None
    assert cam.delete() is None

    cam = softcam.camera(1920, 1080, 30)
    assert cam is not None
    assert cam.delete() is None


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


# TODO: add more tests
