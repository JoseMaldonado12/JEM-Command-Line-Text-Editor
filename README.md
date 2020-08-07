````
_____ ________ __       __
   |     |        |  \     /  \
    \$$$$| $$$$$$$| $$\   /  $$
      | $| $$__   | $$$\ /  $$$
 __   | $| $$  \  | $$$$\  $$$$
|  \  | $| $$$$$  | $$\$$ $$ $$
| $$__| $| $$_____| $$ \$$$| $$
 \$$    $| $$     | $$  \$ | $$
  \$$$$$$ \$$$$$$$$\$$      \$$
````
# JEM-Command-Line-Text-Editor
## General overview:
JEM is a new command-line text editor that heavily follows [this](https://viewsourcecode.org/snaptoken/kilo/index.html) guide. All the code as been written
and changed (from the guide) by me, thus I will refer to this as "my" text editor. As of right now this project is officially done; no more updates
wil be given to this project unless I find some serious flaws and/or I suddenly decide to come back to it on a lazy whim.
 
This is meant more as a personal project of mine rather than an outright "competitor" to something like vim, nano, sublime, emacs, etc. However,
this text editor is very very capable. So much so that this programs source code, after a certain point in development, was written entirely in the JEM text editor. Wild! 
As of version 1.0, these will be all the features of the JEM text editor: *saving and loading files, opening existing files, creating new files from scratch,
line count, current line tracker, iterative search, and syntax text coloring.*

## How to use:
Use this as you would basically any other text editor, running it as "jem" will bring up a blank text file, "jem txt" will open any readable/writeable file
(as long as it exists). Thats basically it! It may not have as many bells as whistles as, say, VIM, but its simplicity gives way to the flexibility
of being able to tweak certain variables on your own. 

## How to tweak:
I highly encourage many people to tweak the parameters and such as you would see fit. Many of the basic functions you may wanna change are 
macros on the top of the program (numbers of spaces tab occupies, row markers, syntax coloring, etc.). Not just that, but I tried my best at
appropriately labeling and commenting all loops, functions, and variables to make it easier to tweak certain aspects of the editor. 

## Warnings:
As a whole, the text editor works very well, and there seemed to be no possible bugs I could discern. However, some
bugs may slip through the cracks as it does with projects (especially C projects) of this size, so shoot me message if
any bug arises. However, I am NOT responsible to what happens to text files if you decide to use this as a text editor.
While I would not say "use at your own risk", you may still want to carefully check the text editor yourself to see if its
safe enough for daily use.

## Extra Stuff:
(message has been copy-pasted from the shout-out comment at the top of my program)
This editor heavily borrows (EI. goes nearly word for word ) from this guide: https://viewsourcecode.org/snaptoken/kilo/index.html
As such I have no plans to claim complete ownership or monetize this in any way; anyone could do what I did. I simply tweaked many different
variables to what I deemed appropriate, renamed and restructured some code, and even added/left out a bit of code myself that I thought was more efficient
and/or necessary. Anyone reading this is encouraged to do the same! Either use my code as a baseline or follow the guide from scratch and perhaps learn a thing or 2
about C text editing. Or you can use my text editor as is for your own purposes. Perhaps tab sizes are a huge issue for you and having a tab size of 3 is just perfect,
or perhaps its just a decent mix of vim features and nano simplicity. Whatever you decide to do with this, it'd be great if you shot me a message at my github
to see what you people are doing with this; I love hearing about other peoples projects. Happy coding!
