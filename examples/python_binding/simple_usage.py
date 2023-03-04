import cv2
import numpy as np

from x64.Release import softcam


class DrawSomething:
    def __init__(self):
        self.t = 0
        self.img = np.zeros((240, 320, 3), dtype=np.uint8)

    def update(self, dt):
        self.t += dt
        x = int(160 + 120*np.sin(self.t*1.5 + 0.8*np.sin(self.t*0.5)))
        y = int(120 + 90*np.sin(self.t*1.9 + 0.8*np.sin(self.t*0.6)))
        r = int(128 + 96*np.sin(self.t*0.3))
        g = int(128 + 96*np.sin(self.t*0.4))
        b = int(128 + 96*np.sin(self.t*0.5 + 1.0))
        cv2.circle(self.img, (x,y), 10, (b,g,r), thickness=-1, lineType=cv2.LINE_AA)
        self.img = cv2.GaussianBlur(self.img, (11,11), 0)
        return self.img


def main():
    dt = 1/60
    cam = softcam.camera(320, 240, 60)
    draw_something = DrawSomething()

    # Here, you can wait for an application to connect to this camera.
    while not cam.wait_for_connection(timeout=1):
        pass

    while True:
        # draw something
        # Note that the color component order should be BGR, not RGB.
        img = draw_something.update(dt)
        # send the image
        cam.send_frame(img)


if __name__ == '__main__':
    main()
