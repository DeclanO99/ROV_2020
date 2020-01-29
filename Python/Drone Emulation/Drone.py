import pyglet

class Drone(pyglet.sprite.Sprite):
    def __init__(self,drone_img,T_still,T_For, *args,**kwargs):
        super().__init__(img = drone_img,*args,**kwargs)
        self.t_still = T_still
        self.t_for = T_For
        self.pos = [self.x,self.y]
        self.vel_vector = [0,0]
        self.motor_thrust = 0
        self.speed = 255
        self.thrusters_batch = pyglet.graphics.Batch()
        self.thrust_A = Thruster(self.t_still, self.t_for,45, x = self.x-self.width//2, y = self.y +self.height//2,batch = self.thrusters_batch)
        self.thrust_B = Thruster(self.t_still, self.t_for,135, x = self.x+self.width//2, y = self.y +self.height//2,batch = self.thrusters_batch)
        self.thrust_C = Thruster(self.t_still, self.t_for,315, x = self.x-self.width//2, y = self.y -self.height//2,batch = self.thrusters_batch)
        self.thrust_D = Thruster(self.t_still, self.t_for,225, x = self.x+self.width//2, y = self.y -self.height//2,batch = self.thrusters_batch)
        
        self.thrusters = [self.thrust_A,self.thrust_B,self.thrust_C,self.thrust_D]
        self.Joystick_input = pyglet.text.Label(f"Joystick Input: {self.vel_vector}",x =0,y=465,batch = self.batch)
        self.motor_A = pyglet.text.Label(f"Motor A Output: {self.motor_thrust} state :{self.thrust_A.state}",x =0,y=445,batch = self.batch)
        self.motor_B = pyglet.text.Label(f"Motor B Output: {self.motor_thrust} state :{self.thrust_A.state}",x =0,y=425,batch = self.batch)
        self.motor_C = pyglet.text.Label(f"Motor C Output: {self.motor_thrust} state :{self.thrust_A.state}",x =0,y=405,batch = self.batch)
        self.motor_D = pyglet.text.Label(f"Motor D Output: {self.motor_thrust} state :{self.thrust_A.state}",x =0,y=385,batch = self.batch)

        
    def update(self,dt):
        
        motor_thrust = 0
        horz = None
        vert = None
        
        direction = [0,0,0,0]
        if self.vel_vector[0] >=50:#right
            horz = [-1,-1,1,1]
            motor_thrust = (abs(self.vel_vector[0])/255)*300
        elif self.vel_vector[0] <=-50:#left
            horz = [1,1,-1,-1]
            motor_thrust = (abs(self.vel_vector[0])/255)*300
        if self.vel_vector[1] >=50:#up
            vert = [-1,1,-1,1]
            motor_thrust = (abs(self.vel_vector[1])/255)*300
        elif self.vel_vector[1] <=-50:#down
            vert = [1,-1,1,-1]
            motor_thrust = (abs(self.vel_vector[1])/255)*300
        if horz != None and vert!= None:
            direction = []
            
            for i in range(len(vert)):
                if vert[i] == horz[i]*-1:
                    direction.append(0) 
                else:
                    direction.append((vert[i]))
        elif horz != None and vert == None:
            direction = horz 
        elif vert != None and horz == None:
            direction = vert
        for i in range(len(self.thrusters)):
            self.thrusters[i].update(motor_thrust,direction[i],dt)
        self.Joystick_input.text = f"Joystick Input: {self.vel_vector}"
        self.motor_A.text = f"Motor A Output: {motor_thrust * self.thrust_A.state} State :{self.thrust_A.state}"
        self.motor_B.text = f"Motor B Output: {motor_thrust * self.thrust_B.state} State :{self.thrust_B.state}"
        self.motor_C.text = f"Motor C Output: {motor_thrust * self.thrust_C.state} State :{self.thrust_C.state}"
        self.motor_D.text = f"Motor D Output: {motor_thrust * self.thrust_D.state} State :{self.thrust_D.state}"

        #print(direction)
        
       # print(motor_thrust,direction[0],)
        """
        temp = [0,0]
        for i in range(len(self.thrusters)):
            self.thrusters[i].vel_vector = [motor_thrust * direction[i],motor_thrust * direction[i]]
            temp[0] = self.thrusters[i].vel_vector[0]
            temp[1] = self.thrusters[i].vel_vector[1]
        print(temp)
        """
        
        self.thrusters_batch.draw()
        
        
    def toggle_animation(self,state):
        """swap between still and walking animations"""
        if self.walking == False and state ==True:
            self.image = self.walk
            self.walking = True
        elif state == False and self.walking==True:
            self.image = self.still
            self.walking = False

    def toggle_direction(self):
        """flip the sprite image to match direction of travel"""
        if self.vel_vector[0]>0 and self.direction =="Left":
            self.walk = self.walk.get_transform(flip_x =True)
            self.still =self.still.get_transform(flip_x =True)
            self.direction = "Right"
            self.image = self.walk
        elif self.vel_vector[0]<0 and self.direction =="Right":
            self.walk = self.walk.get_transform(flip_x = True)
            self.still = self.still.get_transform(flip_x = True)
            self.direction = "Left"
            self.image = self.walk

    def check_bounds(self):
        min_x = -self.image.width / 2
        min_y = -self.image.height / 2
        max_x =  self.world[0]+ self.image.width / 2
        max_y =  self.world[1]+ self.image.height / 2
        if self.x < min_x:
            self.x = max_x
        elif self.x > max_x:
            self.x = min_x
        if self.y < min_y:
            self.y = max_y
        elif self.y > max_y:
            self.y = min_y
        self.pos_vector = Vector(self.x,self.y)

class Thruster(pyglet.sprite.Sprite):
    def __init__(self,still,forward,rotation, *args,**kwargs):
        super().__init__(img = still,*args,**kwargs)
        self.still = still
        self.forward = forward
        self.pos = [self.x,self.y]
        self.vel_vector=[0,0]
        self.state = 0
        
        self.rotation = rotation
        self.f_rotation = rotation
        self.b_roation = rotation +180

    def update(self,motor_thrust,sign,dt):
        if motor_thrust *sign>0:
            self.image = self.forward
            self.rotation = self.b_roation
        elif motor_thrust * sign < 0:
            self.image = self.forward
            self.rotation = self.f_rotation
        else:
            self.image = self.still
        self.state = sign
        
        """self.vel_vector[0]  = motor_thrust * dt * sign
        self.vel_vector[1]  = motor_thrust * dt * sign 
        self.x += self.vel_vector[0]
        self.y += self.vel_vector[1]
        self.pos = [self.x,self.y]"""
    def toggle_animation(self,state):
        """swap between still and walking animations"""
        if self.walking == False and state ==True:
            self.image = self.walk
            self.walking = True
        elif state == False and self.walking==True:
            self.image = self.still
            self.walking = False
