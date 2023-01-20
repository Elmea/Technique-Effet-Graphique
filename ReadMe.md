# **Graphical Effects**
A project by Masseteau Emma and Jammet Maël


## **Instancing** 

When we draw hundreds or thousands of instances of the same simple mesh (like a patch of grass or a leave for exemple) the step in the graphic pipeline that takes the most time is the communication between the CPU and the GPU.
Since each call of the function DrawElement require this communication, looping this function as many time as they are leaves is a very ineffective way to process things.

Instancing is a technic in which rather than calling DrawElement for each mesh we call only the funciton DrawElementsInstanced once.
This function draws the same mesh in the same position as many time as we want and it does so with only one communication between CPU and GPU.

It also provides a variable (gl_InstanceID) which increments for each draw of the mesh. Using this variable and an offset uniform that we create in the code we can modify the position for each instance of the mesh.

The result is that we can render hundreds of forms (or a good number of complex ones) while preserving good performances.




## **Shadow Mapping**

In the basic lighting of OpenGL there is no shadow. Light goes through objects, walls, people without being hindered.
To fix this problem we use Shadow Mapping.

The idea is to render the scene from the lights point of view and to cover in shadows every mesh that is not visible from this point of view.
