#include <softcam/softcam.h>
#include <gtest/gtest.h>


namespace RawAPITest {

TEST(scCreateCamera, Basic) {
    {
        void* cam = scCreateCamera(320, 240, 60);
        EXPECT_NE(cam, nullptr);
        EXPECT_NO_THROW({ scDeleteCamera(cam); });
    }{
        void* cam = scCreateCamera(1920, 1080, 30);
        EXPECT_NE(cam, nullptr);
        EXPECT_NO_THROW({ scDeleteCamera(cam); });
    }
}

TEST(scCreateCamera, MultipleInstancingFails) {
    void* cam1 = scCreateCamera(320, 240, 60);
    void* cam2 = scCreateCamera(320, 240, 60);

    EXPECT_NE(cam1, nullptr);
    EXPECT_EQ(cam2, nullptr);
    scDeleteCamera(cam2);
    scDeleteCamera(cam1);
}

TEST(scCreateCamera, FramerateIsOptional) {
    void* cam = scCreateCamera(320, 240);

    EXPECT_NE(cam, nullptr);
    scDeleteCamera(cam);
}

TEST(scCreateCamera, ZeroMeansUnlimitedVariableFramerate) {
    void* cam = scCreateCamera(320, 240, 0.0f);

    EXPECT_NE(cam, nullptr);
    scDeleteCamera(cam);
}

TEST(scCreateCamera, InvalidArgs) {
    void* cam;
    cam = scCreateCamera(0, 240, 60);
    EXPECT_EQ(cam, nullptr);
    scDeleteCamera(cam);

    cam = scCreateCamera(320, 0, 60);
    EXPECT_EQ(cam, nullptr);
    scDeleteCamera(cam);

    cam = scCreateCamera(0, 0, 60);
    EXPECT_EQ(cam, nullptr);
    scDeleteCamera(cam);

    cam = scCreateCamera(-320, 240, 60);
    EXPECT_EQ(cam, nullptr);
    scDeleteCamera(cam);

    cam = scCreateCamera(320, -240, 60);
    EXPECT_EQ(cam, nullptr);
    scDeleteCamera(cam);

    cam = scCreateCamera(320, 240, -60);
    EXPECT_EQ(cam, nullptr);
    scDeleteCamera(cam);
}

TEST(scCreateCamera, TooLarge) {
    void* cam;
    cam = scCreateCamera(32000, 240, 60);
    EXPECT_EQ(cam, nullptr);
    scDeleteCamera(cam);

    cam = scCreateCamera(320, 24000, 60);
    EXPECT_EQ(cam, nullptr);
    scDeleteCamera(cam);
}

TEST(scDeleteCamera, IgnoresNullPointer) {
    scDeleteCamera(nullptr);
}

TEST(scDeleteCamera, IgnoresDoubleFree) {
    void* cam = scCreateCamera(320, 240, 60);
    scDeleteCamera(cam);
    scDeleteCamera(cam);
}

TEST(scDeleteCamera, IgnoresInvalidPointer) {
    int x = 0;
    scDeleteCamera(&x);
}

} //namespace RawAPITest
