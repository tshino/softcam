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
    assert cam is not None
    cam = None


def test_camera_framerate_is_optional():
    cam = softcam.camera(320, 240)
    assert cam is not None
    cam = None


# TODO: add more tests
