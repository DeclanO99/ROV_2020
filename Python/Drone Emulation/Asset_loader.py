import pyglet
from pyglet.gl import *

class Asset_Loader():
    def __init__(self,Asset_location):
        pyglet.resource.path = [".",Asset_location]
        pyglet.resource.reindex()
    
    def load_image(self,file_name,size=None,center_image=False,center_animation = False):
        image = pyglet.resource.image(file_name)
        glEnable(GL_TEXTURE_2D)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
        if size != None:
            image.width = size[0]
            image.height = size[1]
        if center_image:
            self.center_image(image)
        if center_animation:
            self.center_animation(image)
        return image
    def center_animation(self,animation):
        """center animation frames anchor"""
        for i in range(len(self.animation.frames)):
            self.center_image(self.animation.frames[i].image)

    def center_image(self,image):
        """Sets an image's achor point to its center"""
        image.anchor_x = image.width // 2
        image.anchor_y = image.height // 2