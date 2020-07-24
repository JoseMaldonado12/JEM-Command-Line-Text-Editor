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
and changed (from the guide) by me, thus I will refer to this as "my" text editor. I will be slowly updating this with more features in the coming days/weeks. 
This is meant more as a personal project of mine rather than an outright "competitor" to something like vim, nano, sublime, emacs, etc. However,
this text editor is very very capable. So much so that this programs source code, as of now, is being written in the JEM text editor. Wild! 
As of now, the basic features of these includes *saving and loading files, opening existing files,creating new files from scratch,
line count, current line tracker, and iterative search.*

## How to use:
Use this as you would basically any other text editor, running it as "jem" will bring up a blank text file, "jem txt" will open any text file
(as long as it exists). Thats basically it! It may not have as many bells as whistles as, say, VIM, but its simplicity gives way to the flexibility
of being able to tweak certain variables on your own (no programming experience required!). Speaking of...

## How to tweak:
I highly encourage many people to tweak the parameters and such as you would see fit. Many of the basic functions you may wanna change are 
defined as constants on the top of the program (numbers of spaces tab occupies, row markers, etc.). This may be subject to change as this is
curently an ongoing project, I may need to add or remove constants as necessary. Don't worry thought, if you liked a particular version better/
wanted a particular constant to be up, I will appropriately name my commits so as to have easy access to previous versions.

## Warnings:
Since this is a currently ongoing project, there may be certain builds with minor bugs. As a whole it works very well, but some
stuff may slip through the cracks as it does with projects (especially C projects) of this size, so shoot me message if
any bug arises. Don't worry however; it will NOT crash your computer.

## Extra Stuff:
(message has been copy-pasted from the shout-out comment at the top of my program)
This editor heavily borrows (EI. goes nearly word for word ) from this guide: https://viewsourcecode.org/snaptoken/kilo/index.html
As such I have no plans to claim complete ownership or monetize this in any way; anyone could do what I did. I simply tweaked many different
variables to what I deemed appropriate, renamed and restructured some code, and even added/left out a bit of code myself that I thought was more efficient
and/or necessary. Anyone reading this is encouraged to do the same! Either use my code as a baseline or follow the guide from scratch and perhaps learn a thing or 2
about C text editing. Or you can use my text editor as is for your own purposes. Perhaps tab sizes are a huge issue for you and having a tab size of 3 is just perfect,
or perhaps its just a decent mix of vim difficulty and nano simplicity. Whatever you decide to do with this, it'd be very cool if you shot me a message at my github
to see what you people are doing with this; I love hearing about other peoples projects,or perhaps vast improvements to my own. Happy coding!
