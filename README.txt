PRGSEM - MATYAS GODULA

The whole prgsem program consists of two files
    1. comp_module - this file serves to compute the fractal image via pipe communications with the prgsem control app (further just prgsem).
    2. prgsem - the user interface, used to control the app, apart from the basic functionality also includes: animation (shows the how the fractal looks 
       when the number of iterations changes), image saving (saves the currently displayed image) and movement within the image.
    
For specifying values within the program use the input_parameters text file. If you input faulty data the program will tell you which
input is faulty.

When first starting the program a help message will be shown in the terminal, this message contains info about using the control app.

Keep in mind when using the movement, that the fractals are computed on the cpu locally and the timeout for key presses is 5ms so rapid
key pressing can cause jittering in the image. Also when the video option is selected it is not possible to move in the image, so
before trying to press r to reload the input_parameters file. One more thing, when reloading the image and computing the displayed image
can be in a weird position, this happens if you were moving in the image using the keyboard before changing the parameters, just press +
if you can and the image should recenter;

Controls
    s ....... sets up the computation, you will need to press before initializing any computation because of the ability to read parameters while running
    c ....... starts computing in comp_module and slowly updating the image (can be aborted by a)
    a ....... aborts the computation on the comp_module
    1 ....... calculates the fractal locally in the control app (the computation is os fast pressing a would not make a difference)
    e ....... deletes the whole image currently shown
    p ....... saves the image into a png called "output.png"
    v ....... plays an animation when selected in input_parameters, keep in mind that when animations are enabled movement within the image is not possible
    r ....... in case you change parameters while the program is running this keybind will read the file again and update the values
    h ....... shows these controls within the terminal (help)
    g ....... requests and prints the current comp_module version
    = ....... supposed to be + but made for US keyboard layouts, zooms in the image
    - ....... zooms out the image
    arrow left, arrow right, arrow up, arrow down ... movement within the image you will need to be in the gui window
    q ....... quits the program (there seems to be a problem with freeing SDL memory, this happens to everyone so some leaks are possible) frees memory

Before running the program you will need to create the pipes which the programs use to communicate between each other, using the create_pipes.sh script
the program is given in a cleaned non-compiled state, both prgsem and comp_module have their own separate makefiles.
The reference main app works with the custom comp_module, and vice versa. 
     