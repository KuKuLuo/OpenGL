[KEYS]
W, A, S, D - Moving the camera
Left mouse button - Activating the attractor mode (Camera does not move anymore)
Right mouse button - Activating the attractor as the source of gravity
UP, DOWN - Adjusting the attractor's depth

[USED LIBRARIES]

This particle system uses the following libraries:
Glew 1.11.0
GLFW 3.0.4
GLM 0.9.5.4
xerces-c-3.1.1
boost 1.56.0

[PROBLEMS AND ERRORS]

If no GLFW window can be created nor an OpenGL context then make sure you have got the latest graphics driver which supports OpenGL 4.3. 
If you got some trouble with the xml document then just delete it. A new document will be created with default values during the next execution.

[COMMENT]

The provided source code is far from perfect but at this point everything is working so far and I hope it may help someone.
I started this project to get to know OpenGL, therefore it is my first one with this API and the OpenGL related code might not be the best one. :P
The XML parser is written by myself and uses the xerces library. 

When you got questions regarding this particle system then you may contact me.

[LICENSE]
You can use this code as you like for every purpose. It would be kind to mention my name "Stanislaw Eppinger" when you use the code. =)


Thank you for downloading this humble project! :) 
