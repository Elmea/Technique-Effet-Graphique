# **Graphical Effects**
A project by Masseteau Nino and Jammet Maël


## **Instancing** 

When we draw hundreds or thousands of instances of the same simple mesh (like a patch of grass or a leave for exemple) the step in the graphic pipeline that takes the most time is the communication between the CPU and the GPU.
Since each call of the function DrawElement require this communication, looping this function as many time as they are leaves is a very ineffective way to process things.

Instancing is a technic in which rather than calling DrawElement for each mesh we call only the funciton DrawElementsInstanced once.
This function draws the same mesh in the same position as many time as we want and it does so with only one communication between CPU and GPU.

It also provides a variable (gl_InstanceID) which increments for each draw of the mesh. Using this variable and an offset uniform that we create in the code we can modify the position for each instance of the mesh.

<img src="./ReadMeSources/Instancing.png" style="width:400px;"> <br>

The result is that we can render hundreds of forms (or a good number of complex ones) while preserving good performances.


<img src="./ReadMeSources/Instancing.gif" width="600" height="400" />





## **Shadow Mapping**

In the basic lighting of OpenGL there is no shadow. Light goes through objects and walls without being hindered.
To fix this problem we use Shadow Mapping.

<img src="./ReadMeSources/ShadowMapping.png" style="width:400px;"> <br>

The idea is to render the scene from the lights point of view and to cover in shadows every mesh that is not visible by the light.
To do this we calculate a depth map, which store for every pixel how far it is from the light 

Here is the exemple depth map that we generated. The darker the pixel, the closer the object.

<img src="./ReadMeSources/DepthMap.png" style="width:400px;"> <br>

Then when we render the scene for real from the camera point of view we calculate a new depth map and compare it to the one before. If the depth stored in the former is the same as the later that means that nothing is in the way and that there is no shadow on that pixel. On the other hand if the depth differs the pixel must be darken for there is an object between the light and the pixel, creating a shadow.
